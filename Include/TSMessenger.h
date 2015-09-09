/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This file is a procedural wrapper to the message interface of the 
// Table Service.
// 
// Update Log: 
// 10/14/98	JFL	Created.
//
/*************************************************************************/
#ifndef __TSMessenger_h
#define __TSMessenger_h

#include "CtTypes.h"
#include "PTSCommon.h"
#include "TableMsgs.h"

class TSMessenger {

public:

/**************************************************************************/
// TSDefineTable - Create a new table.
//
// Parameters:
//	*prgbTableName	- Null terminated name of the table to create.
//	crgFieldDefs	- The number of entries in the rgFieldDefs array.
//	prgFieldDefs	- Pointer to FieldDef array that defines the fields
//	cbrgFieldDefs	- size of the above rgFieldDefs array in bytes.
//	cEntriesRsv		- Number of table entries to create initially.
//	fPersistant		- Should this table be persistant?
/**************************************************************************/
static STATUS TSDefineTable( 	String64		prgbTableName,
								U32				crgFieldDefs,	
								fieldDef		*prgFieldDefs,
								U32				cbrgFieldDefs,
								U32				cEntriesRsv,
								BOOL			fPersistant );


/**************************************************************************/
// TSGetTableDef - Get the definition data of an existing table
//
// Parameters:
//	*prgbTableName	- Null terminated name of the table.
//	prgFieldDefsRet	- The returned FieldDef array that defines the fields
//  cbrgFieldDefsRetMax - Size of prgFieldDefsRet array.
//	pcFieldDefsRet	- pointer to returned number of entries in Field Def. array.
//	pcbRowRet		- pointer to returned number of bytes per row.
//	pcRowsret		- pointer to returned count of rows in table
//	pfPersistant	- pointer to returned flag indicating if table is persistant.
/**************************************************************************/
static STATUS TSGetTableDef(	String64		prgbTableName,
								fieldDef		*prgFieldDefsRet,
								U32				cbrgFieldDefsRetMax,
								U32				*pcFieldDefsRet,
								U32				*pcbRowRet,
								U32				*pcRowsRet,
								BOOL			*pfPersistant );


/**************************************************************************/
// TSListenOnTable - Ask for replies when table data is modified.
//
// Parameters:
//	ListenType 		- The type of operation for which you want to listen.
//					  NOTE: See ListenTypeEnum declaration in TableMsgs.h
//	*prgbTableName	- The name of the table on which you want to listen. 
//	rgbFieldName	- The name of the field on which you want to listen.
//					  NOTE: rgbFieldName is only used for some ListenTypes. 
//  ReplyMode		- 0 = reply continuously 1 = reply once only.
/**************************************************************************/
static STATUS TSListenTable(	ListenTypeEnum	ListenType,
								String64		prgbTableName,		// 
								String64		prgbFieldName,
								ReplyModeEnum	ReplyMode );

						
/**************************************************************************/
// TSEnumTable - Read Sequential records from a table.
//
// Parameters:
//  prgbTableName		- Null terminated name of the table.
//  uStartRowNum		- first row number (0 based) to return. Frequently 0. 
//  cbDataRetMax		- limited by 8K transfer buffer size.			
//  *pRowDataRet		- Number of whole rows returned.
//  *pcbRowDataRet		- Number of bytes returned.
/**************************************************************************/
static STATUS TSEnumTable(	String64		prgbTableName,
							U32				uStartRowNum,		// 0 based.
							void			*pbRowDataRet,
							U32				cbDataRetMax,	// limited by 8K transfer buffer size.			
							U32				*pcbRowDataRet );



/**************************************************************************/
// TSInsertRow - Insert a new row into a table
//
// Parameters:
//  prgbTableName	- Null terminated name of the table.
//  prgbRowData		- Pointer to the row data to insert,
//  cbRowData		- size of the row data in bytes.
//  pRowIDRet		- Pointer to returned RowID for newly inserted row.
//					  NOTE: If you dan't want the RowID, pass pRowIDRet = 0;
/**************************************************************************/
static STATUS TSInsertRow(	String64		prgbTableName,
							void			*prgbRowData,
							U32				cbRowData,
							rowID			*pRowIDRet = 0 );

						
/**************************************************************************/
// TSInsertRows - Insert one or more new rows into a table
//
// Parameters:
//  prgbTableName	- Null terminated name of the table.
//  cRowsToInsert	- The number of rows to insert into the table.
//  prgbRowsData	- Pointer to the row data to insert,
//  cbrgRowsData		- size of the row data in bytes.
//  prgRowIDsRet	- Pointer to returned rowID(s) for newly inserted row(s).
//					  NOTE: If you dan't want the rowID(s), pass prgRowIDsRet = 0.
//	cbRowIDsRetMax	- if (prgRowIDsRet) this should be 8 * cRowsToInsert bytes.
//  pcRowsInserted	- Number of rows sucessfully inserted.
//  pcIDsRet		- Number of IDs Actually returned.
/**************************************************************************/
static STATUS TSInsertRows(	String64		prgbTableName,
							U32				cRowsToInsert,
							void			*prgbRowsData,
							U32				cbrgRowsData,
							rowID			*prgRowIDsRet,
							U32				*pcRowsInserted,
							U32				*pcIDsRet );

						
/**************************************************************************/
// TSModifyRow - Modify the contents of a row in a table.
//
// Parameters:
//  prgbTableName	- Null terminated name of the table.
//  rgbKeyFieldName	- The key field name used to identify the row to modify
//  pKeyFieldValue	- The key field value used to search for the row to modify.
//  cbKeyFieldValue	- the size of the key field value in bytes.
//  prgbRowsData	- Pointer to the row data to insert.
//  cbRowsData		- size of the row data in bytes.
//  pRowIDRet		- Pointer to returned rowID(s) for the modified row.
//					  NOTE: If you dan't want the rowID, pass pRowIDRet = 0;
/**************************************************************************/
static STATUS TSModifyRow(	String64		prgbTableName,
							String64		prgbKeyFieldName,
							void			*pKeyFieldValue,
							U32				cbKeyFieldValue,
							void			*prgbRowData,
							U32				cbRowData,
							rowID			*pRowIDRet = 0 );

						
/**************************************************************************/
// TSModifyRows - Modify the contents of one or more rows in a table.
//
// Parameters:
//  prgbTableName		- Null terminated name of the table.

//  prgbKeyFieldNames	- Array of key field names used to identify the row to modify
//  cRowsToModify		- The number of rows to modify
//  cbKeyFieldNames		- the size of the key field names array  in bytes.
//  prgKeyFieldValues	- Array of key field values used to search for row to modify.
//  cbKeyFieldValues	- the size of the key field values in bytes.
//  prgbRowsData		- Pointer to the row data to insert.
//  cbrgRowsData		- size of the row data in bytes.
//  prgRowIDRet			- Pointer to returned rowID(s) for modified row(s).
//  pcRowsModifiedRet	- Returned number of rows modified.
//  pcIDsRet			- Returned number of IDs returned.
/**************************************************************************/
static STATUS TSModifyRows(	String64		Tablename,
							U32				cRowsToModify,
							String64		prgKeyFieldNames,
							void			*prgKeyFieldValues,
							U32				cbKeyFieldValues,
							void			*prgbRowsData,
							U32				cbrgRowsData,
							rowID			*prgRowIDsRet = 0,
							U32				*pcRowsModifiedRet = 0,
							U32				*pcIDsRet = 0 );

						
/**************************************************************************/
// TSModifyField - Modify the contents of a field in a row in a table.
//
// Parameters:
//  prgbTableName		- Null terminated name of the table.
//  prgbKeyFieldName	- The key field name used to identify the row to modify
//  pKeyFieldValue		- The key field value used to search for the row to modify.
//  cbKeyFieldValue		- the size of the key field value in bytes.
//  prgbFieldName		- The name of the field to modify.
//  pbFieldValue		- The modified value for the field.
//  cbFieldValue		- The size of the modified field value in bytes.
//  pRowIDRet			- Pointer to returned rowID(s) for the modified row.
//						  NOTE: If you dan't want the rowID, pass pRowIDRet = 0;
/**************************************************************************/
static STATUS TSModifyField(	String64		prgbTableName,
								String64		rgbKeyFieldName,
								void			*pKeyFieldValue,
								U32				cbKeyFieldValue,
								String64		prgbFieldName,
								void			*pbFieldValue,
								U32				cbFieldValue,
								rowID			*pRowIDRet = 0 );

						
/**************************************************************************/
// TSModifyFields - Modify the contents of one or more fields in one or more
// 				  rows in a table.
//
// Parameters:
//  prgbTableName		- Null terminated name of the table.
//  cFieldsToModify		- Number of entries in Key field arrays. 
//  prgKeyFieldNames	- pointer to array of key field name(s) used to identify the row(s) to modify
//  prgKeyFieldValues	- pointer to array of key field value(s) used to search for the row(s) to modify.
//  cbKeyFieldValues	- the size of the key field value array in bytes.
//  prgFieldNames		- Pointer to array of name(s) of field(s) to modify.
//  prgFieldValues		- Pointer to array of modified value(s) for the field(s).
//  cbFieldValues		- The size of the modified field value array in bytes.
//  prgRowIDsRet		- Pointer to returned rowID(s) for the modified row(s).
//						  NOTE: If you dan't want the rowID, pass pRrgowIDRet = 0;
//  pcFieldsModifiedRet	- Returned number of rows modified.
//  pcIDsRet			- Returned number of IDs returned.
/**************************************************************************/
static STATUS TSModifyFields(	String64		rgbTablename,
								U32				cFieldsToModify,
								String64		prgKeyFieldNames,
								void			*prgKeyFieldValues,
								U32				cbKeyFieldValues,
								String64		prgFieldNames,
								void			*prgFieldValues,
								U32				cbFieldValues,
								rowID			*prgRowIDsRet = 0,
								U32				*pcFieldsModified = 0,
								U32				*pcIDsRet = 0 );

						
/**************************************************************************/
// TSReadRow - Read a specified row from the specified table
//
// Parameters:
//  prgbTableName	- Null terminated name of the table.
//  rgbKeyFieldName	- The key field name used to identify the row to read.
//  pKeyFieldValue	- The key field value used to search for the row to modify.
//  cbKeyFieldValue	- the size of the key field value in bytes.
//  prgbRowDataRet	- Pointer to returned row data.
//  cbRowDataRetMax - size of the above returned row data buffer.
//  pcbRowDataRead	- pointer to returned size of data read in bytes.
//  pRowIDRet		- Pointer to returned rowID for the read row.
//					  NOTE: If you dan't want the rowID, pass pRowIDRet = 0;
/**************************************************************************/
static STATUS TSReadRow(	String64		prgbTableName,
							String64		prgbKeyFieldName,
							void			*pKeyFieldValue,
							U32				cbKeyFieldValue,
							void			*prgbRowDataRet,
							U32				cbRowDataRetMax,
//							U32				*pcbRowDataRead,
							rowID			*pRowIDRet = 0 );

						
/**************************************************************************/
// TSReadRows - Read one or more specified row(s) from the specified table
//
// Parameters:
//  prgbTableName		- Null terminated name of the table.
//  cRowsToRead			- Number of entries in Key field arrays. 
//  prgKeyFieldNames	- pointer to array of key field name(s) used to identify the row(s) to read
//  prgKeyFieldValues	- pointer to array of key field value(s) used to search for the row(s) to read.
//  cbKeyFieldValues	- the size of the key field value array in bytes.
//  prgbRowsDataRet		- Pointer to returned row data.
//  cbRowsDataRetMax	- size of the above returned row data buffer.
//  pcRowsReadRet		- pointer to returned number of rows read.
//  prgRowIDsRet		- Pointer to returned rowID(s) for the read row(s).
//						  NOTE: If you dan't want the rowID, pass pRrgowIDRet = 0;
//  pcRowsReadRet		- Returned number of rows modified.
//  pcIDsRet			- Returned number of IDs returned.
/**************************************************************************/
static STATUS	TSReadRows(	String64		prgbTableName,
							U32				cRowsToRead,
							String64		prgKeyFieldNames,
							void			*prgKeyFieldValues,
							U32				cbKeyFieldValues,
							void			*prgbRowsDataRet,
							U32				cbRowsDataRetMax,
							U32				*pcRowsReadRet,
							rowID			*prgRowIDsRet,
							U32				*pcIDsRet );
						

/**************************************************************************/
// TSDeleteRow - Delete a specified row from the specified table
//
// Parameters:
//  prgbTableName	- Null terminated name of the table.
//  rgbKeyFieldName	- The key field name used to identify the row to read.
//  pKeyFieldValue	- The key field value used to search for the row to modify.
//  cbKeyFieldValue	- the size of the key field value in bytes.
//  pRowIDRet		- Pointer to returned rowID for the read row.
//					  NOTE: If you dan't want the rowID, pass pRowIDRet = 0;
/**************************************************************************/
static STATUS TSDeleteRow(	String64		prgbTableName,
							String64		prgbKeyFieldName,
							void			*pKeyFieldValue,
							U32				cbKeyFieldValue,
							rowID			*pRowIDRet = 0 );

						
/**************************************************************************/
// TSDeleteRows - Delete one or more specified row(s) from the specified table
//
// Parameters:
//  prgbTableName		- Null terminated name of the table.
//  cRowsToDelete		- Number of entries in Key field arrays. 
//  prgKeyFieldNames	- pointer to array of key field name(s) used to identify the row(s) to read
//  prgKeyFieldValues	- pointer to array of key field value(s) used to search for the row(s) to read.
//  cbKeyFieldValues	- the size of the key field value array in bytes.
//  prgRowIDsRet		- Pointer to returned rowID(s) for the deleted row(s).
//						  NOTE: If you dan't want the rowID, pass pRrgowIDRet = 0;
//  pcRowsDeletedRet	- Returned number of rows modified.
//  pcIDsRet			- Returned number of IDs returned.
/**************************************************************************/
static STATUS TSDeleteRows(	String64		prgbTableName,
							U32				cRowsToDelete,
							String64		prgKeyFieldNames,
							void			*prgKeyFieldValues,
							U32				cbKeyFieldValues,
							rowID			*prgRowIDsRet = 0,
							U32				*pcRowsDeletedRet = 0,
							U32				*pcIDsRet = 0 );
								

/**************************************************************************/
// TSListen - Ask for replies when table data is modified.
//
// Parameters:
//	ListenType 		- The type of operation for which you want to listen.
//					  NOTE: See ListenTypeEnum declaration in TableMsgs.h
//	prgbTableName	- The name of the table on which you want to listen. 
//	rgbFieldName	- The name of the field on which you want to listen.
//					  NOTE: rgbFieldName is only used for some ListenTypes. 
//  ReplyMode		- 0 = reply continuously 1 = reply once only.
//  pListenerIDRet	- Pointer to returned Listener ID number.
//					  NOTE: the ListenerID is used to in the call stop listening.
/**************************************************************************/
static STATUS TSListen(	ListenTypeEnum	ListenType,
						String64		prgbTableName,
						String64		prgbFieldName,
						ReplyModeEnum	ReplyMode,
						rowID			*pListenerIDRet );


						
/**************************************************************************/
// TSStopListening - Stop a listen operation previously begun with TSListenTable
//
// Parameters:
//  ListenerID		- The ID returned from the Listen operation.
/**************************************************************************/
static STATUS TSStopListening(	rowID		ListenerID );


};

#endif	// __TSMessenger_h 