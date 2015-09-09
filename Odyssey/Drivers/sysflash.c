/*
 *
 * MODULE: sysflash.c - header file for the System Flash Driver
 *
 * Copyright (C) 1998 by ConvergeNet Technologies, Inc.
 *                       2222 Trade Zone Blvd.
 *                       San Jose, CA  95131  U.S.A.
 *
 * HISTORY:
 * --------
 * 09/04/98	- Created by Sudhir
 * 02/12/99 - Jim Frandeen Change long long to I64; replace
 *		data[i] |= (buf[index++] << 8) | buf[index++] to eliminate warning.
 *
 *
 *
 * This material is a confidential trade secret and proprietary information
 * of ConvergeNet Technologies, Inc. which may not be reproduced, used, sold
 * or transferred to any third party without the prior written consent of
 * ConvergeNet Technologies, Inc.  This material is also copyrighted as an
 * unpublished work under sections 104 and 408 of Title 17 of the United
 * States Code.  Law prohibits unauthorized use, copying or reproduction.
 *
 */

#include "types.h"
#include "sysflash.h"
#include "galileo.h"
#include "pcidev.h"

static int memcmp(u8 *, u8 *, u32 );

void 	sysflash_sector_erase_cmd(int , addr_t);
int 	sysflash_sector_erase_verify(int , addr_t);
void	sys_flash_program_cmd(int, addr_t, u16);
int		sys_flash_program_verify(addr_t, u16);

/* Need to be called if the Galileo is not
 * Initialized for CS3
 */
void
sysflash_init()
{
	/* Init Galileo for Sys Flash*/
#ifdef	INCLUDE_ODYSSEY
	U32 val;
	/* Configure CS2 for 64 bit wide */
	val = *((U32 *)(GT_DEV2_PARMS_REG));
	val = DWSWAP(val);
	val |= 0x00300000;
	val = DWSWAP(val);
	*((U32 *)(GT_DEV2_PARMS_REG)) = val;
#else
	u32 *addr;
	addr = (u32 *)GT_DEV_BANK3_REG_ADDR;
	*addr = GT_DEV_BANK3_VAL;
#endif
}


/* Resets the Flash to the Read Mode 
 *
 * Input : fnum - the flash chip number 0-3
 */

void
sysflash_reset(int fnum)
{
	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR1)) = FLASH_RESET_CMD; 
}

/* Gets the Manufacturer's ID, For AMD is 0x01
 *
 * Input : fnum - the flash chip number 0-3
 */
ulong
sysflash_getmanf(int fnum)
{
	ulong 	ret;

	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR1)) = FLASH_FCMD1;
	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR2)) = FLASH_FCMD2;
	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR3)) = FLASH_AUTO_SELECT_CMD;

	/* The address 0ffset can be 0xXX00 */
	ret = *FLASHX(fnum, (SYSFLASH_START + 0x5500));

	/* Reset the Chip */
	sysflash_reset(fnum);
	return(ret);
}

/* Gets the Device's ID, For Am29F400B the value is 0x2223 in word mode
 *
 * Input : fnum - the flash chip number 0-3
 */
ulong
sysflash_getdevid(int fnum)
{
	ulong 	ret;

	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR1)) = FLASH_FCMD1;
	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR2)) = FLASH_FCMD2;
	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR3)) = FLASH_AUTO_SELECT_CMD;

	/* The address 0ffset can be 0xXX01 */
	ret = *FLASHX(fnum, (SYSFLASH_START + 0x5501));

	/* Reset the Chip */
	sysflash_reset(fnum);
	return(ret);
}


/* Issues the Command for chip Erase, does not wait for the 
 * Completion
 *
 * Input : fnum - the flash chip number 0-3
 */
void
sysflash_chiperase_cmd(int fnum)
{
	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR1)) = FLASH_FCMD1;
	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR2)) = FLASH_FCMD2;
	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR3)) = FLASH_ERASE_CMD;
	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR1)) = FLASH_FCMD1;
	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR2)) = FLASH_FCMD2;
	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR3)) = FLASH_CHIP_ERASE_CMD;
}

/* Verifies whther Chip Erase is Complete 
 * Completion
 *
 * Input 	: fnum - the flash chip number 0-3
 * Return	: 1 - SUCCESS, 0 -FAIL
 */
int
sysflash_chiperase_verify(int fnum)
{
	u8	poll_byte;
	int	not_done = 1;

	while ( not_done) {
		poll_byte = *FLASHX(fnum, (SYSFLASH_START));
		if (poll_byte & FLASH_DATA_POLLING) {
			not_done = 0;
		} else {
			if ( poll_byte & FLASH_TIMING_LIMIT) {
				poll_byte = *FLASHX(fnum, (SYSFLASH_START));
				if ( poll_byte & FLASH_DATA_POLLING) 
					not_done = 0;
				else{
					sysflash_reset(fnum);
					return(FLASH_FAIL);
				}
			}
		}
		wheel(0);
	}
	return(FLASH_SUCCESS);
}

/* Erases the whole system Flash ( 4 chips) and
 * waits for completion
 *
 * Return	: 1 - SUCCESS, 0 -FAIL
 */

int
sysflash_erase()
{
	/* Issue the Erase command first
	 * so that the chip are erased simultaneously
	 */
	sysflash_chiperase_cmd(0);
	sysflash_chiperase_cmd(1);
	sysflash_chiperase_cmd(2);
	sysflash_chiperase_cmd(3);

	if ( sysflash_chiperase_verify(0) != FLASH_SUCCESS ) {
		return(FLASH_FAIL);
	} else if ( sysflash_chiperase_verify(1) != FLASH_SUCCESS ) {
		return(FLASH_FAIL);
	} else if ( sysflash_chiperase_verify(2) != FLASH_SUCCESS ) {
		return(FLASH_FAIL);
	} else if ( sysflash_chiperase_verify(3) != FLASH_SUCCESS ) {
		return(FLASH_FAIL);
	} else {
		return(FLASH_SUCCESS);
	}
}

/* Erases a particular sector within the flash system
 * waits for completion. The given sector includes
 * 4 interleaved flashes
 *
 * Input    : sec_num - Sector Number , 0-7(31 for Toshiba). The sector 
 *                      7(0 for Toshiba) includes sector 8(1), 9(2) and 10(3)
 * Return	: 1 - SUCCESS, 0 -FAIL
 */
int
sysflash_sector_erase(int sec_num)
{
	addr_t	sec_addr;
	addr_t	sec_addr_list[4];
	int 	i;

	sec_addr = SYSFLASH_START + (sec_num * SYSFLASH_SECTORX_SIZEW);
	sec_addr_list[0] = sec_addr;
#ifdef	INCLUDE_ODYSSEY
	sec_addr_list[1] = sec_addr_list[0] + SYSFLASH_SECTOR08_SIZEW;
	sec_addr_list[2] = sec_addr_list[1] + SYSFLASH_SECTOR04_SIZEW;
	sec_addr_list[3] = sec_addr_list[2] + SYSFLASH_SECTOR04_SIZEW;
#else	/* AMD */
	sec_addr_list[1] = sec_addr_list[0] + SYSFLASH_SECTOR16_SIZEW;
	sec_addr_list[2] = sec_addr_list[1] + SYSFLASH_SECTOR04_SIZEW;
	sec_addr_list[3] = sec_addr_list[2] + SYSFLASH_SECTOR04_SIZEW;
#endif	/* AMD */

	for(i=0; i < 4; i++ ) {
		sec_addr = sec_addr_list[i];
		sysflash_sector_erase_cmd(0, sec_addr);
		sysflash_sector_erase_cmd(1, sec_addr);
		sysflash_sector_erase_cmd(2, sec_addr);
		sysflash_sector_erase_cmd(3, sec_addr);

		if ( sysflash_sector_erase_verify(0, sec_addr) != FLASH_SUCCESS ) {
			return(FLASH_FAIL);
		} else if (sysflash_sector_erase_verify(1, sec_addr)!= FLASH_SUCCESS ){
			return(FLASH_FAIL);
		} else if (sysflash_sector_erase_verify(2, sec_addr)!= FLASH_SUCCESS ){
			return(FLASH_FAIL);
		} else if (sysflash_sector_erase_verify(3, sec_addr)!= FLASH_SUCCESS ){
			return(FLASH_FAIL);
		}
		if( sec_num != MULTI_SECTOR) {
			return(FLASH_SUCCESS);
		}
	}
	return(FLASH_SUCCESS);
}

/* Issues the Command for erasing  a particular sector within the 
 * given flash chip, does not wait for completion. 
 * 
 *
 * Input    : fnum    - Flash Number
 *            sec_addr- Sector Address , The word address of the Sector
 *                      for a given flash. The actual address is calculated
 *                      by FLASHX macro
 */
void
sysflash_sector_erase_cmd(int fnum, addr_t sec_addr)
{
	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR1)) = FLASH_FCMD1;
	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR2)) = FLASH_FCMD2;
	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR3)) = FLASH_ERASE_CMD;
	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR1)) = FLASH_FCMD1;
	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR2)) = FLASH_FCMD2;
	*FLASHX(fnum, sec_addr) = FLASH_SECTOR_ERASE_CMD;
}

/* Verifies the Flash erase is successful or not.
 * For a given flash chip
 * 
 *
 * Input    : fnum    - Flash Number
 *            sec_addr- Sector Address , The word address of the Sector
 *                      for a given flash. The actual address is calculated
 *                      by FLASHX macro
 * Return	: 1 - SUCCESS, 0 -FAIL
 */
int
sysflash_sector_erase_verify(int fnum, addr_t sec_addr)
{
	u8	poll_byte;
	int	not_done = 1;

	while ( not_done) {
		poll_byte = *FLASHX(fnum, sec_addr);
		if (poll_byte & FLASH_DATA_POLLING) {
			not_done = 0;
		} else {
			if ( poll_byte & FLASH_TIMING_LIMIT) {
				poll_byte = *FLASHX(fnum, sec_addr);
				if ( poll_byte & FLASH_DATA_POLLING) {
					not_done = 0;
				} else {
					sysflash_reset(fnum);
					return(FLASH_FAIL);
				}
			}
		}
		wheel(0);
	}
	return(FLASH_SUCCESS);
}

/* Programs the Flash with the data from the
 * 'buf' to the Flash with offset 'offset'.
 * 
 * The offset has to be half-word aligned
 * len should be multiple of 2 bytes except 
 * the last buffer of the image to be programmed
 *
 * Input    : buf     - The char array buffer
              offset  - 0 to the SIZE of the Flash in bytes
 *            length  - The number of bytes to be programmed
 * Return	: 1 - SUCCESS, 0 -FAIL
 */
int
sysflash_program(u8 *buf, u32 offset, u32 length)
{
	register int 	i, fnum[4], index = 0;
	register addr_t	addr[4];
	register u16	data[4];
	register u32	off, len;

	off = offset;
	len = length;
	for(i=0; i < 4; i++)
		addr[i] = 0;
	while ( len ) {
		/* Issue the Program Commands for the 4 flashes*/
		for(i=0; ((i < 4)&& len); i++) {
			fnum[i]	= (off & 0x7) >> 1;
			addr[i]	= SYSFLASH_START + (off & ~0x1);
			if ( len == 1 ) {
				data[i] = (buf[index++] << 8) | 0xFF;
				len 	-= 1;
				off 	+= 1;
			} else {
				// Replace following statement to eliminate "warning: Ambiguous lvalue
				// usage: index with Green Hills
				// data[i] |= (buf[index++] << 8) | buf[index++];
				data[i] = buf[index++] << 8;
			        data[i] |=  buf[index++];
				len 	-= 2;
				off 	+= 2;
			}
			sys_flash_program_cmd(fnum[i], addr[i], data[i]);
		}

		/* Verify program success or not */
		for(i=0; i < 4; i++) {
			if( addr[i] ) {
				if ( sys_flash_program_verify(addr[i],data[i]) !=FLASH_SUCCESS){
					sysflash_reset(fnum[i]);
					return(FLASH_FAIL);
				}
				addr[i] = 0;
			}
		}
		wheel(0);
	}
	if(memcmp(buf, (u8 *)(SYSFLASH_START+offset), length) == 0)
		return(FLASH_SUCCESS);
	else
		return(FLASH_FAIL);
}

/* Issues the Program Commands for the flash
 * 
 * Input    : fnum    - The number of Flash 0-3
              addr    - address in of the flash location
 *            data    - a halfword data to be programmed
 *
 * Return	: 1 - SUCCESS, 0 -FAIL
 */
void
sys_flash_program_cmd(int fnum, addr_t addr, u16 data)
{
	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR1)) = FLASH_FCMD1;
	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR2)) = FLASH_FCMD2;
	*FLASHX(fnum, (SYSFLASH_START + FLASH_FCMD_ADDR3)) = FLASH_PROGRAM_CMD;
	*((u16 *)addr) = data;
}

/* Verifies whther programming is successful
 * 
 * Input    : addr    - address in of the flash location
 *            data    - a halfword data which was programmed
 */
int
sys_flash_program_verify(addr_t addr, u16 data)
{
	u8	poll_byte;
	register int	not_done = 1;

	while ( not_done) {
		poll_byte = *((u16 *)addr);
		if ((poll_byte & FLASH_DATA_POLLING) ==((u8)data & FLASH_DATA_POLLING)){
			not_done = 0;
		} else {
			if ( poll_byte & FLASH_TIMING_LIMIT) {
				poll_byte = *((u16 *)addr);
				if ( (poll_byte & FLASH_DATA_POLLING) 
									== ((u8)data & FLASH_DATA_POLLING)) 
					not_done = 0;
				else
					return(FLASH_FAIL);
			}
		}
	}
	return(FLASH_SUCCESS);
}

static int
memcmp(u8 *src, u8 *dst, u32 len)
{
	while(len--){
		if(*src++ != *dst++)
			return(1);
	}
	return(0);
}


/* Erases all the sectors which are used for Image 1
 * 
 * Return	: 1 - SUCCESS, 0 -FAIL
 */
int
sysflash_erase1()
{
	int	i;
	for(i=0; i < IMAGE2_SECTOR_START; i++) {
		if ( sysflash_sector_erase(i) != FLASH_SUCCESS)
			return(FLASH_FAIL);
	}
	return(FLASH_SUCCESS);
}



/* Erases all the sectors which are used for Image 2
 * 
 * Return	: 1 - SUCCESS, 0 -FAIL
 */
int
sysflash_erase2()
{
	int	i;
	for(i=IMAGE2_SECTOR_START; i < TOTAL_SECTORS; i++) {
		if ( sysflash_sector_erase(i) != FLASH_SUCCESS)
			return(FLASH_FAIL);
	}
	return(FLASH_SUCCESS);
}

int
sysflash_erase_image_block(int block)
{
	int	i;
	int	j;

	i = (block * 8);
	j = i + 8;
	for(; i < j; i++) {
		if ( sysflash_sector_erase(i) != FLASH_SUCCESS)
			return(FLASH_FAIL);
	}
	return(FLASH_SUCCESS);
}

#define	SFSIZE	256
U8 sfbuf[SFSIZE];
int
sysflash_test()
{
	int findex;
	int i, eflag;
	U8 *ptr = (U8 * )SYSFLASH_START;
	U16 *hptr = (U16 * )SYSFLASH_START;
	U32 *wptr = (U32 * )SYSFLASH_START;
	I64*lptr = (I64 * )SYSFLASH_START;

	sysflash_init();
	printf("Detected System Flash, Man Id %X, Dev Id %X, Size %d\n\r", 
				sysflash_getmanf(0), sysflash_getdevid(0), SYSFLASH_SIZEB);
	
	/* Erase the flash */
#if 0
	printf("Erasing the entire flash...");
	if ( sysflash_erase() ) {
		printf("Done\n\r");
	} else {
		printf("Fail\n\r");
		return(FLASH_FAIL);
	}
#else
	printf("Erasing the flash1...");
	if ( sysflash_erase1() ) {
		printf("Done\n\r");
	} else {
		printf("Fail\n\r");
		return(FLASH_FAIL);
	}
	printf("Erasing the flash2...");
	if ( sysflash_erase2() ) {
		printf("Done\n\r");
	} else {
		printf("Fail\n\r");
		return(FLASH_FAIL);
	}
#endif

	/* Test whther flash is blank */
	printf("Testing the Flash for blank.....");
	eflag = 0;
	for(i=0; i < SYSFLASH_SIZEB; i++) {
		if (ptr[i] !=  0xFF)  {
			eflag = 1;
			break;
		}
		wheel(0);
	}
	if ( eflag ) {
		printf("Fail at %X\n\r", i);
		return(FLASH_FAIL);
	} else {
		printf("Done\n\r");
	}

	/* Program the Flash */
	for(i=0; i < SFSIZE; i++)
		sfbuf[i] = i;
	
	printf("Programming....");
	findex = 0;
	while(findex < SYSFLASH_SIZEB) {
		if ( !sysflash_program(sfbuf, findex, SFSIZE)) {
			printf("Fail ...... at %d\n\r", findex);
			return(FLASH_FAIL);
		}
		findex += SFSIZE;
	}
	printf("... Done\n\r");
	
	/* Test the Flash */
	printf("Testing the contents of Flash.....");
	eflag = 0;
	for(i=0; i < SYSFLASH_SIZEB; i++) {
		if (ptr[i] != ( i & 0xFF) ) {
			eflag = 1;
			break;
		}
		wheel(0);
	}
	if ( eflag ) {
		printf("Fail\n\r");
		return(FLASH_FAIL);
	} else {
		printf("Done\n\r");
	}
	return(FLASH_SUCCESS);

#ifdef	NOT_USED
#if 0
	printf("Contents(bytes):\n\r");
	for(i=0; i < 24; i++)
		printf("%02X ", ptr[i]);
#endif

#if 0
	hptr = (U16 *)sfbuf;
	printf("\n\rContents(halfwords): \n\r");
	for(i=0; i < 12; i++)
		printf("%04X ", hptr[i]);

	wptr = (U32 *)sfbuf;
	printf("\n\rContents(words): \n\r");
	for(i=0; i < 6; i++)
		printf("%08X ", wptr[i]);
#endif

#if 0
	printf("\n\rContents(long words): \n\r");
	for(i=0; i < 3; i++) {
		printf("%08X", lptr[i] >> 32);
		printf("%08X ", lptr[i]);
	}
	printf("\n\r");
#endif
	return(FLASH_SUCCESS);
#endif
		
}
