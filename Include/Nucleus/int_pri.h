
#ifndef INT_PRI_H
#define INT_PRI_H

/*
 * VS field in ICR Register
 */
#define ICR_VS_00			0x00
#define ICR_VS_32			0x01
#define ICR_VS_64			0x02
#define ICR_VS_128			0x04
#define ICR_VS_256			0x08
#define ICR_VS_512			0x10

#ifdef CONFIG_INT_PRIORITY

/*
 * Interrupt Priorities from 0 to 15. Each of them should be unique
 * 
 * Interrupt numbers on Odyssey Boards are
 * 
 * 			HBC				NAC				SSD
 * 	-------------------------------------------------
 * 	0.		GT-64120		GT-64120		GT-64120
 * 	1.		SCSI			-				- 	
 * 	2.		AVR WR			AVR WR			AVR WR
 * 	3.		AVR RD			AVR RD			AVR RD
 * 	4.		Uart			Uart			Uart
 * 	5.		CPU Timer		CPU Timer		CPU Timer
 * 	6.		Quad Uart		FC_SYNC			-
 * 	7.		Ethernet 0		Qlogic MAC 2	-	
 * 	8.		Ethernet 1		Qlogic MAC 1	-	
 * 	9.		NAND Flash		Qlogic MAC 0	FLA
 */
#define INT0_PRIORITY		0x00
#define INT1_PRIORITY		0x01
#define INT2_PRIORITY		0x02
#define INT3_PRIORITY		0x03
#define INT4_PRIORITY		0x04
#define INT5_PRIORITY		0x05
#define INT6_PRIORITY		0x06
#define INT7_PRIORITY		0x07
#define INT8_PRIORITY		0x08
#define INT9_PRIORITY		0x09
#define INT10_PRIORITY		0x0A
#define INT11_PRIORITY		0x0B
#define INTSW0_PRIORITY		0x0C			/* Software interrupt 0 */
#define INTSW1_PRIORITY		0x0D			/* Software interrupt 1 */

#define BASE_VECTOR			0xA0000200

#define INTSW0_VEC_BASE		(BASE_VECTOR + (INTSW0_PRIORITY * VEC_SPACING))
#define INTSW1_VEC_BASE		(BASE_VECTOR + (INTSW1_PRIORITY * VEC_SPACING))
#define INT0_VEC_BASE		(BASE_VECTOR + (INT0_PRIORITY * VEC_SPACING))
#define INT1_VEC_BASE		(BASE_VECTOR + (INT1_PRIORITY * VEC_SPACING))
#define INT2_VEC_BASE		(BASE_VECTOR + (INT2_PRIORITY * VEC_SPACING))
#define INT3_VEC_BASE		(BASE_VECTOR + (INT3_PRIORITY * VEC_SPACING))
#define INT4_VEC_BASE		(BASE_VECTOR + (INT4_PRIORITY * VEC_SPACING))
#define INT5_VEC_BASE		(BASE_VECTOR + (INT5_PRIORITY * VEC_SPACING))
#define INT6_VEC_BASE		(BASE_VECTOR + (INT6_PRIORITY * VEC_SPACING))
#define INT7_VEC_BASE		(BASE_VECTOR + (INT7_PRIORITY * VEC_SPACING))
#define INT8_VEC_BASE		(BASE_VECTOR + (INT8_PRIORITY * VEC_SPACING))
#define INT9_VEC_BASE		(BASE_VECTOR + (INT9_PRIORITY * VEC_SPACING))
#define INT10_VEC_BASE		(BASE_VECTOR + (INT10_PRIORITY * VEC_SPACING))
#define INT11_VEC_BASE		(BASE_VECTOR + (INT11_PRIORITY * VEC_SPACING))

#define CONFIG_ICR_VS		ICR_VS_512
#else		/* CONFIG_INT_PRIORITY */

#define CONFIG_ICR_VS		ICR_VS_00

#endif		/* CONFIG_INT_PRIORITY */

#define VEC_SPACING			(CONFIG_ICR_VS << 5)
#endif		/* INT_PRI_H */
