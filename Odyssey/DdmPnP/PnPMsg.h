/*
 * Copyright (C) ConvergeNet Technologies, 1998 
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * PnPMsg.h --
 *
 * Revision History:
 *		03/19/99 Huy Do: Created
 *		04/10/99 Huy Do: Add message payload
 *		05/10/99 Huy Do: Add SP_REPLY_PAYLOAD
 *		05/14/99 Huy Do: Add SGL type
 *		05/20/99 Huy Do: Got weird compiler error on NT,
 *						 add WIN_32 part to avoid that.
 *
**/

#ifndef __PNPMSG_H__
#define __PNPMSG_H__

#define	H_TO_N( a ) ((a<<24) | (a>>24) | ((a & 0x0000FF00)<<8) | ((a & 0x00FF0000)>>8))
#define	N_TO_H( a ) ((a<<24) | (a>>24) | ((a & 0x0000FF00)<<8) | ((a & 0x00FF0000)>>8))

#define	H_TO_NS( a ) ( (a << 8) | (a >> 8) )

#ifdef _WIN32

// Setup the basics
typedef    signed char  S8;
typedef    short		S16;
typedef    unsigned char  U8;
typedef    unsigned short  U16;
typedef    unsigned long  U32;
typedef    long  S32;

typedef char	String16[16],
				String32[32],
				String64[64],
				String128[128],
				String256[256],
				String512[512],
				String1024[1024],
				String2048[2048];

// Declare the unique row identifier type used by the Table Service.
typedef struct rowID {
	U16	Table;
	U16	HiPart;
	U32	LoPart;
} rowID;

//
// Values for the iFieldType Field.
//

	typedef enum {
		BINARY_FT,	// Miscellaneous binary data of an unspecified type.
		S32_FT,
		U32_FT,
		S64_FT,
		U64_FT,
		STRING16_FT,
		STRING32_FT,
		STRING64_FT,
		ROWID_FT
	} fieldType;
	
// The field definition structure.
//
	typedef struct {
   		String64		name;				// Null terminated string.
		U32				cbField;			// # of bytes in field.
		fieldType		iFieldType;			// type of data
		U32				persistFlags;		// flags that describe persist state
	} fieldDef;

#define	 ercOK				 0
#define	 ercTableNotfound	 1
#define	 ercNoFreeHeaders	 2
#define	 ercTableExists		 3
#define	 ercEOF				 4		// end of table; entry not found
#define	 ercBufTooSmall		 5
#define	 ercTableEmpty		 6
#define  ercPtsStructsTooSmall  7
#define	 ercBadParameter	 8
#define  ercKeyNotFound		 9
#define	 ercFieldNotFound	10
#define  ercNoModRowId		11
#define	 ercNoListeners		12
#define	 ercNoSuchListener	13
#define  ercNoMoreHeap		14

#else		// For OOS
	#include "TableMsgs.h"
#endif

typedef long CHANNEL;

// SGL sent from SerialDdm to DdmPnP
#define CREATE_TABLE_MSG_SGL		0
#define GET_TABLE_DEF_MSG_SGL		0
#define ENUM_TABLE_MSG_SGL			0
#define INSERT_ROW_MSG_SGL			0
#define DELETE_TABLE_MSG_SGL		0

#define MODIFY_ROW_MSG_SGL			0
#define READ_ROW_MSG_SGL			0
#define DELETE_ROW_MSG_SGL			0
#define MODIFY_FIELD_MSG_SGL		0
#define LISTEN_MSG_SGL				0
#define CONNECT_MSG_SGL				0

// SGL sent from DdmPnP to SerialDdm
#define CREATE_TABLE_REPLY_SGL		1
#define GET_TABLE_DEF_REPLY_SGL		1
#define ENUM_TABLE_MSG_REPLY_SGL	1
#define INSERT_ROW_REPLY_SGL		1
#define DELETE_TABLE_REPLY_SGL		1

#define MODIFY_ROW_REPLY_SGL		1
#define READ_ROW_REPLY_SGL			1
#define DELETE_ROW_REPLY_SGL		1
#define MODIFY_FIELD_REPLY_SGL		1
#define LISTEN_REPLY_SGL			1

typedef struct {
	String64	TableName;		// table name
	fieldDef	*pFieldDefs;	// pointer to an array of fieldDef
	U32			cbFieldDefs;	// counter byte of pFieldDefs
	U32			numFieldDefs;	// number of entries in fieldDef array
	U32			cEntriesRsv;	// number of table entries to create initially
	U32		 	cRow;			// number of row(s) will be create
	U32			persistent;		// Persistence?
} CREATE_TABLE_PAYLOAD;

typedef struct {
	String64	TableName;		// table name
	U32			FieldDefRetMax;	// Max size of returned FieldDef array
} GET_TABLE_DEF_PAYLOAD;

typedef struct {
	String64	TableName;		// table name
	U32			startRow;		// row to start at
	U32			cbDataRetMax;	// max number of byte will be returned
} ENUM_TABLE_PAYLOAD;

typedef struct {
	String64	TableName;		// table name
} DELETE_TABLE_PAYLOAD;

typedef struct {
	String64	TableName;		// table name
	void		*pRowData;		// pointer to row data to insert
	U32			cbRowData;		// size of RowData in bytes
} INSERT_ROW_PAYLOAD;

typedef struct {
	String64	TableName;		// Table name
	String64	KeyFieldName;	// Key Field name
	void		*pKeyFieldValue;// pointer to key field value
	U32			cbKeyFieldValue;// size of key field value in bytes
	void		*pRowData;		// pointer to row data to insert
	U32			cbRowData;		// size of RowData in bytes
	U32			cRowsModify;	// counts of rows to modify; 0=ALL
} MODIFY_ROW_PAYLOAD;

typedef struct {
	String64	TableName;		// Table name
	String64	KeyFieldName;	// Key Field name
	void		*pKeyFieldValue;// pointer to key field value
	U32			cbKeyFieldValue;// size of key field value in bytes
	U32			cbRowRetMax;	// Max size of returned row data
} READ_ROW_PAYLOAD;

typedef struct {
	String64	TableName;		// Table name
	String64	KeyFieldName;	// Key Field name
	void		*pKeyFieldValue;// pointer to key field value
	U32			cbKeyFieldValue;// size of key field value in bytes
	U32			cRowsDelete;	// counts of rows to delete; 0=ALL
} DELETE_ROW_PAYLOAD;

typedef struct {
	String64	TableName;		// Table name
	String64	KeyFieldName;	// Key Field name
	void		*pKeyFieldValue;// pointer to key field value
	U32			cbKeyFieldValue;// size of key field value in bytes
	String64	FieldName;		// Name of Field to modify
	void		*pFieldValue;	// The value
	U32			cbFieldValue;	// Size of modified value in bytes
	U32			cRowsModify;	// counts of rows to modify; 0=ALL
} MODIFY_FIELD_PAYLOAD;

typedef struct {
	String64	TableName;		// table name
	U32			ListenType;
	String64	RowKeyFieldName;
	void		*pRowKeyFieldValue;
	U32			cbRowKeyFieldValue;
	String64	FieldName;
	void		*pFieldValue;
	U32			cbFieldValue;
	U32			ReplyMode;
} LISTEN_PAYLOAD;

typedef enum {
	CONNECT,
	CREATE_TABLE,
	GET_TABLE_DEF,
	ENUM_TABLE,
	DELETE_TABLE,
	INSERT_ROW,
	MODIFY_ROW,
	DELETE_ROW,
	READ_ROW,
	MODIFY_FIELD,
	LISTEN
} CMD;

typedef struct {
	CMD		cmd;		// Command
	U32		cbData;		// counter byte
	CHANNEL		chID;	// channel ID, usually is the window view's handle
	union {
		CREATE_TABLE_PAYLOAD	ct;
		GET_TABLE_DEF_PAYLOAD	gt;
		ENUM_TABLE_PAYLOAD		et;
		DELETE_TABLE_PAYLOAD	dt;
		INSERT_ROW_PAYLOAD		in;
		READ_ROW_PAYLOAD		rr;
		DELETE_ROW_PAYLOAD		dr;
		MODIFY_ROW_PAYLOAD		mr;
		MODIFY_FIELD_PAYLOAD	mf;
		LISTEN_PAYLOAD			lt;
	} Data;
} SP_PAYLOAD;

typedef struct {
	fieldDef	*pFieldDefsRet;	// pointer to an array of fieldDef
	U32			cbFieldDefs;	// counter byte of pFieldDefs
	U32			numFieldDefsRet;// number of entries in fieldDef array
	U32			cbRowRet;		// number of bytes/row
	U32			cRowsRet;		// count of rows
	U32			persistent;
} GET_TABLE_DEF_REPLY_PAYLOAD;

typedef struct {
	void	*pRowsDataRet;	// pointer to the returned data buffer
	U32		cbRowsDataRet;	// number of bytes returned
} ENUM_TABLE_REPLY_PAYLOAD;

typedef struct {
	void	*pRowDataRet;	// pointer to the returned data buffer
	U32		cbRowDataRet;	// number of bytes returned
	U32		numRowsRet;		// number of returned rows
} READ_ROW_REPLY_PAYLOAD;

typedef struct {
	void	*pTableDataRet;	// pointer to the returned data buffer
	U32		cbTableDataRet;	// number of bytes returned
	U32		ListenerIDRet;
	U32		ListenerTypeRet;
	void	*pModifiedRecord;
	U32		cbModifiedRecord;
} LISTEN_REPLY_PAYLOAD;

typedef enum {
	REPLY_CONNECT,
	REPLY_CREATE,
	REPLY_GET_DEF,
	REPLY_ENUM,
	REPLY_DELETE_TABLE,
	REPLY_INSERT,
	REPLY_MODIFY_ROW,
	REPLY_DELETE_ROW,
	REPLY_READ_ROW,
	REPLY_MODIFY_FIELD,
	REPLY_LISTEN
} REPLY;

typedef struct {
	REPLY	cmd;
	CHANNEL chID;
	U32		cbData;
	U32		ErrorCode;
	union {
		rowID	ctR;
		GET_TABLE_DEF_REPLY_PAYLOAD gtR;
		ENUM_TABLE_REPLY_PAYLOAD etR;
		rowID	dtR;
		rowID	inR;
		U32		drR;
		READ_ROW_REPLY_PAYLOAD rrR;
		U32		mrR;
		U32		mfR;
		LISTEN_REPLY_PAYLOAD	ltR;
	} Data;
} SP_REPLY_PAYLOAD;

#endif __PNPMSG_H__