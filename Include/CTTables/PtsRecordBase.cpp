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
// File: PtsRecordBase.cpp
// 
// Description:
//    Contains the non-inlined members of class CPtsRecordBase.
// 
// $Log: /Gemini/Include/CTTables/PtsRecordBase.cpp $
// 
// 5     11/10/99 9:32a Joehler
// account for case of a zero length variable field and advance the
// pVarField for multiple variable length fields
// 
// 4     10/27/99 12:13p Ewedel
// Changed so that CPtsVarField<> instances track ownership and pointer
// mode of individual varfield data buffers.  This considerably cleans up
// CPtsRecordBase's architecture.  And copies now work properly.  :-)
// Also changed TotalRecSize() so that it doesn't round the size of the
// final varfield's data.  This is consistent with the buffer size
// reported by PTS on read-row & such.
// 
// 3     10/01/99 3:35p Ewedel
// Fixed bug in rounding of varfield data size during SGL "spooling."
// 
// 2     9/30/99 2:09p Ewedel
// Compiles cleanly under both Metrowerks and Microsoft.  (Actually, MS
// has a benign warning which I can't get rid of.)
// 
// 1     9/29/99 6:20p Ewedel
// Initial checkin.  Copy and assignment support still to come.
// 
/*************************************************************************/

#include  "PtsRecordBase.h"


#include  <assert.h>



//  and here's a little inline helper for doing the rounding work:
static inline U32  RoundSize (U32 cb)
{
   //  round up to four-byte boundary
   return ((cb + 3) & ~3);
}


//
//  CPtsRecordBase::TotalRecSize (fIncludePhantoms)
//
//  Description:
//    Returns the total size of our record, including variable field
//    content.
//
//    A parameter controls whether we count variable field content
//    of a "truncated" record (one whose varfield data pointers are
//    null, e.g. because it was returned by read-row instead of
//    read-var-row or whatever).
//
//  Inputs:
//    fIncludePhantoms - When TRUE, we include the size of "phantom"
//                   var fields in our returned count.  When FALSE
//                   we include only those var fields whose data is
//                   present in our record instance.
//
//  Outputs:
//    CPtsRecordBase::TotalRecSize - Returns total record size in bytes,
//                   subject to filtering per our fIncludePhantoms input.
//

U32  CPtsRecordBase::TotalRecSize (BOOL fIncludePhantoms /* = TRUE */ )  const
{

U32                  cbMyRet;
const CharVarField * pVarField;
U32                  i;


   cbMyRet = size;

   if (m_cVarFields > 0)
      {
      pVarField = ConstVarField (0);

      for (i = 1;  i <= m_cVarFields;  i ++)
         {
         if (fIncludePhantoms || (pVarField->ConstPtr() != NULL))
            {
            //  is this the record's last varfield?
            if (i < m_cVarFields)
               {
               //  round non-last field size to four-byte size, since
               //  PTS requires that subsequent fields be aligned
               //  on four-byte boundaries in a contiguous row buffer.
               cbMyRet += RoundSize (pVarField->Size());
               }
            else
               {
               //  for the last varfield, leave its size true.
               cbMyRet += pVarField->Size();
               }
            }

         pVarField ++;
         }
      }

   return (cbMyRet);

}  /* end of CPtsRecordBase::TotalRecSize */

//
//  CPtsRecordBase::WriteRecordAsSgl (pMsg, iSgl, nSglDirection)
//
//  Description:
//    Writes our record content to a single SGL of some specified
//    message.
//
//    We always write our variable field content as part of the
//    SGL data.  Because the variable field data may be scattered
//    across several buffers, we always perform an SGL_COPY-style
//    of SGL write.  Of course, if our record is truncated then we
//    will only write as much as is present.
//
//    Variable field data does not need to be a multiple of four bytes
//    in length, but PTS does require that variable field buffers be
//    four-byte aligned.  We ensure this in the single SGL which we
//    build, by inserting pad bytes as necessary between variable
//    field buffers.  [This is only an issue if a record has multiple
//    variable fields.]
//
//    If you want to write just the record header, please just use
//    Message::AddSgl() directly on the fixed part of the record.
//
//  Inputs:
//    pMsg - Points to message which we add an SGL to.
//    iSgl - Index of SGL which we add and write ourselves to.
//                   We make this an SGL_COPY-style of SGL.
//
//  Outputs:
//    none, since AddSgl() has no return.
//

void  CPtsRecordBase::WriteRecordAsSgl (Message *pMsg, U32 iSgl)  const
{

const U32            cbTotalRec  =  TotalRecSize (FALSE);   // only real data
CPtsRecordBase       strHeader (*this);   // modifiable copy of our header data
const CharVarField * pVarField;
const CharVarField * pVarFieldToWrite;
U32                  cbWritten;
CharVarField         strVarField;
U32                  offVarData;
U32                  i;


   assert (pMsg != NULL);

   //  build up a single SGL for all of our data
   pMsg->AllocateSgl(iSgl, cbTotalRec);

   //  we write the fixed header data in a couple of pieces:  base header,
   //  other fixed header data except for trailing varfield descriptors,
   //  and finally the varfield descriptors themselves.

   //  we might need to patch strHeader.m_mFlags; this is why we write
   //  our header separately from the derived record's fixed header.
   //  At present however, we have no flags to modify.
//   strHeader.m_mFlags &= ~k_PRBVarAddressesAreAbsolute;

   //  write header to SGL
   pMsg->CopyToSgl (iSgl, 0, &strHeader, sizeof (strHeader));

   //  (header should always have a round size)
   assert (sizeof (strHeader) == RoundSize (sizeof (strHeader)));

   cbWritten = sizeof (strHeader);

   //  write derived class' fixed data, except for varfield descriptors
   i = size - m_cVarFields * sizeof (CharVarField) - sizeof (strHeader);

   //  (derived class' fixed data should also have a round size)
   assert (i == RoundSize (i));

   pMsg->CopyToSgl (iSgl, cbWritten, ((U8 *) this) + cbWritten, i);
   cbWritten += i;

   //  next, the varfields (if any)
   if (m_cVarFields > 0)
      {
      //  now scoot through writing fixed up offset versions of each
      //  varfield to SGL:
      pVarField = ConstVarField (0);

      //  we're advanced to the start of our varfield descriptors
      assert (cbWritten == (size - m_cVarFields * sizeof (CharVarField)));

      //  each varfield descriptor also must have a round size
      assert (sizeof (*pVarField) == RoundSize (sizeof (*pVarField)));

      //  go through varfields, fixing up & writing each descriptor

      offVarData = size;      // offset of first var field's data

      for (i = 0;  i < m_cVarFields;  i ++)
         {

         if (pVarField->IsPointerForm ())
            {
            //  build up corrected offset form of varfield descriptor

            if (pVarField->HasRealData())
               {
               //  keep size, change to offset in our own synthetic record
               strVarField.SetOffset (offVarData, pVarField->m_cbData);

               //  bump offset of next varfield, if any
               offVarData += RoundSize (strVarField.Size());
               }
            else
               {
               //  if it was a phantom, it's now empty.

               //  (we don't like munging phantoms)
               assert (pVarField->ConstPtr() == NULL);

               strVarField.Set (NULL, 0);
               }

            //  we write our fabricated varfield descriptor:
            pVarFieldToWrite = &strVarField;
            }
         else
            {
            //  varfield descriptor is already in offset form, so we don't
            //  have to do anything to prep it for writing.
            pVarFieldToWrite = pVarField;
            }

         //  write varfield to SGL
         pMsg->CopyToSgl (iSgl, cbWritten,
                          (void *) pVarFieldToWrite, sizeof (*pVarFieldToWrite));
         cbWritten += RoundSize (sizeof (*pVarFieldToWrite));

         //  done with this varfield, move on to next
         pVarField ++;
         }

      //  phew!  All done with header, including varfield descriptors.
      //  Now write varfield data.
      pVarField = ConstVarField(0);

      for (i = 0;  i < m_cVarFields;  i ++)
         {
         //  write varfield's actual data, if any

         //  Note that we copy only the bytes which are actually defined
         //  for the varfield, but we allot it space in the SGL buffer
         //  for the varfield's four-byte rounded varfield data size.
         //  PTS defines this layout; all PTS record fields must be four-byte
         //  aligned, including varfield data.

         if (pVarField->HasRealData ())
            {
            //  we copy just the true amount of varfield data into the SGL
            pMsg->CopyToSgl (iSgl, cbWritten, (void *) pVarField->ConstPtr(),
                             pVarField->Size());

            //  but then we account for it as if we had written the
            //  four-byte rounded equivalent.  This is accounted for
            //  by the total size calculations used for AllocateSgl().
            cbWritten += RoundSize (pVarField->Size());
            }
         // jlo - advance to next pVarfield
         pVarField++;
         }
      }

   //  all done
   return;

}  /* end of CPtsRecordBase::WriteRecordAsSgl */

//
//  CPtsRecordBase::ConvertVarFieldOffsetsToPointers (cbActualTotalRecordSize)
//
//  Description:
//    If our record's varfields are not already in true-pointer form,
//    this call converts them to it.  [The other form is offset, which
//    is how PTS stores records.  We expect that PTS client interface
//    messaging system(s) will automatically invoke this routine as
//    soon as they receive a new record from PTS.
//
//    We assume that all actual varfield data is located contiguous
//    with (immediately following) our fixed header data.
//
//    It is safe to call this routine multiple times for a given
//    record instance:  the additional calls are simply ignored.
//
//  Inputs:
//    cbActualTotalRecordSize - Size of our record's current actual buffer.
//                   We use this value to decide how many of our varfield
//                   data values are real, and how many are phantoms (size
//                   specified, but data not available in record).
//
//  Outputs:
//    none
//

void  CPtsRecordBase::ConvertVarFieldOffsetsToPointers
                                 (U32 cbActualTotalRecordSize)
{

BOOL           fHaveVarData;     // does our record have actual data?
CharVarField * pVarField;
U32            i;


   //  do we have variable field data, or are we a "truncated" record
   //  with just our fixed header part?
   fHaveVarData = (cbActualTotalRecordSize == TotalRecSize());

   //  we only allow all variable length fields, or none.
   assert ((cbActualTotalRecordSize == size) || fHaveVarData);

   if (VarFieldCount() > 0)
      {
      //  may have offset varfields, so change them into pointers
	  pVarField = VarField(0);

      for (i = 0;  i < VarFieldCount();  i ++)
         {
         if (fHaveVarData)
            {
            //  got vardata, but is descriptor converted to pointer form yet?
            if (! pVarField->IsPointerForm ())
               {
               //  nope, let's convert descriptor to pointer form.

               //  offset should be somewhere within our record data, or it
			   //  could be an empty var field (jlo)
               assert ((pVarField->Size()==0) ||
                       ((pVarField->Offset() >= size) &&
                        (pVarField->Offset() <=
                           (cbActualTotalRecordSize - pVarField->Size()))));
     
               //  varfield's data should also be four-byte aligned:
               assert (pVarField->Offset() == RoundSize (pVarField->Offset()));
     
               if ((pVarField->Offset() >= size) &&
                   (pVarField->Offset() <=
                           (cbActualTotalRecordSize - pVarField->Size())))
                  {
                  //  got varfield data, so make an honest varfield pointer
                  pVarField->ChangeOffsetToPointer (this);
                  }
               else
                  {
                  //  this is either an empty var field, or it has an invalid offset.  
                  //  either way, set to be empty. (jlo)
                  pVarField->Set (NULL, 0);
                  }
               }
            }
         else
            {
            //  for now, we mark a phantom by setting its ptr to zero,
            //  while leaving its data size value intact.
            pVarField->SetOffset (0);
            }
		 // jlo - advance to next pVarField
         pVarField++;

         }
      }

   //  all done
   return;

}  /* end of CPtsRecordBase::ConvertVarFieldOffsetsToPointers */


