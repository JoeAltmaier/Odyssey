#define _ODYSSEY			// target odyssey (not eval)
#include "Transport.h"

           case '?':
            	{
            		Tracef("Starting pci loop test...\n");
            		Critical section;
            		U32 pciRemote;
            		printf("Cache Line Size %02X\n\r", pciconf_readb(0x80000000, PCI_CONF_CLS));
            		for (int i=0; i < 1000; i++) {
//	            		Tracef("Getting buffer ");

						U32 cbcb_=*(U32*)P_PA(Address::aSlot[IOP_HBC0].pciBase + GT_I2O_OUTMSG_1_OFF);
//						pciRemote=*(U32*)P_PA(Address::aSlot[IOP_HBC0].pciBase + GT_I2O_INQ_VR_OFF);

//         				Tracef("%d %08lx %08lx\n", i, pciRemote, cbcb_);

//           				if (pciRemote && pciRemote != (U32)-1)
//								*(U32*)P_PA(Address::aSlot[IOP_HBC0].pciBase + GT_I2O_OUTQ_VR_OFF) = pciRemote;
       				}
            	}
