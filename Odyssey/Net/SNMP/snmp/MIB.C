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
 | FILE NAME   : mib.c                                  
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : MIB manipulation routines                                 
 | AUTHOR      : Robert Winter                                              
 *************************************************************************/
#include "xport.h"
#include "xtypes.h"
#include "xtern.h"
#include "math.h"
#include "snmp.h"
#include "mib.h"
#include "xsnmp.h"
#include "agent.h"
#include "link.h"
#include "prott.h"
#include "mac.h"
#include "xcfig.h"

static mib_root_t *MibRoot;

extern snmp_stat_t	SnmpStat;

mib_rmon_t Mib =  { NULL, 0, NULL, 0, };

static i32 MibCmpProfil(const mib_community_t *Prf1, const mib_community_t *Prf2);
static i32 MibCmpComm(const u8 *comm1, u16 comm1len, const u8 *comm2, u16 comm2len);
static i32 MibCmpObject(const mib_element_t *Obj1, const mib_element_t *Obj2);
static i32 MibCmpObjId(const ul32 *ObjId, u16 ObjIdLen1, const ul32 *ObjId2, u16 ObjIdLen2);
static i32 MibObjectFind(ul32 *Id, i32 IdLen, i32 *cmp);
static bool MibObjectInsert(mib_object_t *Object);
static bool MibObjectDelete(mib_object_t *Object);

static u16 Request(snmp_object_t *Obj, i32 *lastindex);

extern xsnmp_cfig_t	xsnmp_cfg;


bool 
MibInit ( mib_element_t *mib, u16 mibsize )
{
i32 			commnr, i, n;
agent_comm_t	*comm;
u16 			support;
mib_community_t	*mibprf, *p;

	mibprf = NULL;
	commnr = 0;

	for( n=1, comm = AgentCommunity(n); comm != 0; comm=comm->next, n++) { 
		if( !comm ) {
			x_dbg("XSNMP, MibInit: no community defined in agent\n", TRUE);
		} else {
			switch( comm->access ) {
				case XSNMP_READ_ONLY:
					support = MIB_READ;
					break;
				case XSNMP_WRITE_ONLY:
					support = MIB_WRITE;
					break;
				case XSNMP_READ_WRITE:
					support = MIB_READ | MIB_WRITE;
					break;
				default:
					x_dbg("XSNMP, mib: invalid access specified\n", TRUE);
					break;
			}
			if( mibprf )
				p = (mib_community_t *)x_realloc(mibprf, (n+1)*sizeof(mib_community_t));
			else
				p = (mib_community_t *)x_malloc(sizeof(mib_community_t));
			if( !p ) {
				x_dbg("XSNMP, MibInit: out of community name space\n", TRUE);
			} else {
				mibprf = p;
				x_bzero( (i8 *)(mibprf[commnr].Comm), SNMP_SIZE_COMM );
				x_memcpy((i8 *)mibprf[commnr].Comm, (i8*)comm->comm, comm->commLen);
				mibprf[commnr].CommLen = comm->commLen;
				mibprf[commnr].Support = support;
				commnr++;
			}
		}
	}

	if(commnr == 0) {
		x_dbg("XSNMP, mib: no external access possible\n", TRUE);
		return FALSE;
	}

	Mib.Prf = mibprf;
    Mib.PrfSze = commnr;

    Mib.Obj = mib;
    Mib.ObjSze = mibsize;

    x_qsort((void *)Mib.Prf, Mib.PrfSze, sizeof(mib_community_t), (void *)MibCmpProfil);
    x_qsort((void *)Mib.Obj, Mib.ObjSze, sizeof(mib_element_t), (void *)MibCmpObject);

    MibRoot = (mib_root_t *)x_malloc(sizeof(mib_root_t));
	if( MibRoot == NULL )
		x_dbg("XSNMP, MibInit: could not allocate MibRoot\n", TRUE);
    MibRoot->Size = 100/*67*/;
    MibRoot->Count = 0;
	if( !MibRoot->Size )
		x_dbg("XSNMP, MibInit: MibRoot->Size == 0\n", TRUE);
    MibRoot->Table = (mib_object_t **)x_malloc(MibRoot->Size * sizeof(mib_object_t *));
	if( MibRoot->Table == NULL )
		x_dbg("XSNMP, MibInit: could not allocate MibRoot\n", TRUE);

    for (i=0; i<mibsize; i++) {
        if (!MibRegister(mib[i].Id, mib[i].IdLen, mib[i].Rqs, mib[i].Type,
            mib[i].Support, 0)) {
			x_dbg("XSNMP, MibInit: register failed\n", TRUE);
			break;
		}
    }

    return TRUE;
}


mib_object_t *
MibRegister( ul32 *Id, u16 IdLen, mib_callback_t Rqs, u16 Type, u16 Support, void *Param )
{
mib_object_t *object;

    object = (mib_object_t *)x_malloc(sizeof(mib_object_t));
    if (object == 0) {
		x_dbg("XSNMP, MibRegister: object is 0\n", TRUE);
        return 0;
	}
	if( !IdLen )
		x_dbg("XSNMP, MibRegister: Idlen == 0\n", TRUE);
    object->Id = (ul32 *)x_malloc(IdLen * sizeof(l32));
    if (object->Id == 0) {
		x_dbg("XSNMP, MibRegister: object->ID is 0\n", TRUE);
        return 0;
	}
    x_memcpy(object->Id, Id, IdLen * sizeof(l32));
    object->IdLen	= IdLen;
    object->Rqs		= Rqs;
    object->Type	= Type;
    object->Support	= Support;
	object->Param	= Param;

    if (!MibObjectInsert(object)) {
        x_free(object->Id);
        x_free(object);
		x_dbg("XSNMP, MibRegister: MibObjectInsert fail\n", TRUE);
        return 0;
    }

    return object;
}


void 
MibDeRegister( mib_object_t *object )
{
    MibObjectDelete(object);
    x_free(object->Id);
    x_free(object);
}


u16 
MibProfil( u8 *comm, u16 commLen )
{
i32 first, last, mindex, cmp;

    first = 0;
    last = Mib.PrfSze-1;
	cmp = 1;

	if (last < 0) {
		return 0;
	}

    while (first < last) {
        mindex = (first + last)/2;
        cmp = MibCmpComm(Mib.Prf[mindex].Comm, Mib.Prf[mindex].CommLen, 
							comm, commLen);
		switch (cmp) {
		case -1:
            first = mindex + 1;
			break;
		case 0:
			first = last = mindex;
    		return Mib.Prf[mindex].Support;
		case 1:
            last = mindex;
			break;
		}
    }

	mindex = first;

    cmp = MibCmpComm(Mib.Prf[mindex].Comm, Mib.Prf[mindex].CommLen, 
						comm, commLen);

    return cmp == 0 ? Mib.Prf[mindex].Support : 0;
}


/* MibRequest allows for different requests per varbind list */

u16 
MibRequest( u32 listLen, snmp_object_t *list, u16 *errindex )
{
u16				status, mindex, errstatus;
static 			snmp_object_t	MibRequestbackup[AGENT_LIST_SIZE];
static 			i32				MibRequestlastindex[AGENT_LIST_SIZE];

    for(mindex = 0, status = SNMP_NOERROR;
         mindex < listLen && status == SNMP_NOERROR;
         mindex++) {
		if (list[mindex].Request != SNMP_PDU_SET) {
			MibRequestbackup[mindex] = list[mindex];
		} else {				/* prevent backup later on */
			MibRequestbackup[mindex].Request = SNMP_PDU_SET;
		}
		/* object request type in list is set correctly */
        status = Request(list + mindex, MibRequestlastindex + mindex);
    }
	errstatus = status;
	*errindex = mindex;

	/*
	 * if it's a set, then go through list a second time and actually
	 * do the set, if there were no errors the first time
	 */
	if(errstatus == SNMP_NOERROR) {
		for(mindex = 0, status = SNMP_NOERROR;
			 mindex < listLen && status == SNMP_NOERROR;
			 mindex++) {
			if(list[mindex].Request == SNMP_PDU_SET) {
				list[mindex].Request = SNMP_PDU_COMMIT;
				status = Request(list + mindex, MibRequestlastindex + mindex);
			}
		}
		errstatus = status;
		*errindex = mindex;
	}

	if(errstatus != SNMP_NOERROR) {
		/*
		 * count the errors by types for reporting snmp group errors
		 */
		switch(errstatus) {
			case SNMP_NOSUCHNAME:
				SnmpStat.OutNoSuchNames++;
				break;
			case SNMP_BADVALUE:
				SnmpStat.OutBadValues++;
				break;
			case SNMP_READONLY:
				break;
			case SNMP_GENERROR:
				SnmpStat.OutGenErrs++;
				break;
			default:
				break;
		}
		for(mindex = 0, status = SNMP_NOERROR;
			 mindex < *errindex && status == SNMP_NOERROR;
			 mindex++) {
			/*
			 * if a set was attempted, this can undo those sets when a set
			 * fails
			 */
			if(list[mindex].Request == SNMP_PDU_SET ||
				list[mindex].Request == SNMP_PDU_COMMIT) {
				list[mindex].Request = SNMP_PDU_UNDO;
				status = Request(list + mindex, MibRequestlastindex + mindex);
			} else {
				list[mindex] = MibRequestbackup[mindex];
			}
		}
	}

	return errstatus;
}


static u16 
Request( snmp_object_t *Obj, i32 *lastindex )
{
i32 mindex, cmp;
u16 error;

	if (MibRoot == NULL) {
		return SNMP_GENERROR;
	}

    error = SNMP_NOSUCHNAME;

    cmp = MibCmpObjId(MibRoot->Table[*lastindex]->Id, (u16)MibRoot->Table[*lastindex]->IdLen, Obj->Id, (u16)Obj->IdLen);

	if(cmp == -2 && Obj->Request == SNMP_PDU_NEXT && 
		++(*lastindex) < MibRoot->Count) {
    	cmp = MibCmpObjId(MibRoot->Table[*lastindex]->Id, (u16)MibRoot->Table[*lastindex]->IdLen, Obj->Id, (u16)Obj->IdLen);
	}

	if ( (cmp == -1) || (cmp == 0) ){
		mindex = *lastindex;
	} else {
    	mindex = *lastindex = MibObjectFind(Obj->Id, Obj->IdLen, &cmp);
	}

    switch(Obj->Request) {
	case SNMP_PDU_GET:
		if( ((cmp != -1) && (cmp != 0)) || MibRoot->Table[mindex]->Rqs == NULL) {
			return error;
		}
		if(!(MibRoot->Table[mindex]->Support & MIB_READ)) {
			return error;
		}
		error = MibRoot->Table[mindex]->Rqs(Obj, MibRoot->Table[mindex]->IdLen, MibRoot->Table[mindex]->Param);
		if(error == SNMP_NOERROR) {
			Obj->Type = MibRoot->Table[mindex]->Type;
		}
		return error;
	case SNMP_PDU_SET:
		if( ((cmp != -1) && (cmp != 0)) || MibRoot->Table[mindex]->Rqs == NULL) {
			return error;
		}
		if(!(MibRoot->Table[mindex]->Support & MIB_WRITE)) {
			return SNMP_BADVALUE;
		}
		if(Obj->Type == SNMP_OBJECTID && Obj->SyntaxLen > SNMP_SIZE_BUFINT) {
			return SNMP_BADVALUE;
		}
		if((Obj->Type == SNMP_OCTETSTR ||
			 Obj->Type == SNMP_DISPLAYSTR ||
			 Obj->Type == SNMP_OPAQUE) &&
			(Obj->SyntaxLen > SNMP_SIZE_BUFCHR)) {
			return SNMP_BADVALUE;
		}
		error = MibRoot->Table[mindex]->Rqs(Obj, MibRoot->Table[mindex]->IdLen, MibRoot->Table[mindex]->Param);
		return error;
	case SNMP_PDU_NEXT:
		switch(cmp) {
		case -2:
			return error;
		case -1:
		case 0:
			if(MibRoot->Table[mindex]->Support & MIB_READ &&
				MibRoot->Table[mindex]->Rqs != NULL) {
				error = MibRoot->Table[mindex]->Rqs(Obj, MibRoot->Table[mindex]->IdLen, MibRoot->Table[mindex]->Param);
				if (error == SNMP_NOERROR) {
					Obj->Type = MibRoot->Table[mindex]->Type;
					break;
				}
			}
			mindex++;
			break;
		}
		while(error == SNMP_NOSUCHNAME && mindex < MibRoot->Count) {
			x_memcpy(Obj->Id, MibRoot->Table[mindex]->Id,
				   MibRoot->Table[mindex]->IdLen * sizeof(l32));
			Obj->IdLen = MibRoot->Table[mindex]->IdLen;
			if(MibRoot->Table[mindex]->Support & MIB_READ &&
				MibRoot->Table[mindex]->Rqs != NULL) {
				error = MibRoot->Table[mindex]->Rqs(Obj, MibRoot->Table[mindex]->IdLen, MibRoot->Table[mindex]->Param);
				if(error == SNMP_NOERROR) {
					Obj->Type = MibRoot->Table[mindex]->Type;
					break;
				}
			}
			mindex++;
		}
		return error;
	case SNMP_PDU_COMMIT:
	case SNMP_PDU_UNDO:
		if( ((cmp != -1) && (cmp != 0)) || MibRoot->Table[mindex]->Rqs == NULL) {
			return error;
		}
		error = MibRoot->Table[mindex]->Rqs(Obj, MibRoot->Table[mindex]->IdLen, MibRoot->Table[mindex]->Param);
		return error;
    }

	return SNMP_GENERROR;
}


bool 
MibSimple( snmp_object_t *Obj, u16 IdLen )
{
    if(Obj->Request != SNMP_PDU_NEXT) {
        if (Obj->IdLen == (u32)(IdLen + 1 && Obj->Id[IdLen]) == 0) {
            return TRUE;
		}
        return FALSE;
    }
    if(Obj->IdLen == IdLen) {
        Obj->Id[IdLen] = 0;
        Obj->IdLen++;
        return TRUE;
    }
    return FALSE;
}


mib_local_t *
MibRmon( snmp_object_t *Obj, mib_local_t *Local, u16 IdLen, u16 IdSize )
{
    if(Local == NULL) {
        return NULL;
	}
    if(Obj->Request != SNMP_PDU_NEXT) {
        if (Obj->IdLen != (u32)(IdLen + IdSize)) {
			return NULL;
		}
        while (Local != NULL && (u32)Local->Index < Obj->Id[IdLen]) {
            Local = Local->Next;
		}
		if (Local == NULL || (u32)Local->Index != Obj->Id[IdLen]) {
            return NULL;
		}
		return Local;
    }
    if(Obj->IdLen == IdLen) {       /* first instance */
        return Local;
	}
    while (Local != NULL && (u32)Local->Index < Obj->Id[IdLen]) {
        Local = Local->Next;
	}
	if(Local == NULL || (u32)Local->Index < Obj->Id[IdLen]) {
        return NULL;
	}
    return Local;
}


mib_local_t *
MibInsert( snmp_object_t *Obj, mib_local_t **local, u16 IdLen, u16 IdSize )
{
mib_local_t * Junk;
mib_local_t	* Local = *local;

    if(Obj->IdLen != (u32)(IdLen + IdSize) || Obj->Id[IdLen] < 1) {
        return NULL;
	}
    if((Junk = (mib_local_t *)x_malloc(sizeof(mib_local_t))) == NULL) {
        return NULL;
	}
    Junk->Index = Obj->Id[IdLen];

    if(Local == NULL || Local->Index > Junk->Index) {
        Junk->Next = Local;
        *local = Junk;
        return Junk;
    }
    while(Local->Next != NULL && Local->Next->Index <= Junk->Index)
        Local = Local->Next;
    if(Local->Index == Junk->Index) {
        x_free(Junk);
        return Local;
    }
    Junk->Next = Local->Next;
    Local->Next = Junk;

    return Junk;
}


bool 
MibRemove( snmp_object_t *Obj, mib_local_t **local, u16 IdLen, u16 IdSize )
{
mib_local_t * Junk;
mib_local_t * Local = *local;

    if(Obj->IdLen != (u32)(IdLen + IdSize) || Obj->Id[IdLen] < 1)
    if(Local == NULL)
        return FALSE;
    if(Obj->IdLen != (u32)(IdLen + IdSize))
        return FALSE;
    if((u32)Local->Index == Obj->Id[IdLen]) {
        Junk = Local->Next;
        x_free(Local);
        *local = Junk;
        return TRUE;
    }
    while(Local->Next != NULL && (u32)Local->Next->Index < Obj->Id[IdLen])
        Local = Local->Next;
    if((u32)Local->Next->Index == Obj->Id[IdLen]) {
        Junk = Local->Next->Next;
        x_free(Local->Next);
        Local->Next = Junk;
        return TRUE;
    }
    return FALSE;
}


i32 
MibCmpProfil( const mib_community_t *Prf1, const mib_community_t *Prf2 )
{
	return MibCmpComm(Prf1->Comm, Prf1->CommLen, Prf2->Comm, Prf2->CommLen);
}


i32 
MibCmpComm( const u8 *comm1, u16 comm1len, const u8 *comm2, u16 comm2len )
{
i32 result;

    if((result = x_memcmp(comm1, comm2, min(comm1len, comm2len))) == 0) {
		if (comm1len < comm2len)
			return -1;
		else
		if(comm1len > comm2len)
			return 1;
		return 0;
	}
	return result < 0 ? -1 : 1;
}


i32 
MibCmpObject( const mib_element_t *Obj1, const mib_element_t *Obj2 )
{
    return MibCmpObjId (Obj1->Id, Obj1->IdLen, Obj2->Id, Obj2->IdLen);
}


i32 
MibCmpObjId( const ul32 *ObjId1, u16 ObjIdLen1, const ul32 *ObjId2, u16 ObjIdLen2 )
{
    while(ObjIdLen1 > 0 && ObjIdLen2 > 0 && *ObjId1 == *ObjId2) {
        ObjIdLen1--;
        ObjIdLen2--;
        ObjId1++;
        ObjId2++;
    }
    if(!ObjIdLen1 && !ObjIdLen2) {
        return 0;
	}
    if(!ObjIdLen1) {
        return -1;
	}
    if(!ObjIdLen2) {
        return 1;
	}
    if(*ObjId1 < *ObjId2) {
        return -2;
	} else {
        return 2;
	}
}


i32 
MibObjectFind( ul32 *Id, i32 IdLen, i32 *cmp )
{
i32 first, last, mindex;

    first = 0;
    last = MibRoot->Count - 1;
    *cmp = 2;

	if(last < 0) {
		return 0;
	}

    while(first < last) {
        mindex = (first + last)/2;
        *cmp = MibCmpObjId(MibRoot->Table[mindex]->Id,
							(u16)MibRoot->Table[mindex]->IdLen, Id, (u16)IdLen);
        switch(*cmp) {
            case -2:
                first = mindex+1;
                break;
            case -1:
            case 0:
                first = last = mindex;
    			return mindex;
            case 1:
            case 2:
                last = mindex;
                break;
        }
    }

	mindex = first;

    *cmp = MibCmpObjId(MibRoot->Table[mindex]->Id,
						(u16)MibRoot->Table[mindex]->IdLen, Id, (u16)IdLen);
	
    return mindex;
}

l32 *
MibObjectFindNext( l32 *Id, i32 IdLen, i32 *IdLenNext)
{
i32 cmp,idx;

   	idx = MibObjectFind((ul32 *)Id, IdLen, &cmp);
	if(!cmp) {
		*IdLenNext 	= MibRoot->Table[idx+1]->IdLen;
		return((l32*)(MibRoot->Table[idx+1]->Id));
	} else {
		return((l32 *)0);
	}
}

bool 
MibObjectInsert( mib_object_t *Object )
{
i32 mindex, cmp, i;

    if(MibRoot->Count >= MibRoot->Size) {
        i32 n;
        mib_object_t **p;

        n = MibRoot->Size + 100;
		if( MibRoot->Table ) 
        	p = (mib_object_t **)x_realloc(MibRoot->Table, n*sizeof(mib_object_t *));
		else
        	p = (mib_object_t **)x_malloc(sizeof(mib_object_t *));
        if (p == 0)
            return FALSE;
        MibRoot->Table = p;
        MibRoot->Size = n;
    }

    MibRoot->Table[MibRoot->Count] = Object;
	MibRoot->Count++;

    mindex = MibObjectFind(Object->Id, Object->IdLen, &cmp);
    if (cmp == 0 && mindex != MibRoot->Count-1) {
		MibRoot->Count--;
        return FALSE;
	}

    for(i = MibRoot->Count-1; i > mindex; i--)
        MibRoot->Table[i] = MibRoot->Table[i-1];
    MibRoot->Table[mindex] = Object;

    return TRUE;
}


bool 
MibObjectDelete( mib_object_t *Object )
{
i32 mindex, cmp, i;

    mindex = MibObjectFind(Object->Id, Object->IdLen, &cmp);
    if (cmp == 0)
        return FALSE;

    MibRoot->Count--;
    for (i = mindex; i < MibRoot->Count; i++)
        MibRoot->Table[i] = MibRoot->Table[i + 1];

    return TRUE;
}

u16
oid_compare( u32 *name1, u32 len1, u32 *name2, u32 len2 )
{
register int    len;

    /* len = minimum of len1 and len2 */
    if(len1 < len2) len = len1;
    else 			len = len2;
    /* find first non-matching byte */
    while(len-- > 0) {
    	if (*name1 < *name2) {		
			return 2;
		}
    	if (*name2++ < *name1++) { 	
			return 1;
		}
    }
    /* bytes match up to length of shorter string */
    if(len1 < len2) { 	
		return 2; 
	}
    if (len2 < len1) { 	
		return 1;
	}
    return 0;   				
}
