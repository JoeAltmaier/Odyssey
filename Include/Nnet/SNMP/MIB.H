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
 | FILE NAME   :                                        
 | VERSION     :      
 | COMPONENT   :
 | DESCRIPTION :                                                           
 | AUTHOR      :                                                            
 *************************************************************************/
#ifndef _MIB_H
#define _MIB_H

#define MIB_READ                0x01
#define MIB_WRITE               0x02

typedef struct mib_instance_s mib_instance_t;

struct mib_instance_s {
	ul32			Index;
	mib_instance_t	*Prev;
	mib_instance_t	*Next;
};

typedef u16 (*mib_callback_t)(snmp_object_t *Obj, u16 IdLen, void *Param);

typedef struct mib_element_s {
	ul32			Id[SNMP_SIZE_SMALLOBJECTID];
	u16				IdLen;
	mib_callback_t	Rqs;
	u16				Type;
	u16				Support;
} mib_element_t;

typedef struct mib_community_s {
	u8				Comm[SNMP_SIZE_COMM];
	u16				CommLen;
	u16				Support;
} mib_community_t;

typedef struct mib_rmon_s {
	mib_community_t	*Prf;
	u16				PrfSze;
	mib_element_t	*Obj;
	u16				ObjSze;
} mib_rmon_t;
                     
typedef struct mib_object_s {
	ul32			*Id;
	u16				IdLen;
	mib_callback_t	Rqs;
	u16				Type;
	u16				Support; 
	void			*Param;
} mib_object_t;

typedef struct mib_root_s {
	mib_object_t	**Table;
	i32				Size;
	i32				Count;
} mib_root_t;
    
typedef struct mib_local_s mib_local_t;

struct mib_local_s {
	l32				Index;
	mib_local_t		*Next;
	void			*Data;
};

u16 			MibProfil(u8 *Community, u16 CommunityLen);
u16 			MibRequest(u32 listLen, snmp_object_t *list, u16 *errindex);
bool     		MibInit(mib_element_t *Obj, u16 ObjSze);
bool     		MibSimple(snmp_object_t *Obj, u16 IdLen);
mib_local_t		*MibRmon(snmp_object_t *Obj, mib_local_t *Local, u16 IdLen, u16 IdSize);
mib_local_t		*MibInsert(snmp_object_t *Obj, mib_local_t **Local, u16 IdLen, u16 IdSize);
bool     		MibRemove(snmp_object_t *Obj, mib_local_t **Local, u16 IdLen, u16 IdSize);
mib_object_t	*MibRegister(ul32 *Id, u16 IdLen, mib_callback_t Rqs, u16 Type, u16 Support, void *Param);
void 			MibDeRegister(mib_object_t *Object);
l32				*MibObjectFindNext(l32 *Id, i32 Idlen, i32 *IdLenNext);

#endif
