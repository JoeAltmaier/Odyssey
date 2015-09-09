/****************************************************************************//*                                                                          */
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
/*  FALFL                                                      1.0          */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*   This file contains the File Abstraction Layer functions.               */
/*   The functions in this file are split to handle the Nucleus             */
/*   file system and another file system.  At the current time the other    */
/*   file system is set to Microsoft's file system.  The user should use    */
/*   either the Nucleus File system or add their embedded file system in    */
/*   the other selection.                                                   */

/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*      Don Sharer Accelerated Technology Inc.                              */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*      None.                                                               */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*     FAL_fopen                                                            */
/*     FAL_fprintf                                                          */
/*     FAL_fclose                                                           */
/*     FAL_findnext                                                         */
/*     FAL_findfirst                                                        */
/*     FAL_fgets                                                            */
/*     FAL_remove                                                           */
/*     FAL_findclose                                                        */
/*     FAL_fwrite                                                           */
/*     FAL_access                                                           */
/*     FAL_mkdir                                                            */
/*     FAL_rmvdir                                                           */
/*     FAL_time                                                             */
/*     FAL_ctime                                                            */
/*     FAL_localtime                                                        */
/*     FAL_seek                                                             */
/*     FAL_fread                                                            */
/*     FAL_Is_Dir                                                           */
/*     FAL_Set_Curr_Dir                                                     */
/*     FAL_Current_Dir                                                      */
/*     FAL_rename                                                           */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*      NAME                            DATE            REMARKS             */
/*                                                                          */
/*                                                                          */
/****************************************************************************/

/*  Include header files */
#include "nucleus.h"

/*  File abstraction layer functions.  */
#include "ps_conf.h"
#ifndef FS_IN_MEMORY
#include "falfl.h"
#ifndef NUCLEUS_FILE_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <time.h>
#else
/*  Header files for Nucleus File */
#include "pcdisk.h"
#include "proto.h"
#endif

FAL_LOCAL *tm;
/*  Supports STDIO fopen and Nucleus File NU_Open */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_fopen                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   File open functions.  This function is used to open a file and      */
/*   return the file descriptor.                                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*   Application                                                         */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

FAL_FILE FAL_fopen(uint8 * path, UCOUNT flag, UCOUNT mode,FAL_FILE file)
{  
  int  error=0;
#ifdef NUCLEUS_FILE_INCLUDED
   /*  Remove Warnings */
   error= error;
   /*  Use NU_Open to open the file */
   file= NU_Open((TEXT *)path,flag,mode);
   return(file);
#else
   /*If not using Nucleus File put your code here */   
   /*  Must decode the flag to find out which function ot use */
   switch(flag)
   {
        case PO_RDONLY:
                  file = fopen(path,"r");
                  break;
        case PO_RDWR:
                  file = fopen(path,"rw");
                  break;
        case PO_WRONLY:
                  file = fopen(path,"w");
                  break;
        case PO_WRONLY|PO_CREAT:
                  file = fopen(path,"w");
                  break;                                 
        default:
                  error= -1;
                  break;
   }
   if (error != -1)
   {
      return(file);
   }
   else
       return(0);
   

#endif


}
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_fprintf                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Writes a string to the file.  A sprintf function must be         */
/*      on the file before calling the function.                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/*************************************************************************/

int32 FAL_fprintf(FAL_FILE file, char *string)
{

#ifdef NUCLEUS_FILE_INCLUDED
   /*  Nu_write the buffer with the size of the buffer */
   return((int32)NU_Write(file,(UTINY *)string,(COUNT)strlen(string)));
#else
    /*If not using Nucleus File put your code here */
   
    return(fprintf(file,string));
#endif


}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_fclose                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*   Closes a file with a particular file descriptor.  The iotype        */
/*   parameter is used to write an EOF(-1) to the end of the file.       */
/*   The iotype should be set a 1 when writing an EOF and a 0 when the   */
/*   file is to be only closed. Note only set iotype to a 1 when you are */
/*   writing data to a function.                                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*   Application                                                         */   
/*                                                                       */
/*************************************************************************/


BOOL FAL_fclose(FAL_FILE file, int iotype)
{
#ifdef NUCLEUS_FILE_INCLUDED
  CHAR msg=FAL_EOF;
  if(iotype)
     NU_Write(file,(UTINY *)&msg,1);

   NU_Close(file);
#else
   /*If not using Nucleus File put your code here */

   fclose(file);
#endif
   return(NU_SUCCESS);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_seek                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Seeks to the end of file or to a predetermined point with the    */
/*      current file.                                                    */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*   Application                                                         */   
/*                                                                       */
/*************************************************************************/


LONG FAL_seek(FAL_FILE file, LONG offset, COUNT origin)
{
#ifdef NUCLEUS_FILE_INCLUDED

    return(NU_Seek(file, offset, origin));
#else
   /*If not using Nucleus File put your code here */
   return(NU_SUCCESS);

   
#endif
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_findnext                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*     Function is used to find a particular file of a specific sequence */
/*     in a subdirectory.                                                */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*    Application                                                        */
/*                                                                       */
/*************************************************************************/
FAL_FIND FAL_findnext(FAL_DIR *statobj, int32 hFile)
{
  
#ifdef NUCLEUS_FILE_INCLUDED
   hFile=hFile;
   /*  Returns a pointer to get Next Object  */
   return(NU_Get_Next(statobj));
#else
   /*If not using Nucleus File put your code here */
   return((int32)_findnext((long)hFile,statobj));   
#endif

}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_findfirst                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Finds the first file in a subdirectory of a particular sequence. */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/*************************************************************************/
FAL_FIND FAL_findfirst(uint8 *path, FAL_DIR *statobj,int attrib)
{
    int32 hFile;
#ifdef NUCLEUS_FILE_INCLUDED
    /*  Remove annoying Warnings */
    hFile=hFile;
    attrib=attrib;
    /*  Get the First entry in the sequence  */
    return(NU_Get_First((DSTAT *)statobj,(TEXT *)path));
#else
   /*If not using Nucleus File put your code here */
    hFile=_findfirst(path, statobj);
    return(hFile);
#endif

}
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_fgets                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      The functions reads a specific amount of data and returns either */
/*      NULL or the character number of bytes.                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/*************************************************************************/
CHAR * FAL_fgets(uint8 *line, int16 size, FAL_FILE file)
{

#ifdef NUCLEUS_FILE_INCLUDED
   CHAR msg=0;
   uint8 count;
   CHAR string[512];
   count = 0;
   msg=0;
   /*  Remove Warnings  */
   size = size;
   /*  This function was written to search the file either an End of line
    *  marker or an EOF marker.
    */
   while ((!(msg == 0x0a)) && (!(msg ==FAL_EOF)) && (count != size))
   {
       NU_Read(file,(UTINY *)&msg,1);
       string[count] = msg;
       count++;
   }
   /*  Send back NULL if end of file marker found.  */
   if (msg == FAL_EOF)
      return(0);
   string[count]='\0';
   /*  Otherwise copy the string into line variable  */
   strcpy((char *)line,(char *)string);
   /*  Return a Non-Null character value */
   return((CHAR *)1);
#else
   /*If not using Nucleus File put your code here */
   return(fgets(line,size,file));
#endif

}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_remove                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Removes the file that is referenced in the given path.           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/*************************************************************************/
BOOL FAL_remove(uint8 *name)
{
#ifdef NUCLEUS_FILE_INCLUDED
   /*  Calls NU_Delete to delete the given file name  */
   return(NU_Delete((TEXT *)name));
#else
   /*If not using Nucleus File put your code here */
   remove(name);
   return(1);
#endif
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_findclose                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function used to free all external and internal resources used   */
/*      with the FAL_findfirst and FAL_findnext functions.               */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/*************************************************************************/

BOOL FAL_findclose(long hfile,FAL_DIR *statobj )
{
#ifdef NUCLEUS_FILE_INCLUDED
   /*  Remove Warnings */
   hfile=hfile;
   /*  Call NU_Done to free the associated resources.  */
   NU_Done(statobj);
   return(NU_SUCCESS);
#else
    /*If not using Nucleus File put your code here */
    /*  Remove Warnings */
    statobj= statobj;
   _findclose(hfile);
   return(NU_SUCCESS);
#endif
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_fwrite                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to write a number of bytes to a file.                   */
/*      The file is size times count.                                    */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*     Application                                                       */
/*                                                                       */
/*************************************************************************/

uint32 FAL_fwrite(uint8 *buf, UCOUNT size, int32 count, FAL_FILE file)
{
#ifdef NUCLEUS_FILE_INCLUDED
   /*  Nu_Write write the number of bytes by size * count */
   return((uint32)NU_Write(file,buf,(UCOUNT)(size * count)));
#else
   /*If not using Nucleus File put your code here */
   return(fwrite(buf, size, count, file));
#endif
}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_fread                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to read a number of bytes from a file.                  */
/*      The file is size times count.                                    */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*     Application                                                       */
/*                                                                       */
/*************************************************************************/

uint32 FAL_fread(uint8 *buf, UCOUNT size, int32 count, FAL_FILE file)
{
#ifdef NUCLEUS_FILE_INCLUDED
   /*  Nu_Write write the number of bytes by size * count */
   return((uint32)NU_Read(file,buf,(UCOUNT)(size * count)));
#else
   /*If not using Nucleus File put your code here */
   return(fread(buf, size, count, file));
#endif
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_access                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*     This function checks for access to a certain path.                */
/*     Note:  Nucleus File does not have a unique function for           */
/*            this capability.                                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/*************************************************************************/

int FAL_access(uint8 *mb)
{
#ifdef NUCLEUS_FILE_INCLUDED

   /*  File does not have an access function so it is not used */
   mb=mb;
   return(0);
#else
   /*If not using Nucleus File put your code here */
   return(_access((const char *)mb, 0));
#endif
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_mkdir                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Creates a directory with the specified path.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/*************************************************************************/

int32 FAL_mkdir(uint8 *mb)
{
#ifdef NUCLEUS_FILE_INCLUDED
   return((int32)NU_Make_Dir((TEXT *)mb));
#else
   /*If not using Nucleus File put your code here */
   _mkdir((unsigned char *)*mb);
   return(0);
#endif
}
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_rmvdir                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Removes a directory with the specified path.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/*************************************************************************/
int32 FAL_rmvdir(uint8 *path)
{
#ifdef NUCLEUS_FILE_INCLUDED
   return((int32)NU_Remove_Dir((TEXT *)path));
#else
   /*If not using Nucleus File put your code here */
   return((int32)_rmdir((const char *)path);
#endif
}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_Is_Dir                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Verifies if the current path is a directory or not.              */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/*************************************************************************/
int32 FAL_Is_Dir(uint8 *path)
{
#ifdef NUCLEUS_FILE_INCLUDED
   return((int32)NU_Is_Dir((TEXT *)path));
#else
   /*If not using Nucleus File put your code here */
   
#endif
}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_Set_Curr_Dir                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Sets the directory path to the current directory.                */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/*************************************************************************/

int32 FAL_Set_Curr_Dir(uint8 *path)
{
#ifdef NUCLEUS_FILE_INCLUDED
   return((int32)NU_Set_Current_Dir((TEXT *)path));
#else
   /*If not using Nucleus File put your code here */
   return((int32)_rmdir((const char *)path);
#endif
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_Current_Dir                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Finds the current working directory and places it in the path    */
/*      variable.                                                        */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/*************************************************************************/

int32 FAL_Current_Dir(uint8 *drive, uint8 *path)
{
#ifdef NUCLEUS_FILE_INCLUDED
   return((int32)NU_Current_Dir((TEXT *)drive,(TEXT *)path));
#else
   /*If not using Nucleus File put your code here */
   
#endif
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_rename                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Renames the file in the current name to a new name given in      */
/*      rename path.                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/*************************************************************************/

int32 FAL_rename(uint8 *rename_path, uint8 *curr_name)
{
#ifdef NUCLEUS_FILE_INCLUDED
   return((int32)NU_Rename((TEXT *)rename_path,(TEXT *)curr_name));
#else
   /*If not using Nucleus File put your code here */
   
#endif
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_time                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Time function-  The user will have to implement their own time   */
/*      data functions.                                                  */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      netsleep                                                         */
/*      NU_EventDispatcher                                               */
/*      timer_task                                                       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      *receive function for the device                                 */
/*                                                                       */
/*************************************************************************/

FAL_TIME *FAL_time( FAL_TIME *ltime )
{
#ifdef NUCLEUS_FILE_INCLUDED
   /*  User must implement time functions for his poarticular platform */
   return(ltime);
#else
   /*If not using Nucleus File put your code here */   
   return((long *)time((long *)&ltime));
#endif
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_ctime                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Time function-  The user will have to implement their own time   */
/*      data functions.                                                  */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/*************************************************************************/

CHAR * FAL_ctime( FAL_TIME *ltime )
{
#ifdef NUCLEUS_FILE_INCLUDED
   /*  A time function will need to be imlemented for you particular
       target.  This function returns a bogus value */
       /*  Remove Warnings */
       ltime=ltime;
       return((char *)1);
#else
   /*If not using Nucleus File put your code here */
   return(ctime((const long *)&ltime));
#endif
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      FAL_localtime                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Time function-  The user will have to implement their own time   */
/*      data functions.                                                  */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/*************************************************************************/
FAL_LOCAL *FAL_localtime( FAL_TIME *timer, FAL_LOCAL *tm )
{

#ifdef NUCLEUS_FILE_INCLUDED
    /*  the Timer functions will have to be implemented for you particular target
     *  A structure was was provided in falfl.h function to put your time values in */
    
    

    timer= timer;
    tm->tm_hour  = 11;
    tm->tm_min   = 36;
    tm->tm_sec   = 22;
    tm->tm_mon   = 5;
    tm->tm_mday  = 25;
    tm->tm_year  = 98;
    return(tm);
#else
   /*If not using Nucleus File put your code here */
   return(localtime((const long *)&timer));
#endif
}

#endif /* FS_IN_MEMORY */