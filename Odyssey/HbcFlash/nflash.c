/*
 *
 * MODULE: nflash.c - Driver for the System E2PROM 
 *
 * Copyright (C) 1998 by ConvergeNet Technologies, Inc.
 *                       2222 Trade Zone Blvd.
 *                       San Jose, CA  95131  U.S.A.
 *
 * HISTORY:
 * --------
 * 11/17/98	- Created by Sudhir
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
#include "CtPrefix.h" // Turn on _DEBUG
#include "nucleus.h"
#include "types.h"
#include "nflash.h"
#include "CtEvent.h"

// Use_Semaphore not defined when used by flash storage system.
//#define Use_Semaphore


#ifdef _DEBUG
// When we return an error code, call a method so we can set a breakpoint.
STATUS FF_Error(STATUS status);
#define FF_ERROR(code) FF_Error(CTS_FLASH_##code)
#else

// NOT _DEBUG
#define FF_ERROR(code) CTS_FLASH_##code
#endif

/* Variable to store the state of the FLASH Control*/
extern  unsigned long long *hbc_ctl;
U32		NFlashStatus=0;

void	calculate_ecc(U8 *dbuf, U8 *ecc1, U8 *ecc2, U8 *ecc3);
STATUS	correct_data(U8 *dbuf, U8 ecc1, U8 ecc2, U8 ecc3, U8 *eccbuf);
NU_SEMAPHORE	NFlashSemaphore;
NU_SEMAPHORE	*nflash_semaptr = NU_NULL;

#ifdef	CONFIG_E2
U8		flash_cntl = 0;

void	nflash_cntl_assert(U8 cntl);
void	nflash_cntl_deassert(U8 cntl);
STATUS	nflash_ready();
U32		nflash_get_status();
void	nflash_send_cmd(U32 cmd);
void	nflash_send_addr(U32 addr, I32 num_addr_cycle);

void	calculate_page_ecc(U8 *buf, U8 *eccbuf);
STATUS  correct_page_data(U8 *buf, U8 *eccbuf);

/* Interface to NAND Flash functions */
/* Resets the Flash */
STATUS
nflash_reset()
{
	int	i = 0;
	/* Reset the Lower Bank */
	nflash_send_cmd(NFLASH_CMD_RESET);
	while(nflash_ready() != OK){
		i++;
		if ( i > NFLASH_TIMEOUT ) {
			return(CTS_FLASH_DEVICE_BUSY);
		}
	}
	return(OK);
}
/* Initializes the Flash, Need to be called before any operations
 * performed on the flash 
 */
STATUS
nflash_init()
{
	if ( !nflash_semaptr) {
		/* Create the Semaphore */
		nflash_semaptr = &NFlashSemaphore;
		if ( NU_Create_Semaphore(nflash_semaptr, "NFlash", 1, NU_FIFO) != 
				NU_SUCCESS) {
			nflash_semaptr = NU_NULL;
			return(CTS_FLASH_DEVICE_SEMA);
		}
	}

	NU_Obtain_Semaphore(nflash_semaptr, NU_SUSPEND);
	/* Put the Flash into 528 bytes per page mode */
	flash_cntl = 0;
	nflash_cntl_assert(NFLASH_CNTL_RW);
	nflash_cntl_assert(NFLASH_CNTL_RE);
	/* Put the Flash into write protect mode */
	nflash_cntl_assert(NFLASH_CNTL_WP);
	if (nflash_reset() != OK ) {
		NU_Release_Semaphore(nflash_semaptr);	
		return(CTS_FLASH_DEVICE_RESET);
	}

	NU_Release_Semaphore(nflash_semaptr);
	return(OK);
}

/* Releases the Semaphores aquired by erase/read/write calls
 */
void
nflash_release_sema()
{
#ifdef Use_Semaphore
	if ( nflash_semaptr)
		NU_Release_Semaphore(nflash_semaptr);
#endif
	return;
}

/* Erases the Flash blockes 
 * INPUT  : block addr will be 0-2047 
 * RETURN : OK/FF_ERROR_NFLASH_ERASE_ERROR
 */
STATUS
nflash_erase_block(U32	block_addr)
{
	U32	addr;
	I32	counter = 0;
	U8	status;
	
#ifdef Use_Semaphore
	if ( !nflash_semaptr) {
		NFlashStatus = CTS_FLASH_DEVICE_SEMA;
		return(CTS_FLASH_DEVICE_SEMA);
	}

	NU_Obtain_Semaphore(nflash_semaptr, NU_SUSPEND);
#endif

	if ( block_addr < 1024 ) {
		/* Lower Bank */
		nflash_cntl_assert(NFLASH_CNTL_CE);
	} else {
		/* Upper Bank */
		nflash_cntl_deassert(NFLASH_CNTL_CE);
		block_addr -= 1024;
	}

	/* Write the Address */
	addr = block_addr << 14;
	nflash_cntl_deassert(NFLASH_CNTL_WP);
	nflash_send_cmd(NFLASH_CMD_ERASE_SETUP);
	nflash_send_addr(addr, 2);
	
	/* Start Erasing */
	nflash_send_cmd(NFLASH_CMD_ERASE_START);
	while(nflash_ready() != OK ) {
		counter++;
		if ( counter > NFLASH_TIMEOUT)
			break;
	}
	if ( counter > NFLASH_TIMEOUT ) {
		/* Error in Erasing */
		nflash_cntl_assert(NFLASH_CNTL_WP);
		nflash_reset();
		NFlashStatus = CTS_FLASH_DEVICE_ERASE;
		return(CTS_FLASH_DEVICE_ERASE);
	} else {
		status = nflash_get_status();
		if ( status & NFLASH_STATUS_FAIL) {
			nflash_cntl_assert(NFLASH_CNTL_WP);
			NFlashStatus = CTS_FLASH_DEVICE_ERASE;
			return(CTS_FLASH_DEVICE_ERASE);
		}
	}
	nflash_cntl_assert(NFLASH_CNTL_WP);
	NFlashStatus = OK;
	return(OK);
}

/* Program the Page
 * INPUT : buf - an array of 2048 bytes
 * 		   pagenum - 0 - 65536
 * RETURN: OK/FF_ERROR_DEVICE_WRITE
 */

STATUS
nflash_write_page(U8 *buf, U32 pagenum)
{
	I32	counter = 0;
	U8	status;
	I32	len = NFLASH_PAGE_SIZE;
	U8	eccbuf[64];
	U32	*lbuf = (U32 *)buf;
	U32 *leccbuf = (U32 *)eccbuf;
	I32	i;

#ifdef Use_Semaphore
	if ( !nflash_semaptr) {
		NFlashStatus = CTS_FLASH_DEVICE_SEMA;
		return(CTS_FLASH_DEVICE_SEMA);
	}
#endif
	/* Calculate the ECC for the 512 bytes and
	 * Store it in eccbuf
	 */
	calculate_page_ecc(buf, eccbuf);

#ifdef	DEBUG
	printf("\n\rEcc:\n\r");
	for(i=0; i < 64; i++) {
		printf("%02X ", eccbuf[i]);
	}
	printf("\n\r");
#endif

#ifdef Use_Semaphore
	NU_Obtain_Semaphore(nflash_semaptr, NU_SUSPEND);
#endif

	if ( pagenum < 32768 ) {
		/* Lower Bank */
		nflash_cntl_assert(NFLASH_CNTL_CE);
	} else {
		/* Upper Bank */
		nflash_cntl_deassert(NFLASH_CNTL_CE);
		pagenum -= 32768;
	}
	nflash_cntl_deassert(NFLASH_CNTL_WP);
	/* write the Address */
	nflash_send_cmd(NFLASH_CMD_DATAIN);
	nflash_send_addr(pagenum << 9, 3);
	nflash_cntl_deassert(NFLASH_CNTL_RW);
	/* write the data */
	while(len) {
		nflash_cntl_assert(NFLASH_CNTL_WE);
		NFLASH_DATA_WRITE(*lbuf);
		nflash_cntl_deassert(NFLASH_CNTL_WE);
		len -= 4;
		lbuf++;
	}
	/* write the ECC */
	len = 64;
	i = 0;
	while(len) {
		nflash_cntl_assert(NFLASH_CNTL_WE);
		NFLASH_DATA_WRITE(leccbuf[i]);
		nflash_cntl_deassert(NFLASH_CNTL_WE);
		len -= 4;
		i++;
	}
	/* Start Progrqamming */
	nflash_send_cmd(NFLASH_CMD_PROGRAM);


	/* Check the operation is complete ?
	 */
	while(nflash_ready() != OK ) {
		counter++;
		if ( counter > NFLASH_TIMEOUT)
			break;
	}
	if ( counter > NFLASH_TIMEOUT ) {
		/* Error in Programming */
		nflash_cntl_assert(NFLASH_CNTL_WP);
		nflash_reset();
		NFlashStatus = CTS_FLASH_DEVICE_WRITE;
		return(CTS_FLASH_DEVICE_WRITE);
	} else {
		status = nflash_get_status();
		if ( status & NFLASH_STATUS_FAIL) {
			nflash_cntl_assert(NFLASH_CNTL_WP);
			NFlashStatus = CTS_FLASH_DEVICE_WRITE;
			return(CTS_FLASH_DEVICE_WRITE);
		}
	}
	nflash_cntl_assert(NFLASH_CNTL_WP);
	NFlashStatus = OK;;
	return(OK);
}
/* Read the Page - Corrects 16 bits( 1 bit/128 bytes) and 
 *                 detects 32 bits ( 2 bits/128 bytes)
 * INPUT : buf - an array of 2048 bytes
 * 		   pagenum - 0 - 65536
 * OUTPUT: buf - contains the 2048 bytes of the Page
 * RETURN: OK/FF_ERROR_DEVICE_READ/FF_ERROR_NFLASH_READ_CORRECTED/
 * 		   FF_ERROR_ECC_CORRECTED/FF_ERROR_NFLASH_READ_UNCORRECTABLE
 */

STATUS
nflash_read_page(U8 *buf, U32 pagenum)
{
	U32 i, counter;
	I32	len = NFLASH_PAGE_SIZE;
	U8	eccbuf[64];
	U32 *lbuf = (U32 *)buf;
	U32 *leccbuf = (U32 *)eccbuf;
	
#ifdef Use_Semaphore
	if ( !nflash_semaptr) {
		NFlashStatus = CTS_FLASH_DEVICE_SEMA;
		return(CTS_FLASH_DEVICE_SEMA);
	}
	NU_Obtain_Semaphore(nflash_semaptr, NU_SUSPEND);
#endif

	if ( pagenum < 32768 ) {
		/* Lower Bank */
		nflash_cntl_assert(NFLASH_CNTL_CE);
	} else {
		/* Upper Bank */
		nflash_cntl_deassert(NFLASH_CNTL_CE);
		pagenum -= 32768;
	}
	/* Issue the Command */
	nflash_send_cmd(NFLASH_CMD_READ1);
	nflash_send_addr(pagenum << 9, 3);
	
	/* Read the Data */
	nflash_cntl_assert(NFLASH_CNTL_RW);
	i = 0;
	counter = 0;
	while(len) {
		if (nflash_ready() == OK) {
			nflash_cntl_assert(NFLASH_CNTL_RE);
			lbuf[i] = NFLASH_DATA_READ;
			i++;
			len -=4;
			counter = 0;
			nflash_cntl_deassert(NFLASH_CNTL_RE);
		} else {
			counter++;
			if ( counter > NFLASH_TIMEOUT)
				break;
		}
	}
	if ( len ) { 
		NFlashStatus = CTS_FLASH_DEVICE_ERROR;
		return(CTS_FLASH_DEVICE_ERROR);
	}
	/* Read the ECC */
	len = 64;
	i = 0;
	counter = 0;
	while(len) {
		if (nflash_ready() == OK) {
			nflash_cntl_assert(NFLASH_CNTL_RE);
			leccbuf[i] = NFLASH_DATA_READ;
			i++;
			len -=4;
			counter = 0;
			nflash_cntl_deassert(NFLASH_CNTL_RE);
		} else {
			counter++;
			if ( counter > NFLASH_TIMEOUT)
				break;
		}
	}
	if ( len ) {
		NFlashStatus = CTS_FLASH_DEVICE_ERROR;
		return(CTS_FLASH_DEVICE_ERROR);
	}
	
	NFlashStatus = correct_page_data(buf, eccbuf);
	return(NFlashStatus);
}
U32
nflash_get_id(U32 array)
{
	// Create a union so we can access the word received from the 
	// device one byte at a time.
	union
	{
		U32		bank_IDs;
		U8		device_ID[4];
	} bank_word;
	
	int same;

	U8 unit_code, maker_code;
	U32 device_index;
	U32  unit_ID;
	STATUS status;


#ifdef Use_Semaphore
	if ( !nflash_semaptr) {
		printf("Error: Call nflash_init() first\n\r");
		return(0);
	}

	NU_Obtain_Semaphore(nflash_semaptr, NU_SUSPEND);
#endif

	if ( array == 0) 
		/* Select lower bank */
		nflash_cntl_assert(NFLASH_CNTL_CE);
	else
		/* Select Upper bank */
		nflash_cntl_deassert(NFLASH_CNTL_CE);
		
	/* Send the Command */
	nflash_send_cmd(NFLASH_CMD_IDREAD);
	/* Send the Address */
	nflash_send_addr(0, 1);
	/* Enable Read */
	nflash_cntl_assert(NFLASH_CNTL_RE);
	nflash_cntl_assert(NFLASH_CNTL_RW);
	/* Get the Maker code from all 4 devices*/
	bank_word.bank_IDs = NFLASH_DATA_READ;
	nflash_cntl_deassert(NFLASH_CNTL_RE);
	nflash_cntl_assert(NFLASH_CNTL_RW);

	/* Assume all 4 maker codes are the same. */
	maker_code = bank_word.device_ID[0];
	same = 1;
	
	/* Each maker_code should be the same. */
	for (device_index = 1; device_index < 4; device_index++)
	{
		if (bank_word.device_ID[device_index] != maker_code)
			same = 0;
	}
	
	/* Check to see if they are all the same. */
	if (same == 0)
	{
		maker_code = 0;
		Tracef("\nInvalid maker code for array %d, register value = %X", 
		array, bank_word.bank_IDs);
		status = FF_ERROR(INVALID_MAKER_CODE);
	}

	/* Get the Unit Code */
	nflash_cntl_assert(NFLASH_CNTL_RE);
	bank_word.bank_IDs = NFLASH_DATA_READ;
	nflash_cntl_deassert(NFLASH_CNTL_RE);
	
	/* Assume all 4 unit codes are the same. */
	unit_code = bank_word.device_ID[0];
	same = 1;
	
	/* Each unit_code should be the same. */
	for (device_index = 1; device_index < 4; device_index++)
	{
		if (bank_word.device_ID[device_index] != unit_code)
			same = 0;
	}
	
	/* Check to see if they are all the same. */
	if (same == 0)
	{
		unit_code = 0;
		Tracef("\nInvalid unit code for array %d, register value = %X", 
		array, bank_word.bank_IDs);
		if (status == FF_ERROR(INVALID_MAKER_CODE))
			status = FF_ERROR(INVALID_UNIT_MAKER_CODE);
		else
			status = FF_ERROR(INVALID_UNIT_CODE);
	}

#ifdef Use_Semaphore
	NU_Release_Semaphore(nflash_semaptr);
#endif
	
	// Return Maker code in low byte, Unit code in high byte.
	unit_ID = (maker_code | (unit_code << 8));
	return(unit_ID);
		
}
/* Check to see whether Flash is Ready/Busy */

STATUS
nflash_ready()
{
	U8 sts;
	
	/* Read the Status Register */
	if ( (sts = NFLASH_STS_READ) == 0xFF) 
		return(OK);

#ifdef	DEBUG
	printf("busy %02X\n\r", sts);
#endif
	return(CTS_FLASH_DEVICE_BUSY);
		
}

/* Read the Status byte of the Flash
 * RETURN : The Status byte 
 */
U32
nflash_get_status()
{
	U32	status;

	nflash_send_cmd(NFLASH_CMD_STATUS);
	nflash_cntl_assert(NFLASH_CNTL_RE);
	nflash_cntl_assert(NFLASH_CNTL_RW);
	status = NFLASH_DATA_READ;
	nflash_cntl_deassert(NFLASH_CNTL_RE);
	return(status);
}


/* Assert the Control Signals for The Flash
 * INPUT : cntl - the Control Bit
 */

void
nflash_cntl_assert(U8 cntl)
{
	U8 val;

	switch(cntl){
	case NFLASH_CNTL_CLE:
	case NFLASH_CNTL_ALE:
	case NFLASH_CNTL_RW:
		/* Active High */
		val =  NFLASH_CNTL_READ;
		val |= cntl;
		NFLASH_CNTL_WRITE(val);
		break;
	case NFLASH_CNTL_WE:
	case NFLASH_CNTL_RE:
	case NFLASH_CNTL_CE:
		/* Active Low */
		val =  NFLASH_CNTL_READ;
		val &= ~cntl;
		NFLASH_CNTL_WRITE(val);
		break;
	default:
		break;
	}
}

/* Deassert the Control Signals for The Flash
 * INPUT : cntl - the Control Bit
 */
void
nflash_cntl_deassert(U8 cntl)
{
	U8 val;

	switch(cntl){
	case NFLASH_CNTL_CLE:
	case NFLASH_CNTL_ALE:
	case NFLASH_CNTL_RW:
		/* Active High */
		val =  NFLASH_CNTL_READ;
		val &= ~cntl;
		NFLASH_CNTL_WRITE(val);
		break;
	case NFLASH_CNTL_WP:
	case NFLASH_CNTL_WE:
	case NFLASH_CNTL_RE:
	case NFLASH_CNTL_CE:
		/* Active Low */
		val =  NFLASH_CNTL_READ;
		val |= cntl;
		NFLASH_CNTL_WRITE(val);
		break;
	default:
		break;
	}
}

/* Writhe Command to the Flash
 * INPUT : cmd - The command bytes
 */
void
nflash_send_cmd(U32 cmd)
{
	nflash_cntl_assert(NFLASH_CNTL_CLE);
	nflash_cntl_assert(NFLASH_CNTL_WE);
	nflash_cntl_deassert(NFLASH_CNTL_RW);
	NFLASH_DATA_WRITE(cmd);
	nflash_cntl_deassert(NFLASH_CNTL_WE);
	nflash_cntl_deassert(NFLASH_CNTL_CLE);
}

/* Send the Address to the Flash
 * INPUT : addr - 4 bytes where only 24 bits A0-A23 is valid
 *         num_addr_cycle -  2 - for block operations, 3 for Page operations
 */
void
nflash_send_addr(U32 addr, I32 num_addr_cycle) 
{
	U32	aval;
	nflash_cntl_assert(NFLASH_CNTL_ALE);
	nflash_cntl_deassert(NFLASH_CNTL_RW);
	/* Send A0 to A7 */
	if (num_addr_cycle == 3) {
		nflash_cntl_assert(NFLASH_CNTL_WE);
		/* Create the 32-bit value for 4 devices by duplicating */
		aval = addr & 0xFF;
		aval |= aval << 8;
		aval |= aval << 16;
		NFLASH_DATA_WRITE(aval);
		nflash_cntl_deassert(NFLASH_CNTL_WE);
		num_addr_cycle--;
	}
	/* Send A9 to A16 */
	addr = addr >> 9;
	if (num_addr_cycle == 2) {
		nflash_cntl_assert(NFLASH_CNTL_WE);
		/* Create the 32-bit value for 4 devices by duplicating */
		aval = addr & 0xFF;
		aval |= aval << 8;
		aval |= aval << 16;
		NFLASH_DATA_WRITE(aval);
		nflash_cntl_deassert(NFLASH_CNTL_WE);
		num_addr_cycle--;
	}
	/* Send A17 to A23 */
	addr = addr >> 8;
	if (num_addr_cycle == 1) {
		nflash_cntl_assert(NFLASH_CNTL_WE);
		/* Create the 32-bit value for 4 devices by duplicating */
		aval = addr & 0xFF;
		aval |= aval << 8;
		aval |= aval << 16;
		NFLASH_DATA_WRITE(aval);
		nflash_cntl_deassert(NFLASH_CNTL_WE);
	}
	nflash_cntl_deassert(NFLASH_CNTL_ALE);
}
/* Calculates the ECC for the entire 2048 bytes
 * INPUT	: buf - array of 2048 bytes
 *            eccbuf - to store the ECC
 * OUPUT	: eccbuf contains the ECC
 */
void
calculate_page_ecc(U8 *buf, U8 *eccbuf)
{
	U8 ecc1, ecc2, ecc3;
	int i;
	
	/* Calculate the ecc for 16 x 128 bytes of the Page */
	for(i=0; i < 16; i++) {
		calculate_ecc((buf + i * 128), &ecc1, &ecc2, &ecc3);
		eccbuf[ i * 4 + 0] = 0xFF;
		eccbuf[ i * 4 + 1] = ecc1;
		eccbuf[ i * 4 + 2] = ecc2;
		eccbuf[ i * 4 + 3] = ecc3;
	}
}
/* Corrects 16bits/ detects 32 bits of error in 2048 bytes of
 * buf
 * INPUT  : buf - array of 2048 bytes
 *        : eccbuf - which contains the ECC read from the Flash
 * OUTPUT : buf - has the corrcted value
 *          eccbuf - has the corrected ECC
 * RETURN: OK/FF_ERROR_DEVICE_READ/FF_ERROR_NFLASH_READ_CORRECTED/
 *         FF_ERROR_ECC_CORRECTED/FF_ERROR_NFLASH_READ_UNCORRECTABLE
 */          
STATUS
correct_page_data(U8 *buf, U8 *eccbuf)
{
	U8 ecc1, ecc2, ecc3;
	int i;
	STATUS rc, rc1 = OK;
	
	/* Calculate the ecc for 16 x 128 bytes of the Page */
	for(i=0; i < 16; i++) {
		calculate_ecc((buf + i * 128), &ecc1, &ecc2, &ecc3);
		rc = correct_data((buf + i * 128), ecc1, ecc2, ecc3, (eccbuf + i * 4)); 
#ifdef DEBUG
		printf("rc %d\n\r", rc);
#endif
		if ( rc == CTS_FLASH_ECC)
			return(rc);
		if (!rc1)
			rc1 = rc;
	}
#ifdef DEBUG
	printf("\n\rEcc:\n\r");
	for(i=0; i < 64; i++) {
		printf("%02X ", eccbuf[i]);
	}
	printf("\n\r");
#endif
	if ( rc1 )
		return(rc1);
		
	return(OK);
}





STATUS nflash_init_noecc() { return(OK);}
STATUS	nflash_read_noecc(U8 *buf, U32 offset, I32 len)
{
#pragma unused (buf)
#pragma unused (offset)
#pragma unused (len)
	return(OK);
}
STATUS	nflash_program_noecc(U8 *buf, U32 offset, I32 len)
{
#pragma unused (buf)
#pragma unused (offset)
#pragma unused (len)
	return(OK);
}
STATUS	nflash_program_page_noecc(U8 *buf, U32 offset, I32 len)
{
#pragma unused (buf)
#pragma unused (offset)
#pragma unused (len)
	return(OK);
}
STATUS	nflash_program_partial_page_noecc(U8 *buf, U32 offset, I32 len)
{
#pragma unused (buf)
#pragma unused (offset)
#pragma unused (len)
	return(OK);
}
#else	/* CONFIG_E2 */

U16		flash_cntl = 0;

void	nflash_cntl_assert(U16 cntl);
void	nflash_cntl_deassert(U16 cntl);
STATUS	nflash_ready();
U8		nflash_get_status();
void	nflash_send_cmd(U8 cmd);
void	nflash_send_addr(U32 addr, I32 num_addr_cycle);
		
void	calculate_page_ecc(U8 *buf, U8 *eccbuf);
STATUS	correct_page_data(U8 *buf, U8 *eccbuf);


/* Interface to NAND Flash functions */
/* Resets the Flash */
STATUS
nflash_reset()
{
	int	i = 0;
	nflash_send_cmd(NFLASH_CMD_RESET);
	while(nflash_ready() != OK){
		i++;
		if ( i > NFLASH_TIMEOUT )
			return(FF_ERROR_NFLASH_BUSY_ERROR);
	}
	return(OK);
}

/* Initializes the Flash, Need to be called before any operations
 * performed on the flash 
 */
STATUS
nflash_init()
{
	U16	val;

	/* Put the Flash into 528 bytes per page mode */
	val =  NFLASH_CNTL_READ;
	NFLASH_CNTL_WRITE(val);
	if (nflash_reset() != OK )
		return(FF_ERROR_NFLASH_RESET);
	
	/* Put the Flash into write protect mode */
	nflash_cntl_assert(NFLASH_CNTL_WP);
	
	return(OK);
		
}

/* Erases the Flash blockes 
 * INPUT  : block addr will be 0-1023 
 * RETURN : OK/FF_ERROR_NFLASH_ERASE_ERROR
 */
STATUS
nflash_erase_block(U32	block_addr)
{
	U32	addr;
	I32	counter = 0;
	U8	status;
	
	/* Write the Address */
	addr = block_addr << 13;
	nflash_cntl_deassert(NFLASH_CNTL_WP);
	nflash_send_cmd(NFLASH_CMD_ERASE_SETUP);
	nflash_send_addr(addr, 2);
	
	/* Start Erasing */
	nflash_send_cmd(NFLASH_CMD_ERASE_START);
	while(nflash_ready() != OK ) {
		counter++;
		if ( counter > NFLASH_TIMEOUT)
			break;
	}
	if ( counter > NFLASH_TIMEOUT ) {
		/* Error in Erasing */
		nflash_reset();
		nflash_cntl_assert(NFLASH_CNTL_WP);
		return(FF_ERROR_NFLASH_ERASE_ERROR);
	} else {
		status = nflash_get_status();
		if ( status & NFLASH_STATUS_FAIL) {
			nflash_cntl_assert(NFLASH_CNTL_WP);
			return(FF_ERROR_NFLASH_ERASE_ERROR);
		}
	}
	nflash_cntl_assert(NFLASH_CNTL_WP);
	return(OK);
}

/* Program the Page
 * INPUT : buf - an array of 512 bytes
 * 		   pagenum - 0 - 16K
 * RETURN: OK/CTS_FLASH_DEVICE_WRITE
 */

STATUS
nflash_write_page(U8 *buf, U32 pagenum)
{
	I32	counter = 0;
	U8	status;
	I32	len = NFLASH_PAGE_SIZE;
	U8	eccbuf[16];
	I32	i;

	/* Calculate the ECC for the 512 bytes and
	 * Store it in eccbuf
	 */
	calculate_page_ecc(buf, eccbuf);
	nflash_cntl_deassert(NFLASH_CNTL_WP);
	/* write the Address */
	nflash_send_cmd(NFLASH_CMD_DATAIN);
	nflash_send_addr(pagenum << 9, 3);
	nflash_cntl_assert(NFLASH_CNTL_CE);
	nflash_cntl_deassert(NFLASH_CNTL_RW);
	/* write the data */
	while(len) {
		nflash_cntl_assert(NFLASH_CNTL_WE);
		NFLASH_DATA_WRITE(*buf);
		nflash_cntl_deassert(NFLASH_CNTL_WE);
		len--;
		buf++;
	}
	/* write the ECC */
	len = 16;
	i = 0;
	while(len) {
		nflash_cntl_assert(NFLASH_CNTL_WE);
		NFLASH_DATA_WRITE(eccbuf[i]);
		nflash_cntl_deassert(NFLASH_CNTL_WE);
		len--;
		i++;
	}
	nflash_cntl_deassert(NFLASH_CNTL_CE);
	/* Start Progrqamming */
	nflash_send_cmd(NFLASH_CMD_PROGRAM);


	/* Check the operation is complete ?
	 */
	while(nflash_ready() != OK ) {
		counter++;
		if ( counter > NFLASH_TIMEOUT)
			break;
	}
	if ( counter > NFLASH_TIMEOUT ) {
		/* Error in Programming */
		nflash_reset();
		nflash_cntl_assert(NFLASH_CNTL_WP);
		return(CTS_FLASH_DEVICE_WRITE);
	} else {
		status = nflash_get_status();
		if ( status & NFLASH_STATUS_FAIL) {
			nflash_cntl_assert(NFLASH_CNTL_WP);
			return(CTS_FLASH_DEVICE_WRITE);
		}
	}
	nflash_cntl_assert(NFLASH_CNTL_WP);
	return(OK);
}

/* Read the Page - Corrects 4 bits( 1 bit/128 bytes) and 
 *                 detects 8 bits ( 2 bits/128 bytes)
 * INPUT : buf - an array of 512 bytes
 * 		   pagenum - 0 - 16K
 * OUTPUT: buf - contains the 512 bytes of the Page
 * RETURN: OK/FF_ERROR_DEVICE_READ/FF_ERROR_NFLASH_READ_CORRECTED/
 *         FF_ERROR_ECC_CORRECTED/FF_ERROR_NFLASH_READ_UNCORRECTABLE
 */

STATUS
nflash_read_page(U8 *buf, U32 pagenum)
{
	U32 i, counter;
	I32	len = NFLASH_PAGE_SIZE;
	U8	eccbuf[16];
	
	/* Issue the Command */
	nflash_send_cmd(NFLASH_CMD_READ1);
	nflash_send_addr(pagenum << 9, 3);
	
	/* Read the Data */
	nflash_cntl_assert(NFLASH_CNTL_CE);
	nflash_cntl_assert(NFLASH_CNTL_RW);
	i = 0;
	counter = 0;
	while(len) {
		if (nflash_ready() == OK) {
			nflash_cntl_assert(NFLASH_CNTL_RE);
			buf[i] = NFLASH_DATA_READ;
			i++;
			len--;
			counter = 0;
			nflash_cntl_deassert(NFLASH_CNTL_RE);
		} else {
			counter++;
			if ( counter > NFLASH_TIMEOUT)
				break;
		}
	}
	if ( len ) { 
		nflash_cntl_deassert(NFLASH_CNTL_CE);
		return(FF_ERROR_DEVICE_READ);
	}
	/* Read the ECC */
	len = 16;
	i = 0;
	counter = 0;
	while(len) {
		if (nflash_ready() == OK) {
			nflash_cntl_assert(NFLASH_CNTL_RE);
			eccbuf[i] = NFLASH_DATA_READ;
			i++;
			len--;
			counter = 0;
			nflash_cntl_deassert(NFLASH_CNTL_RE);
		} else {
			counter++;
			if ( counter > NFLASH_TIMEOUT)
				break;
		}
	}
	nflash_cntl_deassert(NFLASH_CNTL_CE);
	if ( len ) 
		return(FF_ERROR_DEVICE_READ);
	
	return(correct_page_data(buf, eccbuf));
}

/* Assert the Control Signals for The Flash
 * INPUT : cntl - the Control Bit
 */

void
nflash_cntl_assert(U16 cntl)
{
	U16 val;

	switch(cntl){
	case NFLASH_CNTL_CLE:
	case NFLASH_CNTL_ALE:
	case NFLASH_CNTL_RW:
		/* Active High */
		val =  NFLASH_CNTL_READ;
		val |= cntl;
		NFLASH_CNTL_WRITE(val);
		break;
	case NFLASH_CNTL_WE:
	case NFLASH_CNTL_RE:
	case NFLASH_CNTL_CE:
		/* Active Low */
		val =  NFLASH_CNTL_READ;
		val &= ~cntl;
		NFLASH_CNTL_WRITE(val);
		break;
	default:
		break;
	}
}

/* Deassert the Control Signals for The Flash
 * INPUT : cntl - the Control Bit
 */
void
nflash_cntl_deassert(U16 cntl)
{
	U16 val;

	switch(cntl){
	case NFLASH_CNTL_CLE:
	case NFLASH_CNTL_ALE:
	case NFLASH_CNTL_RW:
		/* Active High */
		val =  NFLASH_CNTL_READ;
		val &= ~cntl;
		NFLASH_CNTL_WRITE(val);
		break;
	case NFLASH_CNTL_WP:
	case NFLASH_CNTL_WE:
	case NFLASH_CNTL_RE:
	case NFLASH_CNTL_CE:
		/* Active Low */
		val =  NFLASH_CNTL_READ;
		val |= cntl;
		NFLASH_CNTL_WRITE(val);
		break;
	default:
		break;
	}
}

/* Check to see whether Flash is Ready/Busy */

STATUS
nflash_ready()
{
	/* Read the Status Register */
	if ( NFLASH_STS_READ )
		return(OK);
	else
		return(FF_ERROR_NFLASH_BUSY_ERROR);
		
}

/* Read the Status byte of the Flash
 * RETURN : The Status byte 
 */
U8
nflash_get_status()
{
	U8	status;

	nflash_send_cmd(NFLASH_CMD_STATUS);
	nflash_cntl_assert(NFLASH_CNTL_CE);
	nflash_cntl_assert(NFLASH_CNTL_RE);
	nflash_cntl_assert(NFLASH_CNTL_RW);
	status = NFLASH_DATA_READ;
	nflash_cntl_deassert(NFLASH_CNTL_RE);
	nflash_cntl_deassert(NFLASH_CNTL_CE);
	return(status);
}

/* Writhe Command to the Flash
 * INPUT : cmd - The command bytes
 */
void
nflash_send_cmd(U8 cmd)
{
	nflash_cntl_assert(NFLASH_CNTL_CLE);
	nflash_cntl_assert(NFLASH_CNTL_CE);
	nflash_cntl_assert(NFLASH_CNTL_WE);
	nflash_cntl_deassert(NFLASH_CNTL_RW);
	NFLASH_DATA_WRITE(cmd);
	nflash_cntl_deassert(NFLASH_CNTL_WE);
	nflash_cntl_deassert(NFLASH_CNTL_CE);
	nflash_cntl_deassert(NFLASH_CNTL_CLE);
}

/* Send the Address to the Flash
 * INPUT : addr - 4 bytes where only 23 bits A0-A22 is valid
 *         num_addr_cycle -  2 - for block operations, 3 for Page operations
 */
void
nflash_send_addr(U32 addr, I32 num_addr_cycle) 
{
	nflash_cntl_assert(NFLASH_CNTL_ALE);
	nflash_cntl_assert(NFLASH_CNTL_CE);
	nflash_cntl_deassert(NFLASH_CNTL_RW);
	/* Send A0 to A7 */
	if (num_addr_cycle == 3) {
		nflash_cntl_assert(NFLASH_CNTL_WE);
		NFLASH_DATA_WRITE(addr & 0xFF);
		nflash_cntl_deassert(NFLASH_CNTL_WE);
		num_addr_cycle--;
	}
	/* Send A9 to A16 */
	addr = addr >> 9;
	if (num_addr_cycle == 2) {
		nflash_cntl_assert(NFLASH_CNTL_WE);
		NFLASH_DATA_WRITE(addr & 0xFF);
		nflash_cntl_deassert(NFLASH_CNTL_WE);
		num_addr_cycle--;
	}
	/* Send A17 to A22 */
	addr = addr >> 8;
	if (num_addr_cycle == 1) {
		nflash_cntl_assert(NFLASH_CNTL_WE);
		NFLASH_DATA_WRITE(addr & 0xFF);
		nflash_cntl_deassert(NFLASH_CNTL_WE);
	}
	nflash_cntl_deassert(NFLASH_CNTL_CE);
	nflash_cntl_deassert(NFLASH_CNTL_ALE);
}
/* Calculates the ECC for the entire 512 bytes
 * INPUT	: buf - array of 512 bytes
 *            eccbuf - to store the ECC
 * OUPUT	: eccbuf contains the ECC
 */
void
calculate_page_ecc(U8 *buf, U8 *eccbuf)
{
	U8 ecc1, ecc2, ecc3;
	int i;
	
	/* Calculate the ecc for 4 x 128 bytes of the Page */
	for(i=0; i < 4; i++) {
		calculate_ecc((buf + i * 128), &ecc1, &ecc2, &ecc3);
		eccbuf[ i * 4 + 0] = 0xFF;
		eccbuf[ i * 4 + 1] = ecc1;
		eccbuf[ i * 4 + 2] = ecc2;
		eccbuf[ i * 4 + 3] = ecc3;
	}
}

/* Corrects 4bits/ detects 8 bits of error in 512 bytes of
 * buf
 * INPUT  : buf - array of 512 bytes
 *        : eccbuf - which contains the ECC read from the Flash
 * OUTPUT : buf - has the corrcted value
 *          eccbuf - has the corrected ECC
 * RETURN: OK/CTS_FLASH_DEVICE_READ/FF_ERROR_NFLASH_READ_CORRECTED/
 *         FF_ERROR_ECC_CORRECTED/FF_ERROR_NFLASH_READ_UNCORRECTABLE
 */          
STATUS
correct_page_data(U8 *buf, U8 *eccbuf)
{
	U8 ecc1, ecc2, ecc3;
	int i;
	
	/* Calculate the ecc for 4 x 128 bytes of the Page */
	for(i=0; i < 4; i++) {
		calculate_ecc((buf + i * 128), &ecc1, &ecc2, &ecc3);
		correct_data((buf + i * 128), ecc1, ecc2, ecc3, (eccbuf + i * 4)); 

	}
}
#endif	/* CONFIG_E2 */


/* Support for ECC */

#define	S128 		1

#ifdef	S128
#define	SEC_SIZE	128
#define	CORRECTABLE 0x00155554L
#else	/* S128 */
#define	SEC_SIZE	256
#define	CORRECTABLE 0x00555554L
#endif	/* S128 */

#define    BIT7        0x80
#define    BIT6        0x40
#define    BIT2        0x04
#define    BIT0        0x01
#define    BIT1BIT0    0x03
#define    BIT23       0x00800000L
#define    MASK_CPS    0x3f


unsigned char ecctable[] = {
0x00,0x55,0x56,0x03,0x59,0x0C,0x0F,0x5A,0x5A,0x0F,0x0C,0x59,0x03,0x56,0x55,0x00,
0x65,0x30,0x33,0x66,0x3C,0x69,0x6A,0x3F,0x3F,0x6A,0x69,0x3C,0x66,0x33,0x30,0x65,
0x66,0x33,0x30,0x65,0x3F,0x6A,0x69,0x3C,0x3C,0x69,0x6A,0x3F,0x65,0x30,0x33,0x66,
0x03,0x56,0x55,0x00,0x5A,0x0F,0x0C,0x59,0x59,0x0C,0x0F,0x5A,0x00,0x55,0x56,0x03,
0x69,0x3C,0x3F,0x6A,0x30,0x65,0x66,0x33,0x33,0x66,0x65,0x30,0x6A,0x3F,0x3C,0x69,
0x0C,0x59,0x5A,0x0F,0x55,0x00,0x03,0x56,0x56,0x03,0x00,0x55,0x0F,0x5A,0x59,0x0C,
0x0F,0x5A,0x59,0x0C,0x56,0x03,0x00,0x55,0x55,0x00,0x03,0x56,0x0C,0x59,0x5A,0x0F,
0x6A,0x3F,0x3C,0x69,0x33,0x66,0x65,0x30,0x30,0x65,0x66,0x33,0x69,0x3C,0x3F,0x6A,
0x6A,0x3F,0x3C,0x69,0x33,0x66,0x65,0x30,0x30,0x65,0x66,0x33,0x69,0x3C,0x3F,0x6A,
0x0F,0x5A,0x59,0x0C,0x56,0x03,0x00,0x55,0x55,0x00,0x03,0x56,0x0C,0x59,0x5A,0x0F,
0x0C,0x59,0x5A,0x0F,0x55,0x00,0x03,0x56,0x56,0x03,0x00,0x55,0x0F,0x5A,0x59,0x0C,
0x69,0x3C,0x3F,0x6A,0x30,0x65,0x66,0x33,0x33,0x66,0x65,0x30,0x6A,0x3F,0x3C,0x69,
0x03,0x56,0x55,0x00,0x5A,0x0F,0x0C,0x59,0x59,0x0C,0x0F,0x5A,0x00,0x55,0x56,0x03,
0x66,0x33,0x30,0x65,0x3F,0x6A,0x69,0x3C,0x3C,0x69,0x6A,0x3F,0x65,0x30,0x33,0x66,
0x65,0x30,0x33,0x66,0x3C,0x69,0x6A,0x3F,0x3F,0x6A,0x69,0x3C,0x66,0x33,0x30,0x65,
0x00,0x55,0x56,0x03,0x59,0x0C,0x0F,0x5A,0x5A,0x0F,0x0C,0x59,0x03,0x56,0x55,0x00
};



/*
    Transfer result
    LP14,12,10,... & LP15,13,11,... -> LP15,14,13,... & LP7,6,5,..
*/
void
trans_result(reg2,reg3,ecc1,ecc2)
unsigned char reg2;                           /* LP14,LP12,LP10,...           */
unsigned char reg3;                           /* LP15,LP13,LP11,...           */
unsigned char *ecc1;                          /* LP15,LP14,LP13,...           */
unsigned char *ecc2;                          /* LP07,LP06,LP05,...           */
{
    unsigned char a;                          /* Working for reg2,reg3        */
    unsigned char b;                          /* Working for ecc1,ecc2        */
    unsigned char i;                          /* For counting                 */

    a=BIT7; b=BIT7;
    *ecc1=*ecc2=0;                            /* Clear ecc1,ecc2              */
    for(i=0; i<4; ++i) {
        if ((reg3&a)!=0) *ecc1|=b;            /* LP15,13,11,9 -> ecc1         */
        b>>=1;                                /* Right shift                  */
        if ((reg2&a)!=0) *ecc1|=b;            /* LP14,12,10,8 -> ecc1         */
        b>>=1;                                /* Right shift                  */
        a>>=1;                                /* Right shift                  */
    }
    b=BIT7;
    for(i=0; i<4; ++i) {
        if ((reg3&a)!=0) *ecc2|=b;            /* LP7,5,3,1 -> ecc2            */
        b>>=1;                                /* Right shift                  */
        if ((reg2&a)!=0) *ecc2|=b;            /* LP6,4,2,0 -> ecc2            */
        b>>=1;                                /* Right shift                  */
        a>>=1;                                /* Right shift                  */
    }
}

/*
    Calculating ECC
    dat[0-255] -> ecc1,ecc2,ecc3 using CP0-CP5 code ecctable[0-255]
*/
void
calculate_ecc(U8 *dbuf, U8 *ecc1, U8 *ecc2, U8 *ecc3)
{
    U32	i;                          /* For counting                 */
    U8	a;                         /* Working for ecctable          */
    U8	reg1;                      /* D-all,CP5,CP4,CP3,...         */
    U8	reg2;                      /* LP14,LP12,L10,...             */
    U8	reg3;                      /* LP15,LP13,L11,...             */

    reg1=reg2=reg3=0;                        /* Clear parameter               */

    for(i=0; i<SEC_SIZE; ++i) {
        a=ecctable[dbuf[i]];                 /* Get CP0-CP5 code from ecctable*/
        reg1^=(a&0x3f);                      /* XOR with a                    */
        if ((a&BIT6)!=0) {                   /* If D_all(all bit XOR) = 1     */
            reg3^=(U8)i;          /* XOR with counter              */
            reg2^=~((U8)i);       /* XOR with inv. of counter      */
        }
    }

#ifdef	S128
	/* To make LP14 and LP15 = 1, Make BIT7 of reg2 and reg3 0 */
	reg2 &= ~BIT7;
	reg3 &= ~BIT7;
#endif
    /* Trans LP14,12,10,... & LP15,13,11,... -> LP15,14,13,... & LP7,6,5,..   */
    trans_result(reg2,reg3,ecc1,ecc2);

    *ecc1=~(*ecc1); *ecc2=~(*ecc2);          /* Inv. ecc2 & ecc3              */
    *ecc3=((~reg1)<<2)|BIT1BIT0;             /* Make TEL format               */
}


STATUS
correct_data(U8 *dbuf, U8 ecc1, U8 ecc2, U8 ecc3, U8 *eccbuf)
{
    U32	l;                          /* Working to check d           */
    U32	d;                          /* Result of comparison         */
    U32	i;                           /* For counting                 */
    U8	d1,d2,d3;                   /* Result of comparison         */
    U8	a;                          /* Working for add              */
    U8	add;                        /* Byte address of cor. DATA    */
    U8	b;                          /* Working for bit              */
    U8	bit;                        /* Bit address of cor. DATA     */

    d1=ecc1^eccbuf[1]; d2=ecc2^eccbuf[2];     /* Compare LP's  */
    d3=ecc3^eccbuf[3];                		  /* Comapre CP's                 */
    d=((U32)d1<<16)                 /* Result of comparison         */
        +((U32)d2<<8)
        +(U32)d3;

    if (d==0) return(OK);                     /* If No error, return          */
    if (((d^(d>>1))&CORRECTABLE)==CORRECTABLE) {    /* If correctable         */
        l=BIT23;
        add=0;                                /* Clear parameter              */
        a=BIT7;
        for(i=0; i<8; ++i) {                  /* Checking 8 bit               */
            if ((d&l)!=0) add|=a;             /* Make byte address from LP's  */
            l>>=2; a>>=1;                     /* Right Shift                  */
        }
        bit=0;                                /* Clear parameter              */
        b=BIT2;
        for(i=0; i<3; ++i) {                  /* Checking 3 bit               */
            if ((d&l)!=0) bit|=b;             /* Make bit address from CP's   */
            l>>=2; b>>=1;                     /* Right shift                  */
        }
        b=BIT0;
        dbuf[add]^=(b<<bit);                  /* Put corrected data           */
        return(CTS_FLASH_ECC_CORRECTED);
    }
    i=0;                                      /* Clear count                  */
    d&=0x00ffffffL;                           /* Masking                      */
    while(d) {                                /* If d=0 finish counting       */
        if (d&BIT0) ++i;                      /* Count number of 1 bit        */
        d>>=1;                                /* Right shift                  */
    }
    if (i==1) {                               /* If ECC error                 */
        eccbuf[1]=ecc1; eccbuf[2]=ecc2;       /* Put right ECC code*/
        eccbuf[3]=ecc3;
        return(CTS_FLASH_ECC_CORRECTED);
    }
    return(CTS_FLASH_ECC);                     /* Uncorrectable error        */ 
}


/* IGNORE THE REST OF THE STUFF, USED ONLY FOR DEBUGGING
   ----------------------------------------------------
 */

#ifndef	CONFIG_E2
void
nflash_get_id()
{
	U16 cntl;
	int i= 0;
	U8 id;

	NFLASH_CNTL_WRITE(NFLASH_CNTL_OP |NFLASH_CNTL_CE | 
					NFLASH_CNTL_WE | NFLASH_CNTL_RE);
	

	/* CMD Input */
	cntl = NFLASH_CNTL_READ;
	cntl |= NFLASH_CNTL_CLE ;
	cntl &= ~NFLASH_CNTL_RW;
	NFLASH_CNTL_WRITE(cntl);
	cntl &= ~NFLASH_CNTL_CE;
	NFLASH_CNTL_WRITE(cntl);
	NFLASH_DATA_WRITE(NFLASH_CMD_IDREAD);
	cntl &= ~NFLASH_CNTL_WE;
	NFLASH_CNTL_WRITE(cntl);
	i++; i++; i++; if ( i) i++;
	cntl |= NFLASH_CNTL_WE;
	NFLASH_CNTL_WRITE(cntl);
	cntl |= NFLASH_CNTL_CE;
	NFLASH_CNTL_WRITE(cntl);
	cntl &= ~NFLASH_CNTL_CLE;
	NFLASH_CNTL_WRITE(cntl);
	
	/* Address Input */
	cntl |= NFLASH_CNTL_ALE;
	NFLASH_CNTL_WRITE(cntl);
	cntl &= ~NFLASH_CNTL_CE;
	NFLASH_CNTL_WRITE(cntl);
	NFLASH_DATA_WRITE(0);
	cntl &= ~NFLASH_CNTL_WE;
	NFLASH_CNTL_WRITE(cntl);
	i++; i++; i++; if ( i) i++;
	cntl |= NFLASH_CNTL_WE;
	NFLASH_CNTL_WRITE(cntl);
	cntl &= ~NFLASH_CNTL_ALE;
	NFLASH_CNTL_WRITE(cntl);

	/* Read Maker */
	cntl |= NFLASH_CNTL_RW;
	cntl &= ~NFLASH_CNTL_RE;
	NFLASH_CNTL_WRITE(cntl);
	i++; i++; i++; if ( i) i++;
	id = NFLASH_DATA_READ;

	cntl |= NFLASH_CNTL_RE;
	NFLASH_CNTL_WRITE(cntl);
	
	printf("Maker %X\n\r", id);
	
	/* Read Device */
	cntl &= ~NFLASH_CNTL_RE;
	NFLASH_CNTL_WRITE(cntl);
	i++; i++; i++; if ( i) i++;
	id = NFLASH_DATA_READ;
	cntl |= NFLASH_CNTL_RE;
	NFLASH_CNTL_WRITE(cntl);
		
	printf("Dev %X\n\r", id);
		
}
STATUS
nflash_init_noecc()
{
	U16	val;

	/* Put the Flash into 512 bytes per page mode */
	val =  NFLASH_CNTL_READ;
	val |= NFLASH_CNTL_OP;
	NFLASH_CNTL_WRITE(val);
	if (nflash_reset() != OK )
		return(FF_ERROR_NFLASH_RESET);
	
	/* Put the Flash into write protect mode */
	nflash_cntl_assert(NFLASH_CNTL_WP);
	
	return(OK);
		
}


STATUS
nflash_read_noecc(U8 *buf, U32 offset, I32 len)
{
	U8 cmd;
	U32 i, counter;
	/* Find out whether we need Read1 or Read2 depending upon
	 * offset. If offset(A8=0) use Read1, if offset(A8=1) use Read2
	 */
	if ( offset & 0x100)
		cmd = NFLASH_CMD_READ2;
	else
		cmd = NFLASH_CMD_READ1;
	
	/* Issue the Command */
	nflash_send_cmd(cmd);
	nflash_send_addr(offset, 3);
	
	/* Read the Data */
	nflash_cntl_assert(NFLASH_CNTL_CE);
	i = 0;
	counter = 0;
	nflash_cntl_assert(NFLASH_CNTL_RW);
	while(len) {
		if (nflash_ready() == OK) {
			nflash_cntl_assert(NFLASH_CNTL_RE);
			buf[i] = NFLASH_DATA_READ;
			i++;
			len--;
			counter = 0;
			nflash_cntl_deassert(NFLASH_CNTL_RE);
		} else {
			counter++;
			if ( counter > NFLASH_TIMEOUT)
				break;
		}
	}
	nflash_cntl_deassert(NFLASH_CNTL_CE);
	if ( len)
		return(i);
	else
		return(OK);
}

STATUS
nflash_program_noecc(U8 *buf, U32 offset, I32 len)
{
	I32	partial_len;

	if ( offset & NFLASH_PAGE_MASK) {
		partial_len = NFLASH_PAGE_SIZE - (offset & NFLASH_PAGE_MASK);
		if (nflash_program_partial_page_noecc(buf, offset, partial_len) != OK)
			return(CTS_FLASH_DEVICE_WRITE);
		len    -= partial_len;
		buf    += partial_len;
		offset += partial_len;
	}
	
	while(len >= NFLASH_PAGE_SIZE) {
		if (nflash_program_page_noecc(buf, offset, NFLASH_PAGE_SIZE) != OK)
			return(CTS_FLASH_DEVICE_WRITE);
		len    -= NFLASH_PAGE_SIZE;
		buf    += NFLASH_PAGE_SIZE;
		offset += NFLASH_PAGE_SIZE;
	}
	
	if ( len ) {
		if (nflash_program_partial_page_noecc(buf, offset, len) != OK)
			return(CTS_FLASH_DEVICE_WRITE);
	}
	return(OK);
}

STATUS
nflash_program_page_noecc(U8 *buf, U32 offset, I32 len)
{
	I32	counter = 0;
	U8	status;

	nflash_cntl_deassert(NFLASH_CNTL_WP);
	nflash_send_cmd(NFLASH_CMD_DATAIN);
	nflash_send_addr(offset, 3);
	nflash_cntl_assert(NFLASH_CNTL_CE);
	nflash_cntl_deassert(NFLASH_CNTL_RW);
	while(len) {
		nflash_cntl_assert(NFLASH_CNTL_WE);
		NFLASH_DATA_WRITE(*buf);
		nflash_cntl_deassert(NFLASH_CNTL_WE);
		len--;
		buf++;
	}
	nflash_cntl_deassert(NFLASH_CNTL_CE);
	nflash_send_cmd(NFLASH_CMD_PROGRAM);


	while(nflash_ready() != OK ) {
		counter++;
		if ( counter > NFLASH_TIMEOUT)
			break;
	}
	if ( counter > NFLASH_TIMEOUT ) {
		/* Error in Programming */
		nflash_reset();
		nflash_cntl_assert(NFLASH_CNTL_WP);
		return(CTS_FLASH_DEVICE_WRITE);
	} else {
		status = nflash_get_status();
		if ( status & NFLASH_STATUS_FAIL) {
			nflash_cntl_assert(NFLASH_CNTL_WP);
			return(CTS_FLASH_DEVICE_WRITE);
		}
	}
	nflash_cntl_assert(NFLASH_CNTL_WP);
	return(OK);
}

STATUS
nflash_program_partial_page_noecc(U8 *buf, U32 offset, I32 len)
{
	U8	lbuf[NFLASH_PAGE_SIZE];
	I32	i;
	U32	loffset;
	
	for(i=0; i < NFLASH_PAGE_SIZE; i++)
		lbuf[i] = 0xFF;
	
	for(i=0; i < len; i++) {
		lbuf[offset & NFLASH_PAGE_MASK] = buf[i];
		offset++;
	}
	loffset = offset & ~NFLASH_PAGE_MASK;
	if (nflash_program_page_noecc(lbuf, loffset, NFLASH_PAGE_SIZE) != OK)
		return(CTS_FLASH_DEVICE_WRITE);
	else
		return(OK);

}


/*
    Writing corrected DATA file
*/
put_data(U8 *dbuf)
{
    int i,j;

#ifdef	S128
    for(i=0; i<8; ++i) {
#else
    for(i=0; i<16; ++i) {
#endif
        for(j=0; j<16; ++j)
            printf("%02X ",dbuf[i*16+j]);    /* Wrinting DATA          */
        printf("\n");
    }
														/* Writing ECC */
	printf("\n");
}
#endif
