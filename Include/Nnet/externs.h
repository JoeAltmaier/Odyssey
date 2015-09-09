/*************************************************************************/
/*                                                                       */
/*      Copyright (c) 1993 - 1996 by Accelerated Technology, Inc.        */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/
/*
*
* Portions of this program were written by:       */
/***************************************************************************
*                                                                          *
*                                                                          *
*      NCSA Telnet                                                         *
*      by Tim Krauskopf, VT100 by Gaige Paulsen, Tek by Aaron Contorer     *
*                                                                          *
*      National Center for Supercomputing Applications                     *
*      152 Computing Applications Building                                 *
*      605 E. Springfield Ave.                                             *
*      Champaign, IL  61820                                                *
*                                                                          *
****************************************************************************/
/******************************************************************************/
/*                                                                            */
/* FILE NAME                                            VERSION               */
/*                                                                            */
/*   EXTERNS.H                                          NET 4.0               */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*   External definitions for functions in Nucleus NET.                       */
/*   This include file needs to go after other include files                  */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* DATA STRUCTURES                                                            */
/*                                                                            */
/*                                                                            */
/* FUNCTIONS                                                                  */
/*                                                                            */
/*      None                                                                  */
/*                                                                            */
/* DEPENDENCIES                                                               */
/*                                                                            */
/*      None                                                                  */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*      NAME            DATE                    REMARKS                       */
/*                                                                            */
/******************************************************************************/

#ifndef EXTERNS_H
#define EXTERNS_H
#include "tcpdefs.h"
#include "target.h"
#include "ip.h"
#include "mem_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* new_ip.c */
uint16 ipcheck (uint16 *s, uint16 len);
uint32 longswap (uint32);
uint16 intswap (uint16);
int32 n_clicks (VOID);
VOID init_time (VOID);

uint16 IP_Check_Buffer_Chain (NET_BUFFER *buf_ptr);
uint16 tcpcheck (uint16 *, NET_BUFFER *);

/* net.c */
VOID   statcheck (VOID);
STATUS NET_Demux (VOID);
STATUS  dlayersend (DV_DEVICE_ENTRY *device, DLAYER *ptr, uint16 size);
STATUS NET_Add_Multi(DV_DEVICE_ENTRY *dev, DV_REQ *d_req);
STATUS NET_Del_Multi(DV_DEVICE_ENTRY *dev, DV_REQ *d_req);
extern NET_BUFFER_HEADER MEM_Buffer_List;
extern NET_BUFFER_HEADER MEM_Buffer_Freelist;

/* bootp.c */
STATUS NU_Bootp ( CHAR  *dv_name, BOOTP_STRUCT *bp_ptr);



/* user.c */
int16 netread(struct sock_struct *, char *, uint16);
int16 netwrite(struct sock_struct *sockptr, uint8 *buffer, uint16 nbytes, 
               int16 *status);
int16   netpush (uint16 pnum);
STATUS  netlisten(uint16 serv, struct pseudotcp *tcp_check);
STATUS  netxopen (uint8 *machine, uint16 service, INT socketd);
STATUS  doconnect(int16 pnum, uint16 service);
STATUS  Send_SYN_FIN (PORT *, int16);
int16   netclose (int16, struct sock_struct *);
STATUS  netinit (VOID);
int32   windowprobe(PORT *, uint16, uint8 *);
uint16  netsend(PORT *, NET_BUFFER *);

/* tools.c */
int16   netsleep (int16 n);
uint16  enqueue (struct sock_struct *);
int32   dequeue (struct sock_struct *, char *, uint32);
uint16  rmqueue (struct TCP_Window *wind, int32);
int16   comparen (VOID *s1, VOID *s2, int16 n);
STATUS  TL_Put_Event (uint16 event, UNSIGNED dat);
VOID    *normalize_ptr(VOID *);

/* udp.c */
int16   udpinterpret (NET_BUFFER *, struct pseudotcp *);
int16   UDP_Read (struct uport *, char *, struct addr_struct *, 
                  struct sock_struct *);
int16   UDP_Send (struct uport *, uint8 *, uint16, uint16);
STATUS  UDP_Cache_Route (struct uport *, uint32);
STATUS  UDP_Append(INT port_index, NET_BUFFER *buf_ptr);


/* arp.c */
VOID ARP_Init (VOID);
#define NU_Rarp     ARP_Rarp

/* util.c */
STATUS UTL_Timerset (UNSIGNED, UNSIGNED, UNSIGNED, int32);
STATUS UTL_Timerunset (UNSIGNED, UNSIGNED, int32);
STATUS UTL_Clear_Matching_Timer (struct tqhdr *, UNSIGNED, UNSIGNED, int32);
long   NU_TCP_Time(uint32 *);
VOID   UTL_Zero(VOID *ptr, uint32 size);
uint16 UTL_Checksum (NET_BUFFER *, uint8 *, uint8 *, uint8);

/* dll.c */
void *dll_dequeue(void *h);
void *dll_insert(void *h, void *i, void *l);
void *dll_remove(void *h, void *i);
void *dll_enqueue(void *h, void *i);

/* mem.c */
STATUS MEM_Init(VOID);
NET_BUFFER *MEM_Buffer_Dequeue(NET_BUFFER_HEADER *hdr);
NET_BUFFER *MEM_Buffer_Dequeue_nocrit(NET_BUFFER_HEADER *hdr);
NET_BUFFER *MEM_Buffer_Enqueue(NET_BUFFER_HEADER *hdr, NET_BUFFER *item);
NET_BUFFER *MEM_Buffer_Enqueue_nocrit(NET_BUFFER_HEADER *hdr, NET_BUFFER *item);
NET_BUFFER *MEM_Buffer_Chain_Dequeue (NET_BUFFER_HEADER *header, INT nbytes);
NET_BUFFER *MEM_Update_Buffer_Lists (NET_BUFFER_HEADER *source, 
                                     NET_BUFFER_HEADER *dest);
NET_BUFFER *MEM_Buffer_Insert(NET_BUFFER_HEADER *, NET_BUFFER *, 
                              NET_BUFFER*, NET_BUFFER *);
VOID  MEM_Buffer_Chain_Free (NET_BUFFER_HEADER *source, 
                             NET_BUFFER_HEADER *dest);
VOID  MEM_One_Buffer_Chain_Free (NET_BUFFER *source, NET_BUFFER_HEADER *dest);
VOID  MEM_Cat (NET_BUFFER *dest, NET_BUFFER *src);
VOID  MEM_Trim (NET_BUFFER *buf_ptr, INT length);
VOID  MEM_Buffer_Remove (NET_BUFFER_HEADER *hdr, NET_BUFFER *item);
VOID  MEM_Buffer_Cleanup (NET_BUFFER_HEADER *hdr);
VOID  MEM_Chain_Copy(NET_BUFFER *dest, NET_BUFFER *src, INT off, INT len);
VOID MEM_Buffer_Suspension_HISR (VOID);

/* tcp.c */
int16   TCP_Interpret (NET_BUFFER *buf_ptr, struct pseudotcp *tcp_chk);
STATUS  tcpsend (struct port *pport, NET_BUFFER*);
int16   tcp_sendack(struct port *pport);
int16   tcp_update_headers (struct port *, uint32, int16);
int16   tcp_retransmit(struct port *, int32);
VOID    TCP_Ackit(struct port *prt, INT force);
int16   tcp_ooo_packet(PORT *, int32);
VOID    check_ooo_list(PORT *);
STATUS  tcp_xmit(PORT *, NET_BUFFER *);
STATUS  TCP_Cleanup(struct port *prt);

/* protinit.c */
STATUS  protinit(VOID);
int16   makeport (VOID);
int16   makeuport (int16, INT);
uint16  get_unique_port_number(void);

/* sockets.c */
STATUS NU_Init_Net(VOID);
STATUS NU_Socket (int16, int16, int16);
STATUS NU_Bind (int16, struct addr_struct *, int16);
STATUS NU_Connect (int16, struct addr_struct *, int16);
STATUS NU_Listen (int16, uint16);
STATUS NU_Accept (int16, struct addr_struct *, int16 *);
STATUS NU_Send (int16, CHAR *, uint16, int16);
STATUS NU_Send_To (int16, char *, uint16, int16, struct addr_struct *, int16);
STATUS NU_Recv (int16, char *, uint16, int16);
STATUS NU_Recv_From (int16, char *, int16, int16, struct addr_struct *, int16 *);
STATUS NU_Push (int16);
STATUS NU_Is_Connected (int16);
int16  NU_GetPnum (struct sock_struct *);
int16  NU_Get_UDP_Pnum (struct sock_struct *);
STATUS NU_Fcntl (int16 socketd, int16 command, int16 arguement);
STATUS NU_Get_Host_By_Name(CHAR *name, NU_HOSTENT *host_entry);
STATUS NU_Get_Host_By_Addr(CHAR *, INT, INT, NU_HOSTENT *);
STATUS NU_Get_Peer_Name(int16, struct sockaddr_struct *, int16 *);
STATUS NU_Close_Socket (int16);
#ifdef PLUS
  VOID SCK_Suspend_Task(NU_TASK *);
#else
  VOID SCK_Suspend_Task(sint);
#endif
#ifdef PLUS
struct TASK_TABLE_STRUCT * NU_SearchTaskList(struct TASK_TABLE_STRUCT *,
                                             NU_TASK *, uint16, int16, int16);
#else
struct TASK_TABLE_STRUCT * NU_SearchTaskList(struct TASK_TABLE_STRUCT *,
                                             int, uint16, int16);
#endif
void    NU_UpdateTaskTable(struct TASK_TABLE_STRUCT *);
void    NU_TaskTableAdd(struct TASK_TABLE_STRUCT *);
STATUS  NU_TaskTable_Entry_Delete(int16);

STATUS  NU_Socket_Connected(int16);
STATUS  NU_Setsockopt(int16, INT, INT, void *, INT);
STATUS  NU_Getsockopt(int16, INT, INT, void *, INT *);
STATUS  NU_Abort(int16 socketd);
INT     SCK_Create_Socket(INT protocol);
STATUS  NU_Ioctl(INT optname, SCK_IOCTL_OPTION *option, INT optlen);
STATUS  NU_Add_Route(uint8 *dest, uint8* mask, uint8 *gw_addr);

/* select.c */
#ifdef PLUS
  STATUS NU_Select(INT, FD_SET *, FD_SET *, FD_SET *, UNSIGNED);
#else
  STATUS NU_Select(INT, FD_SET *, FD_SET *, FD_SET *, sint);
#endif
INT  NU_FD_Check(INT, FD_SET *);
VOID NU_FD_Set(int16, FD_SET *);
VOID NU_FD_Init(FD_SET *fd);
VOID NU_FD_Reset(INT, FD_SET *);
STATUS SEL_Check_Recv(INT, FD_SET *);
#ifdef PLUS
  INT SEL_Setup_Recv_Ports(INT, FD_SET *, NU_TASK *);
#else
  INT SEL_Setup_Recv_Ports(INT, FD_SET *, int16);
#endif
STATUS SEL_Enable_Timeout(int16, UNSIGNED);
STATUS SEL_Clear_Timeout(int16);

/* timer.c -- new double link list for the timer stuff, and will be used
   for the defragmentation later */
int16 tqpost (tqe_t *tqlist, tqe_t *tqe);

/* For more optimized data copies, we recommend using an assembly level
   routine in place of memmove and memcpy.  Such a routine is supplied
   with some ports of Nucleus NET. */
VOID *movebytes(VOID *, const VOID *, unsigned);

#define NU_Close_Driver    pketclose

#ifdef __cplusplus
}
#endif

#endif  /* EXTERNS_H */
