/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: diags.c
// 
// Description:
// This file contains miscellaneous terminal and string ui subs.
// 
// Update Log 
// 
// 06/23/99 Bob Weast: Create file
// 
/*************************************************************************/

#include "diags.h"


/*****************************/
// Externals
/*****************************/


/*****************************/
// Forward References
/*****************************/
#ifdef CONFIG_BOOT
int  getKey(void);
#endif
int  AskYN(char*);
void Beep(void);
U64  getHex(void);
void Gripe(void);
U64  htoll(char*);
U32  ImpatientWheel(U32);
int  isdigit(U8);
U32  less(U32);


/*
 * Prompts the user with str and returns
 * YES or NO
 */
int AskYN(char *str)
{
   int c;

   printf("%s (Y/N) ", str);
   while (1) {
      c = GET_KEY();
      if (c == 'y' || c == 'Y')
         return (YES);
      else if (c == 'n' || c == 'N' || c == NEWLINE)
         break;
      else Gripe();
   }
   return (NO);
}


void Beep(void)
{
   putchar(7);
}


void Gripe(void)
{
   Beep();
   printf(bspace);
}


U32 getDec(void)
{
   char c, str[20];
   int i = 0;
   U32 val;

   printf(": ");
   while ((c = GET_KEY()) != NEWLINE) {
      if (c == BACKSPACE) {
         if (i)
            i--;
         else putchar(' ');
         printf(" \b");
      }
      else if (isdigit(c))
         str[i++] = c;
      else Gripe();
      if (i > 10) {
         putchar(7);
         break;
      }
   }
   str[i] = 0;
   val = atoi(str);
   return (val);
}


U64 getHex(void)
{
   char c, str[20];
   int i = 0;
   U64 val;

   printf(": ");
   while ((c = GET_KEY()) != NEWLINE) {
      if (c == BACKSPACE) {
         if (i)
            i--;
         else putchar(' ');
         printf(" \b");
      }
      else if (ishex(c))
         str[i++] = c;
      else Gripe();
      if (i > 16) {
         putchar(7);
         break;
      }
   }
   str[i] = 0;
   val = htoll(str);
   return (val);
}

#ifdef CONFIG_BOOT
/* hack for Nucleized getchar, use old version */
int getKey(void)
{
   int   ch;

   //  spin (!) until a char is ready to read
   while (! kbhit ())
      {}

   //  read it
   ch = ttyA_in();

   if (ch == '\r')
      {
      //  user pressed "enter", map this into our standard C newline char
      ch = '\n';
      }

   //  echo char back to console
   putchar (ch);

   //  and let caller know what it is  :-)
   return (ch);

}
#endif


void getAnyKey(void)
{
   int c;

   printf(pressAnyKey);
   c = GET_KEY();
}


void getReturn(void)
{
   int c;

   printf(pressReturn);
   while (1) {
      c = GET_KEY();
      if (c == '\n') break;
      Gripe();
   }
}


void flushKbd(void)
{
   int c;

   while (kbhit())
      c = GET_KEY();
}


U64 htoll(char *p)
{
   U64 val = 0;
   U8 nib;
   
   for (; isspace (*p); p++)
          ;
   for (; (*p); p++) {
      if (*p >= '0' && *p <= '9')
         nib = *p - '0';
      else if (*p >= 'A' && *p <= 'F')
         nib = *p - 'A' + 10;
      else if (*p >= 'a' && *p <= 'f')
         nib = *p - 'a' + 10;
      else
         break;
      val = (val << 4) + nib;
   }
   return (val);
}


U32 ImpatientWheel(U32 prt)
{
   int c = 0;

   if (kbhit()) {
      c = GET_KEY();
      if (c == ESC) {
         /*
          * Eat all keystrokes, user may have
          * been hammering escape key trying
          * to get out.
          */
         flushKbd();
         sheSaidQuit = YES;
         return (1);
      }
      else Gripe();
   }
   wheel(prt);
   return (0);
}


#ifdef CONFIG_BOOT
/* where is standard c stuff? */
int isdigit(U8 c)
{
   return (c >= '0' && c <= '9' ? 1 : 0);
}
#endif


U32 less(U32 position)
{
   int c;

   if (!position)
      printf(lessq);

   while (1) {

      if (position)
         printf("\033[16;18H\033[K Show more? "); /* tools only */

      c = GET_KEY();
      if (c == 'Y' || c == ' ' ||
          c == 'y' || c == '\n') {
          if (!position)
             printf(nl);
          return (YES);
      }
      else if (c == 'q' || c == 'n' ||
               c == 'Q' || c == 'N')
         break;
      else Gripe();
   }

   if (position)
      printf(prompt);

   return (NO);
}
