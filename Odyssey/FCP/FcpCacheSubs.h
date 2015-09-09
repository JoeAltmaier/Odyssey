/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpCacheSubs.h
// 
// Description:
// Defines for states and actions for cache IO
//
// Update Log 
// 9/6/99 aTaylor: Create file
/*************************************************************************/

// states for read
#define	SIO			0
#define	CACHE_READ	1
#define	MEDIA_READ	2

// states for write
// SIO
#define FILLING_WRITE_CACHE		1

// actions for cache read
#define	START_CACHE_READ			0
#define CONTINUE_CACHE_READ			1
#define	END_CACHE_START_MEDIA_READ	2
#define	END_CACHE_WAIT_ON_RESOURCE	3
#define	START_MEDIA_READ			4
#define CONTINUE_MEDIA_READ			5
#define END_MEDIA_START_CACHE_READ	6
#define	END_MEDIA_WAIT_ON_RESOURCE	7
#define	WAIT_ON_CACHE_RESOURCE		8

// actions for cache write
#define START_WRITE_FILL			0
#define CONTINUE_WRITE_FILL			1
#define END_WRITE_FILL_WAIT			2
// WAIT_ON_CACHE_RESOURCE

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

void				FCP_Build_ReadCTIO (FCP_EVENT_CONTEXT *pContext) ;
void				FCP_Build_Cache_Message (FCP_EVENT_CONTEXT *pContext, U32 relative_offset,
										U32 page_size, U8 NumSGLs);
STATUS				FCP_Cache_Send_CTIO (FCP_EVENT_CONTEXT *p_context, FCP_EVENT_ACTION next_action);
void				FCP_Build_Cache_IOCB(FCP_EVENT_CONTEXT *pContext, U32 relative_offset,
										U32 page_size, U16 Count, U8 SCSI_status, UNSIGNED flags);
void				MakeWaitCacheContext(FcpCacheRequest *pCacheReq, FCP_EVENT_CONTEXT *pContext,
									STATUS status, U32 transfer_count) ;
STATUS				FCP_Handle_Awaken_Cache_Context(FCP_EVENT_CONTEXT *pContext);
STATUS				FCP_Handle_CTIO_CacheWrite (FCP_EVENT_CONTEXT *pContext);
U8					FCP_Get_Cache_Write_Action (U8 state, STATUS status);
STATUS				FCP_Handle_Cache_Write_Command (FCP_EVENT_CONTEXT *pContext, UNSIGNED transfer_length);
U8					FCP_Get_Cache_Read_Action (U8 state, STATUS status) ;
STATUS				FCP_Handle_Cache_Read_Command (FCP_EVENT_CONTEXT *pContext, UNSIGNED transfer_length);
I64					FCP_Combine_Key (U32 HighKey, U32 KeyBlock);
U32					FCP_Get_High_Order_Key (FCP_EVENT_CONTEXT *pContext);
void				FCP_Invalidate_Cache_Buffers(FCP_EVENT_CONTEXT *pContext);
void				FCP_Free_Cache_Buffers(FCP_EVENT_CONTEXT *p_context);
FCP_EVENT_CONTEXT	*FCP_Get_Cache_Context(FCP_EVENT_CONTEXT *pContext);


#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif
