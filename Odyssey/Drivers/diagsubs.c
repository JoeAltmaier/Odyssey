/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: diagsubs.c
// 
// Description:
// This file contains miscellaneous diagnostic subroutines.
// 
// Update Log 
// 
// 06/23/99 Bob Weast: Create file
// 
/*************************************************************************/

#include "diags.h"
#include <bootblock.h>


/*****************************/
// Externals
/*****************************/





/* ------------------ test subroutines -------------------------- */

U32 test_byte_access(void *start, U32 len, U32 verbose)
{
   U8 *p = (U8*)start;
   U8 *e = (U8*)start + len;
   U32 result = SUCCESS;
   register U8 b;
   register U32 i;
   int cont = 1;

   if (verbose)
      printf(verbDoingBytes);

   for ( ; p < e && cont; p += MEG) {
      b = 0x10;
      for (i=0; i<16; i++)
         p[i] = b++;
      b = 0x10;
      for (i=0; i<16; i++) {
         if (p[i] != b) {
            if (verbose)
               printf(errAddrWR_byte, &p[i], b, p[i]);
            result = FAILURE;
            cont = verbose ? AskYN(keepGoing) : NO;
            if (!cont) break;
         }
         b++;
      }
   }
   if (verbose && !result)
      printf(aoknl);
   return (result);
}


U32 test_half_access(void *start, U32 len, U32 verbose)
{
   U16 *p = (U16*)start;
   U16 *e = (U16*)((U32)start + len);
   U32 result = SUCCESS;
   register U16 h;
   register U32 i;
   int cont = 1;

   if (verbose)
      printf(verbDoingHalves);

   for ( ; p < e && cont; (U8*)p += MEG) {
      h = 0x210;
      for (i=0; i<8; i++)
         p[i] = h++;
      h = 0x210;
      for (i=0; i<8; i++) {
         if (p[i] != h) {
            if (verbose)
               printf(errAddrWR_half, &p[i], h, p[i]);
            result = FAILURE;
            cont = verbose ? AskYN(keepGoing) : NO;
            if (!cont) break;
         }
         h++;
      }
   }
   if (verbose && !result)
      printf(aoknl);
   return (result);
}


U32 test_word_access(void *start, U32 len, U32 verbose)
{
   U32 *p = (U32*)start;
   U32 *e = (U32*)((U32)start + len);
   U32 result = SUCCESS;
   register U32 w;
   register U32 i;
   int cont = 1;

   if (verbose)
      printf(verbDoingWords);

   for ( ; p < e && cont; (U8*)p += MEG) {
      w = 0x12345670;
      for (i=0; i<4; i++)
         p[i] = w++;
      w = 0x12345670;
      for (i=0; i<4; i++) {
         if (p[i] != w) {
            if (verbose)
               printf(errAddrWR_word, &p[i], w, p[i]);
            result = FAILURE;
            cont = verbose ? AskYN(keepGoing) : NO;
            if (!cont) break;
         }
         w++;
      }
   }
   if (verbose && !result)
      printf(aoknl);
   return (result);
}


U32 test_long_access(void *start, U32 len, U32 verbose)
{
   U64 *p = (U64*)start;
   U64 *e = (U64*)((U32)start + len);
   U32 result = SUCCESS;
   register U64 d;
   register U32 i;
   int cont = 1;

   if (verbose)
      printf(verbDoingLongs);

   for ( ; p < e && cont; (U8*)p += MEG) {
      d = 0x0fedcba987654321;
      p[0] = d;
      p[1] = ~d;
      for (i=0; i<2; i++) {
         if (p[i] != d) {
            if (verbose)
               printf(errAddrWR_long, &p[i], d, p[i]);
            result = FAILURE;
            cont = verbose ? AskYN(keepGoing) : NO;
            if (!cont) break;
         }
         d = ~d;
      }
   }
   if (verbose && !result)
      printf(aoknl);
   return (result);
}


/*
 * Simple Two-pass algorithm to test a range of memory -
 * every bit flips and all locations are unique to detect
 * address line problems.
 */
U32 memory_test(void *start, U32 len, U64 seed, U32 verbose)
{
   U64 *p = (U64*)start;
   U64 *e = (U64*)((U32)start + len);
   register U64 d = seed;
   U32 i, bank, cont = 1;
   U32 result = SUCCESS;

   if (verbose)
      printf(verbMemTstHerald, d, p, (U32)e-8);
   
   for (i=0; i < 2 && cont; i++) {      /* Two-pass algorithm */

      if (verbose) {
         if (i && !result) printf(aoknl);
         printf(verbWriting);
         wheel(1);
      }
      d = i ? ~seed : seed;

      for (p = start; p < e; p++) {
         if (i)
            *p = d--;
         else
            *p = d++;
         wheel(0);
      }

      if (verbose) {
         printf(verbReadCompare);
         wheel(1);
      }
      d = i ? ~seed : seed;

      for (p = start; p < e && cont; p++) {
         if (*p != d) {
            bank = ((U32)p >> 28) - 0xA;
            printf("\n%s Bank %d\n", dmc, (((U32)p >> 27) & 1) + bank);
            printf(errAddrWR_long, (U32)p, d, *p);
            result = FAILURE;
            cont = AskYN(keepGoing);
         }
         wheel(0);
         if (i)
            d--;
         else
            d++;
      }

   }

   if (verbose && !result) printf(aoknl);
   return (result);
}


#ifdef CONFIG_BOOT

U32 do_memtest_seq(void *start, U32 len, U32 verbose)
{
   U32 result = SUCCESS;
   U64 seed = getSeed();

   result |= test_byte_access(start, len, verbose);

   if (!result)
      result |= test_half_access(start, len, verbose);

   if (!result)
      result |= test_word_access(start, len, verbose);

   if (!result)
      result |= test_long_access(start, len, verbose);

   if (!result)
      result |= memory_test(start, len, seed, verbose);

   return (result);
}


/*
 * Just a wrapper to handle the segments.
 * We skip the lowest 512k of bank0 (vectors, stack)
 * and the highest 512k of bank3 (TRK space)
 */
U32 testAllMemory(U32 verbose)
{
   U32 status = SUCCESS;

   status |= do_memtest_seq((void*)0xA0080000,
     memory_bank1_size ? memory_bank0_size + memory_bank1_size - K512 :
                         memory_bank0_size - K512, verbose);

   if (ImpatientWheel(0))
      return (status);

   if (memory_bank2_size && !status)
      status |= do_memtest_seq((void*)0xC0000000,
      memory_bank3_size ? memory_bank2_size + memory_bank3_size - K512 :
                          memory_bank2_size, verbose);

   return (status);
}

#endif


/*
 * Simple One-pass algorithm to test a range of memory - Quickly!
 * Routine skips quickly thru range by 256k increments, two dwords each
 * Used by HBC to quietly test 64M IOP memory space during boot.
 */
U32 diag_quick_memtest(void *buf, U32 len, U32 verbose)
{
   volatile U64 *p = (U64*)buf;
   U64 *e = (U64*)((U32)buf + len);
   U64 seed = getSeed();
   register U64 d = seed;
   int i;
   U32 result = SUCCESS;

   /* first, ensure that different operation sizes
    * work properly across the bridge.
    */
   if ((result |= test_byte_access(buf, 100, verbose)))
      return (result);

   if ((result |= test_half_access(buf, 100, verbose)))
      return (result);

   if ((result |= test_word_access(buf, 100, verbose)))
      return (result);

   if ((result |= test_long_access(buf, 100, verbose)))
      return (result);

   /* next, fully test lowest 64k to
    * verify low address lines
    */
   for (i=0; i < (0x10000 / 8); i++)
      *p++ = d++;

#if 0
   /*
    * No! Don't start comparing yet.
    * Perform all writes first.
    */
   p = (U64*)buf;
   d = seed;
   for (i=0; i < (0x10000 / 8); i++) {
      if (*p != d) {
         if (verbose)
            printf(errAddrWR_long, (U32)p, d, *p);
         return (FAILURE);
      }
      p++;
      d++;
   }
#endif
   /*
    * skip up the next few address bits
    */
   for (i=0x10000; i < 0x80000; i <<= 1) {
      p = (U64*)((U32)buf + i);
      p[0] = d;
      p[1] = ~d;
      d++;
   }

   /*
    * now skip thru the rest, two longwords per 256K
    */
   p = (U64*)((U32)buf+0x80000);
   while ((U32)p < (U32)e) {
      p[0] = d;
      p[1] = ~d;
      d++;
      (U8*)p += 0x40000;
   }

   /*
    * all writes finished, read 'em back
    */
   p = (U64*)buf;
   d = seed;
   for (i=0; i < (0x10000 / 8); i++) {
      if (*p != d) {
         if (verbose)
            printf(errAddrWR_long, (U32)p, d, *p);
         return (FAILURE);
      }
      p++;
      d++;
   }

   /*
    * skip up the next few address bits
    */
   for (i=0x10000; i < 0x80000; i <<= 1) {
      p = (U64*)((U32)buf + i);
      if (p[0] != d) {
         if (verbose)
            printf(errAddrWR_long, (U32)p, d, p[0]);
         return (FAILURE);
      }
      if (p[1] != ~d) {
         if (verbose)
            printf(errAddrWR_long, (U32)p+8, ~d, p[1]);
         return (FAILURE);
      }
      d++;
   }

   /*
    * now skip thru the rest, two longwords per 256K
    */
   p = (U64*)((U32)buf+0x80000);
   while ((U32)p < (U32)e) {
      if (p[0] != d) {
         if (verbose)
            printf(errAddrWR_long, (U32)p, d, p[0]);
         return (FAILURE);
      }
      if (p[1] != ~d) {
         if (verbose)
            printf(errAddrWR_long, (U32)p+8, ~d, p[1]);
         return (FAILURE);
      }
      d++;
      (U8*)p += 0x40000;
   }

   return (result);
}


/*
 * ------------ Miscellaneous Utility functions -------------------
 */

void showBootBlock(void *p)
{
   bootblock_t *pbb = (bootblock_t*) p;
   mmap_t *pmm = &pbb->b_memmap;
   U32 i;

   printf("\nBoot Block at %X contains:\n", (U32)pbb);
   printf("\nSlotId: %ld   Board Type: %ld  Total Memory: %ld bytes",
          pbb->b_slot, pbb->b_type, pbb->b_dramsize);
   printf("\nBank 01 address: %08X  Bank 01 size: %x",
          pbb->b_bank01_addr, pbb->b_bank01_size);
   printf("\nBank 23 address: %08X  Bank 23 size: %x",
          pbb->b_bank23_addr, pbb->b_bank23_size);
   printf("\nbT:");
   for (i=0; i < 32; i++)
      printf(" %d", pmm->aBoardType[i]);
   printf("\nPCI-IDs skipped, dump %X(32x32) to see those.\naPaPci[32]:\n",
          &pmm->aIdPci);
   for (i=0; i < 32; i++) {
      printf(" %08X ", pmm->aPaPci[i]);
      if ((i % 4) == 3)
         putchar(NEWLINE);
   }
   printf("\nmySlot: %d   pciAddr: %08X   pa: %08X   size: %x",
          pmm->iSlot, pmm->pciSlave, pmm->paSlave, pmm->cbSlave);
   printf("   nSegs: %d", i = pmm->nFragment);
   printf("\naPa: %08X %08X", pmm->aPa[0], pmm->aPa[1]);
   printf("\naVa: %08X %08X", pmm->aP[0], pmm->aP[1]);
   printf("\naCb: %08x %08x", pmm->aCb[0], pmm->aCb[1]);
   printf("\n\n");
}
