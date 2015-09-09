/************************************************************************/
/*                                                                      */
/*            Copyright (c) 1994-1998 Accelerated Technology, Inc.      */
/*                                                                      */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the     */
/* subject matter of this material.  All manufacturing, reproduction,   */
/* use, and sales rights pertaining to this subject matter are governed */
/* by the license agreement.  The recipient of this software implicitly */
/* accepts the terms of the license.                                    */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* FILE NAME                                            VERSION         */
/*                                                                      */
/*      dbt.c                                           DBUG+  1.2      */
/*                                                                      */
/* COMPONENT                                                            */
/*                                                                      */
/*      DEBUG       Nucleus PLUS Debugger                               */
/*                                                                      */
/* DESCRIPTION                                                          */
/*                                                                      */
/*      This file contains Nucleus debugger routines.                   */
/*                                                                      */
/* AUTHOR                                                               */
/*                                                                      */
/*      David L. Lamie, Accelerated Technology, Inc.                    */
/*                                                                      */
/* DATA STRUCTURES                                                      */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* FUNCTIONS                                                            */
/*                                                                      */
/*      DBT_Get_Char                                                    */
/*      DBT_Put_Char                                                    */
/*      DBT_ASCII_To_Integer                                            */
/*      DBT_HEX_ASCII_To_Long                                           */
/*      DBT_Name_Compare                                                */
/*      DBT_String_Compare                                              */
/*      DBT_String_Cat                                                  */
/*                                                                      */
/* DEPENDENCIES                                                         */
/*                                                                      */
/*      stdio.h                             Debugger Service functions  */
/*      stdlib.h                            Debugger Service functions  */
/*      string.h                            Debugger Service functions  */
/*      nucleus.h                           Nucleus PLUS functions      */
/*      db_defs.h                           Debugger Constants          */
/*      db_extr.h                           Debugger Service functions  */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*      MQ Qian         01-20-1995      Verified version 1.2            */
/*      D. Sharer       04-14-1998      Removed Warnings                */
/*      P. Hill         05-07-1998      Cleaned up externs              */
/*                                                                      */
/************************************************************************/

/* Include the necessary files.                                         */

#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  "nucleus.h"
#include  "db_defs.h"
#include  "db_extr.h"

#include  "sockdefs.h"
#include "n_ansi.h"
#include "protocol.h"
#include "windat.h"
#include "externs.h"
#include "tel_extr.h"

/* the following two MACROs is needed to adjust the size of output to match
    the size of terminal of client */
#undef  COL
#undef  ROW
#define COL his_side.width
#define ROW his_side.rows

/* we need set the maximun row and column for global variables */
#define  MAXCOL  85
#define  MAXROW  25

extern int16 telnet_socket;

/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "DBT_Get_Char"              */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function is used to pick up char from STDIN                */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Input_Line                                                  */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      NU_Telnet_Get_Filtered_Char                                     */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      Returns character                                               */
/*                                                                      */
/*  HISTORY                                                             */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*      MQ Qian         01-20-1995      Added telnet stuuff             */
/*                                                                      */
/************************************************************************/
char  DBT_Get_Char()
{
char  alpha, block=1;

#ifdef NETWORK
    /* get a character from telnet client and block flag must be set, here */
    alpha = NU_Telnet_Get_Filtered_Char(telnet_socket, block);

    if (tn_session[telnet_socket]->echo && alpha!=ERR_RETURN)
        DBT_Put_Char(alpha);
#else
     /* Simple polling to see if a character is present.  */
     while (!kbhit())
     {

         /* Sleep to free up time for the other processes.  */
         NU_Sleep(3);
     }

    /* Character is present - get a character.  */
    alpha =  (char) getch();

    /* Determine if we need to echo the character back to the user.  */
    #ifdef DUPLEX
         DBT_Put_Char(alpha);
    #endif
#endif

    /* Return character to the caller.  */
    return(alpha);
}



/************************************************************************/
/*                                                                      */
/*  FUNCTION                           "DBT_Put_Char"                   */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function will print a line, character by character.        */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      DBC_Print_Line(char *string)                                    */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      NU_Send                                                         */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      alpha                               Character to print          */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      Character to STDOUT                                             */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*      MQ Qian         01-20-1995      Added telnet stuuff             */
/*                                                                      */
/************************************************************************/
void DBT_Put_Char(char alpha)
{
#ifdef NETWORK
    if (alpha=='\n')
        NU_Send(telnet_socket, "\n\r", 2, 0);
    else if (alpha!='\r')
        NU_Send(telnet_socket, &alpha, 1, 0);
#else
    putch(alpha);                        /* Prints value of alpha       */
#endif
}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                           "DBT_ASCII_To_Integer"           */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function will attempt to convert the input string into     */
/*      an integer and place the new integer in the specified location. */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      Various DBC routines                                            */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      atoi                                ASCII to integer            */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      string                              String to convert           */
/*      num_ptr                             Pointer to integer dest.    */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      return(DB_SUCCESS)                  If successful conversion    */
/*      return(DB_ERROR)                    If an error exists          */
/*      *num_ptr                            Converted integer value     */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
int  DBT_ASCII_To_Integer(char *string, int *num_ptr)
{

int  i;
int  status;

    /* Check for a sign character at the beginning of the string.  */
    if ((string[0] == '+') || (string[0] == '-'))

       /* Start checking at the next character.  */
       i =  1;
    else

       /* Start checking right at the beginning.  */
       i =  0;

    /* Check to see if the string contains a valid ASCII representation of
       an integer.  */
    do {

        /* Check for a valid character.  */
        if ((string[i] < '0') || (string[i] > '9'))

            /* An error is present.  */
            status =  DB_ERROR;
        else

            /* Everything is still okay.  */
            status =  DB_SUCCESS;

        /* Increment the character index in the string.  */
        i++;
    }  while ((i <= COL) && (string[i] != NUL) && (status == DB_SUCCESS));

    /* If the string represents an ASCII number convert it.  */
    if (status == DB_SUCCESS)

        /* Call the library routine "atoi" to convert the integer.  */
        *num_ptr =  atoi(string);

    /* Return status to the caller.  */
    return(status);
}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                           "DBT_HEX_ASCII_To_Long"          */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function will attempt to convert the input string into     */
/*      an unsigned long location specified in the call.                */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      Various DBC routines                                            */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      string                              String to convert           */
/*      num_ptr                             Pointer to integer dest.    */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      return(DB_SUCCESS)                  If successful conversion    */
/*      return(DB_ERROR)                    If an error exists          */
/*      *num_ptr                            Converted ulong value       */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
int  DBT_HEX_ASCII_To_Long(char *string, unsigned long *num_ptr)
{

int            i;
int            status;
unsigned long  number;

    /* Since there is no sign bit for a HEX number, start making the 
       conversion immediately.  */

    /* Check to see if the string contains a valid ASCII representation of
       a HEX unsigned long.  */
    i =       0;
    status =  DB_SUCCESS;
    number =  0;
    do 
    {

        /* Check for a valid character.  */
        if ((string[i] >= '0') && (string[i] <= '9'))
        {
        
            /* A normal HEX number is present.  */
            number =  (number << 4) + (string[i] - '0');
        }
        else if ((string[i] >= 'a') && (string[i] <= 'f'))
        {
        
            /* A lower-case HEX digit is present.  */
            number =  (number << 4) + (string[i] - 'a' + 10);
        }
        else if ((string[i] >= 'A') && (string[i] <= 'F'))
        {
        
            /* A lower-case HEX digit is present.  */
            number =  (number << 4) + (string[i] - 'A' + 10);
        }
        else
        {
           
            /* An error is present.  */
            status =  DB_ERROR;
            i--;
        }

        /* Increment the character index in the string.  */
        i++;

    }  while ((i <= COL) && (string[i] != NUL) && (status == DB_SUCCESS));

    /* See if the digit exceeded the size limit.  (8 digits).  */
    if (i > 8)
    {
    
        /* An error is present.  */
        status =  DB_ERROR;
    }

    /* If the string represented an ASCII HEX number copy the converted
       number into the destination.  */
    if (status == DB_SUCCESS)

        /* Copy the converted number into the destination.  */
        *num_ptr =  number;

    /* Return status to the caller.  */
    return(status);
}

/************************************************************************/
/*                                                                      */
/*  FUNCTION                           "DBT_Name_Compare"               */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function compares the two supplied strings.  If the they   */
/*      are equal, a DB_TRUE is returned.  Otherwise, a DB_FALSE is     */
/*      returned.                                                       */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      Various DBC routines                                            */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      string1                             First string to compare     */
/*      string2                             Second string to compare    */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      return(DB_TRUE)                     If the strings are equal    */
/*      return(DB_FALSE)                    If the strings are not equal*/
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
int  DBT_Name_Compare(char *string1, char *string2)
{
int     j,i,k;

    i = 0;
    j = 0;

    while (string1[i] == ' ')
        i++;
        
    while (string2[j] == ' ')
        j++;

    /* Comapare two strings.  */
    k = 0;
    while (((k < MAX_NAME) && (string1[i] == string2[j])) && 
                                (string1[i] != NUL))
    {
        i++;
        j++;
        k++;
    }

    if (k == MAX_NAME)
        /* Strings are equal, return a DB_TRUE.  */
        return(DB_TRUE);

    else if ((string1[i] == NUL) && (string2[j] == NUL))

        /* Strings are equal, return a DB_TRUE.  */
        return(DB_TRUE);

    else if ((string1[i] == ' ') && (string2[j] == NUL))

        /* Strings are equal, return a DB_TRUE.  */
        return(DB_TRUE);

    else if ((string2[j] == ' ') && (string1[i] == NUL))

        /* Strings are equal, return a DB_TRUE.  */
        return(DB_TRUE);

    else
         /* Strings are not equal, return a DB_FALSE.  */
         return(DB_FALSE);
}

/************************************************************************/
/*                                                                      */
/*  FUNCTION                           "DBT_String_Compare"             */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function compares the two supplied strings.  If the they   */
/*      are equal, a DB_TRUE is returned.  Otherwise, a DB_FALSE is     */
/*      returned.                                                       */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      Various DBC routines                                            */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      strncmp                              String compare library     */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      string1                             First string to compare     */
/*      string2                             Second string to compare    */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      return(DB_TRUE)                     If the strings are equal    */
/*      return(DB_FALSE)                    If the strings are not equal*/
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
int  DBT_String_Compare(char *string1, char *string2)
{

    /* Comapare two strings.  */
    if (strncmp(string1, string2, COL) == 0)

        /* Strings are equal, return a DB_TRUE.  */
        return(DB_TRUE);
    else

         /* Strings are not equal, return a DB_FALSE.  */
         return(DB_FALSE);
}

/************************************************************************/
/*                                                                      */
/*  FUNCTION                           "DBT_String_Cat"                 */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function appends the source string to the end of the       */
/*      destination string.                                             */
/*                                                                      */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      David L. Lamie,  Accelerated Technology                         */
/*                                                                      */
/*  CALLED BY                                                           */
/*                                                                      */
/*      Various DBC routines                                            */
/*                                                                      */
/*  CALLS                                                               */
/*                                                                      */
/*      strcat                              String append library funct */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      dest                                Destination string          */
/*      source                              String to append            */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      None                                                            */
/*                                                                      */
/* HISTORY                                                              */
/*                                                                      */
/*         NAME            DATE                    REMARKS              */
/*                                                                      */
/*      D. Lamie        07-15-1993      Created initial version 1.0     */
/*      D. Lamie        08-22-1993      Created version 1.0a            */
/*      D. Lamie        09-15-1994      Created version 1.1             */
/*                                                                      */
/*      R. Pfaff        09-16-1994      Verified version 1.1            */
/*                                                                      */
/************************************************************************/
void  DBT_String_Cat(char *dest, char *source)
{

    /* Append source string to destination string.  */
    strcat(dest,source);
}



