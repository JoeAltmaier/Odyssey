#include "types.h"
#include "BootFlash.h"
#include "system.h"

#define WR_OFFSET(offset, val)	bf_addr[offset] = val
#define RD_OFFSET(offset)		(U8)(bf_addr[offset])

static U8	*bf_addr = (U8 *)0xBFC00000;
void
bf_erase(void)
{
	int		i;

	WR_OFFSET(0x5555, 0xAA);
	WR_OFFSET(0x2AAA, 0x55);
	WR_OFFSET(0x5555, 0x80);
	WR_OFFSET(0x5555, 0xAA);
	WR_OFFSET(0x2AAA, 0x55);
	WR_OFFSET(0x5555, 0x10);
	
	for (i = 0; i < 10; i++) {
		delay(1);
		wheel(0);
	}
}

void
bf_getid(U8 *mfg, U8 *dev)
{
	WR_OFFSET(0x5555, 0xAA);
	WR_OFFSET(0x2AAA, 0x55);
	WR_OFFSET(0x5555, 0x90);

	delay_ms(1);	
	*mfg = RD_OFFSET(0);
	*dev = RD_OFFSET(1);

	delay_ms(1);	
	WR_OFFSET(0x5555, 0xAA);
	WR_OFFSET(0x2AAA, 0x55);
	WR_OFFSET(0x5555, 0xF0);
	delay_ms(1);	
}

	
void
print_boot_id(void)
{
	U8		mfg_id;
	U8		dev_id;

	bf_getid(&mfg_id, &dev_id);
	if ((mfg_id != ATMEL_ID) || (dev_id != BOOTFLASH_ID)) {
		printf("--Invalid Device ID. Mfg Id = %d, Device Id = %d\n\r",
						mfg_id, dev_id);
		return;
	}
	printf("--Mfg Id = %x, Device Id = %x\n\r", mfg_id, dev_id);
}

	
STATUS
program_boot_flash(U8 *src, U32 len)
{
	U32		i;
	U8		mfg_id;
	U8		dev_id;
	U8		prev_d, cur_d;
	I32		to;
	
	DISABLE_INT;
	bf_getid(&mfg_id, &dev_id);
	if ((mfg_id != ATMEL_ID) || (dev_id != BOOTFLASH_ID)) {
		printf("Invalid Device ID. Mfg Id = %d, Device Id = %d\n\r",
						mfg_id, dev_id);
		return (1);
	}
	printf("\n\rErasing     ...   ");
	bf_erase();

	printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bProgramming ...   ");
	for (i = 0; i < len; i++) {
		WR_OFFSET(0x5555, 0xAA);
		WR_OFFSET(0x2AAA, 0x55);
		WR_OFFSET(0x5555, 0xA0);
		WR_OFFSET(i, src[i]);
		wheel(0);
		delay_us(50);
		prev_d = RD_OFFSET(i) & 0x40;
		to = 200;
		while (to > 0) {
			cur_d = RD_OFFSET(i) & 0x40;
			if (cur_d == prev_d)
				break;
			prev_d = cur_d;
			delay_us(50);
			to--;
		}
		if (to == 0) {
			printf("Toggle bit not set, off=%d, data=%d. Aborting...\n\r",
							i, src[i] & 0xFF);
			return (1);
		}
	}
	delay_ms(5);
	printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bVerifying   ...   ");
	for (i = 0; i < len; i++) {
		if (src[i] != RD_OFFSET(i)) {
			printf(" Failed at offset %d.\n\r", i);
			return (0);
		}
		wheel(0);
	}
	printf("\b\bDone.");

	ENABLE_INT;
	return (0);
}
