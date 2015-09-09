/*
 *
 * MODULE: mii.h - header file for the MII Interface
 *
 * Copyright (C) 1998 by ConvergeNet Technologies, Inc.
 *                       2222 Trade Zone Blvd.
 *                       San Jose, CA  95131  U.S.A.
 *
 * HISTORY:
 * --------
 * 05/25/99	- Created by Sudhir
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

#ifndef		_MII_
#define		_MII_

/* Standard MII Registers */
#define		MII_CNTL_OFF			0
#define		MII_STAT_OFF			1
#define		MII_ID_1_OFF			2
#define		MII_ANEG_ADV_OFF		4
#define		MII_ANEG_PARTNER_OFF 	5
#define		MII_ANEG_EXP_OFF		6
/* Device Specific Registers */
#define		PHY_INTEN_OFF			17
#define		PHY_INTSTAT_OFF			18
#define		PHY_CONF_OFF			19
#define		PHY_STAT_OFF			20

/* MII Control register Bit definitions */
#define		MII_CNTL_RESET			0x8000
#define		MII_CNTL_LOOPBACK		0x4000
#define		MII_CNTL_100			0x2000
#define		MII_CNTL_ANEG			0x1000
#define		MII_CNTL_PWRDWN			0x0800
#define		MII_CNTL_ISOLATE		0x0400
#define		MII_CNTL_ANEG_START		0x0200
#define		MII_CNTL_FULLDUP		0x0100
#define		MII_CNTL_COLTEST		0x0080
#define		MII_CNTL_DEFAULT		(MII_CNTL_100 | MII_CNTL_ANEG | MII_CNTL_FULLDUP )

/* MII Status register Bit definitions */
#define		MII_STAT_ANEG_DONE		0x0020
#define		MII_STAT_RMT_FAULT		0x0010
#define		MII_STAT_LINK_UP		0x0004
#define		MII_STAT_JABBER			0x0002

/* MII AUTO Negotiation Advertisement register Bit definitions */
#define		MII_ADV_RMT_FAULT		0x2000
#define		MII_ADV_PAUSE			0x0400
#define		MII_ADV_100T4			0x0200
#define		MII_ADV_100TX_FULLDUP	0x0100
#define		MII_ADV_100TX			0x0080
#define		MII_ADV_10T_FULLDUP		0x0040
#define		MII_ADV_10T				0x0020
#define		MII_ADV_SELECTOR		0x0001
#define		MII_ADV_DEFAULT			( MII_ADV_100TX_FULLDUP | MII_ADV_100TX  | MII_ADV_10T_FULLDUP | MII_ADV_10T | MII_ADV_SELECTOR )

/* MII AUTO Negotiation Link Partner register Bit definitions */
#define		MII_PARTNER_RMT_FAULT	0x2000
#define		MII_PARTNER_PAUSE		0x0400
#define		MII_PARTNER_100T4		0x0200
#define		MII_PARTNER_100TX_FULLDUP	0x0100
#define		MII_PARTNER_100TX		0x0080
#define		MII_PARTNER_10T_FULLDUP	0x0040
#define		MII_PARTNER_10T			0x0020
#define		MII_PARTNER_SELECTOR	0x0001

/* MII AUTO Negotiation Link Partner Exapnsion register Bit definitions */
#define		MII_EXP_ANEGABLE		0x0001

/* PHY Interrupt Enable register Bit definitions */
#define		PHY_INTEN_MIIDRVLVL		0x0008
#define		PHY_INTEN_ENABLE		0x0002
#define		PHY_INTEN_TINT			0x0001

/* PHY Interrupt Status register Bit definitions */
#define		PHY_INTSTAT_MINT		0x8000
#define		PHY_INTSTAT_XTALOK		0x4000

/* PHY Configuration register Bit definitions */
#define		PHY_CONF_TXMIT_TEST		0x4000
#define		PHY_CONF_REPEATER		0x2000
#define		PHY_CONF_MDIO_INT		0x1000
#define		PHY_CONF_DEFAULT		0x0000

/* PHY Status register Bit definitions */
#define		PHY_STAT_LINKUP			0x2000
#define		PHY_STAT_FULLDUP		0x1000
#define		PHY_STAT_100			0x0800
#define		PHY_STAT_ANEG_DONE		0x0200
#define		PHY_STAT_LOW_VOLT		0x0004

#endif		/* _MII_ */
