/****************************************************************************/
/*                                                                          */
/*    CopyrIght (c)  1993 - 1998 Accelerated Technology, Inc.               */
/*                                                                          */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the subject */
/* matter of this material.  All manufacturing, reproduction, use and sales */
/* rights pertaining to this subject matter are governed by the license     */
/* agreement.  The recipient of this software implicity accepts the terms   */
/* of the license.                                                          */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* FILENAME                                                 VERSION         */
/*                                                                          */
/*  FALFL.H                                                    1.0          */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*   This file contains the File Abstraction Layer function prototypes      */
/*   a miscellaneus defines.                                                */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*      Don Sharer Accelerated Technology Inc.                              */
/*                                                                          */
/****************************************************************************/

#ifndef FALFL_H
#define FALFL_H
/*  Include Standard header files */
/*  Comment this line out if you are not using Nucleus File */
#include "ps_conf.h"
#ifndef FS_IN_MEMORY
#ifndef NUCLEUS_FILE_INCLUDED
#define NUCLEUS_FILE_INCLUDED
#endif

#include "protocol.h"
#include "ip.h"
#include "target.h"
#include "nucleus.h"

#ifndef NUCLEUS_FILE_INCLUDED
/*  If not using Nucleus File put your header files for your */
/*  embedded file system here.  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <time.h>
#else
/*  Using Nucleus File   */
#include "pcdisk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

/* File access flags */
#define PO_RDONLY 0x0000                /* Open for read only*/
#define PO_WRONLY 0x0001                /* Open for write only*/
#define PO_RDWR   0x0002                /* Read/write access allowed.*/
#define PO_APPEND 0x0008                /* Seek to eof on each write*/
#define PO_CREAT  0x0100                /* Create the file if it does not exist.*/
#define PO_TRUNC  0x0200                /* Truncate the file if it already exists*/
#define PO_EXCL   0x0400                /* Fail if creating and already exists*/
#define PO_TEXT   0x4000                /* Ignored*/
#define PO_BINARY 0x8000                /* Ignored. All file access is binary*/
#define PO_NOSHAREANY   0x0004          /* Wants this open to fail if already
                                           open.  Other opens will fail while
                                           this open is active */
#define PO_NOSHAREWRITE 0x0800          /* Wants this opens to fail if already
                                           open for write. Other open for
                                           write calls will fail while this
                                           open is active. */
#define PS_IWRITE 0000400               /*  Write Mode for Nucleus File */
#define PS_IREAD  0000200


typedef struct timev
{
   int tm_hour;
   int tm_min;
   int tm_sec;
   int tm_mon;
   int tm_mday;
   int tm_year;
} tm_time;

#ifndef NUCLEUS_FILE_INCLUDED
/*  If not using Nucleus File place your data structures and type defines here */
#define FAL_FILE       FILE *
#define FAL_DIR        struct _finddata_t 
#define FAL_TIME       time_t
#define FAL_LOCAL      struct tm
#define ULONG           unsigned long int       /* 32 BIT unsigned */
#define LONG            long int                /* 32 BIT signed   */
#define UCOUNT          unsigned short int      /* 16 BIT unsigned */
#define COUNT           short int               /* 16 BIT unsigned */
#define BOOL            int                     /* native int      */
#define INT             int                     /* native int      */
#define VOID            void                    /* void            */
#define UTINY           unsigned char           /* 8  BIT unsigned */
#define UTEXT           unsigned char                                                /* 8  BIT signed   */
#define BLOCKT          unsigned long int       /* 32 BIT unsigned */
#define PCFD            int                     /* file desc       */
/*  File Initialize Value */
#define FILE_CHECK      0
/*  Type cast for findfirst and next function  */
#define FAL_FIND        int32
/*  Return value if file findfirst fails       */
#define FIND_VALUE     -1L
/*  Return  Value if FAL_FIND Function Passes  */
#define FIND_NEXT_VL    0
#define REN_RESULT      0
#define RESULT          0
#else

#define __RAM "A:"
/*  Set Nucleus File Descriptor type */
#define FAL_FILE       PCFD
/* Set Nucleus File structure type   */
#define FAL_DIR        DSTAT
/* Set Nucleus File Time value       */
#define FAL_TIME       UNSIGNED
/* Set local time type               */
#define FAL_LOCAL      tm_time
/* File Initialized Value For Nucleus File */
#define FILE_CHECK     -1
/* Type cast for File find function        */
#define FAL_FIND       BOOL
/* End of File Value for Nucleus File    */
#define FAL_EOF        -1
/*  Return value if NU_Get_First fails   */
#define FIND_VALUE     NO
/* Return value if NU_Get_First or Next passes */
#define FIND_NEXT_VL   YES
#define REN_RESULT     YES
#define RESULT         YES
#endif


/* Function Prototypes */
FAL_FILE FAL_fopen(uint8 *path, UCOUNT flag, UCOUNT mode, FAL_FILE file);
int32 FAL_fprintf(FAL_FILE file, char *string);
BOOL FAL_fclose(FAL_FILE file, int iotype);
FAL_FIND FAL_findnext(FAL_DIR *statobj, long hfile);
FAL_FIND FAL_findfirst(uint8 *path, FAL_DIR *statobj, int attrib);
CHAR * FAL_fgets(uint8 *line, int16 size, FAL_FILE file);
BOOL FAL_remove(uint8 *name);
BOOL FAL_findclose(long hfile,FAL_DIR *statobj );
uint32 FAL_fwrite(uint8 *buf, UCOUNT size, int32 count, FAL_FILE file);
int FAL_access(uint8 *mb);
int32 FAL_mkdir(uint8 *mb);
int32 FAL_rmvdir(uint8 *path);
FAL_TIME *FAL_time( FAL_TIME *ltime );
CHAR * FAL_ctime( FAL_TIME *ltime );
FAL_LOCAL *FAL_localtime( FAL_TIME *timer,FAL_LOCAL *time );
LONG FAL_seek(FAL_FILE file, LONG offset, COUNT origin);
uint32 FAL_fread(uint8 *buf, UCOUNT size, int32 count, FAL_FILE file);
int32 FAL_Current_Dir(uint8 *drive, uint8 *path);
int32 FAL_Set_Curr_Dir(uint8 *path);
int32 FAL_Is_Dir(uint8 *path);
int32 FAL_rename(uint8 *rename_path, uint8 *curr_name);
#endif  /* FS_IN_MEMORY */
#endif  /*  FALFL_H */
