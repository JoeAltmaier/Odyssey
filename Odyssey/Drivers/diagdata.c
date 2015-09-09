/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: diagData.c
//
// Description:
// This file contains the genData() and compareData() procedures.
// Generic data generator/comparator.
//
/*************************************************************************/

#include  "diags.h"


/*
 * forwards
 */
U64  getSeed(void);
void genData(void *buf, U32 type, U64 seed, U32 len);
/*U32  compareData(U8 *src, U8 *dst, U32 len);*/
void dumpData(void *p, OP_TYPE op, U32 nLines);
void dataTypeShow(U64 seed);


static char onlyOne[] = "Only one bad byte";

char *dtStr[] = { "UNK",
            "BYTE_FILL", "HALF_FILL", "WORD_FILL", "LONG_FILL",
            "BYTE_NOT", "HALF_NOT", "WORD_NOT", "LONG_NOT",
            "BYTE_RAMP", "HALF_RAMP", "WORD_RAMP", "LONG_RAMP",
            "BYTE_LFSR", "HALF_LFSR", "WORD_DRAMP", "LONG_DRAMP",
            "RANDOM", "RANDOMNFZ", "CRPAT", "CSPAT" };

U8 lfsrData[] = {
    0x69, 0x01, 0x02, 0x05, 0x0B, 0x17, 0x2F, 0x5E,
    0xBD, 0x7B, 0xF6, 0xEC, 0xD8, 0xB0, 0x60, 0xC0,
    0x81, 0x03, 0x07, 0x0E, 0x1C, 0x38, 0x71, 0xE3,
    0xC6, 0x8D, 0x1A, 0x34, 0x68, 0xD0, 0xA0, 0x41,
    0x82, 0x04, 0x09, 0x12, 0x24, 0x49, 0x92, 0x25,
    0x4B, 0x97, 0x2E, 0x5C, 0xB8, 0x70, 0xE1, 0xC3,
    0x86, 0x0D, 0x1B, 0x36, 0x6D, 0xDB, 0xB7, 0x6E,
    0xDC, 0xB9, 0x72, 0xE4, 0xC8, 0x91, 0x22, 0x45,
    0x8B, 0x16, 0x2D, 0x5B, 0xB6, 0x6C, 0xD9, 0xB2,
    0x65, 0xCB, 0x96, 0x2C, 0x59, 0xB3, 0x67, 0xCE,
    0x9D, 0x3B, 0x76, 0xED, 0xDA, 0xB5, 0x6B, 0xD7,
    0xAE, 0x5D, 0xBA, 0x75, 0xEA, 0xD4, 0xA9, 0x53,
    0xA6, 0x4D, 0x9B, 0x37, 0x6F, 0xDE, 0xBC, 0x79,
    0xF3, 0xE7, 0xCF, 0x9F, 0x3E, 0x7D, 0xFA, 0xF5,
    0xEB, 0xD6, 0xAC, 0x58, 0xB1, 0x62, 0xC5, 0x8A,
    0x14, 0x28, 0x50, 0xA1, 0x43, 0x87, 0x0F, 0x1E,
    0x3D, 0x7A, 0xF4, 0xE9, 0xD3, 0xA7, 0x4F, 0x9E,
    0x3C, 0x78, 0xF1, 0xE2, 0xC4, 0x88, 0x11, 0x23,
    0x47, 0x8E, 0x1D, 0x3A, 0x74, 0xE8, 0xD1, 0xA2,
    0x44, 0x89, 0x13, 0x26, 0x4C, 0x99, 0x32, 0x64,
    0xC9, 0x93, 0x27, 0x4E, 0x9C, 0x39, 0x73, 0xE6,
    0xCD, 0x9A, 0x35, 0x6A, 0xD5, 0xAB, 0x56, 0xAD,
    0x5A, 0xB4, 0x69, 0xD2, 0xA5, 0x4A, 0x95, 0x2B,
    0x57, 0xAF, 0x5F, 0xBF, 0x7E, 0xFD, 0xFB, 0xF7,
    0xEE, 0xDD, 0xBB, 0x77, 0xEF, 0xDF, 0xBE, 0x7C,
    0xF8, 0xF0, 0xE0, 0xC1, 0x83, 0x06, 0x0C, 0x19,
    0x33, 0x66, 0xCC, 0x98, 0x30, 0x61, 0xC2, 0x84,
    0x08, 0x10, 0x21, 0x42, 0x85, 0x0A, 0x15, 0x2A,
    0x55, 0xAA, 0x54, 0xA8, 0x51, 0xA3, 0x46, 0x8C,
    0x18, 0x31, 0x63, 0xC7, 0x8F, 0x1F, 0x3F, 0x7F,
    0xFF, 0xFE, 0xFC, 0xF9, 0xF2, 0xE5, 0xCA, 0x94,
    0x29, 0x52, 0xA4, 0x48, 0x90, 0x20, 0x40, 0x80,
    0x69, 0x01, 0x02, 0x05, 0x0B, 0x17, 0x2F, 0x5E,
    0xBD, 0x7B, 0xF6, 0xEC, 0xD8, 0xB0, 0x60, 0xC0,
    0x81, 0x03, 0x07, 0x0E, 0x1C, 0x38, 0x71, 0xE3,
    0xC6, 0x8D, 0x1A, 0x34, 0x68, 0xD0, 0xA0, 0x41,
    0x82, 0x04, 0x09, 0x12, 0x24, 0x49, 0x92, 0x25,
    0x4B, 0x97, 0x2E, 0x5C, 0xB8, 0x70, 0xE1, 0xC3,
    0x86, 0x0D, 0x1B, 0x36, 0x6D, 0xDB, 0xB7, 0x6E,
    0xDC, 0xB9, 0x72, 0xE4, 0xC8, 0x91, 0x22, 0x45,
    0x8B, 0x16, 0x2D, 0x5B, 0xB6, 0x6C, 0xD9, 0xB2,
    0x65, 0xCB, 0x96, 0x2C, 0x59, 0xB3, 0x67, 0xCE,
    0x9D, 0x3B, 0x76, 0xED, 0xDA, 0xB5, 0x6B, 0xD7,
    0xAE, 0x5D, 0xBA, 0x75, 0xEA, 0xD4, 0xA9, 0x53,
    0xA6, 0x4D, 0x9B, 0x37, 0x6F, 0xDE, 0xBC, 0x79,
    0xF3, 0xE7, 0xCF, 0x9F, 0x3E, 0x7D, 0xFA, 0xF5,
    0xEB, 0xD6, 0xAC, 0x58, 0xB1, 0x62, 0xC5, 0x8A,
    0x14, 0x28, 0x50, 0xA1, 0x43, 0x87, 0x0F, 0x1E,
    0x3D, 0x7A, 0xF4, 0xE9, 0xD3, 0xA7, 0x4F, 0x9E,
    0x3C, 0x78, 0xF1, 0xE2, 0xC4, 0x88, 0x11, 0x23,
    0x47, 0x8E, 0x1D, 0x3A, 0x74, 0xE8, 0xD1, 0xA2,
    0x44, 0x89, 0x13, 0x26, 0x4C, 0x99, 0x32, 0x64,
    0xC9, 0x93, 0x27, 0x4E, 0x9C, 0x39, 0x73, 0xE6,
    0xCD, 0x9A, 0x35, 0x6A, 0xD5, 0xAB, 0x56, 0xAD,
    0x5A, 0xB4, 0x69, 0xD2, 0xA5, 0x4A, 0x95, 0x2B,
    0x57, 0xAF, 0x5F, 0xBF, 0x7E, 0xFD, 0xFB, 0xF7,
    0xEE, 0xDD, 0xBB, 0x77, 0xEF, 0xDF, 0xBE, 0x7C,
    0xF8, 0xF0, 0xE0, 0xC1, 0x83, 0x06, 0x0C, 0x19,
    0x33, 0x66, 0xCC, 0x98, 0x30, 0x61, 0xC2, 0x84,
    0x08, 0x10, 0x21, 0x42, 0x85, 0x0A, 0x15, 0x2A,
    0x55, 0xAA, 0x54, 0xA8, 0x51, 0xA3, 0x46, 0x8C,
    0x18, 0x31, 0x63, 0xC7, 0x8F, 0x1F, 0x3F, 0x7F,
    0xFF, 0xFE, 0xFC, 0xF9, 0xF2, 0xE5, 0xCA, 0x94,
    0x29, 0x52, 0xA4, 0x48, 0x90, 0x20, 0x40, 0x80
};

char *getTypeStr(U32 type)
{
   switch (type) {
   case BYTE_FILL:  return dtStr[1];
   case HALF_FILL:  return dtStr[2];
   case WORD_FILL:  return dtStr[3];
   case LONG_FILL:  return dtStr[4];
   case BYTE_NOT:   return dtStr[5];
   case HALF_NOT:   return dtStr[6];
   case WORD_NOT:   return dtStr[7];
   case LONG_NOT:   return dtStr[8];
   case BYTE_RAMP:  return dtStr[9];
   case HALF_RAMP:  return dtStr[10];
   case WORD_RAMP:  return dtStr[11];
   case LONG_RAMP:  return dtStr[12];
   case BYTE_LFSR:  return dtStr[13];
   case HALF_LFSR:  return dtStr[14];
   case WORD_DRAMP: return dtStr[15];
   case LONG_DRAMP: return dtStr[16];
   case RANDOM:     return dtStr[17];
   case RANDOMNFZ:  return dtStr[18];
   case CRPAT:      return dtStr[19];
   case CSPAT:      return dtStr[20];
   default:         return dtStr[0];
   }
}



static void
makeByteData(U8 *buff, U8 seed, U32 len)
{
    while (len) {
        *buff++ = seed;
        len--;
    }
}


static void
makeHalfData(U16 *buff, U16 seed, U32 len)
{
    len /= 2;
    while (len) {
        *buff++ = seed;
        len--;
    }
}


static void
makeWordData(U32 *buff, U32 seed, U32 len)
{
    len /= 4;
    while (len) {
        *buff++ = seed;
        len--;
    }
}


static void
makeLongData(U64 *buff, U64 seed, U32 len)
{
    len /= 8;
    while (len) {
        *buff++ = seed;
        len--;
    }
}


static void
makeByteNotData(U8 *buff, U8 seed, U32 len)
{
    while (len) {
        *buff++ = seed;
        if (!--len) break;
        *buff++ = ~seed;
        len--;
    }
}


static void
makeHalfNotData(U16 *buff, U16 seed, U32 len)
{
    len /= 2;
    while (len) {
        *buff++ = seed;
        if (!--len) break;
        *buff++ = ~seed;
        len--;
    }
}


static void
makeWordNotData(U32 *buff, U32 seed, U32 len)
{
    len /= 4;
    while (len) {
        *buff++ = seed;
        if (!--len) break;
        *buff++ = ~seed;
        len--;
    }
}


static void
makeLongNotData(U64 *buff, U64 seed, U32 len)
{
    len /= 8;
    while (len) {
        *buff++ = seed;
        if (!--len) break;
        *buff++ = ~seed;
        len--;
    }
}


static void
makeByteRampData(U8 *buff, U8 seed, U32 len)
{
    while (len--)
        *buff++ = seed++;
}


static void
makeHalfRampData(U16 *buff, U16 seed, U32 len)
{
    len /= 2;
    while (len--)
        *buff++ = seed++;
}


static void
makeWordRampData(U32 *buff, U32 seed, U32 len)
{
    len /= 4;
    while (len--)
        *buff++ = seed++;
}

/*
 * Useful for tests covering > 1MB and <= 256Mb chunks
 * gets MSB incrementing across megabytes
 */
static void
makeWordDrampData(U32 *buff, U32 seed, U32 len)
{
    U32 i = 0;

    len /= 4;
    while (len--) {
        *buff++ = seed++;
        i += 4;
        if (i == MEG) {
            seed += 0x01000000;
            i = 0;
        }
    }
}


static void
makeLongRampData(U64 *buff, U64 seed, U32 len)
{
    len /= 8;
    while (len--)
        *buff++ = seed++;
}


static void
makeLongDrampData(U64 *buff, U64 seed, U32 len)
{
    U32 i = 0;

    len /= 8;
    while (len--) {
        if (i & 1)
            *buff++ = ++seed;
        else {
            if (i)
               seed += 0x1000000000;  /* nine zeros */
            *buff++ = seed;
        }
        i++;
    }

}


static void
makeByteLfsrData(U8 *buff, U8 seed, U32 len)
{
    while (len >= 256) {
/*      memcpy(buff, &lfsrData[seed & 0xFE], 256); */
        bcopy(&lfsrData[seed & 0xFE], buff, 256);
        buff += 256;
        len -= 256;
    }
    if (len)
/*      memcpy(buff, &lfsrData[seed & 0xFE], len); */
        bcopy(&lfsrData[seed & 0xFE], buff, len);
}


static void
makeRandomData(U8 *buff, U32 len, U32 nfz)
{
    U32 i, tmp1, tmp2;
    U8 db1, db2;
    U8 x = 1;
    U8 b;

    tmp1 = ~(r5k_get_count() >> 3);
    db1 = tmp1 + 25;
    db2 = tmp1 - 25;

    while (len >= 16) {
        tmp2 = r5k_get_count();
        for (i=0; i<16; i++) {
            switch (i) {
              case  0:
              case  8: db1 += tmp2;
                       break;
              case  1: 
              case  9: db2 += tmp1 ^ tmp2;
                       break;
              case  2:
              case  4:
              case  6:
              case 10:
              case 12:
              case 14: db1 += lfsrData[db1]; 
                       break;
              case  3:
              case  5:
              case  7:
              case 11:
              case 13:
              case 15: db2 += tmp2 >> i;
                       break;
            }
            b = db1 ^ db2;
            *buff++ = nfz ? ((b == 0) || (b == 0xFF)) ? x++ : b : b;
            len--;
            if (x == 200)
                x = 1;
        } // i
    }

    while (len) {
        tmp1 = r5k_get_count();
        db1 = tmp1;
        db2 = tmp1 >> 5;
        b = db1 ^ db2;
        *buff++ = nfz ? ((b == 0) || (b == 0xFF)) ? x++ : b : b;
        if (x == 200)
            x = 1;
        len--;
    }
}


static void
makeCompliantRPATdata(U8 *buff, U32 len)
{
    static U8 crpat1x5[] = {
        0xbc, 0xbc, 0x23, 0x47, 0x6b, 0x8f,
        0xb3, 0xd7, 0xfb, 0x14, 0x36, 0x59,
        0xbc, 0xbc, 0x23, 0x47, 0x6b, 0x8f, /* repeat */
        0xb3, 0xd7, 0xfb, 0x14, 0x36, 0x59,
        0xbc, 0xbc, 0x23, 0x47, 0x6b, 0x8f, /* repeat */
        0xb3, 0xd7, 0xfb, 0x14, 0x36, 0x59,
        0xbc, 0xbc, 0x23, 0x47, 0x6b, 0x8f, /* repeat */
        0xb3, 0xd7, 0xfb, 0x14, 0x36, 0x59,
        0xbc, 0xbc, 0x23, 0x47, 0x6b, 0x8f, /* repeat */
        0xb3, 0xd7, 0xfb, 0x14, 0x36, 0x59 };

    while (len >= 60) {
/*      memcpy(buff, &crpat1x5[0], 60); */
        bcopy(&crpat1x5[0], buff, 60);
        buff += 60;
        len -= 60;
    }
    if (len)
/*      memcpy(buff, &crpat1x5[0], len); */
        bcopy(&crpat1x5[0], buff, len);
}


static void
makeCompliantSPATdata(U32 *buff, U32 len)
{
    makeWordData(buff, 0x7e7e7e7e, len);
}


/**********************************************************************/
//  genData()
//  Fill the buffer buff with len bytes of data described by type and
//  seed.  Note that len is always expressed in BYTES, regardless of
//  the type specified.  Add types as desired.
/**********************************************************************/

void
genData(void *buff, U32 type, U64 seed, U32 len)
{
    switch (type) {
        case BYTE_FILL :
            makeByteData((U8 *)buff, (U8)seed, len);
            break;
        case HALF_FILL :
            makeHalfData((U16 *)buff, (U16)seed, len);
            break;
        case WORD_FILL :
            makeWordData((U32 *)buff, (U32)seed, len);
            break;
        case LONG_FILL :
            makeLongData((U64 *)buff, seed, len);
            break;
        case BYTE_NOT :
            makeByteNotData((U8 *)buff, (U8)seed, len);
            break;
        case HALF_NOT :
            makeHalfNotData((U16 *)buff, (U16)seed, len);
            break;
        case WORD_NOT :
            makeWordNotData((U32 *)buff, (U32)seed, len);
            break;
        case LONG_NOT :
            makeLongNotData((U64 *)buff, seed, len);
            break;
        case BYTE_RAMP :
            makeByteRampData((U8 *)buff, (U8)seed, len);
            break;
        case HALF_RAMP :
            makeHalfRampData((U16 *)buff, (U16)seed, len);
            break;
        case WORD_RAMP :
            makeWordRampData((U32 *)buff, (U32)seed, len);
            break;
        case LONG_RAMP :
            makeLongRampData((U64 *)buff, seed, len);
            break;
        case BYTE_LFSR :
        case HALF_LFSR : /* half not really here just yet */
            makeByteLfsrData((U8 *)buff, (U8)seed, len);
            break;
        case WORD_DRAMP :
            makeWordDrampData((U32 *)buff, (U32)seed, len);
            break;
        case LONG_DRAMP :
            makeLongDrampData((U64 *)buff, seed, len);
            break;
        case RANDOM :
            makeRandomData((U8 *)buff, len, 0);
            break;
        case RANDOMNFZ :
            makeRandomData((U8 *)buff, len, 1);
            break;
        case CRPAT :
            makeCompliantRPATdata((U8 *)buff, len);
            break;
        case CSPAT :
            makeCompliantSPATdata((U32 *)buff, len);
            break;
    }
}


U64 getSeed()
{
   U64 qp;

   genData(&qp, RANDOM, 0, 8);
   return (qp);
}


static void
asciiOut(U8 *p)
{
    U32 i;

    for (i = 0; i < 16; i++)
        putchar((p[i] > 0x1f && p[i] < 0x7f) ? p[i] : '.');
}

void
dumpData(void *p, OP_TYPE op, U32 nLines)
{
    U32 i, *qp = (U32*) p;
    U8  *bp = (U8*) p;
    U16 *hp = (U16*) p;
    U64 *lp = (U64*) p;

    for (i = 0; i < nLines; i++) {
        switch (op) {
        case OP_BYTE:
            printf("\n%08lX  %02x %02x %02x %02x %02x %02x %02x %02x"
              "  %02x %02x %02x %02x %02x %02x %02x %02x  ",
              bp, bp[0], bp[1], bp[2], bp[3], bp[4], bp[5], bp[6],
              bp[7], bp[8], bp[9], bp[10], bp[11], bp[12], bp[13],
              bp[14], bp[15]);
            asciiOut(bp);
            bp += 16;
            break;
        case OP_HALF:
            printf("\n%08lX  %04x %04x %04x %04x  %04x %04x %04x %04x  ",
              hp, hp[0], hp[1], hp[2], hp[3],
              hp[4], hp[5], hp[6], hp[7]);
            asciiOut((U8*)hp);
            hp += 8;
            break;
        default:
        case OP_WORD:
            printf("\n%08lX  %08x %08x %08x %08x  ",
              qp, qp[0], qp[1], qp[2], qp[3]);
            asciiOut((U8*)qp);
            qp += 4;
            break;
        case OP_LONG:
            printf("\n%08lX  %016Lx %016Lx  ", lp, lp[0], lp[1]);
            asciiOut((U8*)lp);
            lp += 2;
        break;
        }
    }

    putchar(NEWLINE);

}


void
dataTypeShow(U64 seed)
{
    U8 b[16];
    U16 s[8];
    U32 w[4];
    U64 d[3];


    printf("\nPattern     value   example\n");

    genData(b, BYTE_FILL, seed, 16);
    printf("\nByte Fill     1     %02x %02x %02x %02x %02x %02x %02x %02x"
        " %02x %02x %02x %02x %02x %02x %02x %02x", b[0], b[1], b[2], b[3],
        b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13],
        b[14], b[15]);

    genData(s, HALF_FILL, seed, 16);
    printf("\nHalf Fill     2     %04x  %04x  %04x  %04x  %04x  %04x  %04x"
        "  %04x", s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]);

    genData(w, WORD_FILL, seed, 16);
    printf("\nWord Fill     4     %08x     %08x     %08x     %08x",
         w[0], w[1], w[2], w[3]);

    genData(d, LONG_FILL, seed, 24);
    printf("\nLong Fill     8     %016Lx  %016Lx  %016Lx",
         d[0], d[1], d[2]);

    genData(b, BYTE_NOT, seed, 16);
    printf("\nByte Not     11     %02x %02x %02x %02x %02x %02x %02x %02x"
        " %02x %02x %02x %02x %02x %02x %02x %02x", b[0], b[1], b[2], b[3],
        b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13],
        b[14], b[15]);

    genData(s, HALF_NOT, seed, 16);
    printf("\nHalf Not     12     %04x  %04x  %04x  %04x  %04x  %04x  %04x"
        "  %04x", s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]);

    genData(w, WORD_NOT, seed, 16);
    printf("\nWord Not     14     %08x     %08x     %08x     %08x",
         w[0], w[1], w[2], w[3]);

    genData(d, LONG_NOT, seed, 24);
    printf("\nLong Not     18     %016Lx  %016Lx  %016Lx",
         d[0], d[1], d[2]);

    genData(b, BYTE_RAMP, seed, 16);
    printf("\nByte Ramp    21     %02x %02x %02x %02x %02x %02x %02x %02x"
        " %02x %02x %02x %02x %02x %02x %02x %02x", b[0], b[1], b[2], b[3],
        b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13],
        b[14], b[15]);

    genData(s, HALF_RAMP, seed, 16);
    printf("\nHalf Ramp    22     %04x  %04x  %04x  %04x  %04x  %04x  %04x"
        "  %04x", s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]);

    genData(w, WORD_RAMP, seed, 16);
    printf("\nWord Ramp    24     %08x     %08x     %08x     %08x",
         w[0], w[1], w[2], w[3]);

    genData(d, LONG_RAMP, seed, 24);
    printf("\nLong Ramp    28     %016Lx  %016Lx  %016Lx",
         d[0], d[1], d[2]);

    genData(b, BYTE_LFSR, seed, 16);
    printf("\nByte LFSR    31     %02x %02x %02x %02x %02x %02x %02x %02x"
        " %02x %02x %02x %02x %02x %02x %02x %02x", b[0], b[1], b[2], b[3],
        b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13],
        b[14], b[15]);

    genData(w, WORD_DRAMP, seed, 16);
    printf("\nWord Dramp   34     %08x     %08x     %08x     %08x",
         w[0], w[1], w[2], w[3]);

    genData(d, LONG_DRAMP, seed, 24);
    printf("\nLong Dramp   38     %016Lx  %016Lx  %016Lx",
         d[0], d[1], d[2]);

    genData(b, RANDOM, seed, 16);
    printf("\nRandom       41     %02x %02x %02x %02x %02x %02x %02x %02x"
        " %02x %02x %02x %02x %02x %02x %02x %02x", b[0], b[1], b[2], b[3],
        b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13],
        b[14], b[15]);

    genData(b, RANDOMNFZ, seed, 16);
    printf("\nRandomNFZ    42     %02x %02x %02x %02x %02x %02x %02x %02x"
        " %02x %02x %02x %02x %02x %02x %02x %02x", b[0], b[1], b[2], b[3],
        b[4], b[5], b[6], b[7], b[8], b[9], b[10], b[11], b[12], b[13],
        b[14], b[15]);

    putchar(NEWLINE);
}

