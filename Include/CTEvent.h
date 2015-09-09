//
//  CTEvent.mc - Master OOS event code definition file.
//               Contains event code and message definitions
//               for the Odyssey OS and DDMs.
//
//               Third-party DDMs need separate facility codes
//               (see below).  Do we define them, or do they?
//
//               Following are definitions of the common elements
//               used to define message codes.
//               After these, a few common messages are defined,
//               and then the various facility-specific message
//               files are included to define the remainder of the
//               standard Odyssey event codes.
//
//

/*

 The MessageIdTypedef keyword gives a typedef name that is used in a
 type cast for each message code in the generated include file. Each
 message code appears in the include file with the format: #define
 name ((type) 0xnnnnnnnn) The default value for type is empty, and no
 type cast is generated. It is the programmer's responsibility to
 specify a typedef statement in the application source code to define
 the type. The type used in the typedef must be large enough to
 accomodate the entire 32-bit message code.

//MessageIdTypedef=DWORD

 The SeverityNames keyword defines the set of names that are allowed
 as the value of the Severity keyword in the message definition. The
 set is delimited by left and right parentheses. Associated with each
 severity name is a number that, when shifted left by 30, gives the
 bit pattern to logical-OR with the Facility value and MessageId
 value to form the full 32-bit message code. The default value of
 this keyword is:

 SeverityNames=(
   Informational=0x0
   Warning=0x1
   Error=0x2
   Internal=0x3
   )

 Severity values occupy the high two bits of a 32-bit message code.
 Any severity value that does not fit in two bits is an error. The
 severity codes can be given symbolic names by following each value
 with :name

 The FacilityNames keyword defines the set of names that are allowed
 as the value of the Facility keyword in the message definition. The
 set is delimited by left and right parentheses. Associated with each
 facility name is a facility code.

 Facility codes occupy the low order 12 bits of the high order
 16-bits of a 32-bit message code. Any facility code that does not
 fit in 12 bits is an error. This allows for 4,096 facility codes.
 The first 256 codes are reserved for use by the system software. The
 facility codes can be given symbolic names by following each value
 with :name
 *** If you add a facility code, please be sure to also add an entry for it
     in the "Facilities" string section later in this file (near line 280).  ***
 Presently defined facility codes are:
       CTS_FACILITY_SYSTEM - Really common things like "success" and heap
       CTS_FACILITY_OS     - OS (i.e., Nucleus) error codes
       CTS_FACILITY_MESSAGING - I2O error codes
       CTS_FACILITY_RUNTIME - Runtime library error codes (if any)
       CTS_FACILITY_BSA    - BSA driver codes
       CTS_FACILITY_RAID   - RAID driver codes
       CTS_FACILITY_CMB    - CMB interface [DDM] codes
       CTS_FACILITY_SSAPI  - SSAPI layer codes
       CTS_FACILITY_PTS  - Persistant table Sevice status codes
       CTS_FACILITY_VCM  - Virtual Circuit Master status codes
       CTS_FACILITY_HOTSWAP - Hot Swap Master status codes
       CTS_FACILITY_ALARM - Alarm Master status codes
       CTS_FACILITY_CHAOS  - CHAOS status codes
       CTS_FACILITY_UPGRADE - Upgrade Master status codes
       CTS_FACILITY_FILESYS - File System Master status codes
       CTS_FACILITY_UTILITY - Returned by code in Odyssey\Util\.
       CTS_FACILITY_FCM  - Fibre channel master status/event codes
       CTS_FACILITY_FACIL_NAMES - Special section for defining textual names
                                  of facility codes themselves.

 The LanguageNames keyword defines the set of names that are allowed
 as the value of the Language keyword in the message definition. The
 set is delimited by left and right parentheses. Associated with each
 language name is a number and a file name that are used to name the
 generated resource file that contains the messages for that
 language. The number corresponds to the language identifier to use
 in the resource table. The number is separated from the file name
 with a colon.

 The language numbers used here are adapted from NT's "locale" scheme,
 which concatentates two bit fields to form a 16 bit number.  Bits zero
 through nine are the primary language ID, and bits ten through fifteen
 are the sublanguage (or dialect) ID.  Full language IDs may be formed
 using NT's MAKELANGID macro (see WinNt.h).  Note that the sublanguage
 code is normally set to SUBLANG_DEFAULT ("user default"), which results
 in an apparent upper byte value of 0x04.

 The alphabetic language codes used for file extensions come from
 ISO 639-2/T, the ISO's "terminological" language coding scheme.

 English:  k_eLangEnglish   / MAKELANGID(LANG_ENGLISH,  SUBLANG_DEFAULT)
 German:   k_eLangGerman    / MAKELANGID(LANG_GERMAN,   SUBLANG_DEFAULT)
 French:   k_eLangFrench    / MAKELANGID(LANG_FRENCH,   SUBLANG_DEFAULT)
 Italian:   k_eLangItalian  / MAKELANGID(LANG_ITALIAN,  SUBLANG_DEFAULT)
 Spanish:   k_eLangSpanish  / MAKELANGID(LANG_SPANISH,  SUBLANG_DEFAULT)
 Japanese:  k_eLangJapanese / MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT)

 Any new names in the source file which don't override the built-in
 names are added to the list of valid languages. This allows an
 application to support private languages with descriptive names.
*/


//  BEGIN COMMON EVENT CODE DEFINITIONS

//  standard "success" code - shared by all facilities
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|D|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Informational
//          01 - Warning
//          10 - Error
//          11 - (Reserved, do not use)
//
//      C - is the Customer code flag
//
//      D - indicates whether the event code originates in normal code
//          or in diagnostic routines
//           0 - Message generated by normal operating code
//           1 - Message generated by diagnostic code
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define CTS_FACILITY_VCM                 0x9
#define CTS_FACILITY_UTILITY             0xF
#define CTS_FACILITY_UPGRADE             0xD
#define CTS_FACILITY_SYSTEM              0x0
#define CTS_FACILITY_SSAPI               0x7
#define CTS_FACILITY_RUNTIME             0x3
#define CTS_FACILITY_RAID                0x5
#define CTS_FACILITY_PTS                 0x8
#define CTS_FACILITY_OS                  0x1
#define CTS_FACILITY_MESSAGING           0x2
#define CTS_FACILITY_HOTSWAP             0xA
#define CTS_FACILITY_FLASH               0x10
#define CTS_FACILITY_FILESYS             0xE
#define CTS_FACILITY_FCM                 0x11
#define CTS_FACILITY_FACIL_NAMES         0xFFF
#define CTS_FACILITY_CMB                 0x6
#define CTS_FACILITY_CHAOS               0xC
#define CTS_FACILITY_BSA                 0x4
#define CTS_FACILITY_ALARM               0xB


//
// Define the severity codes
//
#define STATUS_SEVERITY_WARNING          0x1
#define STATUS_SEVERITY_INTERNAL         0x3
#define STATUS_SEVERITY_INFORMATIONAL    0x0
#define STATUS_SEVERITY_ERROR            0x2


//
// MessageId: CTS_SUCCESS
//
// MessageText:
//
//  Success
//
#define CTS_SUCCESS                      0x00000000L

//  standard "not implemented" code
//
// MessageId: CTS_NOT_IMPLEMENTED
//
// MessageText:
//
//  Function not implemented.
//
#define CTS_NOT_IMPLEMENTED              0x80000001L

//  standard "out of memory" code (do we want a separate one for each heap?)
//
// MessageId: CTS_OUT_OF_MEMORY
//
// MessageText:
//
//  Out of memory.
//
#define CTS_OUT_OF_MEMORY                0x80000002L

//  event message renderer requested an unsupported language
//
// MessageId: CTS_UNSUPPORTED_LANGUAGE
//
// MessageText:
//
//  Unsupported language.
//
#define CTS_UNSUPPORTED_LANGUAGE         0x80000003L

//  event message renderer encountered an invalid message code
//
// MessageId: CTS_INVALID_MESSAGE_ID
//
// MessageText:
//
//  Unknown event code.
//
#define CTS_INVALID_MESSAGE_ID           0x80000004L

//  IOP Failed to change state indicating power up
//
// MessageId: CTS_IOP_POWER_TIMEOUT
//
// MessageText:
//
//  The IOP Failed to change state indicating power up.
//
#define CTS_IOP_POWER_TIMEOUT            0x80000005L

//  IOP Failed to change state indicating power up
//
// MessageId: CTS_IOP_ACTIVE_TIMEOUT
//
// MessageText:
//
//  The IOP Failed to change state indicating IOP active.
//
#define CTS_IOP_ACTIVE_TIMEOUT           0x80000006L

//  IOP type unknown
//
// MessageId: CTS_IOP_TYPE_UNKNOWN
//
// MessageText:
//
//  An unknown IOP type was discovered by the BootMgr.
//
#define CTS_IOP_TYPE_UNKNOWN             0x80000007L

//
// MessageId: CTS_IOP_OOS_STATE_NONO
//
// MessageText:
//
//  An IOP cannot as requested be taken out of service; it is in state %d.
//
#define CTS_IOP_OOS_STATE_NONO           0x80000008L

//
// MessageId: CTS_IOP_BAD_IMAGE_SIGNATURE
//
// MessageText:
//
//  IOP image has bad signature.
//
#define CTS_IOP_BAD_IMAGE_SIGNATURE      0x80000009L

//
// MessageId: CTS_IOP_NO_SUCH_SLOT
//
// MessageText:
//
//  Requested action cannot be taken on  IOP.  Invalid slot.
//
#define CTS_IOP_NO_SUCH_SLOT             0x8000000AL

//
// MessageId: CTS_IOP_POWERON_TIMED_OUT
//
// MessageText:
//
//  Request to power on Iop in slot %1 timed out.
//
#define CTS_IOP_POWERON_TIMED_OUT        0x8000000BL

//
// MessageId: CTS_IOP_UNABLE_TO_STOP_TRANSPORT
//
// MessageText:
//
//  Failure stopping transport from slot %1.
//
#define CTS_IOP_UNABLE_TO_STOP_TRANSPORT 0x8000000CL

//
// MessageId: CTS_IOP_UNABLE_TO_RESTART_TRANSPORT
//
// MessageText:
//
//  Failure restarting transport from slot %1.
//
#define CTS_IOP_UNABLE_TO_RESTART_TRANSPORT 0x8000000DL

//
// MessageId: CTS_IOP_POWERON_STATE_NONO
//
// MessageText:
//
//  An IOP cannot as requested be powered on; it is in state %d.
//
#define CTS_IOP_POWERON_STATE_NONO       0x8000000EL

//
// MessageId: CTS_IOP_ITS_STATE_NONO
//
// MessageText:
//
//  An IOP cannot as requested be put into service; it is in state %d.
//
#define CTS_IOP_ITS_STATE_NONO           0x8000000FL

//
// MessageId: CTS_IOP_LOCK_STATE_NONO
//
// MessageText:
//
//  An IOP cannot as requested be locked; it is in state %d.
//
#define CTS_IOP_LOCK_STATE_NONO          0x80000010L

//
// MessageId: CTS_IOP_UNLOCK_STATE_NONO
//
// MessageText:
//
//  An IOP cannot as requested be unlocked; it is in state %d.
//
#define CTS_IOP_UNLOCK_STATE_NONO        0x80000011L

//  here's a message which will probably never be logged.  It exists
//  to exercise the message compiler's escape character handling.
//
// MessageId: CTS_MESSAGE_COMPILER_TEST1
//
// MessageText:
//
//  "This" is at least 7%% of the %1%% solution; it's conceivable that we won't
//  catch all possible {likely} escaping problems, as in %2.%.
//
#define CTS_MESSAGE_COMPILER_TEST1       0x80000012L

//  "invalid parameter" would be a logical global candidate, but it would
//  be almost useless -- having one per facility code allows at least some
//  means for identifying where in a stack trace the problem happened.
//  Returned from Ddm::DoWork, must be numeric 13 to match old OsStatus.h
//
// MessageId: CTS_CHAOS_INAPPROPRIATE_FUNCTION
//
// MessageText:
//
//  The service does not process this message function code.
//
#define CTS_CHAOS_INAPPROPRIATE_FUNCTION 0x0000000DL

// * * *  Facility code display strings.
//        Note that the message IDs are exactly the (unshifted) facility
//        codes being described (e.g., facility "OS" has ID 1).
//  common system-wide codes, not associated with any facility
//
// MessageId: CTS_FACIL_SYSTEM
//
// MessageText:
//
//  Odyssey system
//
#define CTS_FACIL_SYSTEM                 0x0FFF0000L

//  for status reported by the operating system
//
// MessageId: CTS_FACIL_OS
//
// MessageText:
//
//  Operating System
//
#define CTS_FACIL_OS                     0x0FFF0001L

//  the CHAOS transport/messaging layer (used to be I2O)
//
// MessageId: CTS_FACIL_MESSAGING
//
// MessageText:
//
//  Transport
//
#define CTS_FACIL_MESSAGING              0x0FFF0002L

//  the compiler / Odyssey runtime library support, excluding CHAOS
//
// MessageId: CTS_FACIL_RUNTIME
//
// MessageText:
//
//  Runtime library
//
#define CTS_FACIL_RUNTIME                0x0FFF0003L

//  the BSA disk driver family (??)
//
// MessageId: CTS_FACIL_BSA
//
// MessageText:
//
//  Disk driver
//
#define CTS_FACIL_BSA                    0x0FFF0004L

//  the RAID driver family
//
// MessageId: CTS_FACIL_RAID
//
// MessageText:
//
//  RAID driver
//
#define CTS_FACIL_RAID                   0x0FFF0005L

//  the Card Management Bus facility (does this have *any* user-visible name?)
//
// MessageId: CTS_FACIL_CMB
//
// MessageText:
//
//  CMB subsystem
//
#define CTS_FACIL_CMB                    0x0FFF0006L

//  the SSAPI events and localized strings
//
// MessageId: CTS_FACIL_SSAPI
//
// MessageText:
//
//  SSAPI layer
//
#define CTS_FACIL_SSAPI                  0x0FFF0007L

//  the Persistent Table Service facility
//
// MessageId: CTS_FACIL_PTS
//
// MessageText:
//
//  Persistent Table Service
//
#define CTS_FACIL_PTS                    0x0FFF0008L

//  the Virtual Circuit Master
//
// MessageId: CTS_FACIL_VCM
//
// MessageText:
//
//  Virtual Circuit Master
//
#define CTS_FACIL_VCM                    0x0FFF0009L

//  the Hot Swap Master
//
// MessageId: CTS_FACIL_HOTSWAP
//
// MessageText:
//
//  Hot Swap Master
//
#define CTS_FACIL_HOTSWAP                0x0FFF000AL

//  the Alarm Master
//
// MessageId: CTS_FACIL_ALARM
//
// MessageText:
//
//  Alarm Master
//
#define CTS_FACIL_ALARM                  0x0FFF000BL

//  the Upgrade Master
//
// MessageId: CTS_FACIL_UPGRADE
//
// MessageText:
//
//  Image Upgrade Master
//
#define CTS_FACIL_UPGRADE                0x0FFF000CL

//  the CHAOS events and localized strings
//
// MessageId: CTS_FACIL_CHAOS
//
// MessageText:
//
//  CHAOS services
//
#define CTS_FACIL_CHAOS                  0x0FFF000DL

//  the (Flash) File System
//
// MessageId: CTS_FACIL_FILESYS
//
// MessageText:
//
//  Flash File System
//
#define CTS_FACIL_FILESYS                0x0FFF000EL

//  utility code in Odyssey\Util\.
//
// MessageId: CTS_FACIL_UTILITY
//
// MessageText:
//
//  Utility services
//
#define CTS_FACIL_UTILITY                0x0FFF000FL

//  the Flash Storage System and driver
//
// MessageId: CTS_FACIL_FLASH
//
// MessageText:
//
//  FlashStorage
//
#define CTS_FACIL_FLASH                  0x0FFF0010L

//  the Fibre Channel Master
//
// MessageId: CTS_FACIL_FCM
//
// MessageText:
//
//  Fibre Circuit Master
//
#define CTS_FACIL_FCM                    0x0FFF0011L


//  BEGIN EVENT CODE DEFINITION FILE INCLUDES

//  CTS_FACILITY_OS:  OS facility codes

//  CTEvtKernel.mc
//
//  -------------------- Nucleus / kernel error codes -------------------
//  (given generic names to help hide our choice of RTOS kernel)

//  CTS_OS_END_OF_LOG
//
// MessageId: CTS_OS_END_OF_LOG
//
// MessageText:
//
//  End of log.
//
#define CTS_OS_END_OF_LOG                0x80010001L

//  CTS_OS_GROUP_DELETED
//
// MessageId: CTS_OS_GROUP_DELETED
//
// MessageText:
//
//  Group deleted.
//
#define CTS_OS_GROUP_DELETED             0x80010002L

//  CTS_OS_INVALID_DELETE
//
// MessageId: CTS_OS_INVALID_DELETE
//
// MessageText:
//
//  Invalid delete.
//
#define CTS_OS_INVALID_DELETE            0x80010003L

//  CTS_OS_INVALID_DRIVER
//
// MessageId: CTS_OS_INVALID_DRIVER
//
// MessageText:
//
//  Invalid driver.
//
#define CTS_OS_INVALID_DRIVER            0x80010004L

//  CTS_OS_INVALID_ENABLE
//
// MessageId: CTS_OS_INVALID_ENABLE
//
// MessageText:
//
//  Invalid enable.
//
#define CTS_OS_INVALID_ENABLE            0x80010005L

//  CTS_OS_INVALID_ENTRY
//
// MessageId: CTS_OS_INVALID_ENTRY
//
// MessageText:
//
//  Invalid entry.
//
#define CTS_OS_INVALID_ENTRY             0x80010006L

//  CTS_OS_INVALID_FUNCTION
//
// MessageId: CTS_OS_INVALID_FUNCTION
//
// MessageText:
//
//  Invalid function.
//
#define CTS_OS_INVALID_FUNCTION          0x80010007L

//  CTS_OS_INVALID_GROUP
//
// MessageId: CTS_OS_INVALID_GROUP
//
// MessageText:
//
//  Invalid group.
//
#define CTS_OS_INVALID_GROUP             0x80010008L

//  CTS_OS_INVALID_HISR
//
// MessageId: CTS_OS_INVALID_HISR
//
// MessageText:
//
//  Invalid HISR.
//
#define CTS_OS_INVALID_HISR              0x80010009L

//  CTS_OS_INVALID_MAILBOX
//
// MessageId: CTS_OS_INVALID_MAILBOX
//
// MessageText:
//
//  Invalid mailbox.
//
#define CTS_OS_INVALID_MAILBOX           0x8001000AL

//  CTS_OS_INVALID_MEMORY
//
// MessageId: CTS_OS_INVALID_MEMORY
//
// MessageText:
//
//  Invalid memory.
//
#define CTS_OS_INVALID_MEMORY            0x8001000BL

//  CTS_OS_INVALID_MESSAGE
//
// MessageId: CTS_OS_INVALID_MESSAGE
//
// MessageText:
//
//  Invalid message.
//
#define CTS_OS_INVALID_MESSAGE           0x8001000CL

//  CTS_OS_INVALID_OPERATION
//
// MessageId: CTS_OS_INVALID_OPERATION
//
// MessageText:
//
//  Invalid operation.
//
#define CTS_OS_INVALID_OPERATION         0x8001000DL

//  CTS_OS_INVALID_PIPE
//
// MessageId: CTS_OS_INVALID_PIPE
//
// MessageText:
//
//  Invalid pipe.
//
#define CTS_OS_INVALID_PIPE              0x8001000EL

//  CTS_OS_INVALID_POINTER
//
// MessageId: CTS_OS_INVALID_POINTER
//
// MessageText:
//
//  Invalid pointer.
//
#define CTS_OS_INVALID_POINTER           0x8001000FL

//  CTS_OS_INVALID_POOL
//
// MessageId: CTS_OS_INVALID_POOL
//
// MessageText:
//
//  Invalid pool.
//
#define CTS_OS_INVALID_POOL              0x80010010L

//  CTS_OS_INVALID_PREEMPT
//
// MessageId: CTS_OS_INVALID_PREEMPT
//
// MessageText:
//
//  Invalid preempt.
//
#define CTS_OS_INVALID_PREEMPT           0x80010011L

//  CTS_OS_INVALID_PRIORITY
//
// MessageId: CTS_OS_INVALID_PRIORITY
//
// MessageText:
//
//  Invalid priority.
//
#define CTS_OS_INVALID_PRIORITY          0x80010012L

//  CTS_OS_INVALID_QUEUE
//
// MessageId: CTS_OS_INVALID_QUEUE
//
// MessageText:
//
//  Invalid queue.
//
#define CTS_OS_INVALID_QUEUE             0x80010013L

//  CTS_OS_INVALID_RESUME
//
// MessageId: CTS_OS_INVALID_RESUME
//
// MessageText:
//
//  Invalid resume.
//
#define CTS_OS_INVALID_RESUME            0x80010014L

//  CTS_OS_INVALID_SEMAPHORE
//
// MessageId: CTS_OS_INVALID_SEMAPHORE
//
// MessageText:
//
//  Invalid semaphore.
//
#define CTS_OS_INVALID_SEMAPHORE         0x80010015L

//  CTS_OS_INVALID_SIZE
//
// MessageId: CTS_OS_INVALID_SIZE
//
// MessageText:
//
//  Invalid size.
//
#define CTS_OS_INVALID_SIZE              0x80010016L

//  CTS_OS_INVALID_START
//
// MessageId: CTS_OS_INVALID_START
//
// MessageText:
//
//  Invalid start.
//
#define CTS_OS_INVALID_START             0x80010017L

//  CTS_OS_INVALID_SUSPEND
//
// MessageId: CTS_OS_INVALID_SUSPEND
//
// MessageText:
//
//  Invalid suspend.
//
#define CTS_OS_INVALID_SUSPEND           0x80010018L

//  CTS_OS_INVALID_TASK
//
// MessageId: CTS_OS_INVALID_TASK
//
// MessageText:
//
//  Invalid task.
//
#define CTS_OS_INVALID_TASK              0x80010019L

//  CTS_OS_INVALID_TIMER
//
// MessageId: CTS_OS_INVALID_TIMER
//
// MessageText:
//
//  Invalid timer.
//
#define CTS_OS_INVALID_TIMER             0x8001001AL

//  CTS_OS_INVALID_VECTOR
//
// MessageId: CTS_OS_INVALID_VECTOR
//
// MessageText:
//
//  Invalid vector.
//
#define CTS_OS_INVALID_VECTOR            0x8001001BL

//  CTS_OS_MAILBOX_DELETED
//
// MessageId: CTS_OS_MAILBOX_DELETED
//
// MessageText:
//
//  Mailbox deleted.
//
#define CTS_OS_MAILBOX_DELETED           0x8001001CL

//  CTS_OS_MAILBOX_EMPTY
//
// MessageId: CTS_OS_MAILBOX_EMPTY
//
// MessageText:
//
//  Mailbox empty.
//
#define CTS_OS_MAILBOX_EMPTY             0x8001001DL

//  CTS_OS_MAILBOX_FULL
//
// MessageId: CTS_OS_MAILBOX_FULL
//
// MessageText:
//
//  Mailbox full.
//
#define CTS_OS_MAILBOX_FULL              0x8001001EL

//  CTS_OS_MAILBOX_RESET
//
// MessageId: CTS_OS_MAILBOX_RESET
//
// MessageText:
//
//  Mailbox reset.
//
#define CTS_OS_MAILBOX_RESET             0x8001001FL

//  CTS_OS_NO_MEMORY
//
// MessageId: CTS_OS_NO_MEMORY
//
// MessageText:
//
//  No memory.
//
#define CTS_OS_NO_MEMORY                 0x80010020L

//  CTS_OS_NO_MORE_LISRS
//
// MessageId: CTS_OS_NO_MORE_LISRS
//
// MessageText:
//
//  No more LISRs.
//
#define CTS_OS_NO_MORE_LISRS             0x80010021L

//  CTS_OS_NO_PARTITION
//
// MessageId: CTS_OS_NO_PARTITION
//
// MessageText:
//
//  No partition.
//
#define CTS_OS_NO_PARTITION              0x80010022L

//  CTS_OS_NOT_DISABLED
//
// MessageId: CTS_OS_NOT_DISABLED
//
// MessageText:
//
//  Not disabled.
//
#define CTS_OS_NOT_DISABLED              0x80010023L

//  CTS_OS_NOT_PRESENT
//
// MessageId: CTS_OS_NOT_PRESENT
//
// MessageText:
//
//  Not present.
//
#define CTS_OS_NOT_PRESENT               0x80010024L

//  CTS_OS_NOT_REGISTERED
//
// MessageId: CTS_OS_NOT_REGISTERED
//
// MessageText:
//
//  Not registered.
//
#define CTS_OS_NOT_REGISTERED            0x80010025L

//  CTS_OS_NOT_TERMINATED
//
// MessageId: CTS_OS_NOT_TERMINATED
//
// MessageText:
//
//  Not terminated.
//
#define CTS_OS_NOT_TERMINATED            0x80010026L

//  CTS_OS_PIPE_DELETED
//
// MessageId: CTS_OS_PIPE_DELETED
//
// MessageText:
//
//  Pipe deleted.
//
#define CTS_OS_PIPE_DELETED              0x80010027L

//  CTS_OS_PIPE_EMPTY
//
// MessageId: CTS_OS_PIPE_EMPTY
//
// MessageText:
//
//  Pipe empty.
//
#define CTS_OS_PIPE_EMPTY                0x80010028L

//  CTS_OS_PIPE_FULL
//
// MessageId: CTS_OS_PIPE_FULL
//
// MessageText:
//
//  Pipe full.
//
#define CTS_OS_PIPE_FULL                 0x80010029L

//  CTS_OS_PIPE_RESET
//
// MessageId: CTS_OS_PIPE_RESET
//
// MessageText:
//
//  Pipe reset.
//
#define CTS_OS_PIPE_RESET                0x8001002AL

//  CTS_OS_POOL_DELETED
//
// MessageId: CTS_OS_POOL_DELETED
//
// MessageText:
//
//  Pool deleted.
//
#define CTS_OS_POOL_DELETED              0x8001002BL

//  CTS_OS_QUEUE_DELETED
//
// MessageId: CTS_OS_QUEUE_DELETED
//
// MessageText:
//
//  Queue deleted.
//
#define CTS_OS_QUEUE_DELETED             0x8001002CL

//  CTS_OS_QUEUE_EMPTY
//
// MessageId: CTS_OS_QUEUE_EMPTY
//
// MessageText:
//
//  Queue empty.
//
#define CTS_OS_QUEUE_EMPTY               0x8001002DL

//  CTS_OS_QUEUE_FULL
//
// MessageId: CTS_OS_QUEUE_FULL
//
// MessageText:
//
//  Queue full.
//
#define CTS_OS_QUEUE_FULL                0x8001002EL

//  CTS_OS_QUEUE_RESET
//
// MessageId: CTS_OS_QUEUE_RESET
//
// MessageText:
//
//  Queue reset.
//
#define CTS_OS_QUEUE_RESET               0x8001002FL

//  CTS_OS_SEMAPHORE_DELETED
//
// MessageId: CTS_OS_SEMAPHORE_DELETED
//
// MessageText:
//
//  Semaphore deleted.
//
#define CTS_OS_SEMAPHORE_DELETED         0x80010030L

//  CTS_OS_SEMAPHORE_RESET
//
// MessageId: CTS_OS_SEMAPHORE_RESET
//
// MessageText:
//
//  Semaphore reset.
//
#define CTS_OS_SEMAPHORE_RESET           0x80010031L

//  CTS_OS_TIMEOUT
//
// MessageId: CTS_OS_TIMEOUT
//
// MessageText:
//
//  Timeout.
//
#define CTS_OS_TIMEOUT                   0x80010032L

//  CTS_OS_UNAVAILABLE
//
// MessageId: CTS_OS_UNAVAILABLE
//
// MessageText:
//
//  Unavailable.
//
#define CTS_OS_UNAVAILABLE               0x80010033L

//  CTS_FACILITY_MESSAGING:  Messaging facility codes

//  CTEvtI2O.mc
//
//  -------------------- I2O messaging error codes ----------------------
//  (note that the original I2O error code values are preserved, albeit
//   with error severity and our "Messaging" facility code ORed in)

//  ?
//
// MessageId: CTS_MSG_INVALID_OBJ_ID
//
// MessageText:
//
//  Invalid object ID.
//
#define CTS_MSG_INVALID_OBJ_ID           0x80020001L

//  ?
//
// MessageId: CTS_MSG_INVALID_OWNER_ID
//
// MessageText:
//
//  Invalid owner ID.
//
#define CTS_MSG_INVALID_OWNER_ID         0x80020002L

//  ?
//
// MessageId: CTS_MSG_NOT_ISR_CALLABLE
//
// MessageText:
//
//  Not ISR-callable.
//
#define CTS_MSG_NOT_ISR_CALLABLE         0x80020003L

//  ?
//
// MessageId: CTS_MSG_TIMER_IN_USE
//
// MessageText:
//
//  Timer in use.
//
#define CTS_MSG_TIMER_IN_USE             0x80020004L

//  ?
//
// MessageId: CTS_MSG_TIMER_EVT_IN_USE
//
// MessageText:
//
//  Timer event in use.
//
#define CTS_MSG_TIMER_EVT_IN_USE         0x80020005L

//  ?
//
// MessageId: CTS_MSG_TIMER_INACTIVE
//
// MessageText:
//
//  Timer inactive.
//
#define CTS_MSG_TIMER_INACTIVE           0x80020006L

//  ?
//
// MessageId: CTS_MSG_INVALID_OBJ_NAME
//
// MessageText:
//
//  Invalid object name.
//
#define CTS_MSG_INVALID_OBJ_NAME         0x80020007L

//  ?
//
// MessageId: CTS_MSG_NOT_IMPLEMENTED
//
// MessageText:
//
//  Not implemented.
//
#define CTS_MSG_NOT_IMPLEMENTED          0x80020008L

//  ?
//
// MessageId: CTS_MSG_ALLOCATION_FAILED
//
// MessageText:
//
//  Allocation failed.
//
#define CTS_MSG_ALLOCATION_FAILED        0x80020009L

//  ?
//
// MessageId: CTS_MSG_MAX_EVENTS_OUTSTANDING
//
// MessageText:
//
//  Maximum events outstanding.
//
#define CTS_MSG_MAX_EVENTS_OUTSTANDING   0x8002000AL

//  ?
//
// MessageId: CTS_MSG_INVALID_ISR_FUNC
//
// MessageText:
//
//  Invalid ISR function.
//
#define CTS_MSG_INVALID_ISR_FUNC         0x8002000BL

//  ?
//
// MessageId: CTS_MSG_LIB_NOT_INSTALLED
//
// MessageText:
//
//  Library not installed.
//
#define CTS_MSG_LIB_NOT_INSTALLED        0x8002000CL

//  ?
//
// MessageId: CTS_MSG_UNKNOWN_ERROR
//
// MessageText:
//
//  Unknown error.
//
#define CTS_MSG_UNKNOWN_ERROR            0x8002000DL

//  ?
//
// MessageId: CTS_MSG_FUNC_NOT_IMPLEMENTED
//
// MessageText:
//
//  Function not implemented.
//
#define CTS_MSG_FUNC_NOT_IMPLEMENTED     0x8002000EL

//  ?
//
// MessageId: CTS_MSG_FRAME_ALREADY_FREE
//
// MessageText:
//
//  Frame already free.
//
#define CTS_MSG_FRAME_ALREADY_FREE       0x8002000FL

//  ?
//
// MessageId: CTS_MSG_FRAME_ALLOCATION_FAILED
//
// MessageText:
//
//  Frame allocation failed.
//
#define CTS_MSG_FRAME_ALLOCATION_FAILED  0x80020010L

//  ?
//
// MessageId: CTS_MSG_FRAME_POST_ERROR
//
// MessageText:
//
//  Frame post error.
//
#define CTS_MSG_FRAME_POST_ERROR         0x80020011L

//  ?
//
// MessageId: CTS_MSG_FRAME_INVALID
//
// MessageText:
//
//  Frame invalid.
//
#define CTS_MSG_FRAME_INVALID            0x80020012L

//  ?
//
// MessageId: CTS_MSG_FRAME_DISPATCH_ERROR
//
// MessageText:
//
//  Frame dispatch error.
//
#define CTS_MSG_FRAME_DISPATCH_ERROR     0x80020013L

//  ?
//
// MessageId: CTS_MSG_DMA_UNSUPPORTED_BUS
//
// MessageText:
//
//  DMA not supported on bus (?).
//
#define CTS_MSG_DMA_UNSUPPORTED_BUS      0x80020014L

//  ?
//
// MessageId: CTS_MSG_DMA_INVALID
//
// MessageText:
//
//  DMA invalid.
//
#define CTS_MSG_DMA_INVALID              0x80020015L

//  ?
//
// MessageId: CTS_MSG_DMA_ERROR
//
// MessageText:
//
//  DMA error.
//
#define CTS_MSG_DMA_ERROR                0x80020016L

//  ?
//
// MessageId: CTS_MSG_DMA_CHAIN_ERROR
//
// MessageText:
//
//  DMA chain error.
//
#define CTS_MSG_DMA_CHAIN_ERROR          0x80020017L

//  ?
//
// MessageId: CTS_MSG_TOO_MANY_ENTRIES
//
// MessageText:
//
//  Too many entries.
//
#define CTS_MSG_TOO_MANY_ENTRIES         0x80020018L

//  ?
//
// MessageId: CTS_MSG_TID_ERROR
//
// MessageText:
//
//  TID error.
//
#define CTS_MSG_TID_ERROR                0x80020019L

//  ?
//
// MessageId: CTS_MSG_ARG_NOT_IMPLEMENTED
//
// MessageText:
//
//  Argument not implemented.
//
#define CTS_MSG_ARG_NOT_IMPLEMENTED      0x8002001AL

//  ?
//
// MessageId: CTS_MSG_DMA_PAGE_SIZE_ERROR
//
// MessageText:
//
//  DMA page size error.
//
#define CTS_MSG_DMA_PAGE_SIZE_ERROR      0x8002001BL

//  ?
//
// MessageId: CTS_MSG_ARG_ERROR
//
// MessageText:
//
//  Argument error.
//
#define CTS_MSG_ARG_ERROR                0x8002001CL

//  ?
//
// MessageId: CTS_MSG_INVALID_USER_FUNC
//
// MessageText:
//
//  Invalid user function.
//
#define CTS_MSG_INVALID_USER_FUNC        0x8002001DL

//  ?
//
// MessageId: CTS_MSG_INVALID_BLOCK
//
// MessageText:
//
//  Invalid block.
//
#define CTS_MSG_INVALID_BLOCK            0x8002001EL

//  ?
//
// MessageId: CTS_MSG_SEM_INIT_ERROR
//
// MessageText:
//
//  Semaphore initialization error.
//
#define CTS_MSG_SEM_INIT_ERROR           0x8002001FL

//  ?
//
// MessageId: CTS_MSG_SEM_TAKE_ERROR
//
// MessageText:
//
//  Semaphore "take" error.
//
#define CTS_MSG_SEM_TAKE_ERROR           0x80020020L

//  ?
//
// MessageId: CTS_MSG_SEM_GIVE_ERROR
//
// MessageText:
//
//  Semaphore "give" error.
//
#define CTS_MSG_SEM_GIVE_ERROR           0x80020021L

//  ?
//
// MessageId: CTS_MSG_PIPE_SEND_ERROR
//
// MessageText:
//
//  Pipe send error.
//
#define CTS_MSG_PIPE_SEND_ERROR          0x80020022L

//  ?
//
// MessageId: CTS_MSG_PIPE_RECEIVE_ERROR
//
// MessageText:
//
//  Pipe receive error.
//
#define CTS_MSG_PIPE_RECEIVE_ERROR       0x80020023L

//  ?
//
// MessageId: CTS_MSG_DMA_FULL
//
// MessageText:
//
//  DMA full.
//
#define CTS_MSG_DMA_FULL                 0x80020024L

//  ?
//
// MessageId: CTS_MSG_INVALID_EVENT_PRI
//
// MessageText:
//
//  Invalid event PRI [priority?].
//
#define CTS_MSG_INVALID_EVENT_PRI        0x80020025L

//  ?
//
// MessageId: CTS_MSG_INVALID_ERR_ACT
//
// MessageText:
//
//  Invalid error ACT [action?].
//
#define CTS_MSG_INVALID_ERR_ACT          0x80020026L

//  ?
//
// MessageId: CTS_MSG_INSUFFICIENT_MEMORY
//
// MessageText:
//
//  Insufficient memory.
//
#define CTS_MSG_INSUFFICIENT_MEMORY      0x80020027L

//  ?
//
// MessageId: CTS_MSG_INVALID_PAGE_ARRAY
//
// MessageText:
//
//  Invalid page array.
//
#define CTS_MSG_INVALID_PAGE_ARRAY       0x80020028L

//  ?
//
// MessageId: CTS_MSG_INVALID_CONFIG_MESSAGE
//
// MessageText:
//
//  Invalid configuration message.
//
#define CTS_MSG_INVALID_CONFIG_MESSAGE   0x80020029L

//  ?
//
// MessageId: CTS_MSG_DMA_CANCEL_FAILED
//
// MessageText:
//
//  DMA cancel failed.
//
#define CTS_MSG_DMA_CANCEL_FAILED        0x8002002AL

//  ?
//
// MessageId: CTS_MSG_DMA_RESUME_FAILED
//
// MessageText:
//
//  DMA resume failed.
//
#define CTS_MSG_DMA_RESUME_FAILED        0x8002002BL

//  ?
//
// MessageId: CTS_MSG_DMA_OBJECT_SUSPENDED
//
// MessageText:
//
//  DMA object suspended.
//
#define CTS_MSG_DMA_OBJECT_SUSPENDED     0x8002002CL

//  ?
//
// MessageId: CTS_MSG_MPB_SIZE_ERROR
//
// MessageText:
//
//  MPB size error.
//
#define CTS_MSG_MPB_SIZE_ERROR           0x8002002DL

//  ?
//
// MessageId: CTS_MSG_NO_NVS
//
// MessageText:
//
//  No NVS [?].
//
#define CTS_MSG_NO_NVS                   0x8002002EL

//  ?
//
// MessageId: CTS_MSG_NVS_CREATE_ERROR
//
// MessageText:
//
//  NVS [?] create error.
//
#define CTS_MSG_NVS_CREATE_ERROR         0x8002002FL

//  ?
//
// MessageId: CTS_MSG_NVS_OPEN_ERROR
//
// MessageText:
//
//  NVS [?] open error.
//
#define CTS_MSG_NVS_OPEN_ERROR           0x80020030L

//  ?
//
// MessageId: CTS_MSG_NVS_CLOSE_ERROR
//
// MessageText:
//
//  NVS [?] close error.
//
#define CTS_MSG_NVS_CLOSE_ERROR          0x80020031L

//  ?
//
// MessageId: CTS_MSG_NVS_DELETE_ERROR
//
// MessageText:
//
//  NVS [?] delete error.
//
#define CTS_MSG_NVS_DELETE_ERROR         0x80020032L

//  ?
//
// MessageId: CTS_MSG_NVS_WRITE_ERROR
//
// MessageText:
//
//  NVS [?] write error.
//
#define CTS_MSG_NVS_WRITE_ERROR          0x80020033L

//  ?
//
// MessageId: CTS_MSG_BBU_NOT_PRESENT
//
// MessageText:
//
//  BBU [?] not present.
//
#define CTS_MSG_BBU_NOT_PRESENT          0x80020034L

//  ?
//
// MessageId: CTS_MSG_INSUFFICIENT_NVRAM
//
// MessageText:
//
//  Insufficient NVRAM.
//
#define CTS_MSG_INSUFFICIENT_NVRAM       0x80020035L

//  ?
//
// MessageId: CTS_MSG_NVRAM_ACCESS_ERROR
//
// MessageText:
//
//  NVRAM access error.
//
#define CTS_MSG_NVRAM_ACCESS_ERROR       0x80020036L

//  ?
//
// MessageId: CTS_MSG_INVALID_EVENT_HANDLER
//
// MessageText:
//
//  Invalid event handler.
//
#define CTS_MSG_INVALID_EVENT_HANDLER    0x80020037L

//  ?
//
// MessageId: CTS_MSG_OBJ_DESTROY_ERROR
//
// MessageText:
//
//  Object destroy error.
//
#define CTS_MSG_OBJ_DESTROY_ERROR        0x80020038L

//  ?
//
// MessageId: CTS_MSG_UNAVAILABLE
//
// MessageText:
//
//  Unavailable.
//
#define CTS_MSG_UNAVAILABLE              0x80020039L

//  ?
//
// MessageId: CTS_MSG_TIMEOUT
//
// MessageText:
//
//  Timeout.
//
#define CTS_MSG_TIMEOUT                  0x8002003AL

//  CTS_FACILITY_RUNTIME:  Standard compiler runtime codes
//*** none presently defined ***

//  CTS_FACILITY_BSA:  Block Storage Access codes (raw disk driver)
//
//  CTErrBSA.mc - Error codes unique to the BSA HDM.
//                Included by CTEvent.mc.  See that file for more info.
//
//
//  a BSA sample error
//
// MessageId: CTS_BSA_DRIVE_OFFLINE
//
// MessageText:
//
//  Drive is offline.
//
#define CTS_BSA_DRIVE_OFFLINE            0x80040001L

//  CTS_FACILITY_RAID:  RAID DDM codes
//
//  CTErrRAID.mc - Error codes unique to the RAID HDM.
//                 Included by CTEvent.mc.  See that file for more info.
//
//
//
//		ARRAY RELATED EVENTS
//
//  New Array [name] created.
//
// MessageId: CTS_RMSTR_ARRAY_ADDED
//
// MessageText:
//
//  Array %1 created successfully.
//
#define CTS_RMSTR_ARRAY_ADDED            0x00050001L

//  Array [name] deleted from the system.
//
// MessageId: CTS_RMSTR_ARRAY_DELETED
//
// MessageText:
//
//  Array %1 deleted.
//
#define CTS_RMSTR_ARRAY_DELETED          0x00050002L

//  Array [name] is now fault tolerant.
//
// MessageId: CTS_RMSTR_ARRAY_FAULT_TOLERANT
//
// MessageText:
//
//  Array %1 is now fault tolerant.
//
#define CTS_RMSTR_ARRAY_FAULT_TOLERANT   0x00050003L

//  Array [name] is critical (non-redundant configuration)
//
// MessageId: CTS_RMSTR_ARRAY_CRITICAL
//
// MessageText:
//
//  Array %1 is critical (non-redundant configuration)
//
#define CTS_RMSTR_ARRAY_CRITICAL         0x80050004L

//  Array [name] is offline
//
// MessageId: CTS_RMSTR_ARRAY_OFFLINE
//
// MessageText:
//
//  Array %1 is off-line.
//
#define CTS_RMSTR_ARRAY_OFFLINE          0x80050005L

//
//		SPARE RELATED EVENTS
//
//  Dedicated Spare [name] added to array [name]
//
// MessageId: CTS_RMSTR_DEDICATED_SPARE_ADDED
//
// MessageText:
//
//  Dedicated spare %1 added to Array %2.
//
#define CTS_RMSTR_DEDICATED_SPARE_ADDED  0x00050006L

//  Dedicated Spare [disk] added to array [name]
//
// MessageId: CTS_RMSTR_DEDICATED_SPARE_DISK_ADDED
//
// MessageText:
//
//  Dedicated spare [Slot=%1] added to Array %2.
//
#define CTS_RMSTR_DEDICATED_SPARE_DISK_ADDED 0x00050007L

//  Pool Spare [name] added to system spare pool
//
// MessageId: CTS_RMSTR_SYSTEM_POOL_SPARE_ADDED
//
// MessageText:
//
//  Pool Spare %1 added to system pool spare.
//
#define CTS_RMSTR_SYSTEM_POOL_SPARE_ADDED 0x00050008L

//  Pool Spare [Disk] added to system spare pool
//
// MessageId: CTS_RMSTR_SYSTEM_POOL_SPARE_DISK_ADDED
//
// MessageText:
//
//  Pool Spare [Slot=%1] added to system pool spare.
//
#define CTS_RMSTR_SYSTEM_POOL_SPARE_DISK_ADDED 0x00050009L

//  Pool Spare [name] added to for host
//
// MessageId: CTS_RMSTR_HOST_POOL_SPARE_ADDED
//
// MessageText:
//
//  Pool Spare %1 added to Host %2
//
#define CTS_RMSTR_HOST_POOL_SPARE_ADDED  0x0005000AL

//  Pool Spare [disk] added for host
//
// MessageId: CTS_RMSTR_HOST_POOL_SPARE_DISK_ADDED
//
// MessageText:
//
//  Host Pool Spare [Slot=%1] added to Host %2
//
#define CTS_RMSTR_HOST_POOL_SPARE_DISK_ADDED 0x0005000BL

//  Dedicated Spare [name] for array [name] deleted 
//
// MessageId: CTS_RMSTR_DEDICATED_SPARE_DELETED
//
// MessageText:
//
//  Dedicated spare %1 for array %2 deleted.
//
#define CTS_RMSTR_DEDICATED_SPARE_DELETED 0x0005000CL

//  Dedicated Spare [disk] for array [name] deleted 
//
// MessageId: CTS_RMSTR_DEDICATED_SPARE_DISK_DELETED
//
// MessageText:
//
//  Dedicated spare [Slot=%1] for array %2 deleted.
//
#define CTS_RMSTR_DEDICATED_SPARE_DISK_DELETED 0x0005000DL

//  System Pool Spare [name] deleted
//
// MessageId: CTS_RMSTR_SYSTEM_POOL_SPARE_DELETED
//
// MessageText:
//
//  System Pool Spare %1 deleted.
//
#define CTS_RMSTR_SYSTEM_POOL_SPARE_DELETED 0x0005000EL

//  System Pool Spare [disk] deleted
//
// MessageId: CTS_RMSTR_SYSTEM_POOL_SPARE_DISK_DELETED
//
// MessageText:
//
//  System Pool Spare [Slot=%1] deleted.
//
#define CTS_RMSTR_SYSTEM_POOL_SPARE_DISK_DELETED 0x0005000FL

//  Host Pool Spare [name] deleted from host [name]
//
// MessageId: CTS_RMSTR_HOST_POOL_SPARE_DELETED
//
// MessageText:
//
//  Host Pool Spare %1 deleted for Host %2.
//
#define CTS_RMSTR_HOST_POOL_SPARE_DELETED 0x00050010L

//  Host Pool Spare [disk] deleted from host [name]
//
// MessageId: CTS_RMSTR_HOST_POOL_SPARE_DISK_DELETED
//
// MessageText:
//
//  Host Pool Spare [Slot=%1] deleted for host %2.
//
#define CTS_RMSTR_HOST_POOL_SPARE_DISK_DELETED 0x00050011L

//  Dedicated spare [name] used to regenerate for array [name]
//
// MessageId: CTS_RMSTR_DEDICATED_SPARE_ACTIVATED
//
// MessageText:
//
//  Dedicated spare %1 used to regenerate array %2.
//
#define CTS_RMSTR_DEDICATED_SPARE_ACTIVATED 0x00050012L

//  Dedicated spare [disk] used to regenerate for array [name]
//
// MessageId: CTS_RMSTR_DEDICATED_SPARE_DISK_ACTIVATED
//
// MessageText:
//
//  Dedicated spare [Slot=%1] used to regenerate array %2.
//
#define CTS_RMSTR_DEDICATED_SPARE_DISK_ACTIVATED 0x00050013L

//  System Pool spare [name] used to regenerate for array [name]
//
// MessageId: CTS_RMSTR_SYSTEM_POOL_SPARE_ACTIVATED
//
// MessageText:
//
//  System Pool spare %1 used to regenerate array %2.
//
#define CTS_RMSTR_SYSTEM_POOL_SPARE_ACTIVATED 0x00050014L

//  System Pool spare [disk] used to regenerate for array [name]
//
// MessageId: CTS_RMSTR_SYSTEM_POOL_SPARE_DISK_ACTIVATED
//
// MessageText:
//
//  System Pool spare [Slot=%1] used to regenerate array %2.
//
#define CTS_RMSTR_SYSTEM_POOL_SPARE_DISK_ACTIVATED 0x00050015L

//  Host Pool spare [name] for host [name] used to regenerate for array [name]
//
// MessageId: CTS_RMSTR_HOST_POOL_SPARE_ACTIVATED
//
// MessageText:
//
//  Host Pool spare %1 for host %2 used to regenerate array %3
//
#define CTS_RMSTR_HOST_POOL_SPARE_ACTIVATED 0x00050016L

//  Host Pool spare [disk] for host [name] used to regenerate for array [name]
//
// MessageId: CTS_RMSTR_HOST_POOL_SPARE_DISK_ACTIVATED
//
// MessageText:
//
//  Host Pool spare [Slot=%1] for host %2 used to regenerate array %3.
//
#define CTS_RMSTR_HOST_POOL_SPARE_DISK_ACTIVATED 0x00050017L

//  No more valid spares available for array [name]
//
// MessageId: CTS_RMSTR_NO_MORE_SPARES
//
// MessageText:
//
//  No more valid spares available for array %1.
//
#define CTS_RMSTR_NO_MORE_SPARES         0x40050018L

//  No valid spare found for array [name]. Regenerate could not be started
//
// MessageId: CTS_RMSTR_REGENERATE_NOT_STARTED
//
// MessageText:
//
//  No valid spare found for array %1. Regenerate could not be started.
//
#define CTS_RMSTR_REGENERATE_NOT_STARTED 0x40050019L

//
//		UTILITY RELATED EVENTS
//
//  Initialize started on array [name]
//
// MessageId: CTS_RMSTR_BKGD_INIT_STARTED
//
// MessageText:
//
//  Initialize started on array %1.
//
#define CTS_RMSTR_BKGD_INIT_STARTED      0x0005001AL

//  Initialize completed on array [name]
//
// MessageId: CTS_RMSTR_BKGD_INIT_COMPLETED
//
// MessageText:
//
//  Initialize completed on array %1.
//
#define CTS_RMSTR_BKGD_INIT_COMPLETED    0x0005001BL

//  Initialize aborted on array [name] due to I/O error.
//
// MessageId: CTS_RMSTR_BKGD_INIT_ABORTED_IOERROR
//
// MessageText:
//
//  Initialize aborted on array %1 due to I/O Error.
//
#define CTS_RMSTR_BKGD_INIT_ABORTED_IOERROR 0x0005001CL

//  Verify started
//
// MessageId: CTS_RMSTR_VERIFY_STARTED
//
// MessageText:
//
//  Verify started on array %1.
//
#define CTS_RMSTR_VERIFY_STARTED         0x0005001DL

//  Verify Priority Changed to High
//
// MessageId: CTS_RMSTR_VERIFY_PRIORITY_CHANGED_HIGH
//
// MessageText:
//
//  Verify running on array %1 has its priority changed to High.
//
#define CTS_RMSTR_VERIFY_PRIORITY_CHANGED_HIGH 0x0005001EL

//  Verify Priority Changed to Low
//
// MessageId: CTS_RMSTR_VERIFY_PRIORITY_CHANGED_LOW
//
// MessageText:
//
//  Verify running on array %1 has its priority changed to Low.
//
#define CTS_RMSTR_VERIFY_PRIORITY_CHANGED_LOW 0x0005001FL

//  Verify Priority Changed to Medium
//
// MessageId: CTS_RMSTR_VERIFY_PRIORITY_CHANGED_MEDIUM
//
// MessageText:
//
//  Verify running on array %1 has its priority changed to Medium.
//
#define CTS_RMSTR_VERIFY_PRIORITY_CHANGED_MEDIUM 0x00050020L

//  Verify completed
//
// MessageId: CTS_RMSTR_VERIFY_COMPLETED
//
// MessageText:
//
//  Verify completed for array %1, %2 miscompares found.
//
#define CTS_RMSTR_VERIFY_COMPLETED       0x00050021L

//  Verify aborted by user
//
// MessageId: CTS_RMSTR_VERIFY_ABORTED_BY_USER
//
// MessageText:
//
//  Verify on array %1 was aborted by user, %2 miscompares found.
//
#define CTS_RMSTR_VERIFY_ABORTED_BY_USER 0x00050022L

//  Verify aborted due to I/O Error
//
// MessageId: CTS_RMSTR_VERIFY_ABORTED_IOERROR
//
// MessageText:
//
//  Verify on array %1 was aborted due to I/O error, %2 miscompares found.
//
#define CTS_RMSTR_VERIFY_ABORTED_IOERROR 0x40050023L

//  Regenerate started
//
// MessageId: CTS_RMSTR_REGENERATE_STARTED
//
// MessageText:
//
//  Regenerate started on array %1.
//
#define CTS_RMSTR_REGENERATE_STARTED     0x00050024L

//  Regenerate Priority Changed High
//
// MessageId: CTS_RMSTR_REGENERATE_PRIORITY_CHANGED_HIGH
//
// MessageText:
//
//  Regenerate running on array %1 has its priority changed to High.
//
#define CTS_RMSTR_REGENERATE_PRIORITY_CHANGED_HIGH 0x00050025L

//  Regenerate Priority Changed Low
//
// MessageId: CTS_RMSTR_REGENERATE_PRIORITY_CHANGED_LOW
//
// MessageText:
//
//  Regenerate running on array %1 has its priority changed to Low.
//
#define CTS_RMSTR_REGENERATE_PRIORITY_CHANGED_LOW 0x00050026L

//  Regenerate Priority Changed Medium
//
// MessageId: CTS_RMSTR_REGENERATE_PRIORITY_CHANGED_MEDIUM
//
// MessageText:
//
//  Regenerate running on array %1 has its priority changed to Medium.
//
#define CTS_RMSTR_REGENERATE_PRIORITY_CHANGED_MEDIUM 0x00050027L

//  Regenerate completed
//
// MessageId: CTS_RMSTR_REGENERATE_COMPLETED
//
// MessageText:
//
//  Regenerate completed on array %1.
//
#define CTS_RMSTR_REGENERATE_COMPLETED   0x00050028L

//  Regenerate aborted by user
//
// MessageId: CTS_RMSTR_REGENERATE_ABORTED_BY_USER
//
// MessageText:
//
//  Regenerate on array %1 was aborted by user.
//
#define CTS_RMSTR_REGENERATE_ABORTED_BY_USER 0x00050029L

//  Regenerate aborted due to I/O Error
//
// MessageId: CTS_RMSTR_REGENERATE_ABORTED_IOERROR
//
// MessageText:
//
//  Regenerate on array %1 was aborted due to I/O error.
//
#define CTS_RMSTR_REGENERATE_ABORTED_IOERROR 0x4005002AL

//  Hot copy started
//
// MessageId: CTS_RMSTR_HOTCOPY_STARTED
//
// MessageText:
//
//  Hot copy started on array %1.
//
#define CTS_RMSTR_HOTCOPY_STARTED        0x0005002BL

//  Hotcopy Priority Changed High
//
// MessageId: CTS_RMSTR_HOTCOPY_PRIORITY_CHANGED_HIGH
//
// MessageText:
//
//  Hotcopy running on array %1 has its priority changed to High.
//
#define CTS_RMSTR_HOTCOPY_PRIORITY_CHANGED_HIGH 0x0005002CL

//  Hotcopy Priority Changed Low
//
// MessageId: CTS_RMSTR_HOTCOPY_PRIORITY_CHANGED_LOW
//
// MessageText:
//
//  Hotcopy running on array %1 has its priority changed to Low.
//
#define CTS_RMSTR_HOTCOPY_PRIORITY_CHANGED_LOW 0x0005002DL

//  Hotcopy Priority Changed Medium
//
// MessageId: CTS_RMSTR_HOTCOPY_PRIORITY_CHANGED_MEDIUM
//
// MessageText:
//
//  Hotcopy running on array %1 has its priority changed to Medium.
//
#define CTS_RMSTR_HOTCOPY_PRIORITY_CHANGED_MEDIUM 0x0005002EL

//  Hot copy completed
//
// MessageId: CTS_RMSTR_HOTCOPY_COMPLETED
//
// MessageText:
//
//  Hot copy completed on array %1.
//
#define CTS_RMSTR_HOTCOPY_COMPLETED      0x0005002FL

//  Hot copy aborted by user
//
// MessageId: CTS_RMSTR_HOTCOPY_ABORTED_BY_USER
//
// MessageText:
//
//  Hot copy on array %1 was aborted by user.
//
#define CTS_RMSTR_HOTCOPY_ABORTED_BY_USER 0x00050030L

//  Hot copy aborted due to I/O Error
//
// MessageId: CTS_RMSTR_HOTCOPY_ABORTED_IOERROR
//
// MessageText:
//
//  Hot copy on array %1 was aborted due to I/O error.
//
#define CTS_RMSTR_HOTCOPY_ABORTED_IOERROR 0x40050031L

//
//		MEMBER RELATED EVENTS
//
//  Member [name] downed by user
//
// MessageId: CTS_RMSTR_MEMBER_DOWN_BY_USER
//
// MessageText:
//
//  Member %2 of Array %1 was downed by user.
//
#define CTS_RMSTR_MEMBER_DOWN_BY_USER    0x00050032L

//  Member [disk] downed by user
//
// MessageId: CTS_RMSTR_MEMBER_DISK_DOWN_BY_USER
//
// MessageText:
//
//  Member [Slot=%2] of Array %1 was downed by user.
//
#define CTS_RMSTR_MEMBER_DISK_DOWN_BY_USER 0x00050033L

//  Member [name] on array [name] down due to IO error
//
// MessageId: CTS_RMSTR_MEMBER_DOWN_IOERROR
//
// MessageText:
//
//  Member %2 on array %1 is down due to I/O Error.
//
#define CTS_RMSTR_MEMBER_DOWN_IOERROR    0x40050034L

//  Member [disk] on array [name] down due to IO error
//
// MessageId: CTS_RMSTR_MEMBER_DISK_DOWN_IOERROR
//
// MessageText:
//
//  Member [Slot=%2] on array %1 is down due to I/O Error.
//
#define CTS_RMSTR_MEMBER_DISK_DOWN_IOERROR 0x40050035L

//  New Member [name] added to array [name]
//
// MessageId: CTS_RMSTR_MEMBER_ADDED
//
// MessageText:
//
//  New member %2 added to Array %1.
//
#define CTS_RMSTR_MEMBER_ADDED           0x00050036L

//  New Member [disk] added to array [name]
//
// MessageId: CTS_RMSTR_MEMBER_DISK_ADDED
//
// MessageText:
//
//  New member [Slot=%2] added to Array %1.
//
#define CTS_RMSTR_MEMBER_DISK_ADDED      0x00050037L

//  Member [name] Removed from array [name]
//
// MessageId: CTS_RMSTR_MEMBER_REMOVED
//
// MessageText:
//
//  Member %2 removed from Array %1.
//
#define CTS_RMSTR_MEMBER_REMOVED         0x00050038L

//  Member [disk] removed from array [name]
//
// MessageId: CTS_RMSTR_MEMBER_DISK_REMOVED
//
// MessageText:
//
//  Member [slot=%2] removed from Array %1.
//
#define CTS_RMSTR_MEMBER_DISK_REMOVED    0x00050039L

//  New source member for array [name] is [name]
//
// MessageId: CTS_RMSTR_SOURCE_MEMBER_CHANGED
//
// MessageText:
//
//  New source member for array %1 is %2
//
#define CTS_RMSTR_SOURCE_MEMBER_CHANGED  0x0005003AL

//  New preferred member for array [name] is [name]
//
// MessageId: CTS_RMSTR_PREFERRED_MEMBER_CHANGED
//
// MessageText:
//
//  New preferred member for array %1 is %2
//
#define CTS_RMSTR_PREFERRED_MEMBER_CHANGED 0x0005003BL

//  New source member for array [name] is [disk]
//
// MessageId: CTS_RMSTR_SOURCE_MEMBER_DISK_CHANGED
//
// MessageText:
//
//  New source member for array %1 is [Slot=%2]
//
#define CTS_RMSTR_SOURCE_MEMBER_DISK_CHANGED 0x0005003CL

//  New preferred member for array [name] is [disk]
//
// MessageId: CTS_RMSTR_PREFERRED_MEMBER_DISK_CHANGED
//
// MessageText:
//
//  New preferred member for array %1 is [Slot=%2]
//
#define CTS_RMSTR_PREFERRED_MEMBER_DISK_CHANGED 0x0005003DL

//  CTS_FACILITY_CMB:  CMB interface DDM codes
//
//  CTEvtCMB.mc - Error codes unique to the CMB interface DDM.
//                Included by CTEvent.mc.  See that file for more info.
//
//
//  the DDM received an invalid parameter in a request message
//
// MessageId: CTS_CMB_INVALID_PARAMETER
//
// MessageText:
//
//  Invalid parameter.
//
#define CTS_CMB_INVALID_PARAMETER        0x80060001L

//  here's a special case of invalid parameter:  the IOP slot we're
//  supposed to do something to doesn't have an IOP in it.
//
// MessageId: CTS_CMB_REQUESTED_IOP_SLOT_EMPTY
//
// MessageText:
//
//  Cannot control an empty IOP slot.
//
#define CTS_CMB_REQUESTED_IOP_SLOT_EMPTY 0x80060002L

//  CMB communication failure - couldn't ask it to do something
//
// MessageId: CTS_CMB_CMA_REQ_FAILED
//
// MessageText:
//
//  Request to CMB microcontroller failed.
//
#define CTS_CMB_CMA_REQ_FAILED           0x80060003L

//  the IOP we're trying to deal with has "unknown" state: it might
//  not be there at all, or it might be busted.
//
// MessageId: CTS_CMB_REQUESTED_IOP_UNRESPONSIVE
//
// MessageText:
//
//  IOP slot is not responding, and may be empty.
//
#define CTS_CMB_REQUESTED_IOP_UNRESPONSIVE 0x80060004L

//  tried to send a message to an IOP's MIPS CPU, but the MIPS
//  is presently powered down.
//
// MessageId: CTS_CMB_REQUESTED_IOP_POWERED_DOWN
//
// MessageText:
//
//  IOP is presently powered down.
//
#define CTS_CMB_REQUESTED_IOP_POWERED_DOWN 0x80060005L

//  IOP's MIPS CPU is not ready to receive unsolicited CMB requests
//
// MessageId: CTS_CMB_IOP_MIPS_NOT_READY
//
// MessageText:
//
//  IOP's MIPS CPU is not ready to receive unsolicited CMB requests.
//
#define CTS_CMB_IOP_MIPS_NOT_READY       0x80060006L

//  CMA didn't recognize command in request packet
//
// MessageId: CTS_CMB_CMA_UNKNOWN_CMD
//
// MessageText:
//
//  CMB request not understood by destination microcontroller.
//
#define CTS_CMB_CMA_UNKNOWN_CMD          0x80060007L

//  CMA reports bad parameter in request packet
//
// MessageId: CTS_CMB_CMA_BAD_PARAM
//
// MessageText:
//
//  CMB microcontroller reports bad parameter in request packet.
//
#define CTS_CMB_CMA_BAD_PARAM            0x80060008L

//  CMA failed to forward request on to its local MIPS CPU - timeout
//
// MessageId: CTS_CMB_CMA_MIPS_TIMEOUT
//
// MessageText:
//
//  CMB microcontroller reports target MIPS CPU unreachable (timeout).
//
#define CTS_CMB_CMA_MIPS_TIMEOUT         0x80060009L

//  Unrecognized NAK reason code returned in CMB response packet
//
// MessageId: CTS_CMB_CMA_UNKNOWN_NAK
//
// MessageText:
//
//  CMB microcontroller returned an unknown NAK reason code.
//
#define CTS_CMB_CMA_UNKNOWN_NAK          0x8006000AL

//  CMB transmit hardware interface is busy (try again later)
//
// MessageId: CTS_CMB_XMTR_BUSY
//
// MessageText:
//
//  CMB microcontroller transmit interface is busy.
//
#define CTS_CMB_XMTR_BUSY                0x8006000BL

//  CMB hw interface class has no unsolicited messages to return
//
// MessageId: CTS_CMB_NO_UNSOLICITED_MSG
//
// MessageText:
//
//  No unsolicited messages pending from CMB microcontroller.
//
#define CTS_CMB_NO_UNSOLICITED_MSG       0x8006000CL

//  Supplied buffer is too small for given CMB message
//
// MessageId: CTS_CMB_BUFFER_TOO_SMALL
//
// MessageText:
//
//  Supplied buffer is too small for given CMB message.
//
#define CTS_CMB_BUFFER_TOO_SMALL         0x8006000DL

//  CMB transmit hardware interface failed to send
//
// MessageId: CTS_CMB_XMTR_ERROR
//
// MessageText:
//
//  CMB microcontroller transmit error.
//
#define CTS_CMB_XMTR_ERROR               0x8006000EL

//  tried to unlock an IOP's solenoid, to enable IOP removal,
//  but IOP is presently flagged as running OS-level code.
//
// MessageId: CTS_CMB_REQUESTED_IOP_IN_USE
//
// MessageText:
//
//  IOP is presently running OS-level code.
//
#define CTS_CMB_REQUESTED_IOP_IN_USE     0x8006000FL

//  wanted to talk to a DDH which appears to not be present in system
//
// MessageId: CTS_CMB_REQUESTED_DDH_NOT_PRESENT
//
// MessageText:
//
//  Requested drive bay's DDH is not present in the system.
//
#define CTS_CMB_REQUESTED_DDH_NOT_PRESENT 0x80060010L

//  wanted to talk to a DDH which appears to be unusable (not responding, etc.)
//
// MessageId: CTS_CMB_REQUESTED_DDH_NOT_USABLE
//
// MessageText:
//
//  Requested drive bay's DDH is not usable at this time.
//
#define CTS_CMB_REQUESTED_DDH_NOT_USABLE 0x80060011L

//  PTS insert op failed in strange and wondrous way
//
// MessageId: CTS_CMB_PTS_INSERT_FAILED
//
// MessageText:
//
//  PTS insert attempt failed with no error return.
//
#define CTS_CMB_PTS_INSERT_FAILED        0x80060012L

//  CMA reports it's busy awaiting response to unsolicited command
//
// MessageId: CTS_CMB_CMA_BUSY_UNSOL
//
// MessageText:
//
//  CMB microcontroller refused send; is awaiting reply to earlier command.
//
#define CTS_CMB_CMA_BUSY_UNSOL           0x80060013L

//  here's a special case of invalid parameter:  the DDH slot we're
//  supposed to do something to doesn't have an DDH in it.
//
// MessageId: CTS_CMB_REQUESTED_DDH_SLOT_EMPTY
//
// MessageText:
//
//  Cannot control an empty DDH slot.
//
#define CTS_CMB_REQUESTED_DDH_SLOT_EMPTY 0x80060014L

//  This one happens when a message gets lost somewhere between the
//  CMB DDM and its local CMA.  This is a "should never happen" error.
//
// MessageId: CTS_CMB_LOCAL_AVR_TIMEOUT
//
// MessageText:
//
//  Communication with local CMB microcontroller timed out.
//
#define CTS_CMB_LOCAL_AVR_TIMEOUT        0x80060015L

//  CTS_FACILITY_SSAPI:  SSAPI localized string and event codes
//
//  CTEvtSSAPI.mc - codes for events and localized strings
//                  Included by CTEvent.mc.  See that file for more info.
//
//
//  The name of the Fan object 
//
// MessageId: CTS_SSAPI_FAN_NAME
//
// MessageText:
//
//  Fan
//
#define CTS_SSAPI_FAN_NAME               0x00070001L

//  The name of the Chassis Power Supply object 
//
// MessageId: CTS_SSAPI_CHASSIS_PS_NAME
//
// MessageText:
//
//  Chassis Power Supply
//
#define CTS_SSAPI_CHASSIS_PS_NAME        0x00070002L

//  The name of the Disk Power Supply object 
//
// MessageId: CTS_SSAPI_DISK_PS_NAME
//
// MessageText:
//
//  Disk Power Supply
//
#define CTS_SSAPI_DISK_PS_NAME           0x00070003L

//  The name of a Battery device
//
// MessageId: CTS_SSAPI_BATTERY_NAME
//
// MessageText:
//
//  Battery
//
#define CTS_SSAPI_BATTERY_NAME           0x00070004L

//  The name of a Bus segment
//
// MessageId: CTS_SSAPI_BUS_SEGMENT_0_NAME
//
// MessageText:
//
//  PCI Bus Segment 'A'
//
#define CTS_SSAPI_BUS_SEGMENT_0_NAME     0x00070005L

//  The name of a Bus segment
//
// MessageId: CTS_SSAPI_BUS_SEGMENT_1_NAME
//
// MessageText:
//
//  PCI Bus Segment 'B'
//
#define CTS_SSAPI_BUS_SEGMENT_1_NAME     0x00070006L

//  The name of a Bus segment
//
// MessageId: CTS_SSAPI_BUS_SEGMENT_2_NAME
//
// MessageText:
//
//  PCI Bus Segment 'C'
//
#define CTS_SSAPI_BUS_SEGMENT_2_NAME     0x00070007L

//  The name of a Bus segment
//
// MessageId: CTS_SSAPI_BUS_SEGMENT_3_NAME
//
// MessageText:
//
//  PCI Bus Segment 'D'
//
#define CTS_SSAPI_BUS_SEGMENT_3_NAME     0x00070008L

//  The name of an IOP slot
//
// MessageId: CTS_SSAPI_SLOT_NAME_A1
//
// MessageText:
//
//  A1
//
#define CTS_SSAPI_SLOT_NAME_A1           0x00070009L

//  The name of an IOP slot
//
// MessageId: CTS_SSAPI_SLOT_NAME_A2
//
// MessageText:
//
//  A2
//
#define CTS_SSAPI_SLOT_NAME_A2           0x0007000AL

//  The name of an IOP slot
//
// MessageId: CTS_SSAPI_SLOT_NAME_A3
//
// MessageText:
//
//  A3
//
#define CTS_SSAPI_SLOT_NAME_A3           0x0007000BL

//  The name of an IOP slot
//
// MessageId: CTS_SSAPI_SLOT_NAME_A4
//
// MessageText:
//
//  A4
//
#define CTS_SSAPI_SLOT_NAME_A4           0x0007000CL

//  The name of an IOP slot
//
// MessageId: CTS_SSAPI_SLOT_NAME_B1
//
// MessageText:
//
//  B1
//
#define CTS_SSAPI_SLOT_NAME_B1           0x0007000DL

//  The name of an IOP slot
//
// MessageId: CTS_SSAPI_SLOT_NAME_B2
//
// MessageText:
//
//  B2
//
#define CTS_SSAPI_SLOT_NAME_B2           0x0007000EL

//  The name of an IOP slot
//
// MessageId: CTS_SSAPI_SLOT_NAME_B3
//
// MessageText:
//
//  B3
//
#define CTS_SSAPI_SLOT_NAME_B3           0x0007000FL

//  The name of an IOP slot
//
// MessageId: CTS_SSAPI_SLOT_NAME_B4
//
// MessageText:
//
//  B4
//
#define CTS_SSAPI_SLOT_NAME_B4           0x00070010L

//  The name of an IOP slot
//
// MessageId: CTS_SSAPI_SLOT_NAME_C1
//
// MessageText:
//
//  C1
//
#define CTS_SSAPI_SLOT_NAME_C1           0x00070011L

//  The name of an IOP slot
//
// MessageId: CTS_SSAPI_SLOT_NAME_C2
//
// MessageText:
//
//  C2
//
#define CTS_SSAPI_SLOT_NAME_C2           0x00070012L

//  The name of an IOP slot
//
// MessageId: CTS_SSAPI_SLOT_NAME_C3
//
// MessageText:
//
//  C3
//
#define CTS_SSAPI_SLOT_NAME_C3           0x00070013L

//  The name of an IOP slot
//
// MessageId: CTS_SSAPI_SLOT_NAME_C4
//
// MessageText:
//
//  C4
//
#define CTS_SSAPI_SLOT_NAME_C4           0x00070014L

//  The name of an IOP slot
//
// MessageId: CTS_SSAPI_SLOT_NAME_D1
//
// MessageText:
//
//  D1
//
#define CTS_SSAPI_SLOT_NAME_D1           0x00070015L

//  The name of an IOP slot
//
// MessageId: CTS_SSAPI_SLOT_NAME_D2
//
// MessageText:
//
//  D2
//
#define CTS_SSAPI_SLOT_NAME_D2           0x00070016L

//  The name of an IOP slot
//
// MessageId: CTS_SSAPI_SLOT_NAME_D3
//
// MessageText:
//
//  D3
//
#define CTS_SSAPI_SLOT_NAME_D3           0x00070017L

//  The name of an IOP slot
//
// MessageId: CTS_SSAPI_SLOT_NAME_D4
//
// MessageText:
//
//  D4
//
#define CTS_SSAPI_SLOT_NAME_D4           0x00070018L

//  The name of an IOP slot
//
// MessageId: CTS_SSAPI_CHASSIS_NAME
//
// MessageText:
//
//  Chassis
//
#define CTS_SSAPI_CHASSIS_NAME           0x00070019L

//  The name of an HBC slot
//
// MessageId: CTS_SSAPI_HBC_SLOT_NAME_00
//
// MessageText:
//
//  0
//
#define CTS_SSAPI_HBC_SLOT_NAME_00       0x0007001AL

//  The name of an HBC slot
//
// MessageId: CTS_SSAPI_HBC_SLOT_NAME_01
//
// MessageText:
//
//  1
//
#define CTS_SSAPI_HBC_SLOT_NAME_01       0x0007001BL

//  The name of a Storage Element
//
// MessageId: CTS_SSAPI_RAID1_ARRAY_NAME
//
// MessageText:
//
//  RAID1 Array
//
#define CTS_SSAPI_RAID1_ARRAY_NAME       0x0007001CL

//  The name of a Storage Element
//
// MessageId: CTS_SSAPI_RAID0_ARRAY_NAME
//
// MessageText:
//
//  RAID0 Array
//
#define CTS_SSAPI_RAID0_ARRAY_NAME       0x0007001DL

//  The name of a Storage Element
//
// MessageId: CTS_SSAPI_DISK_NAME
//
// MessageText:
//
//  Internal Disk
//
#define CTS_SSAPI_DISK_NAME              0x0007001EL

//  The name of a Storage Element
//
// MessageId: CTS_SSAPI_SPARE_POOL_NAME
//
// MessageText:
//
//  Spare Pool
//
#define CTS_SSAPI_SPARE_POOL_NAME        0x0007001FL

//  The name of a Process
//
// MessageId: CTS_SSAPI_RAID_VERIFY_NAME
//
// MessageText:
//
//  Verify
//
#define CTS_SSAPI_RAID_VERIFY_NAME       0x00070020L

//  The name of a Process
//
// MessageId: CTS_SSAPI_RAID_INITIALIZE_NAME
//
// MessageText:
//
//  Zero-Initialize
//
#define CTS_SSAPI_RAID_INITIALIZE_NAME   0x00070021L

//  The name of a Process
//
// MessageId: CTS_SSAPI_RAID_REGENERATE_NAME
//
// MessageText:
//
//  Regenerate
//
#define CTS_SSAPI_RAID_REGENERATE_NAME   0x00070022L

//  The name of a Process
//
// MessageId: CTS_SSAPI_RAID_HOT_COPY_NAME
//
// MessageText:
//
//  Server Hot Copy
//
#define CTS_SSAPI_RAID_HOT_COPY_NAME     0x00070023L

//  The name of a Process
//
// MessageId: CTS_SSAPI_RAID_SMART_COPY_NAME
//
// MessageText:
//
//  Member Hot Copy
//
#define CTS_SSAPI_RAID_SMART_COPY_NAME   0x00070024L

//  The name of a Filler Board
//
// MessageId: CTS_SSAPI_FILLER_BOARD_NAME
//
// MessageText:
//
//  Filler Board
//
#define CTS_SSAPI_FILLER_BOARD_NAME      0x00070025L

//  The name of an IOP
//
// MessageId: CTS_SSAPI_HBC_NAME
//
// MessageText:
//
//  HBC
//
#define CTS_SSAPI_HBC_NAME               0x00070026L

//  The name of an IOP
//
// MessageId: CTS_SSAPI_SSD_NAME
//
// MessageText:
//
//  SSD
//
#define CTS_SSAPI_SSD_NAME               0x00070027L

//  The name of an IOP
//
// MessageId: CTS_SSAPI_NAC_NAME
//
// MessageText:
//
//  NAC
//
#define CTS_SSAPI_NAC_NAME               0x00070028L

//  The name of the HDD device
//
// MessageId: CTS_SSAPI_HDD_DEVICE_NAME
//
// MessageText:
//
//  Fibre Channel Disk
//
#define CTS_SSAPI_HDD_DEVICE_NAME        0x00070029L

//  The name of the FC Port device
//
// MessageId: CTS_SSAPI_FC_PORT_DEVICE_NAME
//
// MessageText:
//
//  Fibre Channel Port
//
#define CTS_SSAPI_FC_PORT_DEVICE_NAME    0x0007002AL

//  The name of the EVC Device Collection
//
// MessageId: CTS_SSAPI_EVC_DEVICE_COLLECTION_NAME
//
// MessageText:
//
//  Environmental Controls
//
#define CTS_SSAPI_EVC_DEVICE_COLLECTION_NAME 0x0007002BL

//  The name of the FC Device Collection
//
// MessageId: CTS_SSAPI_FC_DEVICE_COLLECTION_NAME
//
// MessageText:
//
//  Fibre Channel Subsytem
//
#define CTS_SSAPI_FC_DEVICE_COLLECTION_NAME 0x0007002CL

//  The name of the PCI Device Collection
//
// MessageId: CTS_SSAPI_PCI_DEVICE_COLLECTION_NAME
//
// MessageText:
//
//  PCI Resources
//
#define CTS_SSAPI_PCI_DEVICE_COLLECTION_NAME 0x0007002DL

//  The name of the HBA object
//
// MessageId: CTS_SSAPI_HBA_NAME
//
// MessageText:
//
//  Host Bus Adapter
//
#define CTS_SSAPI_HBA_NAME               0x0007002EL

//  An exception string
//
// MessageId: CTS_SSAPI_CONFIG_ID_INVALID
//
// MessageText:
//
//  The server configuration has changed. Please verify that the request is valid and re-submit it
//
#define CTS_SSAPI_CONFIG_ID_INVALID      0x0007002FL

//  An exception string
//
// MessageId: CTS_SSAPI_INTERNAL_EXCEPTION_NOT_SUPPORTED
//
// MessageText:
//
//  The command issued to the server is not supported.
//
#define CTS_SSAPI_INTERNAL_EXCEPTION_NOT_SUPPORTED 0x00070030L

//  An exception string
//
// MessageId: CTS_SSAPI_INTERNAL_EXCEPTION_COULD_NOT_ADD_LISTENER
//
// MessageText:
//
//  The AddListener() operation failed.
//
#define CTS_SSAPI_INTERNAL_EXCEPTION_COULD_NOT_ADD_LISTENER 0x00070031L

//  An exception string
//
// MessageId: CTS_SSAPI_INTERNAL_EXCEPTION_COULD_NOT_DELETE_LISTENER
//
// MessageText:
//
//  The DeleteListener() operation failed.
//
#define CTS_SSAPI_INTERNAL_EXCEPTION_COULD_NOT_DELETE_LISTENER 0x00070032L

//  An exception string
//
// MessageId: CTS_SSAPI_INTERNAL_EXCEPTION_COULD_NOT_DELETE_HOST
//
// MessageText:
//
//  The DeleteHost() operation failed.
//
#define CTS_SSAPI_INTERNAL_EXCEPTION_COULD_NOT_DELETE_HOST 0x00070033L

//  An exception string
//
// MessageId: CTS_SSAPI_INTERNAL_EXCEPTION_COULD_NOT_ADD_HOST
//
// MessageText:
//
//  The AddHost() operation failed.
//
#define CTS_SSAPI_INTERNAL_EXCEPTION_COULD_NOT_ADD_HOST 0x00070034L

//  An exception string
//
// MessageId: CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED
//
// MessageText:
//
//  Internal Error: Command failed.
//
#define CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED 0x00070035L

//  An exception string
//
// MessageId: CTS_SSAPI_USERNAME_PASSWORD_INVALID
//
// MessageText:
//
//  Login failed. Invlaid username/password.
//
#define CTS_SSAPI_USERNAME_PASSWORD_INVALID 0x00070036L

//  An exception string
//
// MessageId: CTS_SSAPI_USER_WAS_NOT_LOGGED_IN
//
// MessageText:
//
//  Logout failed. The user was not logged in.
//
#define CTS_SSAPI_USER_WAS_NOT_LOGGED_IN 0x00070037L

//  An exception string
//
// MessageId: CTS_SSAPI_INVALID_PARM_EXCEPTION_INVALID_WAY_TO_CHANGE_PASSWORD
//
// MessageText:
//
//  Operation failed. Invalid password change.
//
#define CTS_SSAPI_INVALID_PARM_EXCEPTION_INVALID_WAY_TO_CHANGE_PASSWORD 0x00070038L

//  An exception string
//
// MessageId: CTS_SSAPI_SECURITY_INVALID_PASSWORD
//
// MessageText:
//
//  Old password is invalid.
//
#define CTS_SSAPI_SECURITY_INVALID_PASSWORD 0x00070039L

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_NOT_LOGGED_IN
//
// MessageText:
//
//  User not logged in.
//
#define CTS_SSAPI_EXCEPTION_NOT_LOGGED_IN 0x0007003AL

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_TOO_MANY_TRAP_ADDRESSES
//
// MessageText:
//
//  Too many SNMP trap addresses specified.
//
#define CTS_SSAPI_EXCEPTION_TOO_MANY_TRAP_ADDRESSES 0x0007003BL

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_CONNECTION_ID_INALID
//
// MessageText:
//
//  The connection object ID is invalid.
//
#define CTS_SSAPI_EXCEPTION_CONNECTION_ID_INALID 0x0007003CL

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_PATH_INALID
//
// MessageText:
//
//  The path object is invalid.
//
#define CTS_SSAPI_EXCEPTION_PATH_INALID  0x0007003DL

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_OS_TOO_LONG
//
// MessageText:
//
//  The OS name is too long.
//
#define CTS_SSAPI_EXCEPTION_OS_TOO_LONG  0x0007003EL

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_LOCATION_TOO_LONG
//
// MessageText:
//
//  The location name is too long.
//
#define CTS_SSAPI_EXCEPTION_LOCATION_TOO_LONG 0x0007003FL

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_HOST_NAME_TOO_LONG
//
// MessageText:
//
//  The host name is too long.
//
#define CTS_SSAPI_EXCEPTION_HOST_NAME_TOO_LONG 0x00070040L

//  An exception string
//
// MessageId: CTS_SSAPI_INVALIDPARM_EXCEPTION_UTIL_NOT_RUNNING
//
// MessageText:
//
//  The process is not running.
//
#define CTS_SSAPI_INVALIDPARM_EXCEPTION_UTIL_NOT_RUNNING 0x00070041L

//  An exception string
//
// MessageId: CTS_SSAPI_INVALIDPARM_EXCEPTION_WRONG_PRIORITY
//
// MessageText:
//
//  The process priority is invalid.
//
#define CTS_SSAPI_INVALIDPARM_EXCEPTION_WRONG_PRIORITY 0x00070042L

//  An exception string
//
// MessageId: CTS_SSAPI_INVALIDPARM_EXCEPTION_NO_PRIORITY
//
// MessageText:
//
//  The new priority is not specified.
//
#define CTS_SSAPI_INVALIDPARM_EXCEPTION_NO_PRIORITY 0x00070043L

//  An exception string
//
// MessageId: CTS_SSAPI_INVALIDPARM_EXCEPTION_CANT_START_PROCESS
//
// MessageText:
//
//  This process can not be started.
//
#define CTS_SSAPI_INVALIDPARM_EXCEPTION_CANT_START_PROCESS 0x00070044L

//  An exception string
//
// MessageId: CTS_SSAPI_INVALIDPARM_EXCEPTION_UTIL_NOT_SUPPORTED
//
// MessageText:
//
//  The process is not supported.
//
#define CTS_SSAPI_INVALIDPARM_EXCEPTION_UTIL_NOT_SUPPORTED 0x00070045L

//  An exception string
//
// MessageId: CTS_SSAPI_INVALIDPARM_EXCEPTION_UTIL_ALREADY_RUNNING
//
// MessageText:
//
//  A similar process is already running.
//
#define CTS_SSAPI_INVALIDPARM_EXCEPTION_UTIL_ALREADY_RUNNING 0x00070046L

//  An exception string
//
// MessageId: CTS_SSAPI_INVALIDPARM_EXCEPTION_ARRAY_OFFLINE
//
// MessageText:
//
//  The process can not be started because the array is off-line.
//
#define CTS_SSAPI_INVALIDPARM_EXCEPTION_ARRAY_OFFLINE 0x00070047L

//  An exception string
//
// MessageId: CTS_SSAPI_INVALIDPARM_EXCEPTION_ARRAY_NOT_CRITICAL
//
// MessageText:
//
//  The process can not be started because the array state is not critical.
//
#define CTS_SSAPI_INVALIDPARM_EXCEPTION_ARRAY_NOT_CRITICAL 0x00070048L

//  An exception string
//
// MessageId: CTS_SSAPI_INVALIDPARM_EXCEPTION_ARRAY_CRITICAL
//
// MessageText:
//
//  The process can not be started because the array state is critical.
//
#define CTS_SSAPI_INVALIDPARM_EXCEPTION_ARRAY_CRITICAL 0x00070049L

//  An exception string
//
// MessageId: CTS_SSAPI_INVALIDPARM_EXCEPTION_NUM_OF_IOPS_PORTS_NOT_SAME
//
// MessageText:
//
//  The number of IOPs and Ports must be equal.
//
#define CTS_SSAPI_INVALIDPARM_EXCEPTION_NUM_OF_IOPS_PORTS_NOT_SAME 0x0007004AL

//  An exception string
//
// MessageId: CTS_SSAPI_INVALID_PARAM_EXCEPTION
//
// MessageText:
//
//  Invalid parameter(s).
//
#define CTS_SSAPI_INVALID_PARAM_EXCEPTION 0x0007004BL

//  An exception string
//
// MessageId: CTS_SSAPI_INVALID_PARAM_EXCEPTION_NO_CONNECTION_NAME
//
// MessageText:
//
//  The name was not specified
//
#define CTS_SSAPI_INVALID_PARAM_EXCEPTION_NO_CONNECTION_NAME 0x0007004CL

//  An exception string
//
// MessageId: CTS_SSAPI_INVALID_PARAM_EXCEPTION_STORAGE_IN_USE
//
// MessageText:
//
//  The storage element specified is already in use.
//
#define CTS_SSAPI_INVALID_PARAM_EXCEPTION_STORAGE_IN_USE 0x0007004DL

//  An exception string
//
// MessageId: CTS_SSAPI_INVALID_PARAM_EXCEPTION_LUN_ID_IN_USE
//
// MessageText:
//
//  The LUN and target ID are already in use.
//
#define CTS_SSAPI_INVALID_PARAM_EXCEPTION_LUN_ID_IN_USE 0x0007004EL

//  An exception string
//
// MessageId: CTS_SSAPI_INVALID_PARAM_EXCEPTION_NOT_ALLOWED
//
// MessageText:
//
//  No modifiable members were submitted.
//
#define CTS_SSAPI_INVALID_PARAM_EXCEPTION_NOT_ALLOWED 0x0007004FL

//  An exception string
//
// MessageId: CTS_SSAPI_OBJECT_EXISTS_EXCEPTION
//
// MessageText:
//
//  Could not complete the operation. The object already exists.
//
#define CTS_SSAPI_OBJECT_EXISTS_EXCEPTION 0x00070050L

//  An exception string
//
// MessageId: CTS_SSAPI_INALID_PARM_EXCEPTION_HOST_ID_INVALID
//
// MessageText:
//
//  Invalid Host ID.
//
#define CTS_SSAPI_INALID_PARM_EXCEPTION_HOST_ID_INVALID 0x00070051L

//  An exception string
//
// MessageId: CTS_SSAPI_OBJECT_DOES_NOT_EXIST_EXCEPTION
//
// MessageText:
//
//  Could not complete the operation. The object does not exist.
//
#define CTS_SSAPI_OBJECT_DOES_NOT_EXIST_EXCEPTION 0x00070052L

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_USERNAME_TOO_LONG
//
// MessageText:
//
//  The username is too long.
//
#define CTS_SSAPI_EXCEPTION_USERNAME_TOO_LONG 0x00070053L

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_PASSWORD_TOO_LONG
//
// MessageText:
//
//  The password is too long.
//
#define CTS_SSAPI_EXCEPTION_PASSWORD_TOO_LONG 0x00070054L

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_FIRST_NAME_TOO_LONG
//
// MessageText:
//
//  The first name is too long.
//
#define CTS_SSAPI_EXCEPTION_FIRST_NAME_TOO_LONG 0x00070055L

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_LAST_NAME_TOO_LONG
//
// MessageText:
//
//  The last name is too long.
//
#define CTS_SSAPI_EXCEPTION_LAST_NAME_TOO_LONG 0x00070056L

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_USER_DESCRIPTION_TOO_LONG
//
// MessageText:
//
//  The description is too long.
//
#define CTS_SSAPI_EXCEPTION_USER_DESCRIPTION_TOO_LONG 0x00070057L

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_EMAIL_TOO_LONG
//
// MessageText:
//
//  The email address is too long.
//
#define CTS_SSAPI_EXCEPTION_EMAIL_TOO_LONG 0x00070058L

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_NAME_TOO_LONG
//
// MessageText:
//
//  The name is too long.
//
#define CTS_SSAPI_EXCEPTION_NAME_TOO_LONG 0x00070059L

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_DESCRIPTION_TOO_LONG
//
// MessageText:
//
//  The description is too long
//
#define CTS_SSAPI_EXCEPTION_DESCRIPTION_TOO_LONG 0x0007005AL

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_PHONE1_TOO_LONG
//
// MessageText:
//
//  The Phone#1 entry is too long.
//
#define CTS_SSAPI_EXCEPTION_PHONE1_TOO_LONG 0x0007005BL

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_PHONE2_TOO_LONG
//
// MessageText:
//
//  The Phone#2 entry is too long.
//
#define CTS_SSAPI_EXCEPTION_PHONE2_TOO_LONG 0x0007005CL

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_DEPARTMENT_TOO_LONG
//
// MessageText:
//
//  The department description is too long.
//
#define CTS_SSAPI_EXCEPTION_DEPARTMENT_TOO_LONG 0x0007005DL

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_HOST_NOT_FOUND
//
// MessageText:
//
//  The specified host does not exist.
//
#define CTS_SSAPI_EXCEPTION_HOST_NOT_FOUND 0x0007005EL

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_DESIGNATOR_ID_MISSING
//
// MessageText:
//
//  The object ID is missing or invalid.
//
#define CTS_SSAPI_EXCEPTION_DESIGNATOR_ID_MISSING 0x0007005FL

//  An exception string
//
// MessageId: CTS_SSAPI_EXCEPTION_REQUEST_NOT_IMPLEMENTED
//
// MessageText:
//
//  Request not implemented.
//
#define CTS_SSAPI_EXCEPTION_REQUEST_NOT_IMPLEMENTED 0x00070060L

//  The name of a PHSData
//
// MessageId: CTS_SSAPI_PHS_DATA_NAME_CHASSIS_EXIT_TEMP1
//
// MessageText:
//
//  Temperature of air exiting the enclosure - Sensor 1
//
#define CTS_SSAPI_PHS_DATA_NAME_CHASSIS_EXIT_TEMP1 0x00070061L    

//  The name of a PHSData
//
// MessageId: CTS_SSAPI_PHS_DATA_NAME_CHASSIS_EXIT_TEMP2
//
// MessageText:
//
//  Temperature of air exiting the enclosure - Sensor 2
//
#define CTS_SSAPI_PHS_DATA_NAME_CHASSIS_EXIT_TEMP2 0x00070062L    

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DATA_NAME_CPU_TEMP
//
// MessageText:
//
//  Temperature near the CPU
//
#define CTS_SSAPI_PHS_DATA_NAME_CPU_TEMP 0x00070063L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DATA_MEDIA_ERRORS_NO_DELAY
//
// MessageText:
//
//  The number of recoverable disk media errors without a delay
//
#define CTS_SSAPI_PHS_DATA_MEDIA_ERRORS_NO_DELAY 0x00070064L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DATA_MEDIA_ERRORS_DELAY
//
// MessageText:
//
//  The number of recoverable disk media errors with a delay
//
#define CTS_SSAPI_PHS_DATA_MEDIA_ERRORS_DELAY 0x00070065L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DATA_MEDIA_ERRORS_W_RETRY
//
// MessageText:
//
//  The number of recoverable disk media errors with retry
//
#define CTS_SSAPI_PHS_DATA_MEDIA_ERRORS_W_RETRY 0x00070066L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DATA_MEDIA_ERRORS_W_ECC
//
// MessageText:
//
//  The number of recoverable disk media errors with ECC
//
#define CTS_SSAPI_PHS_DATA_MEDIA_ERRORS_W_ECC 0x00070067L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DATA_NON_MEDIA_ERRORS
//
// MessageText:
//
//  The number of non-disk media errors recoverable
//
#define CTS_SSAPI_PHS_DATA_NON_MEDIA_ERRORS 0x00070068L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DATA_BYTES_PROCESSED_TOTAL
//
// MessageText:
//
//  The total number of bytes processed
//
#define CTS_SSAPI_PHS_DATA_BYTES_PROCESSED_TOTAL 0x00070069L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_UNRECOVERABLE_MEDIA_ERRORS
//
// MessageText:
//
//  The number of unrecoverable disk media errors
//
#define CTS_SSAPI_PHS_UNRECOVERABLE_MEDIA_ERRORS 0x0007006AL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_UP_TIME
//
// MessageText:
//
//  Total time the disk has been spun up
//
#define CTS_SSAPI_PHS_DISK_UP_TIME       0x0007006BL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_AVG_READS_NUMBER
//
// MessageText:
//
//  The average number of reads per second
//
#define CTS_SSAPI_PHS_DISK_AVG_READS_NUMBER 0x0007006CL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_AVG_WRITES_NUMBER
//
// MessageText:
//
//  The average number of writes per second
//
#define CTS_SSAPI_PHS_DISK_AVG_WRITES_NUMBER 0x0007006DL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MAX_READS_NUMBER
//
// MessageText:
//
//  The maximum number of reads per second
//
#define CTS_SSAPI_PHS_DISK_MAX_READS_NUMBER 0x0007006EL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MAX_WRITES_NUMBER
//
// MessageText:
//
//  The maximum number of writes per second
//
#define CTS_SSAPI_PHS_DISK_MAX_WRITES_NUMBER 0x0007006FL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MIN_READS_NUMBER
//
// MessageText:
//
//  The minimum number of reads per second
//
#define CTS_SSAPI_PHS_DISK_MIN_READS_NUMBER 0x00070070L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MIN_WRITES_NUMBER
//
// MessageText:
//
//  The minimum number of writes per second
//
#define CTS_SSAPI_PHS_DISK_MIN_WRITES_NUMBER 0x00070071L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MIN_READ_TIME
//
// MessageText:
//
//  Minimum read latency
//
#define CTS_SSAPI_PHS_DISK_MIN_READ_TIME 0x00070072L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MIN_WRITE_TIME
//
// MessageText:
//
//  Minimum write latency
//
#define CTS_SSAPI_PHS_DISK_MIN_WRITE_TIME 0x00070073L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MAX_READ_TIME
//
// MessageText:
//
//  Maximum read latency
//
#define CTS_SSAPI_PHS_DISK_MAX_READ_TIME 0x00070074L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MAX_WRITE_TIME
//
// MessageText:
//
//  Maximum write latency
//
#define CTS_SSAPI_PHS_DISK_MAX_WRITE_TIME 0x00070075L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_TOTAL_READS_NUMBER
//
// MessageText:
//
//  Total number of reads
//
#define CTS_SSAPI_PHS_DISK_TOTAL_READS_NUMBER 0x00070076L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_TOTAL_BYTES_READ
//
// MessageText:
//
//  Total number of bytes read
//
#define CTS_SSAPI_PHS_DISK_TOTAL_BYTES_READ 0x00070077L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_AVERAGE_READ_SIZE
//
// MessageText:
//
//  The average read size
//
#define CTS_SSAPI_PHS_DISK_AVERAGE_READ_SIZE 0x00070078L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_TOTAL_WRITES_NUMBER
//
// MessageText:
//
//  Total number of writes
//
#define CTS_SSAPI_PHS_DISK_TOTAL_WRITES_NUMBER 0x00070079L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_TOTAL_BYTES_WRITTEN
//
// MessageText:
//
//  Total number of bytes written
//
#define CTS_SSAPI_PHS_DISK_TOTAL_BYTES_WRITTEN 0x0007007AL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_AVERAGE_WRITE_SIZE
//
// MessageText:
//
//  The average write size
//
#define CTS_SSAPI_PHS_DISK_AVERAGE_WRITE_SIZE 0x0007007BL

//  Object state name: Good
//
// MessageId: CTS_SSAPI_OBJECT_STATE_NAME_GOOD
//
// MessageText:
//
//  Good
//
#define CTS_SSAPI_OBJECT_STATE_NAME_GOOD 0x0007007CL

//  Object state name: Dead
//
// MessageId: CTS_SSAPI_OBJECT_STATE_NAME_DEAD
//
// MessageText:
//
//  Dead
//
#define CTS_SSAPI_OBJECT_STATE_NAME_DEAD 0x0007007DL

//  Object state name: Diagnosed
//
// MessageId: CTS_SSAPI_OBJECT_STATE_NAME_BEING_DIAGNOSTED
//
// MessageText:
//
//  Running diagnostics
//
#define CTS_SSAPI_OBJECT_STATE_NAME_BEING_DIAGNOSTED 0x0007007EL

//  Object state name: 
//
// MessageId: CTS_SSAPI_OBJECT_STATE_NAME_QUIESCED
//
// MessageText:
//
//  Quiesced
//
#define CTS_SSAPI_OBJECT_STATE_NAME_QUIESCED 0x0007007FL

//  Object state name: 
//
// MessageId: CTS_SSAPI_OBJECT_STATE_NAME_UNKNOWN
//
// MessageText:
//
//  Unknown
//
#define CTS_SSAPI_OBJECT_STATE_NAME_UNKNOWN 0x00070080L

//  Object state name: 
//
// MessageId: CTS_SSAPI_OBJECT_STATE_NAME_WARNING
//
// MessageText:
//
//  Warning
//
#define CTS_SSAPI_OBJECT_STATE_NAME_WARNING 0x00070081L

//  Host State state name
//
// MessageId: CTS_SSAPI_HOST_STATE_CONNECTION_MESSING
//
// MessageText:
//
//  One or more connections are missing.
//
#define CTS_SSAPI_HOST_STATE_CONNECTION_MESSING 0x00070082L

//  Chassis PowerSupply state name
//
// MessageId: CTS_SSAPI_PS_STATE_NAME_INPUT_BAD
//
// MessageText:
//
//  Input is not acceptable.
//
#define CTS_SSAPI_PS_STATE_NAME_INPUT_BAD 0x00070083L

//  Chassis PowerSupply state name
//
// MessageId: CTS_SSAPI_PS_STATE_NAME_OUTPUT_BAD
//
// MessageText:
//
//  Output is not acceptable.
//
#define CTS_SSAPI_PS_STATE_NAME_OUTPUT_BAD 0x00070084L

//  IOP state name
//
// MessageId: CTS_SSAPI_IOP_STATE_NAME_BEING_RESET
//
// MessageText:
//
//  Being reset
//
#define CTS_SSAPI_IOP_STATE_NAME_BEING_RESET 0x00070085L

//  IOP state name
//
// MessageId: CTS_SSAPI_IOP_STATE_NAME_BOOTING
//
// MessageText:
//
//  Booting
//
#define CTS_SSAPI_IOP_STATE_NAME_BOOTING 0x00070086L

//  IOP state name
//
// MessageId: CTS_SSAPI_IOP_STATE_NAME_NOT_RESPONDING
//
// MessageText:
//
//  Not responding
//
#define CTS_SSAPI_IOP_STATE_NAME_NOT_RESPONDING 0x00070087L

//  IOP state name
//
// MessageId: CTS_SSAPI_IOP_STATE_NAME_POWERED_DOWN
//
// MessageText:
//
//  Powered down
//
#define CTS_SSAPI_IOP_STATE_NAME_POWERED_DOWN 0x00070088L

//  IOP state name
//
// MessageId: CTS_SSAPI_IOP_STATE_NAME_POWERED_ON
//
// MessageText:
//
//  Powered ON
//
#define CTS_SSAPI_IOP_STATE_NAME_POWERED_ON 0x00070089L

//  IOP state name 
//
// MessageId: CTS_SSAPI_IOP_STATE_NAME_AWAITING_BOOT
//
// MessageText:
//
//  Awaiting boot
//
#define CTS_SSAPI_IOP_STATE_NAME_AWAITING_BOOT 0x0007008AL

//  RAID Master exception
//
// MessageId: CTS_SSAPI_INVALIDPARM_EXCEPTION_INSUFF_MEMBER_SIZE
//
// MessageText:
//
//  Insufficient member capacity.
//
#define CTS_SSAPI_INVALIDPARM_EXCEPTION_INSUFF_MEMBER_SIZE 0x0007008BL

//  RAID Master exception
//
// MessageId: CTS_SSAPI_INVALIDPARM_EXCEPTION_INSUFF_SPARE_SIZE
//
// MessageText:
//
//  Insufficient spare capacity.
//
#define CTS_SSAPI_INVALIDPARM_EXCEPTION_INSUFF_SPARE_SIZE 0x0007008CL

//  RAID Master exception
//
// MessageId: CTS_SSAPI_INVALIDPARM_EXCEPTION_STORAGE_ELEMENT_FAILED
//
// MessageText:
//
//  Storage element failed.
//
#define CTS_SSAPI_INVALIDPARM_EXCEPTION_STORAGE_ELEMENT_FAILED 0x0007008DL

//  RAID Master exception
//
// MessageId: CTS_SSAPI_INVALIDPARM_EXCEPTION_INV_CMD
//
// MessageText:
//
//  Invalid command.
//
#define CTS_SSAPI_INVALIDPARM_EXCEPTION_INV_CMD 0x0007008EL

//  RAID Master exception
//
// MessageId: CTS_SSAPI_INVALIDPARM_EXCEPTION_STORAGE_ELEMENT_IN_USE
//
// MessageText:
//
//  The storage element is already in use.
//
#define CTS_SSAPI_INVALIDPARM_EXCEPTION_STORAGE_ELEMENT_IN_USE 0x0007008FL

//  RAID Master exception
//
// MessageId: CTS_SSAPI_INVALIDPARM_EXCEPTION_NAME_EXISTS
//
// MessageText:
//
//  Duplicate name.
//
#define CTS_SSAPI_INVALIDPARM_EXCEPTION_NAME_EXISTS 0x00070090L

//  RAID Master exception
//
// MessageId: CTS_SSAPI_INVALIDPARM_PARTITION_EXISTS
//
// MessageText:
//
//  Array cannot be deleted. There are existing partitions.
//
#define CTS_SSAPI_INVALIDPARM_PARTITION_EXISTS 0x00070091L

//  RAID Master exception
//
// MessageId: CTS_SSAPI_INVALIDPARM_UTIL_RUNNING
//
// MessageText:
//
//  Array cannot be deleted. There are process running on it.
//
#define CTS_SSAPI_INVALIDPARM_UTIL_RUNNING 0x00070092L

//  RAID Master exception
//
// MessageId: CTS_SSAPI_INVALIDPARM_ARRAY_ID_INVALID
//
// MessageText:
//
//  Array ID is invalid.
//
#define CTS_SSAPI_INVALIDPARM_ARRAY_ID_INVALID 0x00070093L

//  RAID Master exception
//
// MessageId: CTS_SSAPI_INVALIDPARM_MAX_SPARES_REACHED
//
// MessageText:
//
//  Maximum number of spares has been reached.
//
#define CTS_SSAPI_INVALIDPARM_MAX_SPARES_REACHED 0x00070094L

//  RAID Master exception
//
// MessageId: CTS_SSAPI_INVALIDPARM_SPARE_DOES_NOT_EXIST
//
// MessageText:
//
//  The spare does not exist.
//
#define CTS_SSAPI_INVALIDPARM_SPARE_DOES_NOT_EXIST 0x00070095L

//  Power Supply state name
//
// MessageId: CTS_SSAPI_PS_STATE_NAME_OVERTEMP_ALERT
//
// MessageText:
//
//  OverTemp alert.
//
#define CTS_SSAPI_PS_STATE_NAME_OVERTEMP_ALERT 0x00070096L

//  PHS value name
//
// MessageId: CTS_SSAPI_PHS_NAME_SMP48VOLTAGE
//
// MessageText:
//
//  The combined 48V supply of the SMP
//
#define CTS_SSAPI_PHS_NAME_SMP48VOLTAGE  0x00070097L

//  PHS value name
//
// MessageId: CTS_SSAPI_PHS_NAME_CURRENT33
//
// MessageText:
//
//  The cuurent output of the 3.3V converter.
//
#define CTS_SSAPI_PHS_NAME_CURRENT33     0x00070098L

//  PHS value name
//
// MessageId: CTS_SSAPI_PHS_NAME_CURRENT5
//
// MessageText:
//
//  The cuurent output of the 5V converter.
//
#define CTS_SSAPI_PHS_NAME_CURRENT5      0x00070099L

//  PHS value name
//
// MessageId: CTS_SSAPI_PHS_NAME_CURRENT12A
//
// MessageText:
//
//  The cuurent output of the 12V 'A' converter.
//
#define CTS_SSAPI_PHS_NAME_CURRENT12A    0x0007009AL

//  PHS value name
//
// MessageId: CTS_SSAPI_PHS_NAME_CURRENT12B
//
// MessageText:
//
//  The cuurent output of the 12V 'B' converter.
//
#define CTS_SSAPI_PHS_NAME_CURRENT12B    0x0007009BL

//  PHS value name
//
// MessageId: CTS_SSAPI_PHS_NAME_CURRENT12C
//
// MessageText:
//
//  The cuurent output of the 12V 'C' converter.
//
#define CTS_SSAPI_PHS_NAME_CURRENT12C    0x0007009CL

//  PHS value name
//
// MessageId: CTS_SSAPI_PHS_NAME_TEMP33
//
// MessageText:
//
//  The temperature of the 3.3V converter
//
#define CTS_SSAPI_PHS_NAME_TEMP33        0x0007009DL

//  PHS value name
//
// MessageId: CTS_SSAPI_PHS_NAME_TEMP5
//
// MessageText:
//
//  The temperature of the 5V converter
//
#define CTS_SSAPI_PHS_NAME_TEMP5         0x0007009EL

//  PHS value name
//
// MessageId: CTS_SSAPI_PHS_NAME_TEMP12A
//
// MessageText:
//
//  The temperature of the 12V 'A' converter
//
#define CTS_SSAPI_PHS_NAME_TEMP12A       0x0007009FL

//  PHS value name
//
// MessageId: CTS_SSAPI_PHS_NAME_TEMP12B
//
// MessageText:
//
//  The temperature of the 12V 'B' converter
//
#define CTS_SSAPI_PHS_NAME_TEMP12B       0x000700A0L

//  PHS value name
//
// MessageId: CTS_SSAPI_PHS_NAME_TEMP12C
//
// MessageText:
//
//  The temperature of the 12V 'C' converter
//
#define CTS_SSAPI_PHS_NAME_TEMP12C       0x000700A1L

//  PHS value name
//
// MessageId: CTS_SSAPI_PHS_NAME_VOLTAGE33
//
// MessageText:
//
//  The combined voltage of the 3.3V coverters
//
#define CTS_SSAPI_PHS_NAME_VOLTAGE33     0x000700A2L

//  PHS value name
//
// MessageId: CTS_SSAPI_PHS_NAME_VOLTAGE5
//
// MessageText:
//
//  The combined voltage of the 5V coverters
//
#define CTS_SSAPI_PHS_NAME_VOLTAGE5      0x000700A3L

//  PHS value name
//
// MessageId: CTS_SSAPI_PHS_NAME_VOLTAGE12
//
// MessageText:
//
//  The combined voltage of the 12V coverters
//
#define CTS_SSAPI_PHS_NAME_VOLTAGE12     0x000700A4L

//  PHS value name
//
// MessageId: CTS_SSAPI_PHS_NAME_BATTERY_TEMP
//
// MessageText:
//
//  The temperature inside the battery
//
#define CTS_SSAPI_PHS_NAME_BATTERY_TEMP  0x000700A5L

//  PHS value name
//
// MessageId: CTS_SSAPI_PHS_NAME_BATTERY_CURRENT
//
// MessageText:
//
//  The current of the battery.
//
#define CTS_SSAPI_PHS_NAME_BATTERY_CURRENT 0x000700A6L

//  exception name
//
// MessageId: CTS_SSAPI_EXCEPTION_MAX_ARRAY_MEMBERS_REACHED
//
// MessageText:
//
//  The maximum allowed number of members is reached.
//
#define CTS_SSAPI_EXCEPTION_MAX_ARRAY_MEMBERS_REACHED 0x000700A7L

//  exception name
//
// MessageId: CTS_SSAPI_EXCEPTION_MEMBER_ALREADY_DOWN
//
// MessageText:
//
//  The member is already DOWN.
//
#define CTS_SSAPI_EXCEPTION_MEMBER_ALREADY_DOWN 0x000700A8L

//  exception name
//
// MessageId: CTS_SSAPI_EXCEPTION_CANT_ABORT_UTILITY
//
// MessageText:
//
//  This process can not be aborted.
//
#define CTS_SSAPI_EXCEPTION_CANT_ABORT_UTILITY 0x000700A9L

//  exception name
//
// MessageId: CTS_SSAPI_EXCEPTION_ARRAY_OFFLINE
//
// MessageText:
//
//  The array state is OFF-LINE
//
#define CTS_SSAPI_EXCEPTION_ARRAY_OFFLINE 0x000700AAL

//  exception name
//
// MessageId: CTS_SSAPI_EXCEPTION_ARRAY_CRITICAL
//
// MessageText:
//
//  The array state is CRITICAL
//
#define CTS_SSAPI_EXCEPTION_ARRAY_CRITICAL 0x000700ABL

//  exception name
//
// MessageId: CTS_SSAPI_EXCEPTION_INVALID_RAID_LEVEL
//
// MessageText:
//
//  The RAID level is invalid.
//
#define CTS_SSAPI_EXCEPTION_INVALID_RAID_LEVEL 0x000700ACL

//  exception name
//
// MessageId: CTS_SSAPI_EXCEPTION_STORAGE_NAME_TOO_LONG
//
// MessageText:
//
//  The name for the storage element is too long.
//
#define CTS_SSAPI_EXCEPTION_STORAGE_NAME_TOO_LONG 0x000700ADL

//  exception name
//
// MessageId: CTS_SSAPI_EXCEPTION_NAME_ALREADY_EXISTS
//
// MessageText:
//
//  The name specified already exists.
//
#define CTS_SSAPI_EXCEPTION_NAME_ALREADY_EXISTS 0x000700AEL

//  Spare Pool Name Prefix
//
// MessageId: CTS_SSAPI_SPARE_POOL_NAME_PREFIX
//
// MessageText:
//
//  Spare Pool
//
#define CTS_SSAPI_SPARE_POOL_NAME_PREFIX 0x000700AFL

//RAID Pecking Order Enumeration
//
// MessageId: CTS_SSAPI_RAID_PECKING_ORDER_ENUMERATION
//
// MessageText:
//
//  First:1, Last:2, Never:3
//
#define CTS_SSAPI_RAID_PECKING_ORDER_ENUMERATION 0x000700B0L

//Process State Enumeration
//
// MessageId: CTS_SSAPI_PROCESS_STATE_ENUMERATION
//
// MessageText:
//
//  Running:1, Paused:2, Aborted by User:3, Aborted I/O Error:4, Complete:5
//
#define CTS_SSAPI_PROCESS_STATE_ENUMERATION 0x000700B1L

//Process Priority Enumeration
//
// MessageId: CTS_SSAPI_PROCESS_PRIORITY_ENUMERATION
//
// MessageText:
//
//  Low:3, Medium:6, High:9
//
#define CTS_SSAPI_PROCESS_PRIORITY_ENUMERATION 0x000700B2L

//Object State Enumeration
//
// MessageId: CTS_SSAPI_OBJECT_STATE_ENUMERATION
//
// MessageText:
//
//  Good:0, Warning:1, Degraded:2, Not Operational:3, Unknown:4
//
#define CTS_SSAPI_OBJECT_STATE_ENUMERATION 0x000700B3L

//Alarm Severity Enumeration
//
// MessageId: CTS_SSAPI_ALARM_SEVERITY_ENUMERATION
//
// MessageText:
//
//  Critical:0, Minor:1, Warning:2
//
#define CTS_SSAPI_ALARM_SEVERITY_ENUMERATION 0x000700B4L

//HostConnection Mode Enumeration
//
// MessageId: CTS_SSAPI_HOST_CONNECTION_MODE_ENUMERATION
//
// MessageText:
//
//  Non-Redundant:0, Active/Passive: 1
//
#define CTS_SSAPI_HOST_CONNECTION_MODE_ENUMERATION 0x000700B5L

//Host OS Enumeration
//
// MessageId: CTS_SSAPI_HOST_OS_ENUMERATION
//
// MessageText:
//
//  Other:0, Solaris 2.6:1, Windows NT 4.0:2, HP UX 10.20:3
//
#define CTS_SSAPI_HOST_OS_ENUMERATION    0x000700B6L

//LogMessage Severity Enumeration
//
// MessageId: CTS_SSAPI_LOG_MESSAGE_SEVERITY_ENUMERATION
//
// MessageText:
//
//  Warning:1, Internal:3, Informational:0, Error:2
//
#define CTS_SSAPI_LOG_MESSAGE_SEVERITY_ENUMERATION 0x000700B7L

//LogMessage Facility Enumeration
//
// MessageId: CTS_SSAPI_LOG_MESSAGE_FACILITY_ENUMERATION
//
// MessageText:
//
//  System:0, SSAPI:7, Runtime:3, RAID:5, OS:1, Messaging:2, CMB:6, BSA:4
//
#define CTS_SSAPI_LOG_MESSAGE_FACILITY_ENUMERATION 0x000700B8L

//Device Status Enumeration
//
// MessageId: CTS_SSAPI_DEVICE_STATUS_ENUMERATION
//
// MessageText:
//
//  Good:458876, Warning:458881, Quiesed:458879, Failed Over:458887, Unlocked:458885, Powered Down:458888, Out of Service:458885, Booting:458886, Post:458890, Dead:458877, Unknown:458880
//
#define CTS_SSAPI_DEVICE_STATUS_ENUMERATION 0x000700B9L

//LunMapManager Exception
//
// MessageId: CTS_SSAPI_NAME_IS_BLANK
//
// MessageText:
//
//  The name can not be blank
//
#define CTS_SSAPI_NAME_IS_BLANK          0x000700BAL

//LunMapManager Exception
//
// MessageId: CTS_SSAPI_DESCRIPTION_IS_BLANK
//
// MessageText:
//
//  The description can not be blank
//
#define CTS_SSAPI_DESCRIPTION_IS_BLANK   0x000700BBL

//HostManager Exception
//
// MessageId: CTS_SSAPI_CONNECTION_IS_VALID
//
// MessageText:
//
//  Can not remove the connection while it is in good state.
//
#define CTS_SSAPI_CONNECTION_IS_VALID    0x000700BCL

//LunManager Exception
//
// MessageId: CTS_SSAPI_LUN_ALREADY_EXPORTED
//
// MessageText:
//
//  Command failed. The entry is already exported.
//
#define CTS_SSAPI_LUN_ALREADY_EXPORTED   0x000700BDL

//LunManager Exception
//
// MessageId: CTS_SSAPI_LUN_ALREADY_UNEXPORTED
//
// MessageText:
//
//  Command failed. The entry is already unexported.
//
#define CTS_SSAPI_LUN_ALREADY_UNEXPORTED 0x000700BEL

//A name of the DDH device
//
// MessageId: CTS_SSAPI_DDH_DEVICE_NAME
//
// MessageText:
//
//  Disk Drive Hub
//
#define CTS_SSAPI_DDH_DEVICE_NAME        0x000700BFL

//A name of the SNAC device
//
// MessageId: CTS_SSAPI_SNAC_DEVICE_NAME
//
// MessageText:
//
//  NAC+
//
#define CTS_SSAPI_SNAC_DEVICE_NAME       0x000700C0L

//A name of the Internal FC Port device
//
// MessageId: CTS_SSAPI_INTERN_FCPORT_DEVICE_NAME
//
// MessageText:
//
//  Internal Fibre Channel Port
//
#define CTS_SSAPI_INTERN_FCPORT_DEVICE_NAME 0x000700C1L

//AlarmLogEntry Action Enumeration
//
// MessageId: CTS_SSAPI_ALARM_LOG_ENTRY_ACTION_ENUMERATION
//
// MessageText:
//
//  Created:1, Acknowledged:2, Unacknowledged:3, Cleared:4, Noted:5
//
#define CTS_SSAPI_ALARM_LOG_ENTRY_ACTION_ENUMERATION 0x000700C2L

//Host Status Enumeration
//
// MessageId: CTS_SSAPI_HOST_STATUS_ENUMERATION
//
// MessageText:
//
//  Good:0
//
#define CTS_SSAPI_HOST_STATUS_ENUMERATION 0x000700C3L

//DataPath Status Enumeration
//
// MessageId: CTS_SSAPI_HOST_CONNECTION_ELEMENT_STATUS_ENUMERATION
//
// MessageText:
//
//  Good:0
//
#define CTS_SSAPI_HOST_CONNECTION_ELEMENT_STATUS_ENUMERATION 0x000700C4L

//Connection Status Enumeration
//
// MessageId: CTS_SSAPI_CONNECTION_STATUS_ENUMERATION
//
// MessageText:
//
//  Good:0, Not Operational:1
//
#define CTS_SSAPI_CONNECTION_STATUS_ENUMERATION 0x000700C5L

//AVR name
//
// MessageId: CTS_SSAPI_AVR_NAME
//
// MessageText:
//
//  AVR
//
#define CTS_SSAPI_AVR_NAME               0x000700C6L

//AVR name
//
// MessageId: CTS_SSAPI_IOP_STATE_NAME_UNLOCKED
//
// MessageText:
//
//  Board is unlocked
//
#define CTS_SSAPI_IOP_STATE_NAME_UNLOCKED 0x000700C7L

//Exception name
//
// MessageId: CTS_SSAPI_EXCEPTION_DEVICE_NOT_LOCKED
//
// MessageText:
//
//  Command failed. The device is unlocked.
//
#define CTS_SSAPI_EXCEPTION_DEVICE_NOT_LOCKED 0x000700C8L

//Exception name
//
// MessageId: CTS_SSAPI_EXCEPTION_DEVICE_IN_SERVICE
//
// MessageText:
//
//  Command failed. The device is in service.
//
#define CTS_SSAPI_EXCEPTION_DEVICE_IN_SERVICE 0x000700C9L

//Exception name
//
// MessageId: CTS_SSAPI_EXCEPTION_DEVICE_POWERED_DOWN
//
// MessageText:
//
//  Command failed. The device is powered down.
//
#define CTS_SSAPI_EXCEPTION_DEVICE_POWERED_DOWN 0x000700CAL

//Exception name
//
// MessageId: CTS_SSAPI_EXCEPTION_DEVICE_POWERED_UP
//
// MessageText:
//
//  Command failed. The device is powered up.
//
#define CTS_SSAPI_EXCEPTION_DEVICE_POWERED_UP 0x000700CBL

//Exception name
//
// MessageId: CTS_SSAPI_EXCEPTION_ALARM_NOT_CLEARABLE
//
// MessageText:
//
//  Command failed. The alarm is not user-clearable.
//
#define CTS_SSAPI_EXCEPTION_ALARM_NOT_CLEARABLE 0x000700CCL

//Alarm description
//
// MessageId: CTS_SSAPI_ALARM_TOO_MANY_WRONG_LOGINS
//
// MessageText:
//
//  The user '%1' has attempted to login three times with a wrong password.
//
#define CTS_SSAPI_ALARM_TOO_MANY_WRONG_LOGINS 0x000700CDL

//IOP state
//
// MessageId: CTS_SSAPI_IOP_STATE_NAME_LOADING
//
// MessageText:
//
//  Loading S/W image.
//
#define CTS_SSAPI_IOP_STATE_NAME_LOADING 0x000700CEL

//IOP state
//
// MessageId: CTS_SSAPI_IOP_STATE_NAME_FAILING
//
// MessageText:
//
//  Being failed over
//
#define CTS_SSAPI_IOP_STATE_NAME_FAILING 0x000700CFL

//IOP state
//
// MessageId: CTS_SSAPI_IOP_STATE_NAME_FAILED
//
// MessageText:
//
//  Failed over
//
#define CTS_SSAPI_IOP_STATE_NAME_FAILED  0x000700D0L

//Exception name
//
// MessageId: CTS_SSAPI_EXCEPTION_CANNOT_DELETE_USER_AS_LOGGED_IN
//
// MessageText:
//
//  Can not delete the user object you're logged in as.
//
#define CTS_SSAPI_EXCEPTION_CANNOT_DELETE_USER_AS_LOGGED_IN 0x000700D1L

//Bus Location Enumeration
//
// MessageId: CTS_SSAPI_BUS_LOCATION_ENUMERATION
//
// MessageText:
//
//  Bus A:0, Bus B:1, Bus C:2, Bus D:3
//
#define CTS_SSAPI_BUS_LOCATION_ENUMERATION 0x000700D2L

//HBC Slot Location Enumeration
//
// MessageId: CTS_SSAPI_HBC_SLOT_LOCATION_ENUMERATION
//
// MessageText:
//
//  HBC 1:0, HBC 2:1
//
#define CTS_SSAPI_HBC_SLOT_LOCATION_ENUMERATION 0x000700D3L

//Battery Location Enumeration
//
// MessageId: CTS_SSAPI_BATTERY_LOCATION_ENUMERATION
//
// MessageText:
//
//  Battery 1:0, Battery 2:1
//
#define CTS_SSAPI_BATTERY_LOCATION_ENUMERATION 0x000700D4L

//Disk Power Supply Location Enumeration
//
// MessageId: CTS_SSAPI_DISK_POWER_SUPPLY_LOCATION_ENUMERATION
//
// MessageText:
//
//  Disk PS 1:0, Disk PS 2:1
//
#define CTS_SSAPI_DISK_POWER_SUPPLY_LOCATION_ENUMERATION 0x000700D5L

//Fan Location Enumeration
//
// MessageId: CTS_SSAPI_FAN_LOCATION_ENUMERATION
//
// MessageText:
//
//  Fan 1:0, Fan 2:1
//
#define CTS_SSAPI_FAN_LOCATION_ENUMERATION 0x000700D6L

//Chassis Power Supply Location Enumeration
//
// MessageId: CTS_SSAPI_CHASSIS_POWER_SUPPLY_LOCATION_ENUMERATION
//
// MessageText:
//
//  Chassis PS 1:0, Chassis PS 2:1, Chassis PS 3:2
//
#define CTS_SSAPI_CHASSIS_POWER_SUPPLY_LOCATION_ENUMERATION 0x000700D7L

//Bay Location Enumeration
//
// MessageId: CTS_SSAPI_BAY_LOCATION_ENUMERATION
//
// MessageText:
//
//  Bay 1:0, Bay 2:1, Bay 3:2, Bay 4:3, Bay 5:4, Bay 6:8, Bay 7:9, Bay 8:10, Bay 9:11, Bay 10:12, Bay 11:16, Bay 12:17, Bay 13:18, Bay 14:19, Bay 15:20, Bay 16:24, Bay 17:25, Bay 18:26, Bay 19:27, Bay 20:28
//
#define CTS_SSAPI_BAY_LOCATION_ENUMERATION 0x000700D8L

//Slot Location Enumeration
//
// MessageId: CTS_SSAPI_SLOT_LOCATION_ENUMERATION
//
// MessageText:
//
//  A1:25, A2:26, A3:27, A4:24, B1:16, B2:17, B3:18, B4:19, C1:29, C2:30, C3:31, C4:28, D1:20, D2:21, D3:22, D4:23
//
#define CTS_SSAPI_SLOT_LOCATION_ENUMERATION 0x000700D9L

//LUNMapEntry Specific Status Enumeration
//
// MessageId: CTS_SSAPI_LUN_MAP_ENTRY_STATUS_ENUMERATION
//
// MessageText:
//
//  Quiesced:458879, Good:458876, Dead:458877
//
#define CTS_SSAPI_LUN_MAP_ENTRY_STATUS_ENUMERATION 0x000700DAL

//StorageElement specific status Enumeration
//
// MessageId: CTS_SSAPI_STORAGE_ELEMENT_STATUS_ENUMERATION
//
// MessageText:
//
//  Offline:458976, Fault Tolerant:45897, On-Line:458975, Critical:458974, Unknown:458880, Regenerating:458980, Down:458979, Up:458978, Good:458876, Dead:458877, Warning:458881
//
#define CTS_SSAPI_STORAGE_ELEMENT_STATUS_ENUMERATION 0x000700DBL

//Exception name
//
// MessageId: CTS_SSAPI_EXCEPTION_INVALID_PARTITION_SIZE
//
// MessageText:
//
//  Command failed. Partition size requested is invalid.
//
#define CTS_SSAPI_EXCEPTION_INVALID_PARTITION_SIZE 0x000700DCL

//Exception name
//
// MessageId: CTS_SSAPI_EXCEPTION_PART_MASTER_NOT_INITED
//
// MessageText:
//
//  Internal error. Partiton Master is not ready.
//
#define CTS_SSAPI_EXCEPTION_PART_MASTER_NOT_INITED 0x000700DDL

//Object state
//
// MessageId: CTS_SSAPI_ARRAY_STATE_CRITICAL
//
// MessageText:
//
//  Critical
//
#define CTS_SSAPI_ARRAY_STATE_CRITICAL   0x000700DEL

//Object state
//
// MessageId: CTS_SSAPI_ARRAY_STATE_ONLINE
//
// MessageText:
//
//  On-line
//
#define CTS_SSAPI_ARRAY_STATE_ONLINE     0x000700DFL

//Object state
//
// MessageId: CTS_SSAPI_ARRAY_STATE_OFFLINE
//
// MessageText:
//
//  Off-line
//
#define CTS_SSAPI_ARRAY_STATE_OFFLINE    0x000700E0L

//Object state
//
// MessageId: CTS_SSAPI_ARRAY_STATE_FAULT_TALERANT
//
// MessageText:
//
//  Fault-tolerant
//
#define CTS_SSAPI_ARRAY_STATE_FAULT_TALERANT 0x000700E1L

//Object state
//
// MessageId: CTS_SSAPI_MEMBER_STATE_UP
//
// MessageText:
//
//  Up
//
#define CTS_SSAPI_MEMBER_STATE_UP        0x000700E2L

//Object state
//
// MessageId: CTS_SSAPI_MEMBER_STATE_DOWN
//
// MessageText:
//
//  Down
//
#define CTS_SSAPI_MEMBER_STATE_DOWN      0x000700E3L

//Object state
//
// MessageId: CTS_SSAPI_MEMBER_STATE_REGENERATING
//
// MessageText:
//
//  Regenerating
//
#define CTS_SSAPI_MEMBER_STATE_REGENERATING 0x000700E4L

//Asset Tag Not Found String
//
// MessageId: CTS_SSAPI_ASSET_TAG_NOT_FOUND_STRING
//
// MessageText:
//
//  Not Available
//
#define CTS_SSAPI_ASSET_TAG_NOT_FOUND_STRING 0x000700E5L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_AVG_XFER
//
// MessageText:
//
//  Average number of data transfers per second
//
#define CTS_SSAPI_PHS_DISK_AVG_XFER      0x000700E6L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MAX_XFER
//
// MessageText:
//
//  Maximum number of data transfers per second
//
#define CTS_SSAPI_PHS_DISK_MAX_XFER      0x000700E7L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MIN_XFER
//
// MessageText:
//
//  Minimum number of data transfers per second
//
#define CTS_SSAPI_PHS_DISK_MIN_XFER      0x000700E8L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_AVG_BYTES_READ
//
// MessageText:
//
//  Average number of bytes read per second
//
#define CTS_SSAPI_PHS_DISK_AVG_BYTES_READ 0x000700E9L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MAX_BYTES_READ
//
// MessageText:
//
//  Maximum number of bytes read per second
//
#define CTS_SSAPI_PHS_DISK_MAX_BYTES_READ 0x000700EAL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MIN_BYTES_READ
//
// MessageText:
//
//  Minimum number of bytes read per second
//
#define CTS_SSAPI_PHS_DISK_MIN_BYTES_READ 0x000700EBL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_AVG_BYTES_WRITTEN
//
// MessageText:
//
//  Average number of bytes written per second
//
#define CTS_SSAPI_PHS_DISK_AVG_BYTES_WRITTEN 0x000700ECL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MAX_BYTES_WRITTEN
//
// MessageText:
//
//  Maximum number of bytes written per second
//
#define CTS_SSAPI_PHS_DISK_MAX_BYTES_WRITTEN 0x000700EDL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MIN_BYTES_WRITTEN
//
// MessageText:
//
//  Minimum number of bytes written per second
//
#define CTS_SSAPI_PHS_DISK_MIN_BYTES_WRITTEN 0x000700EEL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_AVG_BYTES_XFERED
//
// MessageText:
//
//  Average number of bytes transferred per second
//
#define CTS_SSAPI_PHS_DISK_AVG_BYTES_XFERED 0x000700EFL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MAX_BYTES_XFERED
//
// MessageText:
//
//  Maximum number of bytes transferred per second
//
#define CTS_SSAPI_PHS_DISK_MAX_BYTES_XFERED 0x000700F0L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MIN_BYTES_XFERED
//
// MessageText:
//
//  Minimum number of bytes transferred per second
//
#define CTS_SSAPI_PHS_DISK_MIN_BYTES_XFERED 0x000700F1L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_AVG_READ_SIZE
//
// MessageText:
//
//  Average read size
//
#define CTS_SSAPI_PHS_DISK_AVG_READ_SIZE 0x000700F2L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MAX_READ_SIZE
//
// MessageText:
//
//  Maximum read size
//
#define CTS_SSAPI_PHS_DISK_MAX_READ_SIZE 0x000700F3L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MIN_READ_SIZE
//
// MessageText:
//
//  Minimum read size
//
#define CTS_SSAPI_PHS_DISK_MIN_READ_SIZE 0x000700F4L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_AVG_WRITE_SIZE
//
// MessageText:
//
//  Average write size
//
#define CTS_SSAPI_PHS_DISK_AVG_WRITE_SIZE 0x000700F5L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MAX_WRITE_SIZE
//
// MessageText:
//
//  Maximum write size
//
#define CTS_SSAPI_PHS_DISK_MAX_WRITE_SIZE 0x000700F6L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MIN_WRITE_SIZE
//
// MessageText:
//
//  Minimum write size
//
#define CTS_SSAPI_PHS_DISK_MIN_WRITE_SIZE 0x000700F7L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_AVG_XFER_SIZE
//
// MessageText:
//
//  Average transfer size
//
#define CTS_SSAPI_PHS_DISK_AVG_XFER_SIZE 0x000700F8L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MAX_XFER_SIZE
//
// MessageText:
//
//  Maximum transfer size
//
#define CTS_SSAPI_PHS_DISK_MAX_XFER_SIZE 0x000700F9L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MIN_XFER_SIZE
//
// MessageText:
//
//  Minimum transfer size
//
#define CTS_SSAPI_PHS_DISK_MIN_XFER_SIZE 0x000700FAL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_AVG_MICROSEC_PER_READ
//
// MessageText:
//
//  Average number of microseconds per read
//
#define CTS_SSAPI_PHS_DISK_AVG_MICROSEC_PER_READ 0x000700FBL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MAX_MICROSEC_PER_READ
//
// MessageText:
//
//  Maximum number of microseconds per read
//
#define CTS_SSAPI_PHS_DISK_MAX_MICROSEC_PER_READ 0x000700FCL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MIN_MICROSEC_PER_READ
//
// MessageText:
//
//  Minimum number of microseconds per read
//
#define CTS_SSAPI_PHS_DISK_MIN_MICROSEC_PER_READ 0x000700FDL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_AVG_MICROSEC_PER_WRITE
//
// MessageText:
//
//  Average number of microseconds per write
//
#define CTS_SSAPI_PHS_DISK_AVG_MICROSEC_PER_WRITE 0x000700FEL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MAX_MICROSEC_PER_WRITE
//
// MessageText:
//
//  Maximum number of microseconds per write
//
#define CTS_SSAPI_PHS_DISK_MAX_MICROSEC_PER_WRITE 0x000700FFL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MIN_MICROSEC_PER_WRITE
//
// MessageText:
//
//  Minimum number of microseconds per write
//
#define CTS_SSAPI_PHS_DISK_MIN_MICROSEC_PER_WRITE 0x00070100L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_AVG_MICROSEC_PER_XFER
//
// MessageText:
//
//  Average number of microseconds per transfer
//
#define CTS_SSAPI_PHS_DISK_AVG_MICROSEC_PER_XFER 0x00070101L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MAX_MICROSEC_PER_XFER
//
// MessageText:
//
//  Maximum number of microseconds per transfer
//
#define CTS_SSAPI_PHS_DISK_MAX_MICROSEC_PER_XFER 0x00070102L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DISK_MIN_MICROSEC_PER_XFER
//
// MessageText:
//
//  Minimum number of microseconds per transfer
//
#define CTS_SSAPI_PHS_DISK_MIN_MICROSEC_PER_XFER 0x00070103L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_AVG_PAGES_READ
//
// MessageText:
//
//  Average number of pages read per second
//
#define CTS_SSAPI_PHS_SSD_AVG_PAGES_READ 0x00070104L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_MAX_PAGES_READ
//
// MessageText:
//
//  Maximum number of pages read per second
//
#define CTS_SSAPI_PHS_SSD_MAX_PAGES_READ 0x00070105L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_MIN_PAGES_READ
//
// MessageText:
//
//  Minimum number of pages read per second
//
#define CTS_SSAPI_PHS_SSD_MIN_PAGES_READ 0x00070106L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_AVG_PAGES_READ_HIT_CACHE
//
// MessageText:
//
//  Average number of pages read that hit cache per second
//
#define CTS_SSAPI_PHS_SSD_AVG_PAGES_READ_HIT_CACHE 0x00070107L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_MAX_PAGES_READ_HIT_CACHE
//
// MessageText:
//
//  Maximum number of pages read that hit cache per second
//
#define CTS_SSAPI_PHS_SSD_MAX_PAGES_READ_HIT_CACHE 0x00070108L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_MIN_PAGES_READ_HIT_CACHE
//
// MessageText:
//
//  Minimum number of pages read that hit cache per second
//
#define CTS_SSAPI_PHS_SSD_MIN_PAGES_READ_HIT_CACHE 0x00070109L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_AVG_PAGES_READ_MISS_CACHE
//
// MessageText:
//
//  Average number of pages read that missed cache per second
//
#define CTS_SSAPI_PHS_SSD_AVG_PAGES_READ_MISS_CACHE 0x0007010AL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_MAX_PAGES_READ_MISS_CACHE
//
// MessageText:
//
//  Maximum number of pages read that missed cache per second
//
#define CTS_SSAPI_PHS_SSD_MAX_PAGES_READ_MISS_CACHE 0x0007010BL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_MIN_PAGES_READ_MISS_CACHE
//
// MessageText:
//
//  Minimum number of pages read that missed cache per second
//
#define CTS_SSAPI_PHS_SSD_MIN_PAGES_READ_MISS_CACHE 0x0007010CL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_AVG_PAGES_WRITTEN
//
// MessageText:
//
//  Average number of pages written per second
//
#define CTS_SSAPI_PHS_SSD_AVG_PAGES_WRITTEN 0x0007010DL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_MAX_PAGES_WRITTEN
//
// MessageText:
//
//  Maximum number of pages written per second
//
#define CTS_SSAPI_PHS_SSD_MAX_PAGES_WRITTEN 0x0007010EL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_MIN_PAGES_WRITTEN
//
// MessageText:
//
//  Minimum number of pages written per second
//
#define CTS_SSAPI_PHS_SSD_MIN_PAGES_WRITTEN 0x0007010FL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_AVG_PAGES_WRITTEN_HIT_CACHE
//
// MessageText:
//
//  Average number of pages written that hit cache per second
//
#define CTS_SSAPI_PHS_SSD_AVG_PAGES_WRITTEN_HIT_CACHE 0x00070110L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_MAX_PAGES_WRITTEN_HIT_CACHE
//
// MessageText:
//
//  Maximum number of pages written that hit cache per second
//
#define CTS_SSAPI_PHS_SSD_MAX_PAGES_WRITTEN_HIT_CACHE 0x00070111L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_MIN_PAGES_WRITTEN_HIT_CACHE
//
// MessageText:
//
//  Minimum number of pages written that hit cache per second
//
#define CTS_SSAPI_PHS_SSD_MIN_PAGES_WRITTEN_HIT_CACHE 0x00070112L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_AVG_PAGES_WRITTEN_MISS_CACHE
//
// MessageText:
//
//  Average number of pages written that missed cache per second
//
#define CTS_SSAPI_PHS_SSD_AVG_PAGES_WRITTEN_MISS_CACHE 0x00070113L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_MAX_PAGES_WRITTEN_MISS_CACHE
//
// MessageText:
//
//  Maximum number of pages written that missed cache per second
//
#define CTS_SSAPI_PHS_SSD_MAX_PAGES_WRITTEN_MISS_CACHE 0x00070114L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_MIN_PAGES_WRITTEN_MISS_CACHE
//
// MessageText:
//
//  Minimum number of pages written that missed cache per second
//
#define CTS_SSAPI_PHS_SSD_MIN_PAGES_WRITTEN_MISS_CACHE 0x00070115L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_AVG_ERASE_PAGES
//
// MessageText:
//
//  Average number of erase pages available
//
#define CTS_SSAPI_PHS_SSD_AVG_ERASE_PAGES 0x00070116L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_MAX_ERASE_PAGES
//
// MessageText:
//
//  Maximum number of erase pages available
//
#define CTS_SSAPI_PHS_SSD_MAX_ERASE_PAGES 0x00070117L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_MIN_ERASE_PAGES
//
// MessageText:
//
//  Minimum number of erase pages available
//
#define CTS_SSAPI_PHS_SSD_MIN_ERASE_PAGES 0x00070118L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_AVG_BYTES_READ
//
// MessageText:
//
//  Average number of bytes read per second
//
#define CTS_SSAPI_PHS_SSD_AVG_BYTES_READ 0x00070119L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_MAX_BYTES_READ
//
// MessageText:
//
//  Maximum number of bytes read per second
//
#define CTS_SSAPI_PHS_SSD_MAX_BYTES_READ 0x0007011AL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_MIN_BYTES_READ
//
// MessageText:
//
//  Minimum number of bytes read per second
//
#define CTS_SSAPI_PHS_SSD_MIN_BYTES_READ 0x0007011BL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_AVG_BYTES_WRITTEN
//
// MessageText:
//
//  Average number of bytes written per second
//
#define CTS_SSAPI_PHS_SSD_AVG_BYTES_WRITTEN 0x0007011CL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_MAX_BYTES_WRITTEN
//
// MessageText:
//
//  Maximum number of bytes written per second
//
#define CTS_SSAPI_PHS_SSD_MAX_BYTES_WRITTEN 0x0007011DL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_MIN_BYTES_WRITTEN
//
// MessageText:
//
//  Minimum number of bytes written per second
//
#define CTS_SSAPI_PHS_SSD_MIN_BYTES_WRITTEN 0x0007011EL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_PAGE_TABLE_SIZE
//
// MessageText:
//
//  Page table size
//
#define CTS_SSAPI_PHS_SSD_PAGE_TABLE_SIZE 0x0007011FL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_PERCENT_DIRTY
//
// MessageText:
//
//  Percent of dirty pages
//
#define CTS_SSAPI_PHS_SSD_PERCENT_DIRTY  0x00070120L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_SSD_NUM_REPLACEMENT_PAGES
//
// MessageText:
//
//  Number of replacement pages available
//
#define CTS_SSAPI_PHS_SSD_NUM_REPLACEMENT_PAGES 0x00070121L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_RAID_AVG_READS
//
// MessageText:
//
//  Average number of reads per second
//
#define CTS_SSAPI_PHS_RAID_AVG_READS     0x00070122L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_RAID_MAX_READS
//
// MessageText:
//
//  Maximum number of reads per second
//
#define CTS_SSAPI_PHS_RAID_MAX_READS     0x00070123L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_RAID_MIN_READS
//
// MessageText:
//
//  Minimum number of reads per second
//
#define CTS_SSAPI_PHS_RAID_MIN_READS     0x00070124L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_RAID_AVG_WRITES
//
// MessageText:
//
//  Average number of writes per second
//
#define CTS_SSAPI_PHS_RAID_AVG_WRITES    0x00070125L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_RAID_MAX_WRITES
//
// MessageText:
//
//  Maximum number of writes per second
//
#define CTS_SSAPI_PHS_RAID_MAX_WRITES    0x00070126L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_RAID_MIN_WRITES
//
// MessageText:
//
//  Minimum number of writes per second
//
#define CTS_SSAPI_PHS_RAID_MIN_WRITES    0x00070127L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_RAID_AVG_BLOCK_READ
//
// MessageText:
//
//  Average number of blocks read per second
//
#define CTS_SSAPI_PHS_RAID_AVG_BLOCK_READ 0x00070128L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_RAID_MAX_BLOCK_READ
//
// MessageText:
//
//  Maximum number of blocks read per second
//
#define CTS_SSAPI_PHS_RAID_MAX_BLOCK_READ 0x00070129L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_RAID_MIN_BLOCK_READ
//
// MessageText:
//
//  Minimum number of blocks read per second
//
#define CTS_SSAPI_PHS_RAID_MIN_BLOCK_READ 0x0007012AL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_RAID_AVG_BLOCK_WRITTEN
//
// MessageText:
//
//  Average number of blocks written per second
//
#define CTS_SSAPI_PHS_RAID_AVG_BLOCK_WRITTEN 0x0007012BL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_RAID_MAX_BLOCK_WRITTEN
//
// MessageText:
//
//  Maximum number of blocks written per second
//
#define CTS_SSAPI_PHS_RAID_MAX_BLOCK_WRITTEN 0x0007012CL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_RAID_MIN_BLOCK_WRITTEN
//
// MessageText:
//
//  Minimum number of blocks written per second
//
#define CTS_SSAPI_PHS_RAID_MIN_BLOCK_WRITTEN 0x0007012DL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_RAID_AVG_OVERWRITES
//
// MessageText:
//
//  Average number of overwrites issued per second
//
#define CTS_SSAPI_PHS_RAID_AVG_OVERWRITES 0x0007012EL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_RAID_MAX_OVERWRITES
//
// MessageText:
//
//  Maximum number of overwrites issued per second
//
#define CTS_SSAPI_PHS_RAID_MAX_OVERWRITES 0x0007012FL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_RAID_MIN_OVERWRITES
//
// MessageText:
//
//  Minimum number of overwrites issued per second
//
#define CTS_SSAPI_PHS_RAID_MIN_OVERWRITES 0x00070130L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_RAID_SUCCESS_REASSIGN
//
// MessageText:
//
//  Number of successful reassigns
//
#define CTS_SSAPI_PHS_RAID_SUCCESS_REASSIGN 0x00070131L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_RAID_FAILED_REASSIGN
//
// MessageText:
//
//  Number of unsuccessful reassigns
//
#define CTS_SSAPI_PHS_RAID_FAILED_REASSIGN 0x00070132L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_RAID_NUM_RETRIES
//
// MessageText:
//
//  Total number of retries
//
#define CTS_SSAPI_PHS_RAID_NUM_RETRIES   0x00070133L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_RAID_NUM_RECOVERED_ERRORS
//
// MessageText:
//
//  Total number of recovered errors
//
#define CTS_SSAPI_PHS_RAID_NUM_RECOVERED_ERRORS 0x00070134L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_TIMER_TIMEOUTS
//
// MessageText:
//
//  Number of timer timeouts due to BSA command not responding
//
#define CTS_SSAPI_PHS_STS_TIMER_TIMEOUTS 0x00070135L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_ERROR_REPLIES_RECEIVED
//
// MessageText:
//
//  Number of replies with error received
//
#define CTS_SSAPI_PHS_STS_ERROR_REPLIES_RECEIVED 0x00070136L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_ERROR_REPLIES_SENT
//
// MessageText:
//
//  Total number of replies with error sent
//
#define CTS_SSAPI_PHS_STS_ERROR_REPLIES_SENT 0x00070137L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_AVG_READS
//
// MessageText:
//
//  Average number of reads per second
//
#define CTS_SSAPI_PHS_STS_AVG_READS      0x00070138L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_MAX_READS
//
// MessageText:
//
//  Maximum number of reads per second
//
#define CTS_SSAPI_PHS_STS_MAX_READS      0x00070139L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_MIN_READS
//
// MessageText:
//
//  Minimum number of reads per second
//
#define CTS_SSAPI_PHS_STS_MIN_READS      0x0007013AL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_AVG_WRITES
//
// MessageText:
//
//  Average number of writes per second
//
#define CTS_SSAPI_PHS_STS_AVG_WRITES     0x0007013BL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_MAX_WRITES
//
// MessageText:
//
//  Maximum number of writes per second
//
#define CTS_SSAPI_PHS_STS_MAX_WRITES     0x0007013CL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_MIN_WRITES
//
// MessageText:
//
//  Minimum number of writes per second
//
#define CTS_SSAPI_PHS_STS_MIN_WRITES     0x0007013DL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_AVG_BSA_COMMANDS
//
// MessageText:
//
//  Average number of BSA commands served per second
//
#define CTS_SSAPI_PHS_STS_AVG_BSA_COMMANDS 0x0007013EL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_MAX_BSA_COMMANDS
//
// MessageText:
//
//  Maximum number of BSA commands served per second
//
#define CTS_SSAPI_PHS_STS_MAX_BSA_COMMANDS 0x0007013FL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_MIN_BSA_COMMANDS
//
// MessageText:
//
//  Minimum number of BSA commands served per second
//
#define CTS_SSAPI_PHS_STS_MIN_BSA_COMMANDS 0x00070140L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_AVG_SCSI_COMMANDS
//
// MessageText:
//
//  Average number of SCSI commands served per second
//
#define CTS_SSAPI_PHS_STS_AVG_SCSI_COMMANDS 0x00070141L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_MAX_SCSI_COMMANDS
//
// MessageText:
//
//  Maximum number of SCSI commands served per second
//
#define CTS_SSAPI_PHS_STS_MAX_SCSI_COMMANDS 0x00070142L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_MIN_SCSI_COMMANDS
//
// MessageText:
//
//  Minimum number of SCSI commands served per second
//
#define CTS_SSAPI_PHS_STS_MIN_SCSI_COMMANDS 0x00070143L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_AVG_BYTES_READ
//
// MessageText:
//
//  Average number of bytes read per second
//
#define CTS_SSAPI_PHS_STS_AVG_BYTES_READ 0x00070144L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_MAX_BYTES_READ
//
// MessageText:
//
//  Maximum number of bytes read per second
//
#define CTS_SSAPI_PHS_STS_MAX_BYTES_READ 0x00070145L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_MIN_BYTES_READ
//
// MessageText:
//
//  Minimum number of bytes read per second
//
#define CTS_SSAPI_PHS_STS_MIN_BYTES_READ 0x00070146L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_AVG_BYTES_WRITTEN
//
// MessageText:
//
//  Average number of bytes written per second
//
#define CTS_SSAPI_PHS_STS_AVG_BYTES_WRITTEN 0x00070147L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_MAX_BYTES_WRITTEN
//
// MessageText:
//
//  Maximum number of bytes written per second
//
#define CTS_SSAPI_PHS_STS_MAX_BYTES_WRITTEN 0x00070148L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_MIN_BYTES_WRITTEN
//
// MessageText:
//
//  Minimum number of bytes written per second
//
#define CTS_SSAPI_PHS_STS_MIN_BYTES_WRITTEN 0x00070149L

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_AVG_BYTES_TOTAL
//
// MessageText:
//
//  Average number of bytes written & read per second
//
#define CTS_SSAPI_PHS_STS_AVG_BYTES_TOTAL 0x0007014AL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_MAX_BYTES_TOTAL
//
// MessageText:
//
//  Maximum number of bytes written & read per second
//
#define CTS_SSAPI_PHS_STS_MAX_BYTES_TOTAL 0x0007014BL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_STS_MIN_BYTES_TOTAL
//
// MessageText:
//
//  Minimum number of bytes written & read per second
//
#define CTS_SSAPI_PHS_STS_MIN_BYTES_TOTAL 0x0007014CL

//  The name of a PHS data item
//
// MessageId: CTS_SSAPI_PHS_DATA_NAME_FAN_SPEED
//
// MessageText:
//
//  Fan speed (as a percentage of the full speed )
//
#define CTS_SSAPI_PHS_DATA_NAME_FAN_SPEED 0x0007014DL

//Specific Status Enumeration for ChassisPowerSupply Device
//
// MessageId: CTS_SSAPI_CHASSIS_PS_STATUS_ENUMERATION
//
// MessageText:
//
//  Quiesced:458879, Good:458876, Dead:458877, Warning:458881, Input Bad:458883, Output Bad:458884, Over-temp:458902
//
#define CTS_SSAPI_CHASSIS_PS_STATUS_ENUMERATION 0x0007014EL

//  The name of a device
//
// MessageId: CTS_SSAPI_DEVICE_NAME_CORD_SET
//
// MessageText:
//
//  Cord set
//
#define CTS_SSAPI_DEVICE_NAME_CORD_SET   0x0007014FL

//  Specific status for cord set device
//
// MessageId: CTS_SSAPI_DEVICE_CORD_SET_STATE_POWER_ON
//
// MessageText:
//
//  Power On
//
#define CTS_SSAPI_DEVICE_CORD_SET_STATE_POWER_ON 0x00070150L

//  Specific status for cord set device
//
// MessageId: CTS_SSAPI_DEVICE_CORD_SET_STATE_POWER_OFF
//
// MessageText:
//
//  Power Off
//
#define CTS_SSAPI_DEVICE_CORD_SET_STATE_POWER_OFF 0x00070151L

//  Status Enumeration for cord set device
//
// MessageId: CTS_SSAPI_DEVICE_CORD_SET_STATE_ENUMERATION
//
// MessageText:
//
//  Power Off:459089, Power On:459088
//
#define CTS_SSAPI_DEVICE_CORD_SET_STATE_ENUMERATION 0x00070152L

//  Chassis Key Switch Enumeration
//
// MessageId: CTS_SSAPI_CHASSIS_DEVICE_KEY_SWITCH_ENUMERATION
//
// MessageText:
//
//  Off:0, Service:1, Security:2, On:3
//
#define CTS_SSAPI_CHASSIS_DEVICE_KEY_SWITCH_ENUMERATION 0x00070153L

//CordSet Location Enumeration
//
// MessageId: CTS_SSAPI_CORD_SET_LOCATION_ENUMERATION
//
// MessageText:
//
//  Top:1, Upper-Middle:2, Lower-Middle:3, Bottom:4
//
#define CTS_SSAPI_CORD_SET_LOCATION_ENUMERATION 0x00070154L

//Log msg description
//
// MessageId: CTS_SSAPI_EVENT_USER_LOGGED_IN
//
// MessageText:
//
//  The user '%1' has logged in.
//
#define CTS_SSAPI_EVENT_USER_LOGGED_IN   0x00070155L

//Log msg description
//
// MessageId: CTS_SSAPI_EVENT_USER_LOGGED_OUT
//
// MessageText:
//
//  The user '%1' has logged out.
//
#define CTS_SSAPI_EVENT_USER_LOGGED_OUT  0x00070156L

//RAID Master Exception
//
// MessageId: CTS_SSAPI_RAID_MSTR_INVALID_DATA_BLOCK
//
// MessageText:
//
//  Command failed. Invalid stripe size specified.
//
#define CTS_SSAPI_RAID_MSTR_INVALID_DATA_BLOCK 0x00070157L

//RAID Master Exception
//
// MessageId: CTS_SSAPI_RAID_MSTR_INVALID_PARITY_BLOCK
//
// MessageText:
//
//  Command failed. Invalid parity size specified.
//
#define CTS_SSAPI_RAID_MSTR_INVALID_PARITY_BLOCK 0x00070158L

//Failure log message from the ShadowTable
//
// MessageId: CTS_SSAPI_SHADOW_TABLE_OP_FAILURE_EVENT
//
// MessageText:
//
//  Operation [name=%1] of the ShadowTable [name=%2] failed [rc=%3]
//
#define CTS_SSAPI_SHADOW_TABLE_OP_FAILURE_EVENT 0xC0070159L

//Error msg for the HostManager
//
// MessageId: CTS_SSAPI_HM_CONNECTION_UNAVAILABLE
//
// MessageText:
//
//  One or more connection is already assigned to a host
//
#define CTS_SSAPI_HM_CONNECTION_UNAVAILABLE 0xC007015AL

//Error msg for the HostManager
//
// MessageId: CTS_SSAPI_HM_TO_MANY_CONNECTIONS
//
// MessageText:
//
//  The number of connections exceeds the storage space available
//
#define CTS_SSAPI_HM_TO_MANY_CONNECTIONS 0xC007015BL

//Error msg for the ConnectionManager
//
// MessageId: CTS_SSAPI_HM_PATH_IN_USE
//
// MessageText:
//
//  Operation failed. The data path is in use by one or more LUN.
//
#define CTS_SSAPI_HM_PATH_IN_USE         0xC007015CL

//Error msg for the ConnectionManager
//
// MessageId: CTS_SSAPI_CM_INVALID_CONN_COUNT_FOR_RDP
//
// MessageText:
//
//  A Redundant data path can only have 1 or 2 connections
//
#define CTS_SSAPI_CM_INVALID_CONN_COUNT_FOR_RDP 0xC007015DL

//Error msg for the ConnectionManager
//
// MessageId: CTS_SSAPI_CM_CONN_MUST_BELONG_2_SAME_HOST
//
// MessageText:
//
//  A Redundant data path can only have connections to the same host
//
#define CTS_SSAPI_CM_CONN_MUST_BELONG_2_SAME_HOST 0xC007015EL

//Error msg for the ConnectionManager
//
// MessageId: CTS_SSAPI_CM_CONN_MUST_BE_ON_PARTNER_NACS
//
// MessageText:
//
//  All connections must map to a single primary/fail-over IOP pair
//
#define CTS_SSAPI_CM_CONN_MUST_BE_ON_PARTNER_NACS 0xC007015FL

//Error msg for the ConnectionManager
//
// MessageId: CTS_SSAPI_CM_CONN_MUST_BE_PRESENT
//
// MessageText:
//
//  No connection objects were specified
//
#define CTS_SSAPI_CM_CONN_MUST_BE_PRESENT 0xC0070160L

//Error msg for the ConnectionManager
//
// MessageId: CTS_SSAPI_CM_CONN_NOT_ASSIGNED_TO_HOST
//
// MessageText:
//
//  At least one connection is not assigned to a host
//
#define CTS_SSAPI_CM_CONN_NOT_ASSIGNED_TO_HOST 0xC0070161L

//Error msg for the ConnectionManager
//
// MessageId: CTS_SSAPI_CM_ONLY2_CONN_PER_HOST
//
// MessageText:
//
//  Clustered data path can have at most 2 connections to the same host
//
#define CTS_SSAPI_CM_ONLY2_CONN_PER_HOST 0xC0070162L

//Error msg for the ConnectionManager
//
// MessageId: CTS_SSAPI_CM_CONN_FROM_HOST_MUST_BE_ON_PARTNER_NACS
//
// MessageText:
//
//  All connections from each host must map to a single primary/fail-over IOP pair
//
#define CTS_SSAPI_CM_CONN_FROM_HOST_MUST_BE_ON_PARTNER_NACS 0xC0070163L

//  CTS_FACILITY_PTS:  Persistant Table Service localized string and event codes
//
//  CTEvtPTS.mc - Error codes unique to the Persistant Table Service DDM.
//                Included by CTEvent.mc.  See that file for more info.
//
//
//
// MessageId: CTS_PTS_INVALID_COMMAND
//
// MessageText:
//
//  The Persistant Table Service received an invalid command in a message.
//
#define CTS_PTS_INVALID_COMMAND          0x80080001L

//
// MessageId: CTS_PTS_INVALID_PARAMETER
//
// MessageText:
//
//  The Persistant Table Service received an invalid command in a message.
//
#define CTS_PTS_INVALID_PARAMETER        0x80080002L

//
// MessageId: CTS_PTS_LISTEN_TIMED_OUT
//
// MessageText:
//
//  The Persistant Table Service timed out listening for an expected condition.
//
#define CTS_PTS_LISTEN_TIMED_OUT         0x80080003L

//  CTS_FACILITY_VCM:  Virtual Circuit Master localized string and event codes
//
//  CTEvtVCM.mc - Error codes unique to the VCM interface DDM.
//                Included by CTEvent.mc.  See that file for more info.
//
//
//
// MessageId: CTS_VCM_INVALID_COMMAND
//
// MessageText:
//
//  The Virtual Circuit Master received an invalid command in a request.
//
#define CTS_VCM_INVALID_COMMAND          0x80090001L

//
// MessageId: CTS_VCM_INVALID_PARAMETER
//
// MessageText:
//
//  The Virtual Circuit Master received an invalid command in a request.
//
#define CTS_VCM_INVALID_PARAMETER        0x80090002L

//
// MessageId: CTS_VCM_EVCSR_READ_ERROR
//
// MessageText:
//
//  The Virtual Circuit Master could not read the EVC Status Record during a Virtual Circuit Create request.
//
#define CTS_VCM_EVCSR_READ_ERROR         0x80090003L

//
// MessageId: CTS_VCM_SRCR_READ_ERROR
//
// MessageText:
//
//  The Virtual Circuit Master could not read a Storage Roll Call Record specified in a VC Create request.
//
#define CTS_VCM_SRCR_READ_ERROR          0x80090004L

//
// MessageId: CTS_VCM_FCPDBT_READ_ERROR
//
// MessageText:
//
//  The Virtual Circuit Master could not read the Fibre Channel Port Database Table.
//
#define CTS_VCM_FCPDBT_READ_ERROR        0x80090005L

//
// MessageId: CTS_VCM_HCDR_READ_ERROR
//
// MessageText:
//
//  The Virtual Circuit Master could not read a Host Connection Descriptor Record specified in a VC Create request.
//
#define CTS_VCM_HCDR_READ_ERROR          0x80090006L

//
// MessageId: CTS_VCM_LDR_READ_ERROR
//
// MessageText:
//
//  The Virtual Circuit Master could not read the Loop Descriptor Table while executing a VC Create request.
//
#define CTS_VCM_LDR_READ_ERROR           0x80090007L

//
// MessageId: CTS_VCM_TARGETID_IN_USE
//
// MessageText:
//
//  The Virtual Circuit Master could not execute a VC Create request because the specified target ID is already in use.
//
#define CTS_VCM_TARGETID_IN_USE          0x80090008L

//
// MessageId: CTS_VCM_TOO_MANY_TARGETS
//
// MessageText:
//
//  The Virtual Circuit Master could not execute a VC Create request because a specified port has too many targets.
//
#define CTS_VCM_TOO_MANY_TARGETS         0x80090009L

//
// MessageId: CTS_VCM_NO_FCPDBR_ERROR
//
// MessageText:
//
//  The Virtual Circuit Master could not find a Loop Descriptor Record while executing a VC Create request.
//
#define CTS_VCM_NO_FCPDBR_ERROR          0x8009000AL

//
// MessageId: CTS_VCM_STS_CFG_CHECK_ERROR
//
// MessageText:
//
//  The Virtual Circuit Master could not read a SCSI Target Server Config Record while executing a VC Create request.
//
#define CTS_VCM_STS_CFG_CHECK_ERROR      0x8009000BL

//
// MessageId: CTS_VCM_STS_CFG_INSERT_ERROR
//
// MessageText:
//
//  The Virtual Circuit Master could not could not insert a SCSI Target Server config Record while executing a VC Create request.
//
#define CTS_VCM_STS_CFG_INSERT_ERROR     0x8009000CL

//
// MessageId: CTS_VCM_STS_VD_CHECK_ERROR
//
// MessageText:
//
//  The Virtual Circuit Master could not could not Read a SCSI Target Server Virtual Device Record while executing a VC Create request.
//
#define CTS_VCM_STS_VD_CHECK_ERROR       0x8009000DL

//
// MessageId: CTS_VCM_STS_VD_INSERT_ERROR
//
// MessageText:
//
//  The Virtual Circuit Master could not could not insert a SCSI Target Server Virtual Device Record while executing a VC Create request.
//
#define CTS_VCM_STS_VD_INSERT_ERROR      0x8009000EL

//
// MessageId: CTS_VCM_STS_VD_LISTEN_ERROR
//
// MessageText:
//
//  The Virtual Circuit Master could not could not insert a SCSI Target Server Virtual Device Record while executing a VC Create request.
//
#define CTS_VCM_STS_VD_LISTEN_ERROR      0x8009000FL

//
// MessageId: CTS_VCM_TIMER_ERROR
//
// MessageText:
//
//  The Virtual Circuit Master encountered a Timer error while executing a VC Create request.
//
#define CTS_VCM_TIMER_ERROR              0x80090010L

//
// MessageId: CTS_VCM_VDN_NOT_ON_IOP_ERROR
//
// MessageText:
//
//  The Virtual Circuit Master encountered a Time out error waiting for IOPs to hear a new Virtual Device while executing a VC Create request.
//
#define CTS_VCM_VDN_NOT_ON_IOP_ERROR     0x80090011L

//
// MessageId: CTS_VCM_EXPORT_DELETE_ERROR
//
// MessageText:
//
//  The Virtual Circuit Master could not could not delete an Export Table Record while executing a VC Create request.
//
#define CTS_VCM_EXPORT_DELETE_ERROR      0x80090012L

//
// MessageId: CTS_VCM_EIP_RID_INVALID_ERROR
//
// MessageText:
//
//  The Virtual Circuit Master could not find a specified EIP rid in the FCPortDatabase while executing a VC Create request.
//
#define CTS_VCM_EIP_RID_INVALID_ERROR    0x80090013L

//
// MessageId: CTS_VCM_EXPORT_REC_INSERT_ERROR
//
// MessageText:
//
//  The Virtual Circuit Master could not could not insert an Export Table Record while executing a VC Create request.
//
#define CTS_VCM_EXPORT_REC_INSERT_ERROR  0x80090014L

//
// MessageId: CTS_VCM_DELETE_TIMEOUT_ERROR
//
// MessageText:
//
//  The Virtual Circuit Master timed out deleteing a Virtual Device while executing a VC Delete request.
//
#define CTS_VCM_DELETE_TIMEOUT_ERROR     0x80090015L

//
// MessageId: CTS_VCM_STS_VD_DELETE_ERROR
//
// MessageText:
//
//  The Virtual Circuit Master could not delete a Virtual Device while executing a VC Delete request.
//
#define CTS_VCM_STS_VD_DELETE_ERROR      0x80090016L

//
// MessageId: CTS_VCM_RECORD_DELETE_ERROR
//
// MessageText:
//
//  The Virtual Circuit Master could not delete a record while executing a VC Delete request.
//
#define CTS_VCM_RECORD_DELETE_ERROR      0x80090017L

//
// MessageId: CTS_VCM_ATTEMPT_TO_USE_INITITATOR_PORT
//
// MessageText:
//
//  Cannot export storage to hosts connected to internal initiator loops.
//
#define CTS_VCM_ATTEMPT_TO_USE_INITITATOR_PORT 0x80090018L

//  CTS_FACILITY_HOTSWAP:  Hot Swap Master localized string & event codes
//
//  CTEvtHotSwap.mc - Error codes unique to the Hot Swap Master DDM.
//                Included by CTEvent.mc.  See that file for more info.
//
//
//  the DDM received an invalid parameter in a request message
//
// MessageId: CTS_HSW_INVALID_PARAMETER
//
// MessageText:
//
//  Invalid parameter.
//
#define CTS_HSW_INVALID_PARAMETER        0x800A0001L

//  the specified drive is in use, and can't be released from its bay
//
// MessageId: CTS_HSW_DRIVE_IN_USE
//
// MessageText:
//
//  Drive is part of a storage configuration, and cannot be released.
//
#define CTS_HSW_DRIVE_IN_USE             0x800A0002L

//  Some event codes used for the System Log test driver.

//  CTEvtTest.mc
//
// These are event codes for testing the event log.  This file can eventually be removed.
//  ELOG_TEST_INFO1
//
// MessageId: ELOG_TEST_INFO1
//
// MessageText:
//
//  Test Event - Informational 1 with an arg: %1 is the arg.
//
#define ELOG_TEST_INFO1                  0x00010034L

//  ELOG_TEST_INFO2
//
// MessageId: ELOG_TEST_INFO2
//
// MessageText:
//
//  Test Event - Informational 2
//
#define ELOG_TEST_INFO2                  0x00010035L

//  ELOG_TEST_WARNING1
//
// MessageId: ELOG_TEST_WARNING1
//
// MessageText:
//
//  Test Event - Warning 1 with an arg: %1 is the arg.
//
#define ELOG_TEST_WARNING1               0x40010036L

//  ELOG_TEST_WARNING2
//
// MessageId: ELOG_TEST_WARNING2
//
// MessageText:
//
//  Test Event - Warning 2.
//
#define ELOG_TEST_WARNING2               0x40010037L

//  ELOG_TEST_ERROR1
//
// MessageId: ELOG_TEST_ERROR1
//
// MessageText:
//
//  Test Event - Error 1 with an arg: %1 is the arg.
//
#define ELOG_TEST_ERROR1                 0x80010038L

//  ELOG_TEST_ERROR2
//
// MessageId: ELOG_TEST_ERROR2
//
// MessageText:
//
//  Test Event - Error 2.
//
#define ELOG_TEST_ERROR2                 0x80010039L

//  ELOG_TEST_INTERNAL1
//
// MessageId: ELOG_TEST_INTERNAL1
//
// MessageText:
//
//  Test Event - Internal 1 with an arg: %1 is the arg.
//
#define ELOG_TEST_INTERNAL1              0xC001003AL

//  ELOG_TEST_INTERNAL2
//
// MessageId: ELOG_TEST_INTERNAL2
//
// MessageText:
//
//  Test Event - Internal 2.
//
#define ELOG_TEST_INTERNAL2              0xC001003BL

// Event codes for the Alarm Master
//
//  CTEvtAlarmMaster.mc - Event codes for the Alarm Master
//                 Included by CTEvent.mc.  See that file for more info.
//
//
//  Alarm Submitted By Virtual Device
//
// MessageId: CTS_ALARM_VIRTUAL_SUBMITTED
//
// MessageText:
//
//  Alarm %1 with event code %2 submitted by virtual device %3.
//
#define CTS_ALARM_VIRTUAL_SUBMITTED      0x000B0001L

//  Alarm Submitted By Device
//
// MessageId: CTS_ALARM_DEVICE_SUBMITTED
//
// MessageText:
//
//  Alarm %1 with event code %2 submitted by device %3.
//
#define CTS_ALARM_DEVICE_SUBMITTED       0x000B0002L

//  Alarm Resubmitted By Virtual Device
//
// MessageId: CTS_ALARM_VIRTUAL_RESUBMITTED
//
// MessageText:
//
//  Alarm %1 with event code %2 resubmitted by virtual device %3.
//
#define CTS_ALARM_VIRTUAL_RESUBMITTED    0x000B0003L

//  Alarm Resubmitted By Device
//
// MessageId: CTS_ALARM_DEVICE_RESUBMITTED
//
// MessageText:
//
//  Alarm %1 with event code %2 resubmitted by device %3.
//
#define CTS_ALARM_DEVICE_RESUBMITTED     0x000B0004L

//  Alarm Remitted By Virtual Device
//
// MessageId: CTS_ALARM_VIRTUAL_REMITTED
//
// MessageText:
//
//  Alarm %1 remitted by virtual device %2.
//
#define CTS_ALARM_VIRTUAL_REMITTED       0x000B0005L

//  Alarm Remitted By Device
//
// MessageId: CTS_ALARM_DEVICE_REMITTED
//
// MessageText:
//
//  Alarm %1 remitted by device %2.
//
#define CTS_ALARM_DEVICE_REMITTED        0x000B0006L

//  Alarm Remitted By User
//
// MessageId: CTS_ALARM_USER_REMITTED
//
// MessageText:
//
//  Alarm %1 remitted by user.
//
#define CTS_ALARM_USER_REMITTED          0x000B0007L

//  Alarms Recovered
//
// MessageId: CTS_ALARMS_VIRTUAL_RECOVERED
//
// MessageText:
//
//  Alarms recovered by virtual device %1.
//
#define CTS_ALARMS_VIRTUAL_RECOVERED     0x000B0008L

//  Alarms Recovered
//
// MessageId: CTS_ALARMS_DEVICE_RECOVERED
//
// MessageText:
//
//  Alarms recovered by device %1.
//
#define CTS_ALARMS_DEVICE_RECOVERED      0x000B0009L

//  Alarm Acknowledged
//
// MessageId: CTS_ALARM_ACKNOWLEDGED
//
// MessageText:
//
//  Alarm %1 acknowledged.
//
#define CTS_ALARM_ACKNOWLEDGED           0x000B000AL

//  Alarm Unacknowledged
//
// MessageId: CTS_ALARM_UNACKNOWLEDGED
//
// MessageText:
//
//  Alarm %1 unacknowledged.
//
#define CTS_ALARM_UNACKNOWLEDGED         0x000B000BL

//  Alarm Noted
//
// MessageId: CTS_ALARM_NOTED
//
// MessageText:
//
//  Alarm %1 noted.
//
#define CTS_ALARM_NOTED                  0x000B000CL

//  Alarm Killed
//
// MessageId: CTS_ALARM_KILLED
//
// MessageText:
//
//  Alarm %1 killed.
//
#define CTS_ALARM_KILLED                 0x000B000DL

//  CTS_FACILITY_TRANSPORT
// include  "CTEvtTransport.mc"
//  CTS_FACILITY_DMA
// include  "CTEvtDma.mc"
//  CTS_FACILITY_CHAOS

//  CTEvtCHAOS.mc
//
//  -------------------- CHAOS error codes ----------------------

//  ?
//
// MessageId: CTS_CHAOS_DMA_TIMEOUT
//
// MessageText:
//
//  Timeout attempting transfer to IOP in slot %0 at address %1 of size %2.
//
#define CTS_CHAOS_DMA_TIMEOUT            0x800C0001L

//  ?
//
// MessageId: CTS_CHAOS_DMA_ERROR
//
// MessageText:
//
//  Bus error attempting transfer to IOP in slot %0 at address %1 of size %2.
//
#define CTS_CHAOS_DMA_ERROR              0x800C0002L

//  ?
//
// MessageId: CTS_CHAOS_TPT_SEND_CONGESTION
//
// MessageText:
//
//  Unable to allocate a buffer of size %0 for message send to slot %1.
//
#define CTS_CHAOS_TPT_SEND_CONGESTION    0x800C0003L

//  ?
//
// MessageId: CTS_CHAOS_TPT_REPLY_CONGESTION
//
// MessageText:
//
//  Unable to allocate a buffer of size %0 for message reply to slot %1.
//
#define CTS_CHAOS_TPT_REPLY_CONGESTION   0x800C0004L

//  ?
//
// MessageId: CTS_CHAOS_TPT_POST_FAIL
//
// MessageText:
//
//  Unable to post a message to slot %0.
//
#define CTS_CHAOS_TPT_POST_FAIL          0x800C0005L

//  ?
//
// MessageId: CTS_CHAOS_TPT_IOP_NOT_READY
//
// MessageText:
//
//  Processor in slot %0 not active, state=%1.
//
#define CTS_CHAOS_TPT_IOP_NOT_READY      0x800C0006L

//  ?
//
// MessageId: CTS_CHAOS_TPT_IOP_INACCESSIBLE
//
// MessageText:
//
//  Processor in slot %0 is inaccessible.
//
#define CTS_CHAOS_TPT_IOP_INACCESSIBLE   0x800C0007L

//  ?
//
// MessageId: CTS_CHAOS_TPT_IOP_FAILED
//
// MessageText:
//
//  Processor in slot %0 has failed.
//
#define CTS_CHAOS_TPT_IOP_FAILED         0x800C0008L

//  ?
//
// MessageId: CTS_CHAOS_TPT_MESSAGE_CORRUPT
//
// MessageText:
//
//  A corrupt message has been received from slot %0.
//
#define CTS_CHAOS_TPT_MESSAGE_CORRUPT    0x800C0009L

//  ?
//
// MessageId: CTS_CHAOS_TPT_SERVICE_TERMINATED
//
// MessageText:
//
//  The service no longer exists.
//
#define CTS_CHAOS_TPT_SERVICE_TERMINATED 0x800C000AL

//  ?
//
// MessageId: CTS_CHAOS_DEVICE_NOT_AVAILABLE
//
// MessageText:
//
//  Device not available
//
#define CTS_CHAOS_DEVICE_NOT_AVAILABLE   0x800C000BL

//  ?
//
// MessageId: CTS_CHAOS_INSUFFICIENT_RESOURCE_SOFT
//
// MessageText:
//
//  Not enough software resources are available e.g. memory
//
#define CTS_CHAOS_INSUFFICIENT_RESOURCE_SOFT 0x800C000CL

//  ?
//
// MessageId: CTS_CHAOS_UNKNOWN_FUNCTION
//
// MessageText:
//
//  The message function code is not recognized by this service
//
#define CTS_CHAOS_UNKNOWN_FUNCTION       0x800C000DL

//  ?
//
// MessageId: CTS_CHAOS_BAD_KEY
//
// MessageText:
//
//  A payload field value is not valid
//
#define CTS_CHAOS_BAD_KEY                0x800C000EL

//  ?
//
// MessageId: CTS_CHAOS_TIMEOUT
//
// MessageText:
//
//  The specified time interval has elapsed
//
#define CTS_CHAOS_TIMEOUT                0x800C000FL

//  ?
//
// MessageId: CTS_CHAOS_INVALID_PARAMETER
//
// MessageText:
//
//  A payload field value is not valid
//
#define CTS_CHAOS_INVALID_PARAMETER      0x800C0010L

//  ?
//
// MessageId: CTS_CHAOS_INVALID_MESSAGE_FLAGS
//
// MessageText:
//
//  Message flags are set inconsistently
//
#define CTS_CHAOS_INVALID_MESSAGE_FLAGS  0x800C0011L

//  ?
//
// MessageId: CTS_CHAOS_INVALID_INITIATOR_ADDRESS
//
// MessageText:
//
//  The message initiator cannot be found; initiator address fields may be corrupt
//
#define CTS_CHAOS_INVALID_INITIATOR_ADDRESS 0x800C0012L

//  ?
//
// MessageId: CTS_CHAOS_INVALID_TARGET_ADDRESS
//
// MessageText:
//
//  The message target cannot be found; message target address fields may be corrupt
//
#define CTS_CHAOS_INVALID_TARGET_ADDRESS 0x800C0013L

//  ?
//
// MessageId: CTS_CHAOS_INVALID_DID
//
// MessageText:
//
//  The specified Device Id is not valid
//
#define CTS_CHAOS_INVALID_DID            0x800C0014L

//  ?
//
// MessageId: CTS_CHAOS_INVALID_VDN
//
// MessageText:
//
//  The specified Virtual Device Number is not valid
//
#define CTS_CHAOS_INVALID_VDN            0x800C0015L

//  ?
//
// MessageId: CTS_CHAOS_CLASS_NOT_FOUND
//
// MessageText:
//
//  The specified class name was not linked with image
//
#define CTS_CHAOS_CLASS_NOT_FOUND        0x800C0016L

// CTS_FACILITY_UPGRADE
//
//  CTEvtUpgrade.mc - Error codes unique to the Upgrade Master.
//                Included by CTEvent.mc.  See that file for more info.
//
//
//No room to associate image with board
//
// MessageId: CTS_UPGRADE_NO_EMPTY_IMAGE
//
// MessageText:
//
//  No room to associate image with board
//
#define CTS_UPGRADE_NO_EMPTY_IMAGE       0x000D0001L

//Image is not associated with slot
//
// MessageId: CTS_UPGRADE_IMAGE_NOT_ASSOCIATED_WITH_SLOT
//
// MessageText:
//
//  Image is not associated with slot
//
#define CTS_UPGRADE_IMAGE_NOT_ASSOCIATED_WITH_SLOT 0x000D0002L

//Image not found
//
// MessageId: CTS_UPGRADE_IMAGE_NOT_FOUND
//
// MessageText:
//
//  Image not found
//
#define CTS_UPGRADE_IMAGE_NOT_FOUND      0x000D0003L

//Image busy (associated with slot)
//
// MessageId: CTS_UPGRADE_IMAGE_BUSY
//
// MessageText:
//
//  Image busy (associated with slot)
//
#define CTS_UPGRADE_IMAGE_BUSY           0x000D0004L

//Image is primary
//
// MessageId: CTS_UPGRADE_IMAGE_PRIMARY
//
// MessageText:
//
//  Image is primary
//
#define CTS_UPGRADE_IMAGE_PRIMARY        0x000D0005L

//Image is current
//
// MessageId: CTS_UPGRADE_IMAGE_CURRENT
//
// MessageText:
//
//  Image is current
//
#define CTS_UPGRADE_IMAGE_CURRENT        0x000D0006L

//Image is default
//
// MessageId: CTS_UPGRADE_IMAGE_DEFAULT
//
// MessageText:
//
//  Image is default.
//
#define CTS_UPGRADE_IMAGE_DEFAULT        0x000D0007L

//Image is already associated
//
// MessageId: CTS_UPGRADE_IMAGE_ALREADY_ASSOCIATED
//
// MessageText:
//
//  Image is already associated with this slot.
//
#define CTS_UPGRADE_IMAGE_ALREADY_ASSOCIATED 0x000D0008L

//CTS_FACILITY_FILESYS
//
//  CTEvtFileSys.mc - Error codes unique to the File System Master.
//                Included by CTEvent.mc.  See that file for more info.
//
//
//File system full
//
// MessageId: CTS_FILESYS_OUT_OF_MEMORY
//
// MessageText:
//
//  File system is full (out of memory)
//
#define CTS_FILESYS_OUT_OF_MEMORY        0x000E0001L

//File not found
//
// MessageId: CTS_FILESYS_FILE_NOT_FOUND
//
// MessageText:
//
//  File not found
//
#define CTS_FILESYS_FILE_NOT_FOUND       0x000E0002L

//File too large
//
// MessageId: CTS_FILESYS_FILE_TOO_LARGE
//
// MessageText:
//
//  File too large
//
#define CTS_FILESYS_FILE_TOO_LARGE       0x000E0003L

//CTS_FACILITY_UTILITY
//
//  CTEvtUtils.mc - Error codes returned by utility routines found in
//                Odyssey\Util\. source code.
//
//                Included by CTEvent.mc.  See that file for more info.
//
//
//  attempted to start an already running watchdog timer
//
// MessageId: CTS_UTIL_WATCHDOG_ALREADY_RUNNING
//
// MessageText:
//
//  Attempted to start an already-running watchdog timer.
//
#define CTS_UTIL_WATCHDOG_ALREADY_RUNNING 0x800F0001L

//  attempted to reset a stopped (or unstarted) watchdog timer
//
// MessageId: CTS_UTIL_WATCHDOG_NOT_RUNNING
//
// MessageText:
//
//  Watchdog timer operation requires that timer be running.
//
#define CTS_UTIL_WATCHDOG_NOT_RUNNING    0x800F0002L

//CTS_FACILITY_FLASH
//
//  CTEvtFlash.mc - Error codes unique to the FlashStorage Storage Manager and device driver.
//                Included by CTEvent.mc.  See that file for more info.
//
//
//  Invalid handle passed to flash storage method.
//
// MessageId: CTS_FLASH_INVALID_HANDLE
//
// MessageText:
//
//  Invalid handle passed to flash storage method.
//
#define CTS_FLASH_INVALID_HANDLE         0x80100001L

//  No context could be allocated.
//
// MessageId: CTS_FLASH_NO_CONTEXT
//
// MessageText:
//
//  No context could be allocated.
//
#define CTS_FLASH_NO_CONTEXT             0x80100002L

//  Invalid handle passed to flash storage method.
//
// MessageId: CTS_FLASH_INVALID_CONFIG_VERSION
//
// MessageText:
//
//  Handle passed to flash storage method is invalid.
//
#define CTS_FLASH_INVALID_CONFIG_VERSION 0x80100003L

//  Invalid page state found in page table.
//
// MessageId: CTS_FLASH_INVALID_PAGE_STATE
//
// MessageText:
//
//  Invalid page state found in page table.
//
#define CTS_FLASH_INVALID_PAGE_STATE     0x80100004L

//  FlashStorage table of contents is too big.
//
// MessageId: CTS_FLASH_TOC_TOO_BIG
//
// MessageText:
//
//  The flash table of contents will not fit in one page.
//
#define CTS_FLASH_TOC_TOO_BIG            0x80100005L

//  No erased pages are available.
//
// MessageId: CTS_FLASH_NO_ERASED_PAGES
//
// MessageText:
//
//  No erased pages are available.
//
#define CTS_FLASH_NO_ERASED_PAGES        0x80100006L

//  Invalid table of contents.
//
// MessageId: CTS_FLASH_INVALID_TOC
//
// MessageText:
//
//  Invalid flash table of contents.
//
#define CTS_FLASH_INVALID_TOC            0x80100007L

//  Invalid table of contents version.
//
// MessageId: CTS_FLASH_INVALID_TOC_VERSION
//
// MessageText:
//
//  Invalid flash table of contents version.
//
#define CTS_FLASH_INVALID_TOC_VERSION    0x80100008L

//  Available replacement pages has fallen below threshold.
//
// MessageId: CTS_FLASH_REPLACEMENT_PAGE_THRESHOLD
//
// MessageText:
//
//  Available replacement pages has fallen below threshold.
//
#define CTS_FLASH_REPLACEMENT_PAGE_THRESHOLD 0x80100009L

//  No replacement pages are available.
//
// MessageId: CTS_FLASH_NO_REPLACEMENT_PAGES
//
// MessageText:
//
//  No replacement pages are available.
//
#define CTS_FLASH_NO_REPLACEMENT_PAGES   0x8010000AL

//  Table of contents could not be written.
//
// MessageId: CTS_FLASH_WRITE_TABLE_OF_CONTENTS
//
// MessageText:
//
//  Table of contents could not be written.
//
#define CTS_FLASH_WRITE_TABLE_OF_CONTENTS 0x8010000BL

//  Timeout on flash operation.
//
// MessageId: CTS_FLASH_TIMEOUT
//
// MessageText:
//
//  Timeout occurred on a flash operation.
//
#define CTS_FLASH_TIMEOUT                0x8010000CL

//  Timeout on erase.
//
// MessageId: CTS_FLASH_ERASE_TIMEOUT
//
// MessageText:
//
//  Timeout occurred on an erase operation.
//
#define CTS_FLASH_ERASE_TIMEOUT          0x8010000DL

//  Timeout on read.
//
// MessageId: CTS_FLASH_READ_TIMEOUT
//
// MessageText:
//
//  Timeout occurred on a read operation.
//
#define CTS_FLASH_READ_TIMEOUT           0x8010000EL

//  Timeout on write.
//
// MessageId: CTS_FLASH_WRITE_TIMEOUT
//
// MessageText:
//
//  Timeout occurred on a write operation.
//
#define CTS_FLASH_WRITE_TIMEOUT          0x8010000FL

//  Device error detected on erase operation.
//
// MessageId: CTS_FLASH_ERASE_ERROR
//
// MessageText:
//
//  Device error detected on erase operation.
//
#define CTS_FLASH_ERASE_ERROR            0x80100010L

//  Device error detected on flash operation.
//
// MessageId: CTS_FLASH_DEVICE_ERROR
//
// MessageText:
//
//  Device error detected on flash operation.
//
#define CTS_FLASH_DEVICE_ERROR           0x80100011L

//  Verify error detected.
//
// MessageId: CTS_FLASH_VERIFY
//
// MessageText:
//
//  Miscompare on verify operation.
//
#define CTS_FLASH_VERIFY                 0x80100012L

//  Verify error detected on write operation.
//
// MessageId: CTS_FLASH_VERIFY_WRITE
//
// MessageText:
//
//  Verify error detected on a write operation.
//
#define CTS_FLASH_VERIFY_WRITE           0x80100013L

//  Verify error detected on erase operation.
//
// MessageId: CTS_FLASH_VERIFY_ERASE
//
// MessageText:
//
//  Verify error detected on an erase operation.
//
#define CTS_FLASH_VERIFY_ERASE           0x80100014L

//  Page not erased.
//
// MessageId: CTS_FLASH_PAGE_NOT_ERASED
//
// MessageText:
//
//  Attempt to write to a page that is not erased -- internal error.
//
#define CTS_FLASH_PAGE_NOT_ERASED        0x80100015L

//  Data miscompare.
//
// MessageId: CTS_FLASH_DATA_MISCOMPARE
//
// MessageText:
//
//  Data miscompare -- data read does not match data expected.
//
#define CTS_FLASH_DATA_MISCOMPARE        0x80100016L

//  Not enough memory.
//
// MessageId: CTS_FLASH_MEM_SIZE_TOO_SMALL
//
// MessageText:
//
//  Memory allocated for the flash service is too small.
//
#define CTS_FLASH_MEM_SIZE_TOO_SMALL     0x80100017L

//  Bad block table already exists.
//
// MessageId: CTS_FLASH_BAD_BLOCK_TABLE_ALREADY_EXISTS
//
// MessageText:
//
//  Bad block table cannot be created because it already exists.
//
#define CTS_FLASH_BAD_BLOCK_TABLE_ALREADY_EXISTS 0x80100018L

//  Bad block table does not exist.
//
// MessageId: CTS_FLASH_BAD_BLOCK_TABLE_DOES_NOT_EXIST
//
// MessageText:
//
//  FlashStorage storage cannot be opened because the bad block table could not be found.
//
#define CTS_FLASH_BAD_BLOCK_TABLE_DOES_NOT_EXIST 0x80100019L

//  FlashStorage storage was not closed the last time it was opened.
//
// MessageId: CTS_FLASH_NEVER_CLOSED
//
// MessageText:
//
//  FlashStorage storage was not closed the last time it was opened.
//
#define CTS_FLASH_NEVER_CLOSED           0x8010001AL

//  Attempt was made to write over the bad block table -- internal error.
//
// MessageId: CTS_FLASH_WRITING_BAD_BLOCK_TABLE
//
// MessageText:
//
//  Attempt was made to write over the bad block table -- internal error.
//
#define CTS_FLASH_WRITING_BAD_BLOCK_TABLE 0x8010001BL

//  Page map invalid.
//
// MessageId: CTS_FLASH_INVALID_PAGE_MAP_VIRTUAL_ADDRESS
//
// MessageText:
//
//  Page map read from flash storage is invalid.
//  Real address %1 has virtual address %2 > number of virtual pages %3
//
#define CTS_FLASH_INVALID_PAGE_MAP_VIRTUAL_ADDRESS 0x8010001CL

//  Page map invalid.
//
// MessageId: CTS_FLASH_INVALID_PAGE_MAP_DUPLICATE_VIRTUAL_ADDRESS
//
// MessageText:
//
//  Page map read from flash storage is invalid.
//  Virtual address %1 already mapped to real address %2
//  Real address %2 mapped to virtual address %3
//
#define CTS_FLASH_INVALID_PAGE_MAP_DUPLICATE_VIRTUAL_ADDRESS 0x8010001DL

//  Page map invalid.
//
// MessageId: CTS_FLASH_INVALID_PAGE_MAP_STATE
//
// MessageText:
//
//  Page map read from flash storage is invalid.
//  Real address %1 has invalid state %2.
//
#define CTS_FLASH_INVALID_PAGE_MAP_STATE 0x8010001EL

//  FlashStorage storage not open.
//
// MessageId: CTS_FLASH_NOT_OPEN
//
// MessageText:
//
//  FlashStorage storage method was called, but flash storage is not open.
//
#define CTS_FLASH_NOT_OPEN               0x8010001FL

//  No replacement pages available.
//
// MessageId: CTS_FLASH_REPLACEMENT_PAGES_ERASING
//
// MessageText:
//
//  No replacement pages are available.
//  An erase operation is keeping the only available pages busy.
//
#define CTS_FLASH_REPLACEMENT_PAGES_ERASING 0x80100020L

//  No good blocks to allocate system structure.
//
// MessageId: CTS_FLASH_NO_GOOD_BLOCKS
//
// MessageText:
//
//  Internal error -- there are not enough good blocks available to allocate a system block.
//
#define CTS_FLASH_NO_GOOD_BLOCKS         0x80100021L

//  No good blocks to write page map.
//
// MessageId: CTS_FLASH_NO_GOOD_BLOCKS_PAGE_MAP
//
// MessageText:
//
//  The page map could not be written because there are not enough good blocks available.
//
#define CTS_FLASH_NO_GOOD_BLOCKS_PAGE_MAP 0x80100022L

//  No good blocks to write table of contents.
//
// MessageId: CTS_FLASH_NO_GOOD_BLOCKS_TOC
//
// MessageText:
//
//  The table of contents could not be written because there are not enough good blocks available.
//
#define CTS_FLASH_NO_GOOD_BLOCKS_TOC     0x80100023L

//  No good blocks to allocate basic assurance test.
//
// MessageId: CTS_FLASH_NO_GOOD_BLOCKS_BAT
//
// MessageText:
//
//  The basic assurance test block could not be allocated because there are not enough good blocks.
//
#define CTS_FLASH_NO_GOOD_BLOCKS_BAT     0x80100024L

//  Bad flash address detected.
//
// MessageId: CTS_FLASH_BAD_FLASH_ADDRESS
//
// MessageText:
//
//  Internal error -- bad flash address detected.
//
#define CTS_FLASH_BAD_FLASH_ADDRESS      0x80100025L

//  Bad flash address detected in page map table.
//
// MessageId: CTS_FLASH_BAD_PAGE_MAP_TABLE_ADDRESS
//
// MessageText:
//
//  Bad page map table address %1 at index %2.
//
#define CTS_FLASH_BAD_PAGE_MAP_TABLE_ADDRESS 0x80100026L

//  Bad flash address detected in page map table.
//
// MessageId: CTS_FLASH_BAD_PAGE_MAP_ADDRESS
//
// MessageText:
//
//  Bad page map address %1 in page map table at index %2.
//
#define CTS_FLASH_BAD_PAGE_MAP_ADDRESS   0x80100027L

//  Invalid unit code returned by device.
//
// MessageId: CTS_FLASH_INVALID_UNIT
//
// MessageText:
//
//  Invalid unit code in array %1 column %2 bank %3,  register value = %4.
//
#define CTS_FLASH_INVALID_UNIT           0x80100028L

//  ECC error detected.
//
// MessageId: CTS_FLASH_ECC
//
// MessageText:
//
//  An ECC error was detected.
//
#define CTS_FLASH_ECC                    0x80100029L

//  ECC error detected and corrected.
//
// MessageId: CTS_FLASH_ECC_CORRECTED
//
// MessageText:
//
//  An ECC error was detected and corrected.
//
#define CTS_FLASH_ECC_CORRECTED          0x4010002AL

//  Write data parity error.
//
// MessageId: CTS_FLASH_WRITE_DATA_PARITY
//
// MessageText:
//
//  Write data parity error detected.
//
#define CTS_FLASH_WRITE_DATA_PARITY      0x8010002BL

//  Write data overflow error.
//
// MessageId: CTS_FLASH_WRITE_DATA_OVERFLOW
//
// MessageText:
//
//  Write data overflow error detected.
//
#define CTS_FLASH_WRITE_DATA_OVERFLOW    0x8010002CL

//  Read data underflow error detected.
//
// MessageId: CTS_FLASH_READ_DATA_UNDERFLOW
//
// MessageText:
//
//  Read data underflow error detected.
//
#define CTS_FLASH_READ_DATA_UNDERFLOW    0x8010002DL

//  Command overwrite error detected.
//
// MessageId: CTS_FLASH_COMMAND_OVERWRITE
//
// MessageText:
//
//  Command overwrite error detected.
//
#define CTS_FLASH_COMMAND_OVERWRITE      0x8010002EL

//  Invalid flash device detected.
//
// MessageId: CTS_FLASH_INVALID_FLASH_DEVICE
//
// MessageText:
//
//  Invalid flash device, unit ID = %1.
//
#define CTS_FLASH_INVALID_FLASH_DEVICE   0x8010002FL

//  Invalid buffer alignment.
//
// MessageId: CTS_FLASH_BUFFER_NOT_64_BYTE_ALIGNED
//
// MessageText:
//
//  Buffer passed into flash storage method is not aligned on a 64-byte boundary.
//
#define CTS_FLASH_BUFFER_NOT_64_BYTE_ALIGNED 0x80100030L

//  Transfer length error.
//
// MessageId: CTS_FLASH_TRANSFER_LENGTH
//
// MessageText:
//
//  Transfer length error detected.
//
#define CTS_FLASH_TRANSFER_LENGTH        0x80100031L

//  Verify error detected on read operation.
//
// MessageId: CTS_FLASH_VERIFY_READ
//
// MessageText:
//
//  Verify error detected on a read operation.
//
#define CTS_FLASH_VERIFY_READ            0x80100032L

//  Invalid column.
//
// MessageId: CTS_FLASH_INVALID_COLUMN
//
// MessageText:
//
//  Invalid column in array %1 column %2.
//
#define CTS_FLASH_INVALID_COLUMN         0x80100033L

//  Invalid maker code.
//
// MessageId: CTS_FLASH_INVALID_MAKER_CODE
//
// MessageText:
//
//  Invalid maker code in array %1 column %2 bank %3, register value = %4.
//
#define CTS_FLASH_INVALID_MAKER_CODE     0x80100034L

//  Invalid unit code.
//
// MessageId: CTS_FLASH_INVALID_UNIT_CODE
//
// MessageText:
//
//  Invalid unit code in array %1 column %2 bank %3, register value = %4.
//
#define CTS_FLASH_INVALID_UNIT_CODE      0x80100035L

//  Invalid unit and maker code.
//
// MessageId: CTS_FLASH_INVALID_UNIT_MAKER_CODE
//
// MessageText:
//
//  Invalid unit and maker code .
//
#define CTS_FLASH_INVALID_UNIT_MAKER_CODE 0x80100036L

//  Too many bad spots.
//
// MessageId: CTS_FLASH_TOO_MANY_BAD_SPOTS
//
// MessageText:
//
//  Block has too many bad spots to be used.
//
#define CTS_FLASH_TOO_MANY_BAD_SPOTS     0x80100037L

//  No memory.
//
// MessageId: CTS_FLASH_NO_MEMORY
//
// MessageText:
//
//  No memory could be allocated from Mem object.
//
#define CTS_FLASH_NO_MEMORY              0x80100038L

//  NFlash driver device write error.
//
// MessageId: CTS_FLASH_DEVICE_WRITE
//
// MessageText:
//
//  NFlash driver device write error..
//
#define CTS_FLASH_DEVICE_WRITE           0x80100039L

//  NFlash driver device read error.
//
// MessageId: CTS_FLASH_DEVICE_READ
//
// MessageText:
//
//  NFlash driver device write error..
//
#define CTS_FLASH_DEVICE_READ            0x8010003AL

//  NFlash driver device busy error.
//
// MessageId: CTS_FLASH_DEVICE_BUSY
//
// MessageText:
//
//  NFlash driver device write error..
//
#define CTS_FLASH_DEVICE_BUSY            0x8010003BL

//  NFlash driver device semaphore error.
//
// MessageId: CTS_FLASH_DEVICE_SEMA
//
// MessageText:
//
//  NFlash driver device unable to create semaphore.
//
#define CTS_FLASH_DEVICE_SEMA            0x8010003CL

//  NFlash driver device reset error.
//
// MessageId: CTS_FLASH_DEVICE_RESET
//
// MessageText:
//
//  NFlash driver device reset error.
//
#define CTS_FLASH_DEVICE_RESET           0x8010003DL

//  NFlash driver device erase error.
//
// MessageId: CTS_FLASH_DEVICE_ERASE
//
// MessageText:
//
//  NFlash driver device erase error.
//
#define CTS_FLASH_DEVICE_ERASE           0x8010003EL

//  No memory could be allocated for TyDma .
//
// MessageId: CTS_FLASH_NO_MEMORY_TYDMA
//
// MessageText:
//
//  No memory could be allocated for TyDma.
//
#define CTS_FLASH_NO_MEMORY_TYDMA        0x8010003FL

//  End of SGL encountered before data.
//
// MessageId: CTS_FLASH_FLASH_SGL_OVERRUN
//
// MessageText:
//
//  End of SGL encountered before data.
//
#define CTS_FLASH_FLASH_SGL_OVERRUN      0x80100040L

//  No good blocks to write page map table.
//
// MessageId: CTS_FLASH_NO_GOOD_BLOCKS_PAGE_MAP_TABLE
//
// MessageText:
//
//  The page map table could not be written because there are not enough good blocks available.
//
#define CTS_FLASH_NO_GOOD_BLOCKS_PAGE_MAP_TABLE 0x80100041L

//  CTS_FACILITY_FCM
//
//  CTEvtFCM.mc - Error codes unique to the FCM interface DDM.
//                Included by CTEvent.mc.  See that file for more info.
//
//
//
// MessageId: CTS_FCM_ERR_INVALID_COMMAND
//
// MessageText:
//
//  The Fibre Channel Master received an invalid command in a request.
//
#define CTS_FCM_ERR_INVALID_COMMAND      0x80110001L

//
// MessageId: CTS_FCM_ERR_LOOP_ALREADY_UP
//
// MessageText:
//
//  The fibre channel loop is already up.
//
#define CTS_FCM_ERR_LOOP_ALREADY_UP      0x80110002L

//
// MessageId: CTS_FCM_ERR_LOOP_ALREADY_DOWN
//
// MessageText:
//
//  The Fibre Channel loop is already down.
//
#define CTS_FCM_ERR_LOOP_ALREADY_DOWN    0x80110003L

//
// MessageId: CTS_FCM_EVT_LOOP_LIP
//
// MessageText:
//
//  The Fibre Channel Loop %1 on slot %2 received a LIP
//
#define CTS_FCM_EVT_LOOP_LIP             0x80110004L

//
// MessageId: CTS_FCM_EVT_LOOP_DOWN
//
// MessageText:
//
//  The Fibre Channel Loop %1 on slot %2 is down.
//
#define CTS_FCM_EVT_LOOP_DOWN            0x80110005L

//
// MessageId: CTS_FCM_EVT_LOOP_UP
//
// MessageText:
//
//  The Fibre Channel Loop %1 on slot %2 is now up.
//
#define CTS_FCM_EVT_LOOP_UP              0x80110006L

//
// MessageId: CTS_FCM_EVT_NAC_SHUTDOWN
//
// MessageText:
//
//  The Nac in slot %1 is down.
//
#define CTS_FCM_EVT_NAC_SHUTDOWN         0x80110007L

