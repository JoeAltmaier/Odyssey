/* MipsIO.H -- C Runtime I/O Support for MIPS/Metrowerks
 *
 * Copyright ConvergeNet Technologies (c) 1998 
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * Revision History:
 *     9-28-98  TRN  Create for MIPS I/O
 *	   02/12/99 JFL	#if 0d puts, printf, sprintf, and putchar it caused a 
 *					They caused duplicate def's.
 *
**/

#if !defined(MipsIO_H)
#define MipsIO_H

#include "SccDriver.h"

#ifdef __cplusplus
extern "C" {
#endif

// Prototypes

void _putch(char ch); 
int _kbhit(void);
char _getch(void);

#if 0
int puts(char *pszLine);
int printf(char *pFmt, ...);
int sprintf(char *pStr, char *pFmt, ...);
#define putchar	_putch
#endif

/* Debug functions taken from old SerDrv.c module */
/**TRN** Modified to reference SCC Driver */
void Serial_Init(void);
BOOL Char_Present(void);
char Get_Char(void);
void Print_Char(char ch);

void Print_String(char *pStr);	/* Required by Nucleus.lib */

//void    Convert_Number(char *string, UNSIGNED number);
char*   Convert_Number (char *st, unsigned int number, int base, int length, char fPad);
void    exit(int status);

void    Print_Base (int number, int base, int length, char fPad);
void    Print_Dec(unsigned short number);
void    Print_04x(unsigned short number);
void    Print_08x(unsigned long number);
void    Print_StringAt(int row, int col, char *string);
void	Print_Dump(unsigned long *p, int cb);

#ifdef __cplusplus
}
#endif

#endif // MipsIO_H
