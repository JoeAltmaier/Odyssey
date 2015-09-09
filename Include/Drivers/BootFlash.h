
#ifndef BOOTFLASH_H
#define BOOTFLASH_H

STATUS program_boot_flash(U8 *buf, U32 len);

#define ATMEL_ID		0x1F
#define AT49BV040_ID	0x13
#define BOOTFLASH_ID	AT49BV040_ID
#endif /* BOOTFLASH_H */
