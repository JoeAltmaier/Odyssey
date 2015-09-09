/*
 *
 * MODULE: mdio.c - MII Interface routines
 *
 * Copyright (C) 1998 by ConvergeNet Technologies, Inc.
 *                       2222 Trade Zone Blvd.
 *                       San Jose, CA  95131  U.S.A.
 *
 * HISTORY:
 * --------
 * 05/19/99	- Created by Sudhir
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
 *
 */
#include	"types.h"
#include	"pcidev.h"


/*
#define mdio_delay() { U32	tval; tval = *mdio_addr; }
*/
#define mdio_delay() { U32	tval; for(tval=0; tval < 1; tval++) ;}

#define MDIO_CLK		0x10000
#define MDIO_DATA_WRITE 0x20000
#define MDIO_DATA_READ	0x80000
#define MDIO_MODE_WRITE	0x00000
#define MDIO_MODE_READ	0x40000

#define	MDIO_READ_CMD	0xF6
#define	MDIO_WRITE_CMD	0x5002

/* Reads the MII register of the PHY
 * INPUT:	io_addr 	: The physical address of the MAC MII Register
 * 		    phy_address : The 5-bit PHY address
 * 		    reg			: Register offset  inside the PHY
 * RETURNS : 16 - bit contents of the PHY Register
 */
U32 mdio_read(U32 io_addr, U32 phy_addr, U32 reg)
{
	int i;
	U32 read_cmd = (MDIO_READ_CMD << 10) | (phy_addr << 5) | reg;
	U32 retval = 0;
	U32	*mdio_addr;
	U32	val;
#ifdef	INCLUDE_ODYSSEY
	mdio_addr = (U32 *)(io_addr - 0x80000000);
#else
	mdio_addr = (U32 *)(io_addr + 0xA0000000);
#endif	


	/* Send the Preamble  by sending 32 1's */
#if 0
	for (i = 32; i >= 0; i--) {
#else
	for (i = 29; i >= 0; i--) {
#endif
		*mdio_addr = DWSWAP(MDIO_MODE_WRITE | MDIO_DATA_WRITE);
		mdio_delay();
		*mdio_addr = DWSWAP(MDIO_MODE_WRITE | MDIO_DATA_WRITE | MDIO_CLK);
		mdio_delay();
	}
	/* Send SFD, OP Code, PHY Address, Reg address */
	for (i = 15; i >= 0; i--) {
		U32 dataval = (read_cmd & (1 << i)) ? MDIO_DATA_WRITE : 0;

		*mdio_addr = DWSWAP(MDIO_MODE_WRITE | dataval);
		mdio_delay();
		*mdio_addr = DWSWAP(MDIO_MODE_WRITE | dataval | MDIO_CLK);
		mdio_delay();
	}
	/* Read Turn around, 16 data bits and idle bits */
	for (i = 19; i > 0; i--) {
		*mdio_addr = DWSWAP(MDIO_MODE_READ);
		mdio_delay();
		val = *mdio_addr;
		val = DWSWAP(val);
		retval = (retval << 1) | ((val & MDIO_DATA_READ) ? 1 : 0);
		*mdio_addr = DWSWAP(MDIO_MODE_READ | MDIO_CLK);
		mdio_delay();
	}
	*mdio_addr = DWSWAP(MDIO_MODE_READ);
	return (retval>>1) & 0xffff;
}

/* Writes to the MII register of the PHY
 * INPUT:	io_addr 	: The physical address of the MAC MII Register
 * 		    phy_address : The 5-bit PHY address
 * 		    reg			: Register offset  inside the PHY
 * 		    value		: 16 - bit value to be written to the register
 */
void mdio_write(U32 io_addr, U32 phy_addr, U32 reg, U32 value)
{
	int i;
	U32 cmd = (MDIO_WRITE_CMD << 16) | (phy_addr << 23) | (reg<<18) | value;
	U32	*mdio_addr;
#ifdef	INCLUDE_ODYSSEY
	mdio_addr = (U32 *)(io_addr - 0x80000000);
#else
	mdio_addr = (U32 *)(io_addr + 0xA0000000);
#endif	


	/* Send the Preamble  by sending 32 1's */
	for (i = 32; i >= 0; i--) {
		*mdio_addr = DWSWAP(MDIO_MODE_WRITE | MDIO_DATA_WRITE);
		mdio_delay();
		*mdio_addr = DWSWAP(MDIO_MODE_WRITE | MDIO_DATA_WRITE | MDIO_CLK);
		mdio_delay();
	}
	/* Send SFD, OP Code, PHY Address, Reg address  and value */
	for (i = 31; i >= 0; i--) {
		U32 dataval = (cmd & (1 << i)) ? MDIO_DATA_WRITE : 0;
		*mdio_addr = DWSWAP(MDIO_MODE_WRITE | dataval);
		mdio_delay();
		*mdio_addr = DWSWAP(MDIO_MODE_WRITE | dataval | MDIO_CLK);
		mdio_delay();
	}
	/* Clear extra bits. */
	for (i = 2; i > 0; i--) {
		*mdio_addr = DWSWAP(MDIO_MODE_READ);
		mdio_delay();
		*mdio_addr = DWSWAP(MDIO_MODE_READ | MDIO_CLK);
		mdio_delay();
	}
	return;
}

