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
 | FILE NAME   :                                        
 | VERSION     :      
 | COMPONENT   :
 | DESCRIPTION :                                                           
 | AUTHOR      :                                                            
 *************************************************************************/
#ifndef _ASN1_H_
#define _ASN1_H_

/*------------- Class ----------------------*/
#define ASN1_UNI       0     /* Universal   */
#define ASN1_APL       1     /* Application */
#define ASN1_CTX       2     /* Context     */
#define ASN1_PRV       3     /* Private     */

/*------------- Tag -------------------------------*/
#define ASN1_EOC       0     /* End Of Contents    */
#define ASN1_BOL       1     /* Boolean            */
#define ASN1_INT       2     /* Integer            */
#define ASN1_BTS       3     /* Bit String         */
#define ASN1_OTS       4     /* Octet String       */
#define ASN1_NUL       5     /* Null               */
#define ASN1_OJI       6     /* Object Identifier  */
#define ASN1_OJD       7     /* Object Description */
#define ASN1_EXT       8     /* External           */
#define ASN1_SEQ       16    /* Sequence           */
#define ASN1_SET       17    /* Set                */
#define ASN1_NUMSTR    18    /* Numerical String   */
#define ASN1_PRNSTR    19    /* Printable String   */
#define ASN1_TEXSTR    20    /* Teletext String    */
#define ASN1_VIDSTR    21    /* Video String       */
#define ASN1_IA5STR    22    /* IA5 String         */
#define ASN1_UNITIM    23    /* Universal Time     */
#define ASN1_GENTIM    24    /* General Time       */
#define ASN1_GRASTR    25    /* Graphical String   */
#define ASN1_VISSTR    26    /* Visible String     */
#define ASN1_GENSTR    27    /* General String     */

/*------------- Primitive/Constructed -----------------*/
#define ASN1_PRI     0       /* Primitive              */
#define ASN1_CON     1       /* Constructed            */

/*------------- Mode to open ASN11 ---------------*/
#define ASN1_ENC     0       /* Encoding          */
#define ASN1_DEC     1       /* Decoding          */
 

#define ASN1_ERR_NOERROR				0
#define ASN1_ERR_ENC_FULL				1
#define ASN1_ERR_DEC_EMPTY				2
#define ASN1_ERR_DEC_EOC_MISMATCH		3
#define ASN1_ERR_DEC_LENGTH_MISMATCH	4
#define ASN1_ERR_DEC_BADVALUE			5
#define ASN1_ERR_ENC_BADVALUE			6

typedef struct asn1_sck_s asn1_sck_t;

struct asn1_sck_s {        		/* ASN1 socket                         */
	u8		*Pointer;        	/* Octet just encoded or to be decoded */
	u8		*Begin;          	/* First octet                         */
	u8		*End;            	/* Octet after last octet              */
};

extern i32 asn1ErrStatus;

void Asn1Opn __((asn1_sck_t *Asn1, u8 *Buf, u32 Len, u32 Mde));
void Asn1Cls __((asn1_sck_t *Asn1, u8 **Buf, u32 *Len));
bool Asn1OctEnc __((asn1_sck_t *Asn1, u8 Chr));
bool Asn1OctDec __((asn1_sck_t *Asn1, u8 *Chr));
bool Asn1TagEnc __((asn1_sck_t *Asn1, u32 Tag));
bool Asn1TagDec __((asn1_sck_t *Asn1, u32 *Tag));
bool Asn1IdrEnc __((asn1_sck_t *Asn1, u32 Cls, u32 Con, u32 Tag));
bool Asn1IdrDec __((asn1_sck_t *Asn1, u32 *Cls, u32 *Con, u32 *Tag));
bool Asn1LenEnc __((asn1_sck_t *Asn1, u32 Def, u32 Len));
bool Asn1LenDec __((asn1_sck_t *Asn1, u32 *Def, u32 *Len));
bool Asn1HdrEnc __((asn1_sck_t *Asn1, u8 *Eoc, u32 Cls, u32 Con, u32 Tag));
bool Asn1HdrDec __((asn1_sck_t *Asn1, u8 **Eoc, u32 *Cls, u32 *Con, u32 *Tag));
bool Asn1Eoc __((asn1_sck_t *Asn1, u8 *Eoc));
bool Asn1EocEnc __((asn1_sck_t *Asn1, u8 **Eoc));
bool Asn1EocDec __((asn1_sck_t *Asn1, u8 *Eoc));
bool Asn1NulEnc __((asn1_sck_t *Asn1, u8 **Eoc));
bool Asn1NulDec __((asn1_sck_t *Asn1, u8 *Eoc));
bool Asn1BolEnc __((asn1_sck_t *Asn1, u8 **Eoc, bool Bol));
bool Asn1BolDec __((asn1_sck_t *Asn1, u8 *Eoc, bool *Bol));
bool Asn1IntEnc __((asn1_sck_t *Asn1, u8 **Eoc, i32 Int));
bool Asn1IntDec __((asn1_sck_t *Asn1, u8 *Eoc, i32 *Int));
bool Asn1IntEncLng __((asn1_sck_t *Asn1, u8 **Eoc, u32 Int));
bool Asn1IntDecLng __((asn1_sck_t *Asn1, u8 *Eoc, u32 *Int));
bool Asn1IntEncUns __((asn1_sck_t *Asn1, u8 **Eoc, u32 Int));
bool Asn1IntDecUns __((asn1_sck_t *Asn1, u8 *Eoc, u32 *Int));
bool Asn1IntEncLngUns __((asn1_sck_t *Asn1, u8 **Eoc, u32 Int));
bool Asn1IntDecLngUns __((asn1_sck_t *Asn1, u8 *Eoc, u32 *Int));
bool Asn1BtsEnc __((asn1_sck_t *Asn1, u8 **Eoc, u8 *Bts, u32 BtsLen, u8 BtsUnu));
bool Asn1BtsDec __((asn1_sck_t *Asn1, u8 *Eoc, u8 *Bts, u32 BtsSze, u32 *BtsLen, u8 *BtsUnu));
bool Asn1OtsEnc __((asn1_sck_t *Asn1, u8 **Eoc, u8 *Ots, u32 OtsLen));
bool Asn1OtsDec __((asn1_sck_t *Asn1, u8 *Eoc, u8 *Ots, u32 OtsSze, u32 *OtsLen));
bool Asn1SbiEnc __((asn1_sck_t *Asn1, u32 Sbi));
bool Asn1SbiDec __((asn1_sck_t *Asn1, u32 *Sbi));
bool Asn1OjiEnc __((asn1_sck_t *Asn1, u8 **Eoc, u32 *Oji, u32 OjiLen));
bool Asn1OjiDec __((asn1_sck_t *Asn1, u8 *Eoc, u32 *Oji, u32 OjiSze, u32 *OjiLen));

#endif
