/*@*********************************************************************** 
 |                                                                         
 |             Copyright (c) 1995-1997 XACT Incporated                     
 |                                                                         
 | PROPRIETARY RIGHTS of XACT Incorporated are involved in the subject     
 | matter of this material.  All manufacturing, reproduction, use, and     
 | sales rights pertaining to this subject matter are governed by the      
 | license agreement.  The recipient of this software implicitly accepts   
 | the terms of the license.                                               
 |                                                                         
 |                                                                         
 | FILE NAME   : asn1.c                                 
 | VERSION     : 1.1  
 | COMPONENT   : XSNMPv1
 | DESCRIPTION : ASN encode/decode routines                                
 | AUTHOR      : Robert Winter                                              
 *************************************************************************/
#include "xport.h"
#include "xtypes.h"
#include "xtern.h"
#include "xsnmp.h"
#include "snmp.h"
#include "agent.h"
#include "link.h"
#include "prott.h"
#include "mac.h"
#include "xcfig.h"
#include "asn1.h"

i32 asn1ErrStatus = ASN1_ERR_NOERROR;

void 
Asn1Opn( asn1_sck_t *Asn1, u8 *Buf, u32 Len, u32 Mde )
{
    Asn1->Begin = Buf;
    Asn1->End = Buf + Len;
    Asn1->Pointer = (Mde == ASN1_ENC) ? Buf + Len : Buf;
}

void 
Asn1Cls( asn1_sck_t *Asn1, u8 **Buf, u32 *Len )
{
    *Buf = Asn1->Pointer;
    *Len = (u32)(Asn1->End - Asn1->Pointer);
}

bool 
Asn1OctEnc( asn1_sck_t *Asn1, u8 Chr )
{
    if (Asn1->Pointer <= Asn1->Begin) {
		asn1ErrStatus = ASN1_ERR_ENC_FULL;
        return FALSE;
	}
    *--(Asn1->Pointer) = Chr;
    return TRUE;
}

bool 
Asn1OctDec( asn1_sck_t *Asn1, u8 *Chr )
{
    if (Asn1->Pointer >= Asn1->End) {
		asn1ErrStatus = ASN1_ERR_DEC_EMPTY;
        return FALSE;
	}
    *Chr = *(Asn1->Pointer)++;
    return TRUE;
}

bool 
Asn1TagEnc( asn1_sck_t *Asn1, u32 Tag )
{
u8 Chr;

    Chr = (u8) (Tag & 0x7F);
    Tag >>= 7;
    if (!Asn1OctEnc (Asn1, Chr))
        return FALSE;
    while (Tag > 0) {
        Chr = (u8) (Tag | 0x80);
        Tag >>= 7;
        if (!Asn1OctEnc (Asn1, Chr))
            return FALSE;
    }
    return TRUE;
}

bool 
Asn1TagDec( asn1_sck_t *Asn1, u32 *Tag )
{
u8 Chr;

    *Tag = 0;
    do {
        if (!Asn1OctDec (Asn1, &Chr))
            return FALSE;
        *Tag <<= 7;
        *Tag |= Chr & 0x7F;
    } while ((Chr & 0x80) == 0x80);
    return TRUE;
}

bool 
Asn1IdrEnc( asn1_sck_t *Asn1, u32 Cls, u32 Con, u32 Tag)
{
u8 Chr;

    if (Tag >= 0x1F) {
        if (!Asn1TagEnc (Asn1, Tag))
            return FALSE;
        Tag = 0x1F;
    }
    Chr = (u8) ((Cls << 6) | (Con << 5) | (Tag));
    if (!Asn1OctEnc (Asn1, Chr))
        return FALSE;
    return TRUE;
}

bool 
Asn1IdrDec( asn1_sck_t *Asn1, u32 *Cls, u32 *Con, u32 *Tag)
{
u8 Chr;

    if (!Asn1OctDec (Asn1, &Chr))
        return FALSE;
    *Cls = (Chr & 0xC0) >> 6;
    *Con = (Chr & 0x20) >> 5;
    *Tag = (Chr & 0x1F);
    if (*Tag == 0x1F) {
        if (!Asn1TagDec (Asn1, Tag))
            return FALSE;
    }
    return TRUE;
}

bool 
Asn1LenEnc( asn1_sck_t *Asn1, u32 Def, u32 Len )
{
u8 Chr, Cnt;

    if (!Def) {
        Chr = 0x80;
	} else {
        if (Len < 0x80) {
            Chr = (u8) Len;
		} else {
            Cnt = 0;
            while (Len > 0) {
                Chr = (u8) Len;
                Len >>= 8;
                if (!Asn1OctEnc (Asn1, Chr))
                    return FALSE;
                Cnt++;
            }
            Chr = (u8) (Cnt | 0x80);
        }
    }
    if (!Asn1OctEnc (Asn1, Chr))
        return FALSE;
    return TRUE;
}

bool 
Asn1LenDec( asn1_sck_t *Asn1, u32 *Def, u32 *Len )
{
u8 Chr, Cnt;

    if (!Asn1OctDec (Asn1, &Chr))
        return FALSE;
    if (Chr == 0x80) {
        *Def = 0;
	} else {
        *Def = 1;
        if (Chr < 0x80) {
            *Len = Chr;
		} else {
            Cnt = (u8) (Chr & 0x7F);
            *Len = 0;
            while (Cnt > 0) {
                if (!Asn1OctDec (Asn1, &Chr))
                    return FALSE;
                *Len <<= 8;
                *Len |= Chr;
                Cnt--;
            }
        }
    }
    return TRUE;
}

bool 
Asn1HdrEnc( asn1_sck_t *Asn1, u8 *Eoc, u32 Cls, u32 Con, u32 Tag )
{
u32 Def, Len;

    if (Eoc == 0) {
        Def = 0;
        Len = 0;
    } else {
        Def = 1;
        Len = (u32)(Eoc - Asn1->Pointer);
    }
    if (!Asn1LenEnc (Asn1, Def, Len))
        return FALSE;
    if (!Asn1IdrEnc (Asn1, Cls, Con, Tag))
        return FALSE;
    return TRUE;
}

bool 
Asn1HdrDec( asn1_sck_t *Asn1, u8 **Eoc, u32 *Cls, u32 *Con, u32 *Tag )
{
u32 Def, Len;

    if(!Asn1IdrDec (Asn1, Cls, Con, Tag))
        return FALSE;
    if(!Asn1LenDec (Asn1, &Def, &Len))
        return FALSE;
    if(Def)
        *Eoc = Asn1->Pointer + Len;
    else
        *Eoc = 0;
    return TRUE;
}


bool 
Asn1Eoc( asn1_sck_t *Asn1, u8 *Eoc )
{
    if (Eoc == 0)
        return (Asn1->Pointer [0] == 0x00 && Asn1->Pointer [1] == 0x00);
    else
        return (Asn1->Pointer >= Eoc);
}

bool 
Asn1EocEnc( asn1_sck_t *Asn1, u8 **Eoc )        
{
    if (Eoc == 0) {
        if (!Asn1OctEnc (Asn1, 0x00))
            return FALSE;
        if (!Asn1OctEnc (Asn1, 0x00))
            return FALSE;
        return TRUE;
    } else {
        *Eoc = Asn1->Pointer;
        return TRUE;
    }
}

bool 
Asn1EocDec( asn1_sck_t *Asn1, u8 *Eoc )
{
u8 Chr;

    if (Eoc == 0) {
        if (!Asn1OctDec (Asn1, &Chr))
            return FALSE;
        if (Chr != 0x00) {
			asn1ErrStatus = ASN1_ERR_DEC_EOC_MISMATCH;
            return FALSE;
		}
        if (!Asn1OctDec (Asn1, &Chr))
            return FALSE;
        if (Chr != 0x00) {
			asn1ErrStatus = ASN1_ERR_DEC_EOC_MISMATCH;
            return FALSE;
		}
        return TRUE;
    } else {
        if (Asn1->Pointer != Eoc) {
			asn1ErrStatus = ASN1_ERR_DEC_LENGTH_MISMATCH;
            return FALSE;
		}
        return TRUE;
    }
}

bool 
Asn1NulEnc( asn1_sck_t *Asn1, u8 **Eoc )
{
    *Eoc = Asn1->Pointer;
    return TRUE;
}

bool 
Asn1NulDec( asn1_sck_t *Asn1, u8 *Eoc )
{
    Asn1->Pointer = Eoc;
    return TRUE;
}

bool Asn1BolEnc( asn1_sck_t *Asn1, u8 **Eoc, bool Bol )
{
u8 Chr;

    *Eoc = Asn1->Pointer;
    Chr = (u8) (Bol ? 0xFF : 0x00);
    if (!Asn1OctEnc (Asn1, Chr))
        return FALSE;
    return TRUE;
}

bool 
Asn1BolDec( asn1_sck_t *Asn1, u8 *Eoc, bool *Bol )
{
u8 Chr;

    if (!Asn1OctDec (Asn1, &Chr))
        return FALSE;
    *Bol = Chr ? 1 : 0;
    if (Asn1->Pointer != Eoc) {
		asn1ErrStatus = ASN1_ERR_DEC_LENGTH_MISMATCH;
        return FALSE;
	}
    return TRUE;
}

bool 
Asn1IntEnc( asn1_sck_t *Asn1, u8 **Eoc, i32 Int )
{
u8  Chr,Sgn;
i32 Lim;

    *Eoc = Asn1->Pointer;
    if (Int < 0) {
        Lim = -1;
        Sgn = 0x80;
    } else {
        Lim = 0;
        Sgn = 0x00;
    }
    do {
        Chr = (u8) Int;
        Int >>= 8;
        if (!Asn1OctEnc (Asn1, Chr))
            return FALSE;
    } while ((Int != Lim) || (u8) (Chr & 0x80) != Sgn);
    return TRUE;
}

bool 
Asn1IntDec( asn1_sck_t *Asn1, u8 *Eoc, i32 *Int )
{
u8 Chr;
u32 Len;

    if (!Asn1OctDec (Asn1, &Chr))
        return FALSE;
    *Int = (i32) Chr;
    Len = 1;
    while (Asn1->Pointer < Eoc) {
        if (++Len > sizeof (i32)) {
			asn1ErrStatus = ASN1_ERR_DEC_BADVALUE;
            return FALSE;
		}
        if (!Asn1OctDec (Asn1, &Chr))
            return FALSE;
        *Int <<= 8;
        *Int |= Chr;
    }
    return TRUE;
}

bool 
Asn1IntEncLng( asn1_sck_t *Asn1, u8 **Eoc, u32 Int )
{
u8 Chr, Sgn;
u32 Lim;

    *Eoc = Asn1->Pointer;
    if (Int < 0) {
        Lim = -1;
        Sgn = 0x80;
    } else {
        Lim = 0;
        Sgn = 0x00;
    }
    do {
        Chr = (u8) Int;
        Int >>= 8;
        if (!Asn1OctEnc (Asn1, Chr))
            return FALSE;
    } while ((Int != Lim) || (u8) (Chr & 0x80) != Sgn);
    return TRUE;
}

bool 
Asn1IntDecLng( asn1_sck_t *Asn1, u8 *Eoc, u32 *Int )
{
u8 Chr;
u32 Len;

    if (!Asn1OctDec (Asn1, &Chr))
        return FALSE;
    *Int = (i8) Chr;
    Len = 1;
    while (Asn1->Pointer < Eoc) {
        if (++Len > sizeof (u32)) {
			asn1ErrStatus = ASN1_ERR_DEC_BADVALUE;
            return FALSE;
		}
        if (!Asn1OctDec (Asn1, &Chr))
            return FALSE;
        *Int <<= 8;
        *Int |= Chr;
    }
    return TRUE;
}

bool 
Asn1IntEncUns( asn1_sck_t *Asn1, u8  **Eoc, u32 Int )
{
u8 Chr;

    *Eoc = Asn1->Pointer;
    do { Chr = (u8) Int;
        Int >>= 8;
        if (!Asn1OctEnc (Asn1, Chr))
            return FALSE;
    } while ((Int != 0) || (Chr & 0x80) != 0x00);
    return TRUE;
}

bool 
Asn1IntDecUns( asn1_sck_t *Asn1, u8 *Eoc, u32 *Int )
{
u8 Chr;
u32 Len;

    if (!Asn1OctDec (Asn1, &Chr))
        return FALSE;
    *Int = Chr;
    if (Chr == 0)
        Len = 0;
    else
        Len = 1;
    while (Asn1->Pointer < Eoc) {
        if (++Len > sizeof (i32)) {
			asn1ErrStatus = ASN1_ERR_DEC_BADVALUE;
            return FALSE;
		}
        if (!Asn1OctDec (Asn1, &Chr))
            return FALSE;
        *Int <<= 8;
        *Int |= Chr;
    }
    return TRUE;
}

bool 
Asn1IntEncLngUns( asn1_sck_t *Asn1, u8 **Eoc, u32 Int )
{
u8 Chr;

    *Eoc = Asn1->Pointer;
    do {
        Chr = (u8) Int;
        Int >>= 8;
        if (!Asn1OctEnc (Asn1, Chr))
            return FALSE;
    } while ((Int != 0) || (Chr & 0x80) != 0x00);
    return TRUE;
}

bool 
Asn1IntDecLngUns( asn1_sck_t *Asn1, u8 *Eoc, u32 *Int )
{
u8 Chr;
u32 Len;

    if (!Asn1OctDec (Asn1, &Chr))
        return FALSE;
    *Int = Chr;
    if (Chr == 0)
        Len = 0;
    else
        Len = 1;
    while (Asn1->Pointer < Eoc) {
        if (++Len > sizeof (u32)) {
			asn1ErrStatus = ASN1_ERR_DEC_BADVALUE;
            return FALSE;
		}
        if (!Asn1OctDec (Asn1, &Chr))
            return FALSE;
        *Int <<= 8;
        *Int |= Chr;
    }
    return TRUE;
}

bool 
Asn1BtsEnc( asn1_sck_t *Asn1, u8 **Eoc, u8 *Bts, u32	BtsLen, u8 BtsUnu )
{
    *Eoc = Asn1->Pointer;
    Bts += BtsLen;
    while (BtsLen-- > 0) {
        if (!Asn1OctEnc (Asn1, *--Bts))
            return FALSE;
    }
    if (!Asn1OctEnc (Asn1, BtsUnu))
        return FALSE;
    return TRUE;
}

bool 
Asn1BtsDec( asn1_sck_t *Asn1, u8 *Eoc, u8 *Bts, u32 BtsSze, u32 *BtsLen, u8 *BtsUnu )
{
    if (!Asn1OctDec (Asn1, BtsUnu))
        return FALSE;
    *BtsLen = 0;
    while (Asn1->Pointer < Eoc) {
        if (++(*BtsLen) > BtsSze) {
			asn1ErrStatus = ASN1_ERR_DEC_BADVALUE;
            return FALSE;
		}
        if (!Asn1OctDec (Asn1, (u8 *)Bts++))
            return FALSE;
    }
    return TRUE;
}

bool 
Asn1OtsEnc( asn1_sck_t *Asn1, u8 **Eoc, u8 *Ots, u32 OtsLen )
{
    *Eoc = Asn1->Pointer;
    Ots += OtsLen;
    while (OtsLen-- > 0) {
        if (!Asn1OctEnc (Asn1, *--Ots))
            return FALSE;
    }
    return TRUE;
}

bool 
Asn1OtsDec( asn1_sck_t *Asn1, u8 *Eoc, u8 *Ots, u32 OtsSze, u32 *OtsLen)
{
    *OtsLen = 0;
    while (Asn1->Pointer < Eoc) {
        if (++(*OtsLen) > OtsSze) {
			asn1ErrStatus = ASN1_ERR_DEC_BADVALUE;
            return FALSE;
		}
        if (!Asn1OctDec (Asn1, (u8 *)Ots++))
            return FALSE;
    }
    return TRUE;
}

bool 
Asn1SbiEnc( asn1_sck_t *Asn1, u32 Sbi)
{
u8 Chr;

    Chr = (u8) (Sbi & 0x7F);
    Sbi >>= 7;
    if (!Asn1OctEnc (Asn1, Chr))
        return FALSE;
    while (Sbi > 0) {
        Chr = (u8) (Sbi | 0x80);
        Sbi >>= 7;
        if (!Asn1OctEnc (Asn1, Chr))
            return FALSE;
    }
    return TRUE; 
}

bool 
Asn1SbiDec( asn1_sck_t *Asn1, u32 *Sbi )
{
u8 Chr;

    *Sbi = 0;
    do {
        if (!Asn1OctDec (Asn1, &Chr))
            return FALSE;
        *Sbi <<= 7;
        *Sbi |= Chr & 0x7F;
    } while ((Chr & 0x80) == 0x80);
    return TRUE;
}

bool 
Asn1OjiEnc( asn1_sck_t *Asn1, u8 **Eoc, u32 *Oji, u32 OjiLen )
{
u32 Sbi;

    *Eoc = Asn1->Pointer;
    if (OjiLen < 2) {
		asn1ErrStatus = ASN1_ERR_ENC_BADVALUE;
        return FALSE;
	}
    Sbi = Oji [1] + Oji [0] * 40;
    Oji += OjiLen;
    while (OjiLen-- > 2) {
        if (!Asn1SbiEnc (Asn1, *--Oji)) {
            return FALSE;
		}
    }
    if (!Asn1SbiEnc (Asn1, Sbi)) {
        return FALSE;
	}
    return TRUE;
}

bool 
Asn1OjiDec( asn1_sck_t *Asn1, u8 *Eoc, u32 *Oji, u32 OjiSze, u32 *OjiLen )
{
u32 Sbi;

    if (OjiSze < 2) {
		asn1ErrStatus = ASN1_ERR_DEC_BADVALUE;
        return FALSE;
	}
    if (!Asn1SbiDec (Asn1, &Sbi))
        return FALSE;
    if (Sbi < 40) {
        Oji [0] = 0;
        Oji [1] = Sbi;
    } else {
		if (Sbi < 80) {
    	    Oji [0] = 1;
    	    Oji [1] = Sbi - 40;
    	} else {
    	    Oji [0] = 2;
    	    Oji [1] = Sbi - 80;
    	}
	}
    *OjiLen = 2;
    Oji += 2;
    while (Asn1->Pointer < Eoc) {
        if (++(*OjiLen) > OjiSze) {
			asn1ErrStatus = ASN1_ERR_DEC_BADVALUE;
        	return FALSE;
		}
        if (!Asn1SbiDec (Asn1, Oji++))
            return FALSE;
    }
    return TRUE;
}

