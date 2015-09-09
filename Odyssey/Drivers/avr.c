/*************************************************************************
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * File: avr.c 
 * 
 * Description:
 * This file contains SPI driver for AVR 
 * 
 * Update Log:
 * 7/29/99 Raghava Kondapalli: Created 
 ************************************************************************/

#include "types.h"
#include "avr.h"

static U8	eeprombuf[512];
static U8	databuf[8092];
static U32	word_size = 2;
static U32	buflen = 0;
static U32	reccount = 0;
static U32	filelen = 0;
static U8 	*filebuf = 0;
static U32	reg_val = 0;
static U32	max_addr = 0;
static I8	*slot_to_str[32] = {
		"A1", "A2", "A3", "A4",
		"B1", "B2", "B3", "B4",
		"C1", "C2", "C3", "C4",
		"D1", "D2", "D3", "D4",
		"DDH_3", "DDH_2", "DDH_1", "DDH_0",
		"EVC_1", "EVC_0", "HBC"
};
static I8	*slot_ptr;

void
avr_set_clk(U32 on)
{
	if (on) 
		reg_val |= 0x80; 
	else
		reg_val &= 0x7F;
	*(U32 *)0xBF020004 = reg_val;
}

void
avr_set_mosi(U32 on)
{
	if (on) 
		reg_val |= 0x40; 
	else
		reg_val &= 0xBF;
	*(U32 *)0xBF020004 = reg_val;
}

U32
avr_get_miso(void)
{
	return (((*(U32 *)0xBF000000) & 0x10) ? 1 : 0);	
}

/*
 * Send write byte command
 */
U32
avr_send_byte(int byte)
{
	int		i;

	avr_set_clk(0);
	for (i = 7; i >= 0; i--) {
		avr_set_clk(0);
		avr_set_mosi(byte & (1 << i));
		delay_us(1);
		avr_set_clk(1);
		delay_us(1);
		avr_set_clk(0);
		delay_us(1);
	}
	avr_set_mosi(1);
	return (0);
}

/*
 * Send read byte command
 */
U32
avr_read_byte(void)
{
	int		i;
	U32		val = 0;

	avr_set_clk(0);
	for (i = 7; i >= 0; i--) {
		avr_set_clk(0);
		delay_us(1);
		avr_set_clk(1);
		delay_us(1);
		val |= (avr_get_miso() << i);
		avr_set_clk(0);
		delay_us(1);
	}
	avr_set_mosi(1);
	return (val);
}

/*
 * Read a byte from AVR from a specified address
 */
U32
avr_read_flash(U32 addr, U32 msb)
{
	if (msb)
		avr_send_byte(0x28);
	else
		avr_send_byte(0x20);
	avr_send_byte(addr >> 8);
	avr_send_byte(addr);
	
	return (avr_read_byte());
}

/*
 * Write a byte to AVR flash at specified address 
 */
U32
avr_write_flash(U32 addr, U32 data, U32 msb)
{
	U32		i;
	U32		val;

	if (msb)
		avr_send_byte(0x48);
	else
		avr_send_byte(0x40);
	avr_send_byte(addr >> 8);
	avr_send_byte(addr);
	avr_send_byte(data);

	/*
	 * Wait for next write
	 */
	delay_ms(4);
	if ((data != 0x7F) && (data != 0xFF)) {
		for (i = 0; i < 10; i++) {
			val = avr_read_flash(addr, msb);
			if (val == data) {
				break;	
			} else {
				delay_ms(4);
			}
		}
	} else {
		delay_ms(4);
	}
	return (0);
}

/*
 * Read a byte from  AVR EEPROM at specified address 
 */
U32
avr_read_eeprom(U32 addr)
{
	avr_send_byte(0xA0);
	avr_send_byte((addr & 0xFFFF) >> 8);
	avr_send_byte(addr);
	return (avr_read_byte());
}


/*
 * Write a byte to AVR EEPROM at specified address 
 */
U32
avr_write_eeprom(U32 addr, U32 data)
{
	U32		i;
	U32		val;

	avr_send_byte(0xC0);
	avr_send_byte((addr & 0xFFFF) >> 8);
	avr_send_byte(addr);
	avr_send_byte(data);

	/*
	 * Wait for next write
	 */
	delay_ms(4);
	if (data != 0x7F) {
		for (i = 0; i < 10; i++) {
			val = avr_read_eeprom(addr);
			if (val == data) {
				break;	
			} else {
				delay_ms(4);
			}
		}
	}
	return (0);
}

/*
 * Read Atmel AVR AT90S8515 device code at a specified location.
 */
U32
avr_get_devcode(U32 addr)
{
	avr_send_byte(0x30);
	avr_send_byte(0x0);
	avr_send_byte(addr & 0x03);
	
	return (avr_read_byte());
}

/*
 * Release Reset line to AVR
 */
void
avr_rel_reset(U32 slot)
{
	slot = 0;
	*(U32 *)0xBF030004 = 0x00000000;	
	*(U8 *)0xBC0E8000 = 0; 
}

/*
 * Hold Reset line to AVR
 */
void
avr_reset(U32 slot)
{
	avr_set_clk(0);
	avr_set_mosi(1);
	delay_ms(20);
	*(U32 *)0xBF030004 = 0x00000000;	
	delay_ms(1);
	*(U8 *)0xBC0E8000 = 0; 
	
	delay_ms(100);
	*(U32 *)0xBF030004 = slot;	
	*(U8 *)0xBC0E8000 = 0; 
	delay_ms(5);	
}	

/*
 * Read Atmel AVR AT90S8515 device ID 
 */
U32
avr_probe_slot(U32 slot)
{
	U32		code0, code1, code2;

	avr_send_byte(0xAC);
	avr_send_byte(0x53);
	avr_read_byte();
	avr_send_byte(0);
	delay_ms(50);

	code0 = avr_get_devcode(0);
	code1 = avr_get_devcode(1);
	code2 = avr_get_devcode(2);
	
	if ((code0 != 0x1E) || (code1 != 0x93) || (code2 != 0x01)) {
			
		/*
		 * Try again
		 */
		avr_rel_reset(slot);
		delay_ms(100);
		avr_reset(slot);
		delay_ms(100);
		avr_send_byte(0xAC);
		avr_send_byte(0x53);
		avr_read_byte();
		avr_send_byte(0);
		delay_ms(50);

		code0 = avr_get_devcode(0);
		code1 = avr_get_devcode(1);
		code2 = avr_get_devcode(2);
		if ((code0 != 0x1E) || (code1 != 0x93) || (code2 != 0x01)) {

   			printf("\033[21;1H\033[K"); 
			printf("Could not read AVR ID at %s. Expected 0x1E9301. Got 0x%02x%02x%02x\n\r",
							slot_ptr, code0, code1, code2);
			return (1);
		}
	}
	return (0);
}

/*
 * Read a line from Intel HEX File
 */
U8 *
getnextline(void)
{
	buflen++;
	while ((filebuf[buflen] != ':') && (buflen < filelen)) {
			buflen++;
	}
	if (buflen == filelen)
		return (0);
	reccount++;
	return (&filebuf[buflen]);
}

/*
 * Conver n bytes in ASCII to hex
 */
int
atoi_n(U8 *s, I32 n)
{
	int i = 0, ret = 0;

	while((s[i] != '\0') && n) {
		ret <<= 4;
		if((s[i] <= 'F') && (s[i] >= 'A'))
			ret |= s[i] - 'A' + 10;
		else if((s[i] <= 'f') && (s[i] >= 'a'))
			ret |= s[i] - 'a' + 10;
		else if((s[i] <= '9') && (s[i] >= '0'))
			ret |= s[i] - '0';
		i++;
		n--;
	}
	return(ret);
}

/*
 * Go Thru' Intel hex file and fill databuf with values
 */

U32
avr_parse_file(U8 *databufp, U32 len)
{
	U8		*bufp;
	U32		cur = 0;
	U32		count;
	int		i;
	int		j;
	U8		cksum = 0;
	U8		rectype = 0;
	U16		addr = 0;
	U8		data;
	U8		*p;

	for (i = 0; i < 8092; i++)
		databuf[i] = 0xFF;
	filelen = len;
	filebuf = databufp;
	bufp = databufp;
	buflen = 0;
	max_addr = 0;
	while (bufp != 0) {
		if (bufp[0] == ':') {
			count = atoi_n(&bufp[1], 2);
			cksum = (U8)count;
			addr = (atoi_n(&bufp[3], 4));
			cksum += (U8)(addr >> 8);
			cksum += (U8)addr;
			rectype = atoi_n(&bufp[7], 2);
			cksum += rectype;
			if (rectype == 0) {
				if ((filebuf + addr + count) > (filebuf + len)) {
					printf("Buffer over flow %d\n", reccount);
					break;
				}
				for (i = 9, j = 0, p = databuf + addr; j < count; j++) {
					data = atoi_n(&bufp[i], 2);
					*p++ = data;
					i += 2;
					cksum += data;
					addr++;
				}
				data = atoi_n(&bufp[i], 2);
				if (data != (U8)(~cksum + 1))
					printf("Invalid cksum %x %x %d %d\n", cksum, data, i, count);
				else 
				if (addr > max_addr)
					max_addr = addr;
			} else {
				if (rectype == 2) {
					printf("Got Address Record\n");
				} else {
					if (rectype == 1) {
						break;
					}
				}
			}
		}
		bufp = getnextline();
	}
#if 0
	printf("Max addr= %08x\n", max_addr);	
	for (i = 0;  i < max_addr;) {
		printf("%04x: ", i);
		for (j = 0; j < 8; j++) {
			printf("%02x ", databuf[i++] & 0xFF);
		}
		printf("- ", i);
		for (j = 0; j < 8; j++) {
			printf("%02x ", databuf[i++] & 0xFF);
		}

		printf("\n");
	}
	printf("\n\r\n\r\n\r\n\r\n\r\n\r");
#endif
	return (0);
}

void
save_eeprom(void)
{
	U32		addr;

	for (addr = 0; addr < 512; addr++) {
		eeprombuf[addr] = avr_read_eeprom(addr);
		wheel(0);
	}
}

void
restore_eeprom(void)
{
	U32		addr;

	for (addr = 0; addr < 512; addr++) {
		avr_write_eeprom(addr, eeprombuf[addr]);
		wheel(0);
	}
}

/*
 * Program AVR Flash/EEPROM
 */
void
avr_write(U32 flash)
{
	U16		addr = 0;
	U8		data0, data1;
	U16		*bufp = (U16 *)databuf;

	
	if (flash) {
		avr_send_byte(0xAC);
		avr_send_byte(0x80);
		avr_send_byte(0);
		avr_send_byte(0);
		delay_ms(50);
		for (addr = 0; addr <= (max_addr / 2); addr++) {
			data0 = bufp[addr] & 0xFF;
			data1 = (bufp[addr] >> 8) & 0xFF;
			if (data1 != 0xFF)
				avr_write_flash(addr, data1, 0);
			if (data0 != 0xFF)
				avr_write_flash(addr, data0, 1);
			wheel(0);
		}
		
	} else {
		for (addr = 0; addr < max_addr; addr++) {
			data0 = databuf[addr] & 0xFF;
			avr_write_eeprom(addr, data0);
		}
	}
}

/*
 * Verify an earlier Write
 */
U32
avr_verify(U32 flash)
{
	U8		data0, data1;
	U16		addr = 0;
	U16		*bufp = (U16 *)databuf;

	if (flash) {
		for (addr = 0; addr <= (max_addr / 2); addr++) {
			data1 = avr_read_flash(addr, 1);
			data0 = avr_read_flash(addr, 0);
			if (bufp[addr] != ((data0 << 8) | data1)) {
				goto _error;
			}
			wheel(0);
		}
		return (0);
	} else {
		for (addr = 0; addr <= max_addr; addr++) {
			data0 = avr_read_eeprom(addr);
			if (data0 != databuf[addr]) 
				goto _error;
			wheel(0);
		}
		return (0);
	}
_error:
   	printf("\033[20;1H\033[K"); 
	printf("Read failed\(%s\) at addr %x.Expected %04x got %02x%02x\n\r",
				slot_ptr, (addr * 2), bufp[addr], data0, data1);
   	printf("\033[18;34H\033[K"); 
	return (1);
}


/*
 * Print out the list of AVR's. Get user input to select AVR's. If programming
 * EEPROM then only one AVR should be selected
 */
void
select_avr(U32 slot[], U32 *n, U32 flash)
{
	U32		slot_tmp[32];
	U32		sel;
	U32		i, j, k;
	I8		c;

	sel = 0;
	printf("\n\r\t\t");
	for (i = 0; i < (*n); i++) {
		slot_tmp[i] = slot[i];
		if ((i & 3) == 3)
			printf("\n\r\t\t");
		j = 0;
		k = slot[i];
		while (!(k & 1)) {
			j++;
			k = k >> 1;
		}
		printf("%c) %s   ", i + 'a', slot_to_str[j]);
	}
	if (flash) {
		printf("\n\r\t\tA) All  S) Start Programming\n\r",
						i + 'a');
		printf("\n\r\tSelect AVR's. Press 'S' when done -->> ");
	} else {
		printf("\n\r\tSelect one AVR -->> ");
	}
	while (1) {
		if (kbhit()) {
			c = ttyA_in();
			ttyA_out(c);
			c = c - 'a';
			if ((c >= 0) && (c < (*n))) {
				slot_tmp[sel] = slot[c];
				if (!flash) {
					slot[0] = slot[c];
					*n = 1;
					return;
				}
				sel++;
			} else {
				c = c + 'a'; 
				if (c == 'A')
					break;
				else if (c == 'S') {
					for (i = 0; i < sel; i++)
						slot[i] = slot_tmp[i];
					*n = sel;
					break;
				}
			}
		}
	}
}

/*
 * Program AVR's on IOP's/DDH's/HBC's/EVC's
 * Following are slot number as read from AVR Program Register
 * 
 * Bit0 -Slot A1, Bit1 -Slot A2, Bit2 -Slot A3, Bit3 -A4
 * Bit4 -Slot A1, Bit5 -Slot A2, Bit6 -Slot A3, Bit7 -A4
 * Bit8 -Slot A1, Bit9 -Slot A2, Bit10-Slot A3, Bit11-A4
 * Bit12-Slot A1, Bit13-Slot A2, Bit14-Slot A3, Bit15-A4
 * Bit16-DDH D,   Bit17-DDH C  , Bit18-DDH B  , Bit19-DDH_A
 */
U32
avr_program(U32 avr, U8 *databufp, U32 len, U32 flash)
{
	U32		slot_info = *(U32 *)0xBF010000;
	U32		slot[32];
	U32		i, j, k;
	U32		n = 0;
	U32		delay_time = 0;
	U32		failed_slots[32];
	U32		failed = 0;
	extern	char *clreop;

	printf(clreop);
	if (avr_parse_file(databufp, len))
			return (1);

	for (i = 0; i < 32; i++)
		failed_slots[i] = 0;
	switch(avr) {
		case AVR_IOP:
			for (i = 0; i < 16; i++) {
				if ((slot_info & (1 << i)) == 0)
					slot[n++] = (~(slot_info & 0x1FFFFF) & (1 << i));
			}
			if (n == 0) {
				printf("\n\rNo IOP's Found!!\n\r");
				goto done;		
			} else {
				if (n != 1) {
					select_avr(slot, &n, flash);
				}	
			}
			if (n == 0)
				return (1);
			printf(clreop);
			break;
		case AVR_DDH:
			i = 16;
			if ((slot_info & (1 << i)) == 0)
				slot[n++] = 0x080000; 
			i = 17;
			if ((slot_info & (1 << i)) == 0)
				slot[n++] = 0x040000; 
			i = 18;
			if ((slot_info & (1 << i)) == 0)
				slot[n++] = 0x020000; 
			i = 19;
			if ((slot_info & (1 << i)) == 0)
				slot[n++] = 0x010000; 
			if (n == 0) {
				printf("\n\rNo DDH's Found!!\n\r");
				goto done;
			}
			select_avr(slot, &n, flash);
			break;
		case AVR_HBC:
			i = 20;
			if ((slot_info & (1 << i)) == 0)
				slot[n++] = 0x400000; 
			if (n == 0) {
				printf("\n\rCould not find other HBC!!\n\r");
				goto done;
			}
			break;
		case AVR_EVC:
			/*
			 * Assume both evc's are present.
			 */
			slot[n++] = 0x100000;
			slot[n++] = 0x200000;
			select_avr(slot, &n, flash);
			delay_time = 5;
			break;
		default:
			printf("\n\rUnknown Board Type %d\n\r", avr);
			goto done;
	}
	printf(clreop);
   	printf("\033[16;1H\033[KBoards Selected: "); 
	for (i = 0; i < n; i++) {
		j = 0;
		k = slot[i];
		while (!(k & 1)) {
			j++;
			k = k >> 1;
		}
		printf("%s ", slot_to_str[j]);
	}
	printf("\n\r");
	for (i = 0; i < n; i++) {
		j = 0;
		k = slot[i];
		while (!(k & 1)) {
			j++;
			k = k >> 1;
		}
		avr_reset(slot[i]);
   		printf("\033[18;1H\033[K");
		slot_ptr = slot_to_str[j];
		if (avr_probe_slot(slot[i])) {
			failed_slots[failed++] = slot[i];
			delay(2);
		} else {

			if (flash) {
   				printf("\033[18;1H\033[KSaving EEPROM at slot %s ...   ", 
							slot_to_str[j]);
				save_eeprom();
			}

   			printf("\033[18;1H\033[KProgramming AVR at slot %s ...   ", 
							slot_to_str[j]);
			avr_write(flash);
   			printf("\033[18;1H\033[KVerifying Write at slot %s ...   ", 
							slot_to_str[j]);
			if (avr_verify(flash))
				failed_slots[failed++] = slot[i];
			if (flash) {
   				printf("\033[18;1H\033[KRestoring EEPROM at slot %s ...   ", 
							slot_to_str[j]);
				restore_eeprom();
			}
		}
		avr_rel_reset(slot[i]);
		if (delay_time) 
			delay(delay_time);
	}
	if (failed) {
   		printf("\033[22;1H\033[K"); 
		printf("Programming Failed on slots: ");
		for (i = 0; i < failed; i++) {
			j = 0;
			k = failed_slots[i];
			while (!(k & 1)) {
				j++;
				k = k >> 1;
			}
			printf("%s ", slot_to_str[j]);
		}			
		printf("\n\r");
	}

done:
   	printf("\033[23;1H\033[K"); 
	printf("Press any key to continue ...");
	while (!kbhit())
		;
	ttyA_in();
	return (0);
}


