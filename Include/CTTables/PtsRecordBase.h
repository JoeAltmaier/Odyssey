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
// Description:
//   This file defines the CPtsRecordBase class.  This class is used
//   as a base class for defining PTS record classes.
// 
// $Log: /Gemini/Include/CTTables/PtsRecordBase.h $
// 
// 4     10/27/99 12:10p Ewedel
// Changed so that CPtsVarField<> instances track ownership and pointer
// mode of individual varfield data buffers.  This considerably cleans up
// CPtsRecordBase's architecture.  And copies now work properly.  :-)
// 
// 3     9/30/99 2:09p Ewedel
// Compiles cleanly under both Metrowerks and Microsoft.  (Actually, MS
// has a benign warning which I can't get rid of.)
// 
// 2     9/29/99 6:20p Ewedel
// Added first pass of VarField support.  Copy and assignment support
// still to come.
// 
// 1     9/03/99 5:39p Ewedel
// Initial revision.
//
/*************************************************************************/

#ifndef _PtsRecordBase_h
#define _PtsRecordBase_h


#ifndef Simple_H
# include  "Simple.h"      // simple types, like U32
#endif

#ifndef CTtypes_H
# include  "CtTypes.h"     // for class RowId
#endif

#ifndef _Message_h
# include  "Message.h"     // for SGL awareness
#endif

#ifndef assert
# include  "assert.h"
#endif

#ifndef _STRING_H
# include  <string.h>      // for memcpy()
#endif


//  this causes a bit too much trouble for other headers.. and our own
//  structure takes care of itself.
//#pragma  pack(1)           // we want to keep our U16s & U32s tightly packed


//  forward ref for friendliness
class CPtsRecordBase;


//  here is what a single varfield descriptor looks like.  These are
//  required by PTS to occur all in a row at the end of the fixed part
//  of a PTS record.  Thus, these may be found in a record by indexing
//  back from the end of the record as defined by its size member.
//  (Recall that CPtsRecordBase::size defines the size of only the
//   fixed part, plus all varfield descriptors.  This means that
//   CPtsRecordBase::size == sizeof (rec).)

template <class T> class CPtsVarField
{

friend CPtsRecordBase;

   //  some private enum defs which are convenient.  These bit value are
   //  ORed into our m_cbData member.  The bit values are carefully chosen
   //  so that when we ship a record off to PTS in offset form, all the
   //  special bits are naturally set to zero.  Thus, PTS sees a pure
   //  size value in the m_cbData field, which is what it expects.
   enum Flags {
      k_mSizeData  = 0x0FFFFFFF,    // max size value == 256MB
      k_mIsPointer = 0x80000000,    // data ref is pointer, not offset
      k_mWeOwnData = 0x40000000,    // ref'd data buf is owned by us
      k_mReserved1 = 0x20000000,
      k_mReserved2 = 0x10000000
   };

public:

   //  typical null constructor (empty varfields are valid in PTS,
   //  although not terribly useful :-)
   inline CPtsVarField<T> (void)
   {
      m_cbData = 0;
      m_Data.ptr = NULL;
   };

   //  initialize our varfield -
   //  pData is saved by reference not by value.  Thus, this record
   //  does *not* own *pData, and will not attempt to delete it at
   //  destruction time.
   //  cData is a one-based count of T items, *not* a count of bytes.
   inline CPtsVarField<T> (T *pData, U32 cData)
            {  m_cbData = 0;     // flag no old buffer to free
               Set (pData, cData);  };

   inline ~CPtsVarField<T> ()
            {  ClearBuf ();  };  // free owned buffer, if any

   //  assignment *always* copies the RHS's value into a new buffer:
   inline operator = (const CPtsVarField<T>& Rhs)
      {  assert ((Rhs.m_cbData == 0) || Rhs.IsPointerForm ());

         Set (Rhs.ConstPtr (), Rhs.Size (), k_eMakeCopy);
      }

   //  return size of varfield data.  This is valid even in a truncated
   //  record (whose varfield data is not available).
   //  This returns a count of T items, *not* a count of bytes.
   U32  Size (void)  const
            {  return ((m_cbData & k_mSizeData) / sizeof (T));  };

   //  return value of varfield.  This will return NULL if either
   //  the field is empty, or it is part of a truncated record.

   const T *ConstPtr (void)  const
            {  return (m_Data.ptr);  };

   T *Ptr (void)
            {  return (m_Data.ptr);  };

   //  set some buffer as our value.  Note that by default we save a
   //  reference to the buffer, so the buffer is an extension of our
   //  containing PTS record instance, and must exist as long as it does.
   //  cData is a one-based count of T items, *not* a count of bytes.
   enum OwnershipMode {
      k_eMakeCopy = 1,     // we allocate (& own) a local buf to hold the data
      k_eRefOnly = 2       // we just point to caller's data (we don't own buf)
   };

   inline void  Set (T *pData, U32 cData, OwnershipMode eMode = k_eRefOnly)
      {
         //  dispose of old buffer, if any
         ClearBuf ();

         if (cData == 0)
            //  for an empty value, ClearBuf() took care of us.
            return;

         //  size is always stored in bytes
         assert ((((UI64) cData) * sizeof (T)) < k_mSizeData);
         m_cbData = ((cData * sizeof (T)) & k_mSizeData) | k_mIsPointer;

         if (eMode == k_eRefOnly)
            {
            m_Data.ptr = pData;
            }
         else
            {
            assert (eMode == k_eMakeCopy);
            m_Data.ptr = new T [cData];
            //NOTE: we do memcpy(), and not memberwise copy of array elements!
            //      This is incorrect C++, but better fits our expected
            //      usage where only simple types are used for T, and the
            //      array size may be very large (>100k).
            memcpy (m_Data.ptr, pData, cData * sizeof (T));

            //  flag that we own buffer
            m_cbData |= k_mWeOwnData;
            }

         return;
      };  /* end of CPtsVarField<T>::Set (pData, cData, eMode) */

private:

   //  this is a one-based count of bytes in the variable field's content.
   //  This value is valid even in a record whose variable field content
   //  is not provided (e.g., as read by a simple read-row request).
   //  This value need not be a multiple of four bytes, but each field's
   //  data is stored in a four byte rounded buffer.
   U32   m_cbData;

   //  Here is the address of the variable field's content.  In a truncated
   //  record (e.g., as read by a simple read-row request), this value is
   //  zero/NULL to indicate that the variable field content is not available.
   union {
      //  When stored in PTS-normal form, this is the offset of the variable
      //  field's data, from the start of the record (i.e., first byte of rid).
      //  PTS-normal form should only be seen in replies coming straight from
      //  PTS, before their message wrappers have shifted them to true pointers.
      U32   off;

      //  When stored in "true MIPS pointer" form, this is a pointer to
      //  the start of the variable field's content.  The content might
      //  be located contiguous with the record's fixed part, or it might
      //  be in an altogether separate buffer.
      T   * ptr;
   } m_Data;

   //  helper to indicate whether varfield descriptor is in pointer form
   inline BOOL  IsPointerForm (void)  const
            {  return ((m_cbData & k_mIsPointer) != 0);  };

   //  helper to indicate whether varfield has real data attached now
   inline BOOL  HasRealData (void)  const
            {  return ((Size() > 0) && (ConstPtr() != NULL));  };

   //  helper to set our offset value (mostly just type cleanliness)
   //  Note that cbSize is a byte count, not a T count.
   inline void  SetOffset (U32 offset, U32 cbSize)
            {  m_Data.off = offset;
               m_cbData = cbSize & k_mSizeData;  };

   inline void  SetOffset (U32 offset)
            {  m_Data.off = offset;  };

   //  helper just so no friends have to touch our raw member data
   inline U32  Offset (void)  const
            {  return (m_Data.off);  };

   //  how we change from a record-relative offset to a true MIPS pointer
   //  (assumes our varfield data is embedded in our record buffer)
   inline void  ChangeOffsetToPointer (CPtsRecordBase *pBase)
            {  assert ((m_cbData & ~k_mSizeData) == 0);
               m_Data.ptr = (T *) (((U8 *) pBase) + m_Data.off);
               m_cbData |= k_mIsPointer;  };

   //  helper for disposing of allocated buffer, if any (also clears data)
   inline void  ClearBuf (void)
      {
         if ((m_cbData & k_mWeOwnData) != 0)
            {
            assert ((m_cbData & k_mIsPointer) != 0);
            delete [] m_Data.ptr;
            }

         m_cbData   = 0;
         m_Data.ptr = NULL;
      }

};  /* end of class CPtsVarField<T> */



//
//  PTS Record Base class - Contains common data used by all PTS
//          record types.  This is not an actual table row definition,
//          but rather is a base class from which table row classes
//          may derive.  See IOPStatusTable.h / .cpp for an example.
//
//          Note that this class is not, and must not be, virtual.
//          It provides some useful base functions, but is primarily
//          a common data structure used with PTS record structures.
//          [Because they are kept in PTS, such structures may not
//           be virtual class instances.]

class CPtsRecordBase
{
public:

   //  rowID of this record (in enhanced, class form)
   const RowId    rid;

   //  one-based count of bytes in this record.  Includes any
   //  variable-length field descriptors, but does *not* include
   //  variable length field values.
   const U32      size;

   //  Version of this Record -- a table-specific value
   const U32      version;

   //  a more formal C++ means of accessing the above members, which
   //  are public only for historical reasons:

   inline U32 FixedSize (void)  const
            {  return (size);  };

   inline U32 Version (void)  const
            {  return (version);  }

   //  nb: this makes warning C4244 under Visual C++, and I can't find any
   //      way to make it go away.  Even #pragma warning won't make it
   //      go away!
   inline CPtsRecordBase(U32 _size, U32 _version, U16 cVarFields = 0) :
                                                    rid(), 
                                                    size(_size),
                                                    version(_version),
                                                    m_cVarFields(cVarFields)
            {  m_mFlags = 0;  };

   //  copy constructor, simply copies our instance data without doing
   //  anything about derived class data or varfields
   CPtsRecordBase (const CPtsRecordBase& src) :
                              rid (src.rid),
                              size (src.size),
                              version (src.version),
                              m_cVarFields (src.m_cVarFields),
                              m_mFlags (src.m_mFlags)
            {  };

   //  return size of this record, including its variable-length field content
   //  ("phantoms" are varfield contents which are not present in our record
   //   because it was truncated, e.g. by a simple read-row reply)
   //  Total record size is the space required to place the entire record
   //  in a single buffer; this thus includes any pad bytes needed to ensure
   //  that each variable field's data is quad-byte aligned.
   U32  TotalRecSize (BOOL fIncludePhantoms = TRUE)  const;

   //  return count of variable fields in this record
   inline U32  VarFieldCount (void)  const
               {  return (m_cVarFields);  };

   //  copies a record to a message as a single SGL.  This routine includes
   //  the record's variable-length field content within the SGL data,
   //  and ensures that the varfield pointers within the SGL are
   //  written in offset form.
   void  WriteRecordAsSgl (Message *pMsg, U32 iSgl)  const;

   //  convert a record's varfield offsets into true MIPS pointers
   //  (may be called multiple times with no ill effects).
   //  This call is meant for use by PTS message wrapper code
   //  (e.g., TS*, and message templates) and should not need
   //  to be called by DDM client code.
   void  ConvertVarFieldOffsetsToPointers (U32 cbActualTotalRecordSize);

private:

   //  count of variable-length fields in this record
   const U16      m_cVarFields;

   //  collection of Flags bit values for this record (all zero at present):
   U16            m_mFlags;

   //  for our own use, we have a generic varfield def:
   //  (useful 'cause sizes are in bytes)
   typedef CPtsVarField<U8>  CharVarField;

   //  grab a given varfield descriptor struct within our record
   inline const CharVarField *ConstVarField (U32 iVarField)  const
   {  return ((CharVarField *)
              (((U8 *) this) + size -
               ((m_cVarFields - iVarField) * sizeof (CharVarField))));  };

   //  grab a given varfield descriptor struct within our record
   inline CharVarField *VarField (U32 iVarField)
   {  return ((CharVarField *)
              (((const CPtsRecordBase *)this)->ConstVarField(iVarField)));  };

};  /* end of class CPtsRecordBase */


//  here are compiler-checkable aliases for PTS field definition names

#define  CT_PTS_RID_FIELD_NAME   "rid"
#define  CT_PTS_SIZE_FIELD_NAME  "rec_size"
#define  CT_PTS_VER_FIELD_NAME   "rec_version"
#define  CT_PTS_VFI_FIELD_NAME   "rec_var_field_info" /* count & flags */


//  here are the field defs which need to be added to the field defs list
//  of each class derived from CPtsRecordBase.  These are in the form of
//  a macro, so they can simply be inserted in the usual static array
//  field defs list.
//  PersistFlag should be one of Persistant_PT or NotPersistant_PT.
//  NotPersistant_PT should only be used with non-persistent tables.
#define  CPTS_RECORD_BASE_FIELDS(PersistFlag)                     \
            /* RowId - automatically supplied by PTS */           \
            CT_PTS_SIZE_FIELD_NAME,    0, U32_FT,  PersistFlag,   \
            CT_PTS_VER_FIELD_NAME,     0, U32_FT,  PersistFlag,   \
            CT_PTS_VFI_FIELD_NAME,     0, U32_FT,  PersistFlag  /* count & flags */



#endif  // #ifndef _PtsRecordBase_h

