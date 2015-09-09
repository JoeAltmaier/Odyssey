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
#ifndef _LINK_H_
#define _LINK_H_

#define LINK_FLAG_LINK    0x01
#define LINK_FLAG_BUFFER  0x02

typedef struct link_s	link_t;

struct link_s {
	u8			*buffer;
	u16			offset;
	u16			length;
	u16			size;
	u8			flags;
	link_t		*next;
};

u8    	*LinkPush(link_t **link, u16 size);
u8    	*LinkPop(link_t **link, u16 size);
link_t 	*LinkAlloc(link_t *link, u8 *buffer, u16 size, u16 length, u16 offset, link_t *next);
bool 	LinkCopy(link_t *link, u8 *buffer, u16 size);
void    LinkFree(link_t *link);
u16    	LinkAvailSpace(link_t *link);
u16    	LinkAvailData(link_t *link);
u16    	LinkLength(link_t *link);
u16    	LinkSize(link_t *link);
bool 	LinkSplit(link_t *head, link_t *tail, u16 length);
void 	LinkDump(link_t *link);

#endif
