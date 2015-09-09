/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: PCI.h
// 
// Description:
// This file is the interface to the basic PCI access methods
// 
// Update Log 
// 8/24/98 Michael G. Panas: Create file
// 9/2/98 Michael G. Panas: add C++ stuff
// 9/13/98 Michael G. Panas: Use Galileo.h now, add PCI Register defines
// 02/19/99 Jim Frandeen: Add i2odep.h
/*************************************************************************/

#if !defined(PCI_H)
#define PCI_H

#include "Galileo.h"
#include "Simple.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

// Methods defined in PCI.c
U32		GetPciReg(U32 Bus, U32 Slot, U32 Func, U32 Reg);
void	SetPciReg(U32 Bus, U32 Slot, U32 Func, U32 Reg, U32 Data);


/*************************************************************************/
// The following macros will byte swap 16-bit and 32-bit words.
/*************************************************************************/
#define BYTE_SWAP16(X) (((X)&0xff) << 8) + (((X)&0xff00) >> 8)
#define BYTE_SWAP32(X) (((X)&0xff)<<24)+(((X)&0xff00)<<8)+(((X)&0xff0000)>>8)+(((X)&0xff000000)>>24)

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

//*********************************************************************************
// PCI Configuration Registers
// Not accessable from Virtual Memory
//*********************************************************************************

/* Configuration ID Register (R/O) */
#define PCICONFIG_IDRL				0x0
#define PCICONFIG_VENDORIDM				0x0000ffff
#define PCICONFIG_DEVICEIDM				0xffff0000

/* PCI command Register (R/W) */
#define PCICOMMANDRS				0x4
#define PCICMDIOENABLEM					0x0001	/* I/O space enable */
#define PCICMDMEMORYENABLEM				0x0002	/* Memory space enable */
#define PCICMDMASTERENABLEM				0x0004	/* Master enable */

#define PCICMDMEMWRTINVALIDATEM			0x0010	/* Memory Write Invalidate */
#define PCICMDPARITYENABLEM				0x0040	/* Parity checking enable */
#define PCICMDWAITCYCLEENABLEM			0x0080	/* Wait Cycle Control enable */

#define PCICMDSERRENABLEM				0x0100	/* SERR enable */


/* PCI status Register (R/W) */
#define PCISTATUSRS					0x6
#define PCISTATUSFASTBACK2BACKM			0x0080	/* Supports fast Back2back xsaction */
#define PCISTATUSMASTERPARITYERRM		0x0100	/* Master Data parity Error */
#define PCISTATUSDEVICESELM				0x0600	/* DEVSEL Timing */
#define PCISTATUSSIGNALTARGETABORTM		0x0800	/* Target Abort */

#define PCISTATUSRCVTARGETABORTM		0x1000	/* Received Target Abort */
#define PCISTATUSRCVMASTERABORTM		0x2000	/* Received master Abort */
#define PCISTATUSSIGNALSERRM			0x4000	/* Signaled System Error */
#define PCISTATUSPARITYERRM				0x8000	/* Parity Error */

#define PCISTATUSCLEARERRORSK			0xf900	/* Clear Error */


/* Revision and PCI class code Register (R/O) */
#define PCIREVISIONRL				0x8
#define PCIREVISIONM					0x000000ff	/* Revision ID */
#define PCIPROGRAMMINGLEVELM			0x0000ff00	/* Specific Register Level Programming */
#define PCISUBCLASSM					0x00ff0000	/* Sub-Class Eencoding */
#define PCIBASECLASSM					0xff000000	/* Base Class Encoding */


/* PCI cache line size Register (R/O) */
#define PCICACHELINERB				0xc


/* PCI Latency timer Register (R/W) */
#define PCILATENCYTIMERRB			0xd


/* PCI header type Register (R/O) */
#define PCIHEADERTYPERB				0xe
#define PCIHEADERTYPECONFIGTYPEM		0x7f		/* Config Layout Type */
#define PCIHEADERTYPEM					0x80		/* Header Type */



/* PCI Built-In Self Test Register (R/W) */
#define PCIBISTRB					0xf
#define PCIBISTSTATUSM					0x0f		/* BIST Status */
#define PCIBISTSTARTM					0x40		/* Start BIST */
#define PCIBISTSUPPORTEDM				0x80		/* Device supports BIST */


/* 32 Bit PCI Base Address Registers
/* PCI Base Address Register for Memory Access to Local, Runtime and DMA Registers (R/W) */
#define PCIMEMORYBASERL				0x10
#define PCIMEMORYBASEIOMAPM				0x00000001	/* Hardwired to 0 */
#define PCIMEMORYBASEREGIONM			0x00000006	/* Hardwired to 0 */
#define PCIMEMORYBASEPREFETCHABLEM		0x00000008	/* Hardwired to 0 */
#define PCIMEMORYBASEADDRESSM			0xfffffff0	/* Memory Base Address */


/* PCI Base Address Register for I/O Access to Local, Runtime and DMA Registers (R/W) */
#define PCIIOBASERL					0x14
#define PCIIOBASEIOMAPM					0x00000001	/* Hardwired to 1 */
#define PCIIOBASEADDRESSM				0xfffffffc	/* IO Base Address */


/* PCI Base Address Register for Memory Access to Local Address Space 0 (R/W) */
#define PCILOCALADDSP0BASERL		0x18
#define PCILOCALADDSP0BASEIOMAPM		0x00000001	/* 0 is memory space, 1 is I/O space */
#define PCILOCALADDSP0BASEREGIONM		0x00000006	/* Hardwired to 0 for memory */
#define PCILOCALADDSP0BASEPREFETCHABLEM	0x00000008	/* Hardwired to 0 for memory */
#define PCILOCALADDSP0BASEADDRESSM		0xfffffff0	/* Memory Base Address */


/* Configuration ID Register (R/O) */
#define PCICONFIGIDRS				0x2c
#define PCICONFIGVENDORIDM				0x00ff		/* Vendor ID */
#define PCICONFIGDEVICEIDM				0xff00		/* Device ID */


/* PCI Interrupt line Register */
#define PCIINTERRUPTLINERB			0x3c


/* PCI interrupt pin Register */
#define PCIINTERRUPTPINRB			0x3d
#define PCIINTERRUPTPINM				0xff
#define PCIINTERRUPTPININTAK			0x01
#define PCIINTERRUPTPININTBK			0x02
#define PCIINTERRUPTPININTCK			0x03
#define PCIINTERRUPTPININTDK			0x04


/* PCI Min_Gnt Register */
#define PCIMINGNTRB					0x3e
#define PCIMINGNTVALUEM					0xff

/* PCI Max_Lat Register */
#define PCIMAXLATRB					0x3f
#define PCIMAXLATVALUEM					0xff


#endif /* PCI_H  */
