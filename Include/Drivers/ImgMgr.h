/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: ImgMgr.h
// 
// Description:
//    MIPS firmward image manager routine interfaces.
// 
// $Log: /Gemini/Odyssey/Hbc/ImgMgr.h $
// 
// 2     9/01/99 8:06p Iowa
// 
// 1     6/21/99 7:10p Ewedel
// Initial revision.
// 
/*************************************************************************/

#ifndef _ImgMgr_h_
#define _ImgMgr_h_


#ifdef __cplusplus
extern "C" {
#endif


/* image_dnl - Code to move an image across PCI.  This is in imgmgr.c.
 * Block is
 *    0 for HBC
 *    1 for NIC
 *    2 for RAC image
 *    3 for SSD 
 * Slot is the actual slot number in the chassis.
 */
int image_dnl(U32 block,  U32 slot, U32 *pImageOffset, U32 *pParamOffset);

//  erase image from given HBC flash "block"
int erase_image(U32 block);

//  list images presently installed in flash "blocks"
int list_images(void);


#ifdef __cplusplus
}  /* extern "C" */
#endif


#endif  /* #ifndef _ImgMgr_h_ */

