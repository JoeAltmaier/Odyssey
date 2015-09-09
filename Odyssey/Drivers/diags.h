/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: diags.h
// 
// Description:
// This file contains board-level diagnostics that run in MetroTRK prom
// and Nucleus Init environment for the Odyssey platform.
// 
// Update Log 
// 
// 05/24/99 Bob Weast: Create file
// 
/*************************************************************************/
#ifndef Simple_H
#include "types.h"
typedef unsigned long long UI64;
#endif

/* misc constants */
#define    OK            0
#define    YES           1
#define    NO            0
#define    SILENT        0
#define    VERBOSE       1
#define    VERBOSE_LOUD  2
#define    SUCCESS       0
#define    FAILURE       1
#define    MEG           1024 * 1024
#define    K512          1024 * 512
#define    DMAGIC        1114595927
#define    DDMAGIC       0x426f625765617374
#define    NEWLINE       '\n'
#define    BACKSPACE     '\b'


#define    SIZE_BYTE     1
#define    SIZE_HALF     2
#define    SIZE_WORD     4
#define    SIZE_LONG     8
#define    SIZE_MASK    0x0f
#define    FILL_DATA    0x00
#define    NOT_DATA     0x10
#define    RAMP_DATA    0x20
#define    LFSR_DATA    0x30
#define    RNDM_DATA    0x40
#define    FC_DATA      0x50

/*
 * Data pattern types for frame payloads.
 */
#define    BYTE_FILL     (FILL_DATA + SIZE_BYTE)
#define    HALF_FILL     (FILL_DATA + SIZE_HALF)
#define    WORD_FILL     (FILL_DATA + SIZE_WORD)
#define    LONG_FILL     (FILL_DATA + SIZE_LONG)
#define    BYTE_NOT      (NOT_DATA + SIZE_BYTE)
#define    HALF_NOT      (NOT_DATA + SIZE_HALF)
#define    WORD_NOT      (NOT_DATA + SIZE_WORD)
#define    LONG_NOT      (NOT_DATA + SIZE_LONG)
#define    BYTE_RAMP     (RAMP_DATA + SIZE_BYTE)
#define    HALF_RAMP     (RAMP_DATA + SIZE_HALF)
#define    WORD_RAMP     (RAMP_DATA + SIZE_WORD)
#define    LONG_RAMP     (RAMP_DATA + SIZE_LONG)
#define    BYTE_LFSR     (LFSR_DATA + SIZE_BYTE)
#define    HALF_LFSR     (LFSR_DATA + SIZE_HALF)
#define    WORD_DRAMP    (LFSR_DATA + SIZE_WORD)
#define    LONG_DRAMP    (LFSR_DATA + SIZE_LONG)
#define    RANDOM        (RNDM_DATA + SIZE_BYTE)
/* just like random, but with no 00 bytes and no FF bytes */
#define    RANDOMNFZ     (RNDM_DATA + SIZE_HALF)
#define    CRPAT         (FC_DATA + SIZE_WORD + SIZE_LONG)
#define    CSPAT         (FC_DATA + SIZE_BYTE)



/* unit size for memory operations */
typedef enum { OP_BYTE, OP_HALF, OP_WORD, OP_LONG } OP_TYPE;

/* keyboard keys */
#define ESC 0x1b

/* temporary stuff that should be declared elsewhere */
#ifdef CONFIG_BOOT
extern void printf(const char *fmt, ...);
extern void putchar(int);

/* imported data */
extern U32 boardType;
extern U32 memory_bank0_size;
extern U32 memory_bank1_size;
extern U32 memory_bank2_size;
extern U32 memory_bank3_size;
extern U32 total_memory_size;
#endif

/* global data */
extern U32 sheSaidQuit;

/* diagnostics subroutines -- from diagdata.c */
extern UI64 getSeed(void);
extern void genData(void*, U32, UI64, U32);
extern void dumpData(void*, OP_TYPE, U32);
extern void dataTypeShow(UI64 seed);

/* diagnostics subroutines -- from diagsubs.c */
extern U32 test_byte_access(void*, U32, U32);
extern U32 test_half_access(void*, U32, U32);
extern U32 test_word_access(void*, U32, U32);
extern U32 test_long_access(void*, U32, U32);
extern U32 memory_test(void*, U32, UI64, U32);
extern U32 do_memtest_seq(void*, U32, U32);
extern U32 testAllMemory(U32);
extern U32 diag_quick_memtest(void*, U32, U32);
extern void showBootBlock(void*);

/* diagnostics subroutines -- from diagui.c */
extern int  AskYN(char*);
extern void Beep(void);
extern void Gripe(void);
extern void flushKbd(void);
#ifdef CONFIG_BOOT
extern int  getKey(void); /* prom version of getchar, busy wait */
#define GET_KEY() getKey()
#else
extern int  getchar(void);
#define GET_KEY() getchar()
#endif
extern void getAnyKey(void);
extern void getReturn(void);
extern U32  getDec(void);
extern UI64 getHex(void);
extern UI64 htoll(char*);
extern U32  ImpatientWheel(U32);
extern int  isdigit(U8);
extern U32  less(U32);

/* diagnostics subroutines -- from diags.c */
extern U32 diag_sysflash_test(void);

/* other useful subroutines from various places */
extern int  atoi(char*);
extern U32  r5k_get_compare(void);
extern U32  r5k_get_count(void);
extern void r5k_set_compare(U32 val);
extern void galileo_regdump(void);
extern void delay(int);
extern void delay_ms(int);
extern void delay_us(int);
extern int  kbhit(void);
extern void wheel(int);

/* cursor positioning stuff */
extern char *clreol;     /* clear to end-of-line */
extern char *clreop;     /* clear screen bottom */
extern char *clrscn;     /* clear entire screen  */
extern char *bspace;     /* bs-space-bs */
extern char *cpecho;     /* input cursor position */
extern char *stars1;     /* menu title stars border */
extern char *toppos;     /* ascii cur-pos sequence for stars */
extern char *botpos;     /* ascii cur-pos sequence for stars */
extern char *prompt;     /* pos, clear, Choice -->>          */

/* message strings */
extern char *diag_title;
extern char *allTest1;
extern char *allTest2;
extern char *hbcTest3;
extern char *hbcTest4;
extern char *hbcTest5;
extern char *hbcTest6;
extern char *hbcTest7;
extern char *hbcTest8;
extern char *hbcTestT;
extern char *nacTest3;
extern char *nacTest4;
extern char *nacTest5;
extern char *nacTestT;
extern char *ssdTest3;
extern char *ssdTest4;
extern char *ssdTest5;
extern char *ssdTest6;
extern char *ssdTestT;
extern char *allCmdA;
extern char *allCmdB;
extern char *allCmdO;

extern char *sysTestOK;
extern char *sysTestBAD;
extern char *burnOK;
extern char *burnBAD;
extern char *keepGoing;
extern char *pressAnyKey;
extern char *pressReturn;
extern char *aoknl;
extern char *datpos;
extern char *opStr[];
extern char *names[];
extern char *lessq;
extern char *changeq;
extern char *nl;
extern char *nl2;
extern char *ttr;
extern char *verb;

extern char *dmc;
extern char *errAddrWR_byte;
extern char *errAddrWR_half;
extern char *errAddrWR_word;
extern char *errAddrWR_long;

extern char *verbMemTstHerald;
extern char *verbDoingBytes;
extern char *verbDoingHalves;
extern char *verbDoingWords;
extern char *verbDoingLongs;
extern char *verbWriting;
extern char *verbReadCompare;
