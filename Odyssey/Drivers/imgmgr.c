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
// File: ImgMgr.c
//
// Description:
//    MIPS firmware flash memory management & PCI download support.
//
// $Log: /Gemini/Odyssey/Hbc/imgmgr.c $
// 
// 9     1/26/00 2:43p Joehler
// fixed bug in image_hdr()
// 
// 8     11/21/99 4:59p Jlane
// Add Image_hdr to a return pointer to a Boot ROM Flash image header and
// image_dnl_usr for use by the BootMgr downloading images from PTS.
// 
// 7     9/01/99 8:06p Iowa
// 
// 6     8/04/99 1:29p Rkondapalli
// Changes for new Image header
// 
// 5     7/24/99 5:40p Jlane
// bootblockp defined in Drives now
// 
// 4     6/21/99 7:06p Ewedel
// Moved public prototypes into new header ImgMgr.h.  Added params to
// image_dnl() for returning correct image & param offsets.
//
/*************************************************************************/

#include  "nucleus.h"
#include  "tc_defs.h"
#include "types.h"
#include "imghdr.h"
#include "sysflash.h"
#include "system.h"
#include "bootblock.h"
#include "null.h"

#include "ImgMgr.h"        /* our own function defs */


#define IMAGE_SIZE		0x200000
#define TMP_LOAD_ADDRESS	(0xA0800000)
#define HBC_IMAGE_SIZE		0x200000
#define IOP_IMAGE_SIZE		0x400000

unsigned long  image_start_address = 0xdeaddead;

extern void bcopy64(U8 *, U8 *, int);
extern void bcopy(U8 *, U8 *, int);
extern void printf(char *, ...);
extern int sysflash_erase_image_block(int);
extern mmap_t	memmaps;
extern bootblock_t	bootblock;


/* int image_hdr(U32 block, U32* pImgHdrRet. U32*pImgSizRet); 
 *
 * Returns a status code.
 * 
 * block is
 *	0 for HBC
 * 	1 for NIC
 * 	2 for RAC image
 * 	3 for SSD
 */
 int image_hdr(	U32 		block,
 				void**		pImgAdrRet,
 				U32*		pImgSizRet) 
{
	img_hdr_t*	pImgHdr;

	//  Return address and size of firmware image.
	pImgHdr = (img_hdr_t *)(SYSFLASH_START + (block * IMAGE_SIZE));

	if (pImgHdr->i_signature != IMG_SIGNATURE)
	{
		printf("Image not valid at %08lx\n\r", pImgHdr);
		return (-1);
	}
	
	*pImgSizRet = ROUNDUP(	pImgHdr->i_imageoffset
							+ pImgHdr->i_zipsize
							+ pImgHdr->i_sym_table_size,
							64);

	*pImgAdrRet = (void*)pImgHdr;
	return OK;
 }
 
 
/* int image_dnl_usr(void* pbImage, U32 cbImage ,U32 slot, U32 *pImageOffset, U32 *pParamOffset)
 * Downloads user supplied image and current boot block to IOP's PCI window returning offsets.
 * Returns a status code.
 *
 * pImgHdr is adress of image header.
 * cbImgSiz is the size of the image (and header).
 * slot is pci download target ala TySLot.
 * 
*/
int image_dnl_usr(void* pbImage, U32 cbImage ,U32 slot, U32 *pImageOffset, U32 *pParamOffset)
{
	U32	dst;
	U32	offset = M(48);
	U32	param_offset = M(63);
	img_hdr_t*	pImgHdr = (img_hdr_t*)pbImage;

	
	//  return offset params to caller, if desired
	if (pImageOffset != NULL)
		*pImageOffset = offset;
	if (pParamOffset != NULL)
		*pParamOffset = param_offset;
	
	if (pImgHdr->i_signature != IMG_SIGNATURE)
	{
		printf("Image not valid at %08lx\n\r", pImgHdr);
		return (-1);
	};

	//  copy the image to target IOP slot's PCI window
	dst = PCITOV(memmaps.aPaPci[slot]) + offset;
	bcopy64((U8 *)pbImage, (U8 *)dst, cbImage); 

	/*
	 * Now copy Boot Block. IOP needs memmaps structure, so fill it
	 * in the bootblock.
	 */
	dst = PCITOV(memmaps.aPaPci[slot]) + param_offset;
	bcopy((U8 *)&memmaps, (U8 *)&bootblock.b_memmap, sizeof (mmap_t));
	bcopy((U8 *)&bootblock, (U8 *)dst, sizeof (bootblock_t)); 
	
	return (OK);
}


/* int image_dnl(U32 block,  U32 slot, U32 *pImageOffset, U32 *pParamOffset)
 * Downloads image from flash block and current boot block to IOP's PCI window
 * returning offsets.
 *
 * Block is
 * 	1 for NIC
 * 	2 for RAC image
 * 	3 for SSD 
 * Slot is the actual slot number in the chassis.
 */
int
image_dnl(U32 block,  U32 slot, U32 *pImageOffset, U32 *pParamOffset)
{
	U32	dst;
	U32	offset = M(48);
	U32	param_offset = M(63);
	U32	src;
	U32	size;
	img_hdr_t	*imgp;


   //  return offset params to caller, if desired
   if (pImageOffset != NULL)
      {
      *pImageOffset = offset;
      }
   if (pParamOffset != NULL)
      {
      *pParamOffset = param_offset;
      }

   //  copy actual firmward image to target IOP slot's PCI window
	src = SYSFLASH_START + (block * IMAGE_SIZE);
	imgp = (img_hdr_t *)src;
	if (imgp->i_signature != IMG_SIGNATURE) {
		printf("Image not valid in flash at %08lx\n\r", imgp);
		return (1);
	}
	size = imgp->i_imageoffset + imgp->i_zipsize + imgp->i_sym_table_size;
	size = ROUNDUP(size, 64);	
	dst = PCITOV(memmaps.aPaPci[slot]) + offset;
	bcopy64((U8 *)src, (U8 *)dst, size); 

	/*
	 * Now copy Boot Block. IOP needs memmaps structure, so fill it
	 * in the bootblock.
	 */
	dst = PCITOV(memmaps.aPaPci[slot]) + param_offset;
	bcopy((U8 *)&memmaps, (U8 *)&bootblock.b_memmap, sizeof (mmap_t));
	bcopy((U8 *)&bootblock, (U8 *)dst, sizeof (bootblock_t)); 
	
	return (block);
}

int
erase_image(U32 block)
{
	printf("Erasing Image at Block %d ...", block);
	return (sysflash_erase_image_block(block));
}

int
list_images(void)
{
	img_hdr_t	*imgp;
	U32		i;

	for (i = 0; i < 4; i++) {
		imgp = (img_hdr_t *)((IMAGE_SIZE * i) + SYSFLASH_START);
		if (imgp->i_signature == IMG_SIGNATURE) {
			printf("%d: Image Name: %-10s ", (i + 1),
				imgp->i_imagename);
			printf("Ver:%d.%02d. ",
				imgp->i_mjr_ver, imgp->i_mnr_ver);
			printf("Date:%d/%d/%d Time:%d:%d:%d\n\r",
				imgp->i_month, imgp->i_day, imgp->i_year,
				imgp->i_hour, imgp->i_min, imgp->i_sec);
		} else {
			printf("%d: No valid Image.\n\r", (i + 1));
		}
	}
	return (0);

}

