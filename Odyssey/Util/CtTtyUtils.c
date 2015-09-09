/*************************************************************************/
// File: CtTtyUtils.c
// 
// Description:
//
// 
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
//
//*************************************************************************
// Update Log 
//	$Log: /Gemini/Odyssey/Util/CtTtyUtils.c $
// 
// 3     1/26/00 2:46p Joehler
// Added ttyport_* string processing functionality.
// 
// 2     12/09/99 4:12p Joehler
// 
/*************************************************************************/


/*************************************************************************/

#include "CtTtyUtils.h"
#include "Pci.h"
#include "Odyssey_Trace.h"
#include "string.h"
#include "cstdlib"

/*************************************************************************/
// Globals
/*************************************************************************/

// useful ascii escape sequences
char		*clreol = "\033[K";                /* clear to the end of the line */
char		*clrscn = "\033[H\033[2J\033[0m";  /* clear the screen */
char		*bspace = "\033[D \033[D";         /* backspace */

void
printf_at (int row, int col, char *string)
{
    printf("\033[%d;%dH%s", row, col, string);

}

void
ttyport_printf_at (I32 port, int row, int col, char *string)
{
    ttyport_printf(port, "\033[%d;%dH%s", row, col, string);

}

void
Print_Dump(unsigned long *p, int cb)
{
   int i;
   int j;
   for (i=0; i < cb; i += 4)
   {
      printf("%04x", i*4 + (((int)p) & 0xFFFF));
      printf(": ");
      for (j=0; j < 4; j++)
      {
        char fNMI=(((int)p & 0xFFFF) == 0xC00 && (i+j==12 || i+j == 61));
        unsigned long l=(fNMI? 0 :p[i+j]);
        if (fNMI)
	        printf("XXXXXXXX");
	    else
			printf("%08x", GTREG(l));
      	putchar(' ');
      }
      printf("\r\n");
   }
}

void
ttyport_Print_Dump(I32 port, unsigned long *p, int cb)
{
   int i;
   int j;
   for (i=0; i < cb; i += 4)
   {
      ttyport_printf(port, "%04x", i*4 + (((int)p) & 0xFFFF));
      ttyport_printf(port, ": ");
      for (j=0; j < 4; j++)
      {
        char fNMI=(((int)p & 0xFFFF) == 0xC00 && (i+j==12 || i+j == 61));
        unsigned long l=(fNMI? 0 :p[i+j]);
        if (fNMI)
	        ttyport_printf(port, "XXXXXXXX");
	    else
			ttyport_printf(port, "%08x", GTREG(l));
      	ttyport_putchar(port, ' ');
      }
      ttyport_printf(port, "\r\n");
   }
}

/*************************************************************************/
// 	Get_Hex
// Print message to prompt user, then Get hex number from keyboard.
/*************************************************************************/
UI64 Get_Hex(char *p_message)
{
	U32 invalid_digit = 1;
	UI64 number = 0;
	char next_char = 0;
	UI64 next_value;
	while(invalid_digit)
	{
		Tracef(p_message);
		invalid_digit = 0;
		while (invalid_digit == 0)
		{
			next_char = getchar();
			switch (next_char)
			{
				case '\n':
					return number;
				case '\r':
					return number;
				case '0': next_value = 0; break;
				case '1': next_value = 1; break;
				case '2': next_value = 2; break;
				case '3': next_value = 3; break;
				case '4': next_value = 4; break;
				case '5': next_value = 5; break;
				case '6': next_value = 6; break;
				case '7': next_value = 7; break;
				case '8': next_value = 8; break;
				case '9': next_value = 9; break;
				case 'a': case 'A': next_value = 10; break;
				case 'b': case 'B': next_value = 11; break;
				case 'c': case 'C': next_value = 12; break;
				case 'd': case 'D': next_value = 13; break;
				case 'e': case 'E': next_value = 14; break;
				case 'f': case 'F': next_value = 15; break;
				default:
					invalid_digit = 1;
					Tracef("\nInvalid digit");
					break;
			}
			number = number * 16 + next_value;
		}
	}
	
	return number;
}

/*************************************************************************/
// 	ttyport_Get_Hex
// Print message to prompt user, then Get hex number from keyboard.
/*************************************************************************/
UI64 ttyport_Get_Hex(I32 port, char *p_message)
{
	U32 invalid_digit = 1;
	UI64 number = 0;
	char next_char = 0;
	UI64 next_value;
	while(invalid_digit)
	{
		ttyport_printf(port, p_message);
		invalid_digit = 0;
		while (invalid_digit == 0)
		{
			next_char = ttyport_getchar(port);
			switch (next_char)
			{
				case '\n':
					return number;
				case '\r':
					return number;
				case '0': next_value = 0; break;
				case '1': next_value = 1; break;
				case '2': next_value = 2; break;
				case '3': next_value = 3; break;
				case '4': next_value = 4; break;
				case '5': next_value = 5; break;
				case '6': next_value = 6; break;
				case '7': next_value = 7; break;
				case '8': next_value = 8; break;
				case '9': next_value = 9; break;
				case 'a': case 'A': next_value = 10; break;
				case 'b': case 'B': next_value = 11; break;
				case 'c': case 'C': next_value = 12; break;
				case 'd': case 'D': next_value = 13; break;
				case 'e': case 'E': next_value = 14; break;
				case 'f': case 'F': next_value = 15; break;
				default:
					invalid_digit = 1;
					ttyport_printf(port, "\nInvalid digit");
					break;
			}
			number = number * 16 + next_value;
		}
	}
	
	return number;
}

U32 char_to_num( char *pDigits )
{
U32	number = 0;

	while ((*pDigits >= '0') && (*pDigits <= '9'))
		number = number * 10 + (*pDigits++ - '0');
	return number;
}


U32 Get_U32(char *p_message)
{

	U32 invalid_digit = 1;
	U32 number = 0;
	char next_char = 0;
	U32 next_value;
	#define	MAX_DIGITS 3
	char Digits[MAX_DIGITS+1] = {0};
	U32	cDigits = 0;

	while(invalid_digit)
	{
		Tracef(p_message);
		invalid_digit = 0;
		while (invalid_digit == 0)
		{
			next_char = getchar();
			switch (next_char)
			{
			case '\n':
			case '\r':
				return char_to_num( Digits );
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (cDigits < MAX_DIGITS)
				{
					Digits[cDigits] = next_char;
					Digits[++cDigits] = '\0';
				}
				else
					putchar(7);		// BEEP!
				
				break;
    		case 0x08:	 /* BACKSPACE */
    			if (cDigits)
				{
					printf(" \b");	// BACKSPACE
					Digits[--cDigits] = '\0';
				}
				else
					putchar(7);		// BEEP!
				break;
			case 0x1B:  /* ESC */
        		if (cDigits)
					while (cDigits--)
						printf("\008 \008");
        		break;						
			default:
				invalid_digit = 1;
				Tracef("\nInvalid digit");
				putchar(7);		// BEEP!
				break;
			}
		}
	}		
}
#if false
{
	U32 invalid_digit = 1;
	U32 number = 0;
	char next_char = 0;
	U32 next_value;
	while(invalid_digit)
	{
		Tracef(p_message);
		invalid_digit = 0;
		while (invalid_digit == 0)
		{
			next_char = getchar();
			switch (next_char)
			{
				case '\n':
					return number;
				case '\r':
					return number;
				case '0': next_value = 0; number = number * 10 + next_value; break;
				case '1': next_value = 1; number = number * 10 + next_value; break;
				case '2': next_value = 2; number = number * 10 + next_value; break;
				case '3': next_value = 3; number = number * 10 + next_value; break;
				case '4': next_value = 4; number = number * 10 + next_value; break;
				case '5': next_value = 5; number = number * 10 + next_value; break;
				case '6': next_value = 6; number = number * 10 + next_value; break;
				case '7': next_value = 7; number = number * 10 + next_value; break;
				case '8': next_value = 8; number = number * 10 + next_value; break;
				case '9': next_value = 9; number = number * 10 + next_value; break;
					break;
           		case 0x08:	 /* BACKSPACE */
           			number = (number - next_value);
           			if (number > 10)
           				number = number  / 10;
           			printf(" \0x08");
           			break;
				default:
					invalid_digit = 1;
					Tracef("\nInvalid digit");
					break;
			}
		}
	}
	return number;
}
#endif

U32 ttyport_Get_U32(I32 port, char *p_message)
{

	U32 invalid_digit = 1;
	U32 number = 0;
	char next_char = 0;
	U32 next_value;
	#define	MAX_DIGITS 3
	char Digits[MAX_DIGITS+1] = {0};
	U32	cDigits = 0;

	while(invalid_digit)
	{
		ttyport_printf(port, p_message);
		invalid_digit = 0;
		while (invalid_digit == 0)
		{
			next_char = ttyport_getchar(port);
			switch (next_char)
			{
			case '\n':
			case '\r':
				return char_to_num( Digits );
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (cDigits < MAX_DIGITS)
				{
					Digits[cDigits] = next_char;
					Digits[++cDigits] = '\0';
				}
				else
					ttyport_putchar(port, 7);		// BEEP!
				
				break;
    		case 0x08:	 /* BACKSPACE */
    			if (cDigits)
				{
					ttyport_printf(port, " \b");	// BACKSPACE
					Digits[--cDigits] = '\0';
				}
				else
					ttyport_putchar(port, 7);		// BEEP!
				break;
			case 0x1B:  /* ESC */
        		if (cDigits)
					while (cDigits--)
						ttyport_printf(port, "\008 \008");
        		break;						
			default:
				invalid_digit = 1;
				ttyport_printf(port, "\nInvalid digit\n");
				ttyport_putchar(port, 7);		// BEEP!
				break;
			}
		}
	}		
}


