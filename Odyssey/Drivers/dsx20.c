/*
 *
 * MODULE: dsx20.c - driver for Temperature Sensor
 *
 * Copyright (C) 1998 by ConvergeNet Technologies, Inc.
 *                       2222 Trade Zone Blvd.
 *                       San Jose, CA  95131  U.S.A.
 *
 * HISTORY:
 * --------
 * 09/18/98	- Created by Sudhir
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

#include "nucleus.h"
#include "dsx20.h"

NU_HISR		DSX_HISR;
void		(*old_lisr_dsx)(int);
int 		dsx_addintr(void (*)());
void		dsx_lih(int );
unsigned char *dsx_hisr_stack = (unsigned char*)0;

extern NU_MEMORY_POOL   System_Memory;

void
dsx_write(int num_bits, int val)
{
	int i;
	
	for(i=0; i < num_bits; i++) {
		*DSX_DQ_CLK = 1;
		*DSX_DQ_REG = val & 0x01;
		val = val >> 1;
		delay_us(1);
		*DSX_DQ_CLK = 0;
		delay_us(1);
	}
}

unsigned int
dsx_read(int num_bits)
{
	int i;
	unsigned int val = 0;
	
	for(i=0; i < num_bits; i++) {
		*DSX_DQ_CLK = 0;
		delay_us(1);
		*DSX_DQ_CLK = 1;
		val = val << 1;
		val = val | (*DSX_DQ_REG & 0x1 );
		delay_us(1);
	}
	return(val);
}

void
dsx_reset()
{
	*DSX_DQ_RST = 0x00;
	delay_us(1);
	*DSX_DQ_RST = 0x01;
	delay_us(1);
		
}

int
dsx_init(int temp_hi, int temp_low)
{
	int temp_hi_data = temp_hi * 2;
	int temp_low_data = temp_low * 2;
	
	if ( (temp_hi_data) < -55 || (temp_hi_data > 125) )
		return(DSX_FAIL);
	if ( (temp_low_data) < -55 || (temp_low_data > 125) )
		return(DSX_FAIL);
	
	/* Register the Interrupt Handler */
	dsx_addintr(dsxisr);
	
	/* Write to Config Register */
	dsx_reset();
	dsx_write(8, DSX_CMD_WRITECONFIG);
	dsx_write(8, 0);
	
	/* Write The Temp Hi Threshold */
	dsx_reset();
	dsx_write(8, DSX_CMD_WRITETH);
	dsx_write(9, temp_hi_data);

	/* Write The Temp Low Threshold */
	dsx_reset();
	dsx_write(8, DSX_CMD_WRITETL);
	dsx_write(9, temp_low_data * 2);

	/* Read The Temp Hi Threshold */
	dsx_reset();
	dsx_write(8, DSX_CMD_READTH);
	if ( dsx_read(9) != temp_hi_data ) 
		return(DSX_FAIL);

	/* Read The Temp Low Threshold */
	dsx_reset();
	dsx_write(8, DSX_CMD_READTL);
	if ( dsx_read(9) != temp_low_data ) 
		return(DSX_FAIL);
	
	/* Start Conversion */
	dsx_reset();
	dsx_write(8, DSX_CMD_START);
	return(DSX_SUCCESS);
		
}


void
dsxisr()
{
	unsigned char status;
	/* Read the Config Register */
	dsx_reset();
	dsx_write(8, DSX_CMD_READCONFIG);
	status = (unsigned char)dsx_read(8);
	if ( status & DSX_CONFIG_THF ) {
		/* Trap The User for High Temperature */
			
	}

	if ( status & DSX_CONFIG_TLF ) {
		/* Trap The User for Low Temperature */
			
	}

	/* Write to Config Register */
	dsx_reset();
	dsx_write(8, DSX_CMD_WRITECONFIG);
	dsx_write(8, 0);
	dsx_reset();
}


int
dsx_addintr(void (*pintr)())
{
	STATUS  status;

	if( dsx_hisr_stack )
		return(DSX_SUCCESS);
	
	status = NU_Register_LISR(DSX_INT_VECTOR, dsx_lih, &old_lisr_dsx);
	if ( status != NU_SUCCESS ) {
		goto dsx_addintr_abort;
	}
	
	status = NU_Allocate_Memory(&System_Memory,(void **)&(dsx_hisr_stack),
				   DSX_HISR_STACK_SIZE, NU_NO_SUSPEND);	
	if ( status != NU_SUCCESS ) {
		goto dsx_addintr_abort;
	}
		
	status = NU_Create_HISR(&DSX_HISR, "dsx_hisr", pintr, 2,
							dsx_hisr_stack, DSX_HISR_STACK_SIZE);
	if ( status != NU_SUCCESS ) {
		goto dsx_addintr_abort;
	}
	return(DSX_SUCCESS);
	
dsx_addintr_abort:
	
	if ( dsx_hisr_stack)
		NU_Deallocate_Memory(dsx_hisr_stack);
	return(DSX_FAIL);

}

void
dsx_lih(int vector)
{
	int i = vector;     /* To get rid of the warning */
	NU_Activate_HISR(&DSX_HISR);
}
