/*************************************************************************
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * File: cmbdrv.h
 * 
 * Description:
 * This file contains definitions for CMB 
 * 
 * Update Log:
 * 07/10/99 Raghava Kondapalli: Created 
 ************************************************************************/


#ifndef		CMBDRV_H
#define		CMBDRV_H

#define STATE_UNKNOWN		0
#define STATE_NOT_FOUND		1
#define STATE_POWERED_OFF	2
#define STATE_POWERED_ON	3
#define STATE_UNKNOWN_BOOT	4
#define STATE_RUNNING_DIAGS	5
#define STATE_AWAITING_DIAG	6
#define STATE_BOOTING_PCI	7
#define STATE_BOOTING_IMAGE	7
#define STATE_RUNNING_OS	8

#define CMD_STATUS_POLL			0x03
#define CMD_GET_CMD				0x04
#define CMD_GET_SLOT_STATUS		0x05
#define CMD_GET_BOARD_INFO		0x08
#define CMD_SET_PCI_WINDOW		0x09
#define CMD_SET_BOOT_PARAMS		0x0A
#define CMD_SET_MIPS_STATE		0x0B
#define CMD_AUTO_POLL			0x0C
#define CMD_POWER_ON			0x10
#define CMD_PCI_ACCESS			0x11
#define CMD_RESET_CPU			0x12
#define CMD_NMI_CPU				0x13
#define CMD_SPI_RESET			0x14
#define CMD_CHANGE_MASTER		0x15
#define CMD_GET_LAST_VALUE		0x20
#define CMD_READ_PARAM			0x21
#define CMD_FAN_SPEED			0x22
#define CMD_TEMP_THRESHOLD		0x23
#define CMD_SERIAL_NO			0x24
#define CMD_DAC_VALUE			0x25
#define CMD_PS_ENABLE			0x26
#define CMD_AUX_ENABLE			0x27
#define CMD_CHANGE_EVC_MASTER	0x28
#define CMD_DDH_STATUS			0x40
#define CMD_DRIVE_LOCK			0x41
#define CMD_PORT_BYPASS 		0x42

#define PARAM_PS_STATUS		1
#define PARAM_PS_VOLTAGE	2
#define PARAM_AUX_OUT_C		4
#define PARAM_AUX_OUT_T		5
#define PARAM_AUX_OUT_V		6
#define PARAM_AUX_FLAGS		7
#define PARAM_BAT_FUSE_V	8
#define PARAM_BAT_FUSE_T	9
#define PARAM_FAN_SPEED		0x0B
#define PARAM_KEY_POS		0x0C
#define PARAM_EVC_MASTER	0x0D
#define PARAM_TEMP			0x10
#define PARAM_B_TYPE		0x11
#define PARAM_HOST_SERIAL	0x12
#define PARAM_B_SERIAL		0x13
#define PARAM_B_HW_REV		0x14
#define PARAM_B_STR			0x15
#define PARAM_B_AVR_REV		0x16
#define PARAM_B_HW_PARAMS	0x17

#define DST			0
#define SRC			1
#define CMD			2
#define _STATUS		3
#define COUNT		4
#define CRC0		5
#define DATA0		6
#define DATA1		7
#define DATA2		8
#define DATA3		9
#define DATA4		10	
#define DATA5		11	
#define DATA6		12	
#define DATA7		13	
#define DATA8		14	
#define DATA9		15	
#define DATA10		16	
#define DATA11		16	
#define DATA12		16	
#define DATA13		16	

#define ACK			0x40
#define NAK			2
#pragma pack(1)

typedef struct {
	U8	dst;
	U8	src;
	U8	cmd;
	U8	status;
	U8	count;
	U8	crc0;
} cmb_msghdr_t;

typedef struct {
	U8	slot_no;
	U8	btype;
	U8	flags;
} cmb_binfo_t;

typedef struct {
	U32	winsize;
	U32	pcibaseaddr;
	U32	slavebaseaddr;
} cmb_pciwin_t;

typedef struct {
	U8	boot_type;
	U32	image_offset;
	U32	params_offset;
} cmb_bootparams_t;

typedef struct {
	U8	slot;
	U8	status;
} cmb_slotstatus_t;

typedef struct {
	U8	state;
	U8	crc1;
} cmb_mipsstate_t;

typedef struct {
	cmb_msghdr_t	hdr;
	U8		data[26];
} cmb_msg_t;

#pragma pack(0)

#ifdef CONFIG_E1

/*
 * Base Addresses
 */
#define	CMB_DATA_REG_ADDR	((unsigned char * )0xBC0C0000)
#define	CMB_INT_REG_ADDR	((unsigned char * )0xBF000003)
#define	CMB_STATUS_REG_ADDR	((unsigned char * )0xBF000004)

/* define interrupt bits */
#define	CMB_NMI_BIT			0x08

/* define status bits */
#ifdef CONFIG_HBC
#define	CMB_DATA_RDY_BIT	0x0000000100000000
#define	CMB_RDY_FLAG_BIT	0x0000000200000000
#define	CMB_INT_BIT			0x0000000004000000
#else
#define	CMB_INT_BIT			0x04
#define	CMB_DATA_RDY_BIT	0x40
#define	CMB_RDY_FLAG_BIT	0x80
#endif

/*
 * Base Addresses
 */
#define	CMB_DATA_REG_ADDR	((unsigned char * )0xBC0C0000)
#define	CMB_INT_REG_ADDR	((unsigned char * )0xBF000003)
#define	CMB_STATUS_REG_ADDR	((unsigned char * )0xBF000004)

/* define interrupt bits */
#define	CMB_NMI_BIT			0x08

#define	CMB_INT_BIT			0x04
#define	CMB_DATA_RDY_BIT	0x40
#define	CMB_RDY_FLAG_BIT	0x80
#define FIRST_BYTE_BIT		0x20	
#else /* CONFIG_E1 */
#define FIRST_BYTE_BIT		0x20	
#define CMB_WRITE_INT_MASK	0x1000
#define CMB_READ_INT_MASK	0x2000
#define	CMB_DATA_REG_ADDR	((unsigned char * )0xBC0C0000)
#define	CMB_CLR_REG_ADDR	((unsigned char * )0xBC0B0000)
#endif

#define	SLOT_EVC0			0x20
#define	SLOT_EVC1			0x21
#define	SLOT_DDH0			0x22
#define	SLOT_DDH1			0x23
#define	SLOT_DDH2			0x24
#define	SLOT_DDH3			0x25

STATUS	cmb_send_msg(U8 *msgbuf, U32 n, U32 timeout);
STATUS	cmb_read_msg(U8 *msgbuf, U32 *n, U32 timeout);

#endif		/* CMBDRV */
