#include "stdio.h"
#include "types.h"
#include "cmbdrv.h"
#include "IopTypes.h"
#include "bootblock.h"

extern  bootblock_t bootblock;
U32 __debug = 0;

extern void bcopy(unsigned char*, unsigned char*, int);

typedef struct {
	U32		temp;
	U32		rs;
	U32		tv;
	U32		fv;
	U32		fcv;
	U32		fmax;
} btable_t;

btable_t btable[15] = {
	{-5,	42340,	1982,	59510,	63050, 50230},
	{00,	32660,	1876,	58700,	62180, 51060},
	{05,	25400,	1758,	57880,	61310, 50230},
	{10,	19900,	1631,	57060,	60440, 52720},
	{15,	15710,	1497,	56240,	59580, 53540},
	{20,	12490,	1361,	55420,	58710, 54370},
	{25,	10000,	1225,	54600,	57840, 55200},
	{30,	 8058,	1093,	53780,	56970, 00000},
	{35,	06352,	 968,	52960,	56100, 00000},
	{40,	05326,	 851,	52140,	55240, 56430},
	{45,	 4368,	 745,	51320,	54370, 57240},
	{50,	03602,	 649,	50510,	53500, 58050},
	{55,	 2986,	 563,	49690,	52630, 58860},
	{00,	00000,	0000,	00000,	00000, 00000}
};
typedef struct {
	U32		found;
	U8		state;
	U32		type;
	U32		block;
} slot_info_t;

slot_info_t	slot_infot[32];
typedef struct {
	U8		*cmd;
	U32		len;
	I32		type;
} cmb_cmd_t;

#define ALL		0xFF
#define EVC0	0x20
#define EVC1	0x21
#define DDH0	0x30
#define DDH1	0x31
#define DDH2	0x32
#define DDH3	0x33

#define SENT_BOOT	1
#define BOOTED		2
U8	sinfo_cmd[10] = {0x7D, 0, CMD_GET_SLOT_STATUS, 0x80, 1, 0, 0, 0};
U8	btype_cmd[10] = {0, 0, CMD_GET_LAST_VALUE, 0x80, 1, 0,
									PARAM_B_TYPE, 0};
U8	pwr_on_cmd[10] = {0, 0, CMD_POWER_ON, 0x80, 1, 0, 1, 0};
U8	pci_win_cmd[20] = {0x80, 0, CMD_SET_PCI_WINDOW, 0x80, 0x0C, 0,
		   							0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
U8	boot_param_cmd[20] = {0x80, 0, CMD_SET_BOOT_PARAMS, 0x80, 0x09, 0,
		   							0, 0, 0, 0, 0, 0, 0, 0, 0};
U8	pci_enabl_cmd[10] = {0x0, 0, CMD_PCI_ACCESS, 0x80, 0x01, 0, 0x01};
U8	fan_speed_cmd[10] = {0x0, 0, CMD_FAN_SPEED, 0x80, 2, 0, 0, 0, 0};
U8	dac_val_cmd[10] = {0x0, 0, CMD_DAC_VALUE, 0x80, 1, 0, 0, 0};
U8	v48_cmd[10] = {0, 0, CMD_GET_LAST_VALUE, 0x80, 1, 0, PARAM_PS_VOLTAGE, 0};
U8	temp_th_cmd[10] = {0x0, 0, CMD_TEMP_THRESHOLD, 0x80, 2, 0, 0, 0, 0};
U8	ps_enable_cmd[10] = {0x0, 0, CMD_PS_ENABLE, 0x80, 1, 0, 0, 0};
U8	aux_enable_cmd[10] = {0x0, 0, CMD_AUX_ENABLE, 0x80, 1, 0, 0, 0};
U8	master_cmd[10] = {0x0, 0, CMD_CHANGE_EVC_MASTER, 0x80, 1, 0, 0, 0};
U8	btherm_cmd[10] = {0, 0, CMD_GET_LAST_VALUE, 0x80, 1, 0, PARAM_BAT_FUSE_T,0};
U8	tempr_cmd[10] = {0, 0, CMD_GET_LAST_VALUE, 0x80, 1, 0, PARAM_TEMP, 0};
U8	rev_cmd[10] = {0, 0, CMD_GET_LAST_VALUE, 0x80, 1, 0, PARAM_B_AVR_REV, 0};
U8	pstat_cmd[10] = {0, 0, CMD_GET_LAST_VALUE, 0x80, 1, 0, PARAM_PS_STATUS, 0};
U8	pvolt_cmd[10] = {0, 0, CMD_GET_LAST_VALUE, 0x80, 1, 0, PARAM_PS_VOLTAGE, 0};
U8	auxc_cmd[10] = {0, 0, CMD_GET_LAST_VALUE,	0x80, 1, 0, PARAM_AUX_OUT_C, 0};
U8	auxv_cmd[10] = {0, 0, CMD_GET_LAST_VALUE, 0x80, 1, 0, PARAM_AUX_OUT_V, 0};
U8	auxt_cmd[10] = {0, 0, CMD_GET_LAST_VALUE, 0x80, 1, 0, PARAM_AUX_OUT_T, 0};
U8	batv_cmd[10] = {0, 0, CMD_GET_LAST_VALUE, 0x80, 1, 0, PARAM_BAT_FUSE_V, 0};
U8	batt_cmd[10] = {0, 0, CMD_GET_LAST_VALUE, 0x80, 1, 0, PARAM_BAT_FUSE_T, 0};
U8	auxf_cmd[10] = {0, 0, CMD_GET_LAST_VALUE, 0x80, 1, 0, PARAM_AUX_FLAGS, 0};
U8	keyp_cmd[10] = {0, 0, CMD_GET_LAST_VALUE, 0x80, 1, 0, PARAM_KEY_POS, 0};
U8	fanspeed_cmd[10] = {0, 0, CMD_GET_LAST_VALUE, 0x80, 1,
	   						0, PARAM_FAN_SPEED, 0};
U8	bstr_cmd[10] = {0, 0, CMD_GET_LAST_VALUE, 0x80, 1, 0, PARAM_B_STR, 0};
U8	hserial_cmd[10] = {0, 0, CMD_GET_LAST_VALUE, 0x80, 1,
	   						0, PARAM_HOST_SERIAL, 0};
U8	bserial_cmd[10] = {0, 0, CMD_GET_LAST_VALUE, 0x80, 1, 0, PARAM_B_SERIAL, 0};
U8	hwrev_cmd[10] = {0, 0, CMD_GET_LAST_VALUE, 0x80, 1, 0, PARAM_B_HW_REV, 0};
U8	null_cmd[10] =	{0, 0, 0, 0, 0, 0, 0, 0};
U8	ddh_stat_cmd[10] = {0, 0, CMD_DDH_STATUS,  0x80, 0, 0, 0};
U8	drv_lock_cmd[10] = {0, 0, CMD_DRIVE_LOCK,  0x80, 2, 0, 0, 0};
U8	bypass_cmd[10] = {0, 0, CMD_PORT_BYPASS,  0x80, 2, 0, 0, 0};

cmb_cmd_t	cmb_cmds[] = {
	{sinfo_cmd,		8, ALL},
	{btype_cmd,		8, ALL},
	{rev_cmd,		8, ALL},
	{tempr_cmd,		8, ALL},
#if 0
	{bstr_cmd,		8, ALL},
	{hserial_cmd,	8, ALL},
	{bserial_cmd,	8, ALL},
	{hwrev_cmd,		8, ALL},
	{pstat_cmd,		8, IOPTY_EVC},
	{pvolt_cmd,		8, IOPTY_EVC},
	{auxc_cmd,		8, IOPTY_EVC},
	{auxt_cmd,		8, IOPTY_EVC},
	{auxv_cmd,		8, IOPTY_EVC},
	{batv_cmd,		8, IOPTY_EVC},
	{batt_cmd,		8, IOPTY_EVC},
	{auxf_cmd,		8, IOPTY_EVC},
	{keyp_cmd,		8, IOPTY_EVC},
	{fanspeed_cmd,	8, IOPTY_EVC},
#endif
#if 0
	{ddh_stat_cmd,	6, IOPTY_DDH},
#endif
	{null_cmd,		0, 0}
};
static I8	*slot_str[32] = {
	"B1  ", "B2  ", "B3  ", "B4  ",
	"D1  ", "D2  ", "D3  ", "D4  ",
	"A4  ", "A1  ", "A2  ", "A3  ",
	"C4  ", "C1  ", "C2  ", "C3  ",
	"EVC0", "EVC1",
	"DDH0", "DDH1", "DDH2", "DDH3",
	"HBC0", "HBC1"
};
static I8	*state_str[32] = {
	"Unknown",
	"Not Found",
	"Powered Off",
	"NON-IOP OK",
	"Awaiting Boot",
	"Running Diags",
	"Awaiting Diags Cmd",
	"Running OS Image",
	"--",
	"Image Corrupt",
	"Insufficient Memory"
};
static I8	*type_str[10] = {
	"--",
	"HBC",
	"NAC",
	"SSD",
	"NIC",
	"RAC",
	"EVC",
	"DDH"
};
static U32	slot_id[32] = {
	0x10, 0x11, 0x12, 0x13,
   	0x14, 0x15, 0x16, 0x17,
   	0x18, 0x19, 0x1A, 0x1B,
   	0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21,
	0x22, 0x23, 0x24, 0x25,
	0x00, 0x01
};
static U32 nslots = 24;
#define SLOT_TO_STR(id)     (slot_str[((id) <= 1) ? ((id) + 22) : ((id) - 16)])
#define STATE_TO_STR(id)    (state_str[id])
#define SLOT_TO_INDEX(s)    (((s) <= 1) ? ((s) + 22) : ((s) - 16))
#define PRINT_DRV_STATUS(str, val) \
{\
	printf("%s: ", str);\
	if (val & 0x01) {	\
		if (val & 0x02) \
			printf("Drive Failed. "); \
		else	\
			printf("Drive Ok. "); \
		if (val & 0x04) \
			printf("Loop A Bypassed"); \
		if (val & 0x08)	\
			printf("Loop B Bypassed. "); \
		if (val & 0x10)	\
			printf("Solenoid Enabled"); \
	} else  \
		printf("No Drive"); \
}
void	cmb_dump_msg(U8 *omsg, U8 *imsg, U32 len);
void	cmb_scan(void);

void
cmb_scan(void)
{
	U32		len;
	U8		omsg[32];
	U8		imsg[64];
	int		i;
	int		j;
	U32		cmd_to_slots[32];
	U32		n;
	U32		idx;
	char pStars[] = 
	"*********************************************************************";

	/* Clear Screeen */
	printf("\033[H\033[2J\033[0m");
	while (1) {
#ifndef	PRINT_ALL
		/* print the header */
		printf("\033[01;07H%s", pStars);
		printf("\033[02;15H System Temperature Monitor");
		printf("\033[03;07H%s", pStars);
		/* Set the Cursor Position */
		printf("\033[04;01H");
#endif

		for (i = 0; i < nslots; i++) {
			slot_infot[i].state = 0;
			slot_infot[i].type = 0;
			slot_infot[i].found = 0;
		}
#ifdef	PRINT_ALL
		for (i = 0; i < 78; i++)
			ttyA_out('-');
#endif
		printf("\n\r");
		for (i = 0; cmb_cmds[i].len != 0; i++) {
			n = 0;	
			switch(cmb_cmds[i].type) {
				case ALL:
					if (cmb_cmds[i].cmd[DST] == 0x7D) {
						for (j = 0; j < nslots; j++) {
							cmd_to_slots[n++] = slot_id[0];
						}
					} else {
						for (j = 0; j < nslots; j++) {
							if (slot_infot[j].found) {
								cmd_to_slots[n] = slot_id[j];
								n++;
							}
						}
					}
					break;
				case IOPTY_EVC:
					idx = SLOT_TO_INDEX(SLOT_EVC0);
					if (slot_infot[idx].found) {
						cmd_to_slots[n++] = SLOT_EVC0;
					}
					idx = SLOT_TO_INDEX(SLOT_EVC1);
					if (slot_infot[idx].found) {
						cmd_to_slots[n++] = SLOT_EVC1;
					}
					break;
				case IOPTY_DDH:
					for (j = 0; j < 4; j++) {
						idx = SLOT_TO_INDEX(SLOT_DDH0 + j);
						if (slot_infot[idx].found)
							cmd_to_slots[n++] = SLOT_DDH0 + j;
					}
					break;
				default:
					printf("Cmd to Unknown Slot %d\n\r", cmb_cmds[i].type);
					n = 0;
					break;
			}
			for (j = 0; j < n; j++) {
				bcopy(cmb_cmds[i].cmd, omsg, cmb_cmds[i].len);
				if (omsg[DST] == 0x7D)
					omsg[DATA0] = slot_id[j];
				else
					omsg[DST] = cmd_to_slots[j];
				omsg[SRC] = bootblock.b_slot;
				cmb_send_msg(omsg, cmb_cmds[i].len,1);
				cmb_read_msg(imsg, &len,1);
				cmb_dump_msg(omsg, imsg, len);
				if (kbhit())
					break;
			}
			if (kbhit())
				break;
		}
		if (kbhit())
			break;
	}	
#ifdef	PRINT_ALL
	ttyA_in();
#endif
}

void
cmb_dump_msg(U8 *omsg, U8 *imsg, U32 len)
{
	U32		idx;
	U32		x, y;
	U32		src;
	U32		i;
	U16		val;

	len = 0;
	if ((imsg[_STATUS] & ACK) == 0) {
		printf("Got NAK for Msg=%02x:%02x:%02x:%02x:%02x:%02x:-",
					  omsg[DST], omsg[SRC], omsg[CMD], omsg[_STATUS],
					  omsg[COUNT], omsg[CRC0]);
		for (i = 0; i < imsg[COUNT]; i++)
			  printf("%02x:", imsg[DATA0 + i] & 0xFF);	
		printf("\n\r");	
		return;
	}
	
	src = imsg[DATA0] & 0xFF;
	idx = SLOT_TO_INDEX(src);
	switch(omsg[CMD]) {
		case CMD_GET_SLOT_STATUS:
			if (imsg[COUNT] != 2) {
				printf("cmb_dump: Invalid Len %d for GET_SLOT_STATUS\n\r",
								imsg[COUNT]);
				return;
			}
			if (src <= 0x1F) {
				if (imsg[DATA1] >= 2) {
#ifdef	PRINT_ALL
					printf("Found IOP at %s State: \"%s\"\n\r",
							SLOT_TO_STR(imsg[DATA0]),
							STATE_TO_STR(imsg[DATA1]));
#endif
					slot_infot[idx].found = 1;
					src = imsg[DATA0];
				}
			} else {
				if (imsg[DATA1] >= 2) {
#ifdef	PRINT_ALL
					printf("Found %s\n\r", SLOT_TO_STR(src));
#endif
					slot_infot[idx].found = 1;
				}
			}
			break;
		case CMD_DDH_STATUS:
			printf("Status of Drives on %s: ", SLOT_TO_STR(imsg[SRC]));
			printf("\n\r\t");
			printf("Switch A %s. Switch B %s.",
							(imsg[DATA0] & 0x01) ? "OK" : "Failed",
							(imsg[DATA0] & 0x02) ? "OK" : "Failed");
			PRINT_DRV_STATUS("\n\r\tSlot0", imsg[DATA1]);
			PRINT_DRV_STATUS("\n\r\tSlot1", imsg[DATA2]);
			PRINT_DRV_STATUS("\n\r\tSlot2", imsg[DATA3]);
			PRINT_DRV_STATUS("\n\r\tSlot3", imsg[DATA4]);
			PRINT_DRV_STATUS("\n\r\tSlot4", imsg[DATA5]);
			printf("\n\r");
			break;	
		case CMD_GET_LAST_VALUE:
		case CMD_READ_PARAM:
			src = imsg[SRC] & 0xFF;
			idx = SLOT_TO_INDEX(src);
			switch(omsg[DATA0]) {
				case PARAM_B_TYPE:
					x = 1;
					y = (src == EVC0) ? 1 : 41;
					slot_infot[idx].type = imsg[SRC];
					break;
				case PARAM_PS_STATUS:
					x = 1;
					y = (src == EVC0) ? 1 : 41;
					printf("Power Supplies I/O/E on %s: %01x/%01x/%01x\n\r",
									SLOT_TO_STR(imsg[SRC]),
								   	imsg[DATA0] & 0xFF,
									imsg[DATA1] & 0xFF, imsg[DATA2] & 0xFF);
					break;
				case PARAM_TEMP:
					val = (imsg[DATA0] << 8) | imsg[DATA1];
					printf("Temperature on %s: %02d.%dC ~ %dF                            \n\r",
								   	SLOT_TO_STR(imsg[SRC]), (val / 2),
									((val & 0x01) ? 5 : 0), 
									((((val / 2) * 9) / 5) + 32));
					break;
				case PARAM_PS_VOLTAGE:
					val = (imsg[DATA0] << 8) | imsg[DATA1];
					printf("48V on %s: %dmv\n\r", SLOT_TO_STR(imsg[SRC]),
									val * 51);
					break;
				case PARAM_AUX_OUT_C:
					printf("Currents on %s ", SLOT_TO_STR(imsg[SRC]));
					val = (imsg[DATA0] << 8) | imsg[DATA1];
					printf("v33-%04d: ", val);
					val = (imsg[DATA2] << 8) | imsg[DATA3];
					printf("v5-%04d: ", val);
					val = (imsg[DATA4] << 8) | imsg[DATA5];
					printf("v12_A-%04d: ", val);
					val = (imsg[DATA6] << 8) | imsg[DATA7];
					printf("v12_B-%04d: ", val);
					val = (imsg[DATA8] << 8) | imsg[DATA9];
					printf("v12_C-%04d", val);
					printf("\n\r");
					break;
				case PARAM_AUX_OUT_T:
					printf("Temp's(C)   on %s ", SLOT_TO_STR(imsg[SRC]));
					val = (imsg[DATA0] << 8) | imsg[DATA1];
					printf("v33-%d: ", ((val * 200) - 273000) / 1000);
					val = (imsg[DATA2] << 8) | imsg[DATA3];
					printf("v5-%d: ", ((val * 200) - 273000) / 1000);
					val = (imsg[DATA4] << 8) | imsg[DATA5];
					printf("v12_A-%d: ", ((val * 200) - 273000) / 1000);
					val = (imsg[DATA6] << 8) | imsg[DATA7];
					printf("v12_B-%d: ", ((val * 200) - 273000) / 1000);
					val = (imsg[DATA8] << 8) | imsg[DATA9];
					printf("v12_C-%d", ((val * 200) - 273000) / 1000);
					printf("\n\r");
					break;
				case PARAM_AUX_OUT_V:
					printf("Voltages(mv) on %s ", SLOT_TO_STR(imsg[SRC]));
					val = (imsg[DATA0] << 8) | imsg[DATA1];
					printf("v33-%04d: ", (val * 2));
					val = (imsg[DATA2] << 8) | imsg[DATA3];
					printf("v5-%04d: ", (val * 3));
					val = (imsg[DATA4] << 8) | imsg[DATA5];
					printf("v12_A-%04d", (val * 11));
					printf("\n\r");
					break;
				case PARAM_BAT_FUSE_V:
					printf("Battery Fuse Voltages on %s: ",
									SLOT_TO_STR(imsg[SRC]));
					val = (imsg[DATA0] << 8) | imsg[DATA1];
					printf("Fuse_0-%04d: ", val);
					val = (imsg[DATA2] << 8) | imsg[DATA3];
					printf("Fuse_1-%04d\n\r", val);
					break;
				case PARAM_BAT_FUSE_T:
					printf("Battery  Temperatures on %s: ",
									SLOT_TO_STR(imsg[SRC]));
					val = (imsg[DATA0] << 8) | imsg[DATA1];
					printf("Therm_A-%04d mv: ", (val * 2450) / 256);
					val = (imsg[DATA2] << 8) | imsg[DATA3];
					printf("Therm_B-%04d mv\n\r", (val * 2450) / 256);
					break;
				case PARAM_KEY_POS:
					printf("Key Position on %s: %02x\n\r",
									SLOT_TO_STR(imsg[SRC]), imsg[DATA0] & 0xFF);
					break;
				case PARAM_AUX_FLAGS:
					printf("Aux Enabled on %s: %02x\n\r",
									SLOT_TO_STR(imsg[SRC]), imsg[DATA0] & 0xFF);
					break;
				case PARAM_B_AVR_REV:
#ifdef	PRINT_ALL
					printf("AVR Rev of %s: %d.%02d\n\r",
									SLOT_TO_STR(imsg[SRC]), imsg[DATA0] & 0xFF,
									imsg[DATA1] & 0xFF);
#endif
					break;
				case PARAM_FAN_SPEED:
					printf("Fan Speeds's on %s: ", SLOT_TO_STR(imsg[SRC]));
					val = (imsg[DATA0] << 8) | imsg[DATA1];
					printf("Fan0-%04d: ", val);
					val = (imsg[DATA2] << 8) | imsg[DATA3];
					printf("Fan1-%04d: ", val);
					val = (imsg[DATA4] << 8) | imsg[DATA5];
					printf("Fan2-%04d: ", val);
					val = (imsg[DATA6] << 8) | imsg[DATA7];
					printf("Fan3-%04d\n\r", val);
					break;
				case PARAM_HOST_SERIAL:
					printf("Serial No. of Host on %s: ",
									SLOT_TO_STR(imsg[SRC]));
					for (i = DATA0; i < (DATA0 + 16); i++)
						printf("%c", imsg[i] & 0xFF);
					printf("\n\r");
					break;
				case PARAM_B_SERIAL:
					printf("Serial No. of Board on %s: ",
							SLOT_TO_STR(imsg[SRC]));
					for (i = DATA0; i < (DATA0 + 16); i++)
						printf("%c", imsg[i] & 0xFF);
					printf("\n\r");
					break;
				case PARAM_B_HW_REV:
					printf("%s Part No.: ", SLOT_TO_STR(imsg[SRC]));
					for (i = DATA0; i < (DATA0 + 9); i++)
						printf("%c", imsg[i] & 0xFF);
					printf(",  H/W Rev: ", SLOT_TO_STR(imsg[SRC]));
					printf("%d-%d,  ",
							imsg[22] & 0xFF,
							imsg[23] & 0xFF);
					printf("Build Date: %02x%02x%02x%02x\n\r",
							imsg[24] & 0xFF,
							imsg[25] & 0xFF,
							imsg[26] & 0xFF,
							imsg[27] & 0xFF);
					break;
				case PARAM_B_STR:
					printf("Board Manufacturer of %s: ", 
									SLOT_TO_STR(imsg[SRC]));
					for (i = DATA0; i < (DATA0 + 24); i++)
						printf("%c", imsg[i] & 0xFF);
					printf("\n\r");
					break;
				default:
					printf("Unknown GET_LAST_VALUE CMD = %02x, %02x %02x\n\r",
									omsg[CMD], imsg[DATA0] & 0xFF,
									imsg[DATA1] & 0xFF);
			}		
	}
}
