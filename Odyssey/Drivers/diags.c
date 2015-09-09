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
// This file contains board-level diagnostics that run in MetroTRK prom
// environment for the Odyssey platform.
// 
// Update Log 
// 
// 05/06/99 Bob Weast: Create file
// 
/*************************************************************************/
#ifdef CONFIG_BOOT

#include "diags.h"
#include "sysflash.h"
#include "imghdr.h"
#include "pcimap.h"
#include "ioptypes.h"


/*****************************/
// Externals
/*****************************/
extern U32 bootblock;


/*****************************/
// Forward References
/*****************************/
void run_diags_menu(void);
void run_tools_menu(void);
void comingSoon(char*);


/*****************************/
// Local Data
/*****************************/
typedef struct {
   U32 Verbose;
   U32 TimesToRun;
} LOCAL_VARS;

static LOCAL_VARS lv = {
   VERBOSE,
   1
   };

typedef struct {
   OP_TYPE op;
   U32 a1;
   U32 a2;
   U32 range; /* units, actually */
   U64 datr;
   U32 type;
   U32 loop;
} TOOLS_PARMS;

static TOOLS_PARMS tp = {
   OP_WORD,
   0xA0100000,
   0xA0200000,
   1,
   0x696969695a5a5a5a,
   WORD_FILL,
   1
   };

#endif
/*****************************/
// Global Data
/*****************************/
unsigned int sheSaidQuit;


#ifdef CONFIG_BOOT

static void mapTypeToOp()
{
   if ((tp.type & 0xf0) < RNDM_DATA)
   switch (tp.op) {
      case OP_BYTE: tp.type = (tp.type & ~SIZE_MASK) | SIZE_BYTE; break;
      case OP_HALF: tp.type = (tp.type & ~SIZE_MASK) | SIZE_HALF; break;
      case OP_WORD: tp.type = (tp.type & ~SIZE_MASK) | SIZE_WORD; break;
      case OP_LONG: tp.type = (tp.type & ~SIZE_MASK) | SIZE_LONG; break;
   }
}


static void mapOpToType()
{
   if ((tp.type & 0xf0) < RNDM_DATA)
   switch (tp.type & SIZE_MASK) {
      default:
      case 1: tp.op = OP_BYTE; break;
      case 2: tp.op = OP_HALF; break;
      case 4: tp.op = OP_WORD; break;
      case 8: tp.op = OP_LONG; break;
   }
}


static void align_addrs(void)
{
   U32 mask;

   switch (tp.op) {
   case OP_BYTE:
      return;
   case OP_HALF:
      mask = 0xfffffffe;
      break;
   case OP_WORD:
      mask = 0xfffffffc;
      break;
   case OP_LONG:
      mask = 0xfffffff8;
      break;
   }

   tp.a1 &= mask;
   tp.a2 &= mask;
}


void displayHbcDiagsMenu(void)
{
   printf(clrscn);
   printf(stars1, toppos);
   printf(diag_title, names[boardType]);
   printf(stars1, botpos);
   printf(allTest1);
   printf(allTest2);
   printf(hbcTest3);
   printf(hbcTest4);
   printf(hbcTest5);
   printf(hbcTest6);
   printf(hbcTest7);
   printf(hbcTest8);
   printf(hbcTestT);
   printf(allCmdA);
   printf(allCmdB);
   printf(allCmdO);
   printf(prompt);
   printf(cpecho);
}

void displayNacDiagsMenu(void)
{
   printf(clrscn);
   printf(stars1, toppos);
   printf(diag_title, names[boardType]);
   printf(stars1, botpos);
   printf(allTest1);
   printf(allTest2);
   printf(nacTest3);
   printf(nacTest4);
   printf(nacTest5);
   printf(nacTestT);
   printf(allCmdA);
   printf(allCmdB);
   printf(allCmdO);
   printf(prompt);
   printf(cpecho);
}

void displaySsdDiagsMenu(void)
{
   printf(clrscn);
   printf(stars1, toppos);
   printf(diag_title, names[boardType]);
   printf(stars1, botpos);
   printf(allTest1);
   printf(allTest2);
   printf(ssdTest3);
   printf(ssdTest4);
   printf(ssdTest5);
   printf(ssdTest6);
   printf(ssdTestT);
   printf(allCmdA);
   printf(allCmdB);
   printf(allCmdO);
   printf(prompt);
   printf(cpecho);
}

void displayToolsParms(void)
{
   static char *opStr[4] = {"Bhwd", "bHwd", "bhWd", "bhwD"};
   static char *datpos = "\033[04;07H";

/*
      op:bhWd  adr1:A0100000  adr2:A0200000  units:00000001  loop:00000001
               data:12345678
*/
   align_addrs();
   printf("%s%sop:%s  adr1:%08lX  adr2:%08lX  units:%08lx  loop:%08lx\n",
          datpos,clreol, opStr[tp.op], tp.a1, tp.a2, tp.range, tp.loop);
   printf(clreol);

   if (tp.type < BYTE_NOT) { /* fill patterns */
    switch (tp.op) {
    case OP_BYTE: printf("\t       data:%02x ", (U8) tp.datr); break;
    case OP_HALF: printf("\t       data:%04x ", (U16) tp.datr); break;
    case OP_WORD: printf("\t       data:%08lx ", (U32) tp.datr); break;
    case OP_LONG: printf("\t       data:%016Lx ",       tp.datr); break;
    }
   } else {                  /* fancy patterns */
   switch (tp.op) {
    case OP_BYTE: printf("\t       type:%s   seed:%02x",
                         getTypeStr(tp.type), (U8) tp.datr);
                  break;
    case OP_HALF: printf("\t       type:%s   seed:%04x",
                         getTypeStr(tp.type), (U16) tp.datr);
                  break;
    case OP_WORD: printf("\t       type:%s   seed:%08lx",
                         getTypeStr(tp.type), (U32) tp.datr);
                  break;
    case OP_LONG: printf("\t       type:%s   seed:%016Lx",
                         getTypeStr(tp.type), tp.datr);
                  break;
    }
   }
}

void displayToolsMenu(void)
{
   printf(clrscn);
   printf(stars1, toppos);
   printf("\033[02;19HConvergeNet Technologies-MetroTRK Tools Menu. (v0.2)");
   printf(stars1, botpos);
   displayToolsParms();

   printf("\033[07;25H1) Read Memory");
   printf("\033[08;25H2) Write Memory");
   printf("\033[09;25H3) Write/Read Memory");
   printf("\033[10;25H4) Copy Memory");
   printf("\033[11;25H5) Compare Memory");
   printf("\033[12;25H6) Dump Memory");
   printf("\033[13;25H7) Test Memory Area");
   printf(prompt);
   printf(cpecho);
}


void setOptions(void)
{
   U32 v;

   printf("\n%s: %u", ttr, lv.TimesToRun);
   if (AskYN(changeq)) {
      putchar(NEWLINE);
      printf(ttr);
      v = (U32) getDec();
      lv.TimesToRun = v ? v : 1;
   }
   printf("\n%s: %u", verb, lv.Verbose);
   if (AskYN(changeq)) {
      putchar(NEWLINE);
      printf(verb);
      v = (U32) getHex();
      lv.Verbose = v;
   }
   
}


void diags_hbc_menu(void)
{
   char c, cont = 1;
   U32 passcnt;
   U32 status;

   displayHbcDiagsMenu();
   while (cont) {
      c = GET_KEY();
      if (isdigit(c))
         printf(clreop);
      sheSaidQuit = NO;
      switch(c) {
      case '1':
         status = passcnt = 0;
         while (passcnt++ < lv.TimesToRun && !status && !sheSaidQuit)
            status |= testAllMemory(lv.Verbose);
         if (lv.TimesToRun > 1 && lv.Verbose && !status) {
            printf("\nDone, %d passes complete.", passcnt-1);
            getReturn();
            displayHbcDiagsMenu();
         }
         break;
      case '2':
         diag_sysflash_test();
         break;
      case '3':
         comingSoon("RTC Test");
         break;
      case '4':
         comingSoon("NAND FlashTest");
         break;
      case '5':
         comingSoon("EMAC_RegisterTest");
         break;
      case '6':
         comingSoon("EMAC_RegisterTest");
         break;
      case '7':
         comingSoon("EthInternalLoopbackTest");
         break;
      case '8':
         comingSoon("EthInternalLoopbackTest");
         break;
      case '9':
      default:
         Gripe();
         break;
      case 't':
      case 'T':
         run_tools_menu();
      /* break; need redraw here, fall thru */
      case ' ':
         displayHbcDiagsMenu();
         break;
      case 'o':
      case 'O':
         printf(clreop);
         setOptions();
         break;
      case 'a':
      case 'A':
         printf(clreop);
         status = testAllMemory(lv.Verbose);
         if (sheSaidQuit)
            break;
         if (!status)
            status |= diag_sysflash_test();
         if (sheSaidQuit)
            break;
         comingSoon("RTC Test");
         comingSoon("NAND FlashTest");
         comingSoon("EMAC_RegisterTest");
         comingSoon("EthInternalLoopbackTest");
         if (!status)
            printf(sysTestOK);
         else
            printf(sysTestBAD);
         getReturn();
         displayHbcDiagsMenu();
         break;
      case 'b':
      case 'B':
         printf(clreop);
         passcnt = status = 0;
         while (1) {
            status |= testAllMemory(lv.Verbose);
            if (sheSaidQuit)
               break;
            if (!status)
               status |= diag_sysflash_test();
            if (sheSaidQuit)
               break;
            comingSoon("RTC Test");
            comingSoon("NAND FlashTest");
            comingSoon("EMAC_RegisterTest");
            comingSoon("EthInternalLoopbackTest");
            passcnt++;
         }
         if (!status)
            printf(burnOK, passcnt);
         else
            printf(burnBAD);
         getReturn();
         displayHbcDiagsMenu();
         break;
      case 'q':
      case 'Q':
      case ESC:
         cont = 0;
      }
   }
}

void diags_nac_menu(void)
{
   char c, cont = 1;
   U32 passcnt;
   U32 status;

   displayNacDiagsMenu();
   while (cont) {
      c = GET_KEY();
      if (isdigit(c))
         printf(clreop);
      sheSaidQuit = NO;
      switch(c) {
      case '1':
         status = passcnt = 0;
         while (passcnt++ < lv.TimesToRun && !status && !sheSaidQuit)
            status |= testAllMemory(lv.Verbose);
         if (lv.TimesToRun > 1 && lv.Verbose && !status) {
            printf("\nDone, %d passes complete.", passcnt-1);
            getReturn();
            displayNacDiagsMenu();
         }
         break;
      case '2':
         diag_sysflash_test();
         break;
      case '3':
      case '4':
      case '5':
         comingSoon("FC_MAC_X Register Test");
         break;
      case '6':
      case '7':
      case '8':
      case '9':
      default:
         Gripe();
         break;
      case 't':
      case 'T':
         run_tools_menu();
      /* break; need redraw here, fall thru */
      case ' ':
         displayNacDiagsMenu();
         break;
      case 'o':
      case 'O':
         printf(clreop);
         setOptions();
         break;
      case 'a':
      case 'A':
         printf(clreop);
         status = testAllMemory(lv.Verbose);
         if (sheSaidQuit)
            break;
         if (!status)
            status |= diag_sysflash_test();
         if (sheSaidQuit)
            break;
         comingSoon("FC_MAC_X Register Test");
         if (!status)
            printf(sysTestOK);
         else
            printf(sysTestBAD);
         getReturn();
         displayNacDiagsMenu();
         break;
      case 'b':
      case 'B':
         printf(clreop);
         passcnt = status = 0;
         while (1) {
            status |= testAllMemory(lv.Verbose);
            if (sheSaidQuit)
               break;
            if (!status)
               status |= diag_sysflash_test();
            if (sheSaidQuit)
               break;
            comingSoon("FC_MAC_X Register Test");
            passcnt++;
         }
         if (!status)
            printf(burnOK, passcnt);
         else
            printf(burnBAD);
         getReturn();
         displayNacDiagsMenu();
         break;
      case 'q':
      case 'Q':
      case ESC:
         cont = 0;
      }
   }
}


void diags_ssd_menu(void)
{
   char c, cont = 1;
   U32 passcnt;
   U32 status;

   displaySsdDiagsMenu();
   while (cont) {
      c = GET_KEY();
      if (isdigit(c))
         printf(clreop);
      sheSaidQuit = NO;
      switch(c) {
      case '1':
         status = passcnt = 0;
         while (passcnt++ < lv.TimesToRun && !status && !sheSaidQuit)
            status |= testAllMemory(lv.Verbose);
         if (lv.TimesToRun > 1 && lv.Verbose && !status) {
            printf("\nDone, %d passes complete.", passcnt-1);
            getReturn();
            displaySsdDiagsMenu();
         }
         break;
      case '2':
         diag_sysflash_test();
         break;
      case '3':
         comingSoon("NAND Flash Test");
         break;
      case '4':
         comingSoon("NAND Flash Test");
         break;
      case '5':
         comingSoon("NAND Flash Test");
         break;
      case '6':
         comingSoon("NAND Flash Test");
         break;
      case '7':
      case '8':
      case '9':
      default:
         Gripe();
         break;
      case 't':
      case 'T':
         run_tools_menu();
      /* break; need redraw here, fall thru */
      case ' ':
         displaySsdDiagsMenu();
         break;
      case 'a':
      case 'A':
         printf(clreop);
         status = testAllMemory(lv.Verbose);
         if (sheSaidQuit)
            break;
         if (!status)
            status |= diag_sysflash_test();
         comingSoon("NAND Flash Test");
         if (!status)
            printf(sysTestOK);
         else
            printf(sysTestBAD);
         getReturn();
         displaySsdDiagsMenu();
         break;
      case 'b':
      case 'B':
         printf(clreop);
         passcnt = status = 0;
         while (1) {
            status |= testAllMemory(lv.Verbose);
            if (sheSaidQuit)
               break;
            if (!status)
               status |= diag_sysflash_test();
            if (sheSaidQuit)
               break;
            comingSoon("NAND Flash Test");
            passcnt++;
         }
         if (!status)
            printf(burnOK, passcnt);
         else
            printf(burnBAD);
         getReturn();
         displaySsdDiagsMenu();
         break;
      case 'q':
      case 'Q':
      case ESC:
         cont = 0;
      }
   }
}


void run_tools_menu(void)
{
   char c, cont = 1;
   U32 i, r, v;
   U64 lv;
   volatile U8 *bp, *cp;
   volatile U16 *hp, *sp;
   volatile U32 *wp, *qp;
   volatile U64 *lp, *dp;
#define QUIT { cont = 0; break; }

   displayToolsMenu();
   while (1) {
      c = GET_KEY();
      if (isdigit(c))
         printf(clreop);
      cont = 1;
      switch(c) {

      case '1':                          /* read data */
         for (i=0; i < tp.loop && cont; i++) {
            r = 0;
            switch(tp.op) {
            case OP_BYTE:
               bp = (U8*) tp.a1;
               while (r < tp.range) {
                  v = *bp++;
                  if (!(r|i)) printf("%02x  ", v);
                  if (ImpatientWheel(0)) QUIT;
                  ++r;
               }
               break;
            case OP_HALF:
               hp = (U16*) tp.a1;
               while (r < tp.range) {
                  v = *hp++;
                  if (!(r|i)) printf("%04x  ", v);
                  if (ImpatientWheel(0)) QUIT;
                  ++r;
               }
               break;
            case OP_WORD:
               wp = (U32*) tp.a1;
               while (r < tp.range) {
                  v = *wp++;
                  if (!(r|i)) printf("%08lx  ", v);
                  if (ImpatientWheel(0)) QUIT;
                  ++r;
               }
               break;
            case OP_LONG:
               lp = (U64*) tp.a1;
               while (r < tp.range) {
                  lv = *lp++;
                  if (!(r|i)) printf("%016Lx  ", lv);
                  if (ImpatientWheel(0)) QUIT;
                  ++r;
               }
               break;
            }
            if (i && ImpatientWheel(1)) QUIT;
         }
         break;

      case '2':                          /* write data */
         for (i=0; i < tp.loop && cont; i++) {
            if (tp.type < BYTE_NOT) {
	           r = 0;
	           switch(tp.op) {
	           case OP_BYTE:
	              bp = (U8*) tp.a1;
	              while (r < tp.range) {
	                 *bp++ = (U8) tp.datr;
	                 if (ImpatientWheel(0)) QUIT;
	                 ++r;
	              }
	              break;
	           case OP_HALF:
	              hp = (U16*) tp.a1;
	              while (r < tp.range) {
	                 *hp++ = (U16) tp.datr;
	                 if (ImpatientWheel(0)) QUIT;
	                 ++r;
	              }
	              break;
	           case OP_WORD:
	              wp = (U32*) tp.a1;
	              while (r < tp.range) {
	                 *wp++ = (U32) tp.datr;
	                 if (ImpatientWheel(0)) QUIT;
	                 ++r;
	              }
	              break;
	           case OP_LONG:
	              lp = (U64*) tp.a1;
	              while (r < tp.range) {
	                 *lp++ = tp.datr;
	                 if (ImpatientWheel(0)) QUIT;
	                 ++r;
	              }
	              break;
	           }
	        } else {
	           switch(tp.op) {
	           case OP_BYTE:
	              r = tp.range;
	              break;
	           case OP_HALF:
	              r = tp.range * 2;
	              break;
	           case OP_WORD:
	              r = tp.range * 4;
	              break;
	           case OP_LONG:
	              r = tp.range * 8;
	              break;
	           }
	           genData((void*)tp.a1, tp.type, tp.datr, r);
	        }
            if (i && ImpatientWheel(1)) QUIT;
         }
         break;

      case '3':                          /* write/read data */
         for (i=0; i < tp.loop && cont; i++) {
            r = 0;
            switch(tp.op) {
            case OP_BYTE:
               bp = (U8*) tp.a1;
               while (r < tp.range) {
                  *bp = (U8) tp.datr;
                  v = *bp++;
                  if (!(r|i)) printf("%02x  ", v);
                  if (ImpatientWheel(0)) QUIT;
                  ++r;
               }
               break;
            case OP_HALF:
               hp = (U16*) tp.a1;
               while (r < tp.range) {
                  *hp = (U16) tp.datr;
                  v = *hp++;
                  if (!(r|i)) printf("%04x  ", v);
                  if (ImpatientWheel(0)) QUIT;
                  ++r;
               }
               break;
            case OP_WORD:
               wp = (U32*) tp.a1;
               while (r < tp.range) {
                  *wp = (U32) tp.datr;
                  v = *wp++;
                  if (!(r|i)) printf("%08lx  ", v);
                  if (ImpatientWheel(0)) QUIT;
                  ++r;
               }
               break;
            case OP_LONG:
               lp = (U64*) tp.a1;
               while (r < tp.range) {
                  *lp = tp.datr;
                  lv = *lp++;
                  if (!(r|i)) printf("%016Lx  ", lv);
                  if (ImpatientWheel(0)) QUIT;
                  ++r;
               }
               break;
            }
            if (i && ImpatientWheel(1)) QUIT;
         }
         break;

      case '4':                          /* copy data */
         for (i=0; i < tp.loop && cont; i++) {
            r = 0;
            switch(tp.op) {
            case OP_BYTE:
               bp = (U8*) tp.a1;
               cp = (U8*) tp.a2;
               while (r < tp.range) {
                  *cp++ = *bp++;
                  if (ImpatientWheel(0)) QUIT;
                  ++r;
               }
               break;
            case OP_HALF:
               hp = (U16*) tp.a1;
               sp = (U16*) tp.a2;
               while (r < tp.range) {
                  *sp++ = *hp++;
                  if (ImpatientWheel(0)) QUIT;
                  ++r;
               }
               break;
            case OP_WORD:
               wp = (U32*) tp.a1;
               qp = (U32*) tp.a2;
               while (r < tp.range) {
                  *qp++ = *wp++;
                  if (ImpatientWheel(0)) QUIT;
                  ++r;
               }
               break;
            case OP_LONG:
               lp = (U64*) tp.a1;
               dp = (U64*) tp.a2;
               while (r < tp.range) {
                  *dp++ = *lp++;
                  if (ImpatientWheel(0)) QUIT;
                  ++r;
               }
               break;
            }
            if (i && ImpatientWheel(1)) QUIT;
         }
         break;

      case '5':                          /* compare data */
         for (i=0; i < tp.loop && cont; i++) {
            r = 0;
            switch(tp.op) {
            case OP_BYTE:
               bp = (U8*) tp.a1;
               cp = (U8*) tp.a2;
               while (r < tp.range) {
                  if (*cp != *bp) {
                     printf("%sbyte #%u, want %02x, got %02x "
                            "at %08lX\n", dmc, r, *bp, *cp, cp);
                     dumpData((void*)((U32)bp & 0xfffffff0), tp.op, 2);
                     dumpData((void*)((U32)cp & 0xfffffff0), tp.op, 2);
                     QUIT;
                  }
                  if (i && ImpatientWheel(0)) QUIT;
                  ++cp;
                  ++bp;
                  ++r;
               }
               break;
            case OP_HALF:
               hp = (U16*) tp.a1;
               sp = (U16*) tp.a2;
               while (r < tp.range) {
                  if (*sp != *hp) {
                     printf("%shalfword #%u, want %04x, got %04x "
                            "at %08lX\n", dmc, r, *hp, *sp, sp);
                     dumpData((void*)((U32)hp & 0xfffffff0), tp.op, 2);
                     dumpData((void*)((U32)sp & 0xfffffff0), tp.op, 2);
                     QUIT;
                  }
                  if (i && ImpatientWheel(0)) QUIT;
                  ++hp;
                  ++sp;
                  ++r;
               }
               break;
            case OP_WORD:
               wp = (U32*) tp.a1;
               qp = (U32*) tp.a2;
               while (r < tp.range) {
                  if (*qp != *wp) {
                     printf("%sword #%u, want %08x, got %08x "
                            "at %08lX\n", dmc, r, *wp, *qp, qp);
                     dumpData((void*)((U32)wp & 0xfffffff0), tp.op, 2);
                     dumpData((void*)((U32)qp & 0xfffffff0), tp.op, 2);
                     QUIT;
                  }
                  if (i && ImpatientWheel(0)) QUIT;
                  ++wp;
                  ++qp;
                  ++r;
               }
               break;
            case OP_LONG:
               lp = (U64*) tp.a1;
               dp = (U64*) tp.a2;
               while (r < tp.range) {
                  if (*dp != *lp) {
                     printf("%sdouble #%u,\n want %016Lx, got %016Lx "
                            "at %08lX\n", dmc, r, *lp, *dp, dp);
                     dumpData((void*)((U32)lp & 0xfffffff0), tp.op, 2);
                     dumpData((void*)((U32)dp & 0xfffffff0), tp.op, 2);
                     QUIT;
                  }
                  if (i && ImpatientWheel(0)) QUIT;
                  ++lp;
                  ++dp;
                  ++r;
               }
               break;
            }
            if (cont) {
               if (!i) printf("Compare Successful  ");
               else if (ImpatientWheel(1)) QUIT;
            }
         }
         break;

      case '6':                          /* dump data */
         printf(cpecho); /* avoid bletch */
         bp = (U8*) tp.a1;
         do {
            dumpData((void*) bp, tp.op, 7);
            bp += 7 * 16;
            } while (less(1));
         break;

      case '7':                          /* test specific area */
         if (tp.a1 > tp.a2  ||
             tp.a1 == tp.a2 ||
             tp.a1 & 0x7    ||
             tp.a2 & 0x7) {
            printf("Bogus addresses specified for test.\n");
            break;
         }
         r = tp.a2 - tp.a1;
         if (r < 8) r = 8;
         v = 0;
         for (i=0; i < tp.loop && cont; i++) {
            v |= memory_test((void*)tp.a1, r, getSeed(), VERBOSE);
            if (v || ImpatientWheel(0)) break;
         }
         if (tp.loop > 1 && i > 1 && !v) {
            printf("\nTestMemArea: %d passes complete.\n", i);
            getReturn();
            displayToolsMenu();
         }
         break;
      case '8':                          /*  */
      case '9':                          /*  */
      default:
         Beep();
         break;

      /* ------ hidden functions ------ */
      case 'g':                          /* dump Galileo */
      case 'G':
         galileo_regdump();
         getAnyKey();
         displayToolsMenu();
         break;

      case 'b':                          /* dump boot block */
      case 'B':
         printf(clrscn);
         showBootBlock(&bootblock);
         getAnyKey();
         displayToolsMenu();
         break;

      /* --------- parameters --------- */
      case 'a':                          /* addresses */
      case 'A':
         c = GET_KEY();
         if (c == '1')
            tp.a1 = (U32) getHex();
         else if (c == '2')
            tp.a2 = (U32) getHex();
         else Beep();
         displayToolsParms();
         break;

      case 'd':                          /* datr value */
      case 'D':
         tp.datr = getHex();
         displayToolsParms();
         break;

      case 'l':                          /* loop count */
      case 'L':
         tp.loop = (U32) getHex();
         if (!tp.loop)
            tp.loop = 1;
         displayToolsParms();
         break;

      case 'o':                          /* op size */
      case 'O':
         c = GET_KEY() | 0x20;
         if      (c == 'b' || c == '1') tp.op = OP_BYTE;
         else if (c == 'h' || c == '2') tp.op = OP_HALF;
         else if (c == 'w' || c == '4') tp.op = OP_WORD;
         else if (c == 'd' || c == '8') tp.op = OP_LONG;
         else Beep();
         mapTypeToOp();
         displayToolsParms();
         break;

      case 'u':                          /* num units */
      case 'U':
         tp.range = (U32) getHex();
         if (!tp.range)
            tp.range = 1;
         displayToolsParms();
         break;

      case 's':                          /* swap addresses */
      case 'S':
         v = tp.a1;
         tp.a1 = tp.a2;
         tp.a2 = v;
         displayToolsParms();
         break;

      case 'T':                          /* verbosely ... */
         printf(clrscn);
         dataTypeShow(tp.datr);
         printf("\nEnter choice from 'value' column");
      case 't':                          /* set data type */
         tp.type = (U32)getHex() & 0x7f;
         mapOpToType();
         /* fall thru to display */

      case ' ':
         displayToolsMenu();
         break;

      case 'q':
      case 'Q':
      case ESC:
         return;
      } /* c */

   printf(cpecho);
   printf(clreol);
   cont = 1;
   }
}


void run_diags_menu(void)
{

   switch (boardType) {
   case IOPTY_HBC:
      diags_hbc_menu();
      break;
   default:
   case IOPTY_NAC:
      diags_nac_menu();
      break;
   case IOPTY_SSD:
      diags_ssd_menu();
      break;
   case IOPTY_NIC:
      diags_nac_menu();
      break;
   case IOPTY_RAC:
      diags_nac_menu();
      break;
   }
}


#define SFSIZE 512
/* bletch, why are these within ctmain.c and not sysflash.h? */
#define HBC_IMAGE_SIZE			0x200000

#ifdef INCLUDE_ODYSSEY
#define IOP_IMAGE_SIZE			0x400000
#else
#define IOP_IMAGE_SIZE			0x100000
#endif

U32
diag_sysflash_test()
{
	int findex;
	int i, j, eflag;
	U32 seed;
	U8 *pBuf = (U8 *)0xA0100000;
	U32 *pwBuf = (U32 *)0xA0100000;
	U8 *ptr1 = (U8 *)SYSFLASH_START;
	U32 *wptr = (U32 *)SYSFLASH_START;
	img_hdr_t	*img1p = (img_hdr_t *)ptr1;
	U8 *ptr2 = (U8 *)SYSFLASH_START + IOP_IMAGE_SIZE;
	img_hdr_t	*img2p = (img_hdr_t *)ptr2;

	sysflash_init();
	if (lv.Verbose == VERBOSE_LOUD)
		printf("Detected System Flash, Man Id %X, Dev Id %X, Size %d\n", 
			sysflash_getmanf(0), sysflash_getdevid(0), SYSFLASH_SIZEB);

	/*
	 * Wait a minute!
	 * Don't erase a valid image!!
	 */
	if (img1p->i_signature == IMG_SIGNATURE ||
		img2p->i_signature == IMG_SIGNATURE) {
		printf("\n\nFlash contains a valid image, skipping test\n");
		return (SUCCESS);
	}

	/* Erase the flash */
	for (j=0; j < 2; j++) {  /* one level of indentation omitted here */
	printf("Erasing flash1.....");
	if ( sysflash_erase1() ) {
		printf("Done, ");
	} else {
		printf("Fail\n");
		return(FAILURE);
	}
	printf("Erasing flash2.....");
	if ( sysflash_erase2() ) {
		printf("Done, ");
	} else {
		printf("Fail\n");
		return(FAILURE);
	}

	/* Test whther flash is blank */
	printf("Verifying blank.....");
	eflag = 0;
	for(i=0; i < SYSFLASH_SIZEB / 4; i++) {
		if (wptr[i] !=  0xFFFFFFFF) {
			eflag = 1;
			break;
		}
		wheel(0);
	}
	if ( eflag ) {
		printf("Fail at offset %X, want ffffffff, got %08x\n", i, wptr[i]);
		return(FAILURE);
	} else {
		printf("Done\n");
	}

	/* Program the Flash */
	if (j) {
		for (i=0; i < SYSFLASH_SIZEB / 4; i++)
			pwBuf[i] = ~pwBuf[i];
	} else {
		seed = (U32)getSeed();
		genData(pBuf, WORD_DRAMP, seed, SYSFLASH_SIZEB);
	}
	
	printf("Programming (%08x).....", pwBuf[0]);
	findex = 0;
	while(findex < SYSFLASH_SIZEB) {
		if ( !sysflash_program(&pBuf[findex], findex, SFSIZE)) {
			printf("Fail ...... at %d\n", findex);
			return(FAILURE);
		}
		findex += SFSIZE;
	}
	printf("Done\n");
	
	/* Test the Flash */
	printf("Testing flash contents.....");
	eflag = 0;
	for(i=0; i < SYSFLASH_SIZEB / 4; i++) {
		if (wptr[i] != pwBuf[i] ) {
			eflag = 1;
			break;
		}
		wheel(0);
	}
	if ( eflag ) {
		printf("Fail, wrote %08x, read %08x at %08X\n", pwBuf[i], wptr[i],
		        &wptr[i]);
		return(FAILURE);
	} else {
		printf("Done\n");
	}
	} /* j */

	return(SUCCESS);

}

void comingSoon(char *str)
{
   printf("%s: coming soon to an Odyssey near you.\n", str);
}
#endif /* CONFIG_BOOT */