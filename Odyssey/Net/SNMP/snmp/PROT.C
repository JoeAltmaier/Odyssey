/*@*********************************************************************** 
 |                                                                         
 |             Copyright (c) 1995-1997 XACT Incporated                     
 |                                                                         
 | PROPRIETARY RIGHTS of XACT Incorporated are involved in the subject     
 | matter of this material.  All manufacturing, reproduction, use, and     
 | sales rights pertaining to this subject matter are governed by the      
 | license agreement.  The recipient of this software implicitly accepts   
 | the terms of the license.                                               
 |                                                                         
 |                                                                         
 | FILE NAME   : prot.c                                 
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : Special protocol support                                  
 | AUTHOR      : Robert Winter                                              
 *************************************************************************/
#include "xport.h"
#include "xtypes.h"
#include "xtern.h"
#include "xsnmp.h"
#include "snmp.h"
#include "agent.h"
#include "link.h"
#include "prott.h"
#include "mac.h"
#include "xcfig.h" 
#include "prot.h"

#define PROT_TYPE_OTHER               1
#define PROT_TYPE_REGULAR_1822        2
#define PROT_TYPE_HDH_1822            3
#define PROT_TYPE_DDN_X25             4
#define PROT_TYPE_RFC877_X25          5
#define PROT_TYPE_ETHERNET_CSMACD     6
#define PROT_TYPE_88023_CSMACD        7
#define PROT_TYPE_88024_TOKENBUS      8
#define PROT_TYPE_88025_TOKENRING     9
#define PROT_TYPE_88026_MAN           10
#define PROT_TYPE_SOFT_LOOPBACK       24

i8 *protocolString = NULL;

i8 *protXsnmpString[] = {
    "TYPE",
    "INTERFACE",
    "ID",
    "SIZE",
    "LEN",
    "TIME"
};

bool 
ProtFrame( prot_pkt_t *Pkt, u8 *frame, u16 size, u16 length, ul32 time, 
		u16 status, u16 type, u16 pindex, ul32 id )
{
    if((Pkt->Frame = (prot_frame_t *)x_malloc(sizeof(prot_xsnmp_t))) == NULL)
        return FALSE;
    Pkt->Ptr 					= 0;
    Pkt->Frame->Xsnmp.Type 		= type;
    Pkt->Frame->Xsnmp.IfIndex 	= pindex;
    Pkt->Frame->Xsnmp.ID 		= id;
    Pkt->Frame->Xsnmp.Size 		= size;
    Pkt->Frame->Xsnmp.Len 		= length;
    Pkt->Frame->Xsnmp.Time 		= time;
    Pkt->Frame->Xsnmp.Status 	= status;
    Pkt->Frame->Xsnmp.Data 		= frame;
    Pkt->Child 					= NULL;
    
    switch(Pkt->Frame->Xsnmp.Type) {
        case PROT_TYPE_ETHERNET_CSMACD:
            Pkt->ChildProt = PROT_PKTETHERNET;
            break;
        default:
            Pkt->ChildProt = PROT_PKTUNKNOWN;
            break;
    }
    return TRUE;
}


bool 
ProtFile( prot_pkt_t *Pkt, u8 *PktPtr )
{
    if((Pkt->Frame = (prot_frame_t *)x_malloc (sizeof(prot_xsnmp_t))) == NULL)
        return FALSE;
    Pkt->Ptr = PktPtr;
    Pkt->Frame->Xsnmp.Type = mem2word(Pkt->Ptr);
    Pkt->Ptr += 2;
    Pkt->Frame->Xsnmp.IfIndex = mem2word(Pkt->Ptr);
    Pkt->Ptr += 2;
    Pkt->Frame->Xsnmp.ID = mem2lword(Pkt->Ptr);
    Pkt->Ptr += 4;
    Pkt->Frame->Xsnmp.Size = mem2word(Pkt->Ptr);
    Pkt->Ptr += 2;
    Pkt->Frame->Xsnmp.Len = mem2word(Pkt->Ptr);
    Pkt->Ptr += 2;
    Pkt->Frame->Xsnmp.Time = mem2lword(Pkt->Ptr);
    Pkt->Ptr += 4;
    Pkt->Frame->Xsnmp.Status = mem2word(Pkt->Ptr);
    Pkt->Ptr += 2;
    Pkt->Frame->Xsnmp.Data = Pkt->Ptr;
    Pkt->Child = NULL;

    switch(Pkt->Frame->Xsnmp.Type) {
        case PROT_TYPE_ETHERNET_CSMACD:
            Pkt->ChildProt = PROT_PKTETHERNET;
            break;
        default:
            Pkt->ChildProt = PROT_PKTUNKNOWN;
            break;
    }
    return TRUE;
}


bool 
ProtGetField( prot_pkt_t *Pkt, prot_obj_t *Obj )
{
    if(Obj->Level == 0 && Obj->Id[1] == PROT_TYPE) {
        x_memset(&Obj->Id[2], 0, SNMP_SIZE_OBJECTID - 4);
        Obj->Type = SNMP_GAUGE;
        Obj->Syntax.LngUns = (ul32) Pkt->ChildProt;
        return TRUE;
    }

    if(Obj->Id[0] == PROT_PKTXSNMP) {
        switch (Obj->Id[1]) {
            case 1:
                Obj->Type = SNMP_INTEGER;
                Obj->Syntax.LngInt = (l32) Pkt->Frame->Xsnmp.Type;
                x_memset(&Obj->Id[2], 0, SNMP_SIZE_OBJECTID - 4);
                return TRUE;
            case 2:
                Obj->Type = SNMP_INTEGER;
                Obj->Syntax.LngInt = (ul32) Pkt->Frame->Xsnmp.IfIndex;
                x_memset(&Obj->Id[2], 0, SNMP_SIZE_OBJECTID - 4);
                return TRUE;
            case 3:
                Obj->Type = SNMP_INTEGER;
                Obj->Syntax.LngInt = Pkt->Frame->Xsnmp.ID;
                x_memset(&Obj->Id[2], 0, SNMP_SIZE_OBJECTID - 4);
                return TRUE;
            case 4:
                Obj->Type = SNMP_GAUGE;
                Obj->Syntax.LngUns = (ul32) Pkt->Frame->Xsnmp.Size;
                x_memset(&Obj->Id[2], 0, SNMP_SIZE_OBJECTID - 4);
                return TRUE;
            case 5:
                Obj->Type = SNMP_GAUGE;
                Obj->Syntax.LngUns = (ul32) Pkt->Frame->Xsnmp.Len;
                x_memset(&Obj->Id[2], 0, SNMP_SIZE_OBJECTID - 4);
                return TRUE;
            case 6:
                Obj->Type = SNMP_TIMETICKS;
                Obj->Syntax.LngUns = Pkt->Frame->Xsnmp.Time;
                x_memset(&Obj->Id[2], 0, SNMP_SIZE_OBJECTID - 4);
                return TRUE;
            case 7:
                Obj->Type = SNMP_NULL;
                Obj->Syntax.Ptr = Pkt->Frame->Xsnmp.Data;
                Obj->SyntaxLen = Pkt->Frame->Xsnmp.Len;
                x_memset(&Obj->Id[2], 0, SNMP_SIZE_OBJECTID - 4);
                return TRUE;
            case 8:
                Obj->Type = SNMP_INTEGER;
                Obj->Syntax.LngInt = (l32) Pkt->Frame->Xsnmp.Status;
                x_memset(&Obj->Id[2], 0, SNMP_SIZE_OBJECTID - 4);
                return TRUE;
            default:
                return FALSE;
        }
    }

    if(Obj->Level > 0 && Pkt->ChildProt != PROT_PKTUNKNOWN) {
        if (Pkt->Child == NULL) {
            if ((Pkt->Child = (prot_pkt_t *)x_malloc(sizeof(prot_pkt_t))) == NULL)
                return FALSE;
            Pkt->Child->Ptr = Pkt->Frame->Xsnmp.Data;
            Pkt->Child->Child = NULL;
            Pkt->Child->DataLen = Pkt->Frame->Xsnmp.Len;
            if ((ProtPtr[Pkt->ChildProt].Header(Pkt->Child)) == FALSE) {
                x_free(Pkt->Child);
                Pkt->Child = NULL;
                return FALSE;
            }
        }
        Obj->Level--;
        return ProtPtr[Pkt->ChildProt].Field (Pkt->Child, Obj);
    }
    return FALSE;
}


void 
ProtFree( prot_pkt_t *Pkt )
{
    if(Pkt->Child != NULL) {
        ProtFree (Pkt->Child);
        x_free (Pkt->Child);
        Pkt->Child = NULL;
    }
    x_free(Pkt->Frame);
    Pkt->Frame = NULL;
    return;
}


void 
ProtExit( void )
{
    if (protocolString != NULL)
        x_free(protocolString);
    return;
}


bool 
ProtStringNr( prot_obj_t *Obj )
{
bool 	Flag = 0;
u16 	Ind, Size;
u8 		*Ptr1, *Ptr2;

    x_memset(&Obj->Id[2], 0, SNMP_SIZE_OBJECTID - 4);

    if((Ptr1 = (u8 *)x_strchr((i8 *)Obj->FieldStr,'.')) != NULL) {
        *Ptr1 = '\0';
        Size = (u16)x_strlen((i8 *)(Ptr1 + 1));
        for(Ind = 0; Size > 0; Ind++) {
            Ptr1++;
            if ((Ptr2 = (u8 *)x_strchr((i8 *)Ptr1,'.')) != NULL) {
                *Ptr2 = '\0';
                Obj->Id[2+Ind] = (u16) x_atoi((i8 *)Ptr1);
                Size = (u16)x_strlen((i8 *)(Ptr2 + 1));
                Ptr1 = Ptr2;
            } else {
                Obj->Id[2+Ind] = (u16) x_atoi((i8 *)Ptr1);
                Size = 0;
            }
        }
    }

    for(Ind = 1; Ind < sizeof(ProtPtr)/sizeof(prot_ptr_t); Ind++) {
        if (!x_strcmp((i8 *)ProtPtr[Ind].Name, (i8 *)Obj->ProtStr)) {
            Obj->Id[0] = Ind;
            Flag = 1;
        }
    }
    if(Flag) {
        for (Ind = 0; Ind < ProtPtr[(u16)(Obj->Id[0])].StringLen; Ind++) {
            if (!x_strcmp((i8 *)(ProtPtr[(u16)(Obj->Id[0])].String[Ind]), (i8 *)(Obj->FieldStr))) {
                Obj->Id[1] = Ind + 1;
                return TRUE;
            }
        }
    }
    return FALSE;
}

bool 
ProtPrint( prot_obj_t *Obj, u8 **StrPtr )
{
    if(protocolString == NULL) {
        if ((protocolString = (i8 *)x_malloc(SNMP_SIZE_BUFCHR)) == NULL) /* worst case */
            return FALSE;
    }
    if(Obj->Id[0] < sizeof(ProtPtr)/sizeof(prot_ptr_t))
        return (bool)(ProtPtr[(u16)(Obj->Id[0])].Print (Obj, StrPtr));
	return FALSE;
}


bool 
ProtXsnmpPrint( prot_obj_t *Obj, u8 **StrPtr )
{
    switch (Obj->Id[1]) {
        case 1:                                 /* type */
        case 2:                                 /* IfIndex */
        case 4:                                 /* size */
        case 5:                                 /* len */
#ifdef XLIB_XSNMP
			plist[0].longval = Obj->Syntax.LngUns;
            x_sprintf(protocolString, "%4lu");
#else
            sprintf(protocolString, "%4lu", Obj->Syntax.LngUns);
#endif
            *StrPtr = (u8 *)protocolString;
            return TRUE;
        case 3:                                 /* ID */
#ifdef XLIB_XSNMP
			plist[0].intval = Obj->Syntax.LngInt;
            x_sprintf(protocolString, "%6ld");
#else
            sprintf(protocolString, "%6ld", Obj->Syntax.LngInt);
#endif
            *StrPtr = (u8 *)protocolString;
            return TRUE;
        case 6:                                 /* time */
#ifdef XLIB_XSNMP
			plist[0].longval = (u16)((Obj->Syntax.LngUns / 1000000L) % 1000);
			plist[1].longval = (u16)((Obj->Syntax.LngUns / 1000L) % 1000);
			plist[2].longval = (u16)(Obj->Syntax.LngUns % 1000);
            x_sprintf(protocolString, "%u:%03u:%03u");
#else
            sprintf(protocolString, "%u:%03u:%03u",
                (u16) ((Obj->Syntax.LngUns / 1000000L) % 1000),
                (u16) ((Obj->Syntax.LngUns / 1000L) % 1000),
                (u16) (Obj->Syntax.LngUns % 1000));
#endif
            *StrPtr = (u8 *)protocolString;
            return TRUE;
        default:
            return FALSE;
    }
}
