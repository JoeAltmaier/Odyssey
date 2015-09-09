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
#ifndef _TIMER_H_
#define _TIMER_H_


#define TIMER_FOREVER           0xFFFFFFFFL
#define TIMER_TYPE_RECOVER      0
#define TIMER_TYPE_SKIP         1
#define TIMER_EVENT_ERROR       0x01
#define TIMER_EVENT_EXPIRE      0x02


typedef struct timer_descr_s    timer_descr_t;
typedef void                    (*timer_callback_t)(timer_descr_t *timer, ul32 now, void *parm);
typedef u16        			timer_event_t;    

struct timer_descr_s {
	timer_callback_t		callback;
	void					*parm;
	ul32					msecs;
	ul32					alarm;
	ul32					count;
	i32						type;
	struct timer_descr_s	*next;
};

bool         	TimerInit(void);
timer_descr_t  	*TimerRegister(timer_callback_t callback, void *parm, ul32 msecs, ul32 count, i32 type);
ul32           	TimerRemove(timer_descr_t *timer);
bool         	TimerChange(timer_descr_t *timer, ul32 msecs);
ul32           	TimerRemaining(void);
bool         	TimerCheck(void);
ul32           	TimerNow(void);

#endif







