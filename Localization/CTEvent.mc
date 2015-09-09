;//
;//  CTEvent.mc - Master OOS event code definition file.
;//               Contains event code and message definitions
;//               for the Odyssey OS and DDMs.
;//
;//               Third-party DDMs need separate facility codes
;//               (see below).  Do we define them, or do they?
;//
;//               Following are definitions of the common elements
;//               used to define message codes.
;//               After these, a few common messages are defined,
;//               and then the various facility-specific message
;//               files are included to define the remainder of the
;//               standard Odyssey event codes.
;//
;//

;
;/*
;
; The MessageIdTypedef keyword gives a typedef name that is used in a
; type cast for each message code in the generated include file. Each
; message code appears in the include file with the format: #define
; name ((type) 0xnnnnnnnn) The default value for type is empty, and no
; type cast is generated. It is the programmer's responsibility to
; specify a typedef statement in the application source code to define
; the type. The type used in the typedef must be large enough to
; accomodate the entire 32-bit message code.
;

;//MessageIdTypedef=DWORD

;
; The SeverityNames keyword defines the set of names that are allowed
; as the value of the Severity keyword in the message definition. The
; set is delimited by left and right parentheses. Associated with each
; severity name is a number that, when shifted left by 30, gives the
; bit pattern to logical-OR with the Facility value and MessageId
; value to form the full 32-bit message code. The default value of
; this keyword is:
;
; SeverityNames=(
;   Informational=0x0
;   Warning=0x1
;   Error=0x2
;   Internal=0x3
;   )
;
; Severity values occupy the high two bits of a 32-bit message code.
; Any severity value that does not fit in two bits is an error. The
; severity codes can be given symbolic names by following each value
; with :name

SeverityNames=(Informational=0x0:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x1:STATUS_SEVERITY_WARNING
               Error=0x2:STATUS_SEVERITY_ERROR
               Internal=0x3:STATUS_SEVERITY_INTERNAL
              )

;
; The FacilityNames keyword defines the set of names that are allowed
; as the value of the Facility keyword in the message definition. The
; set is delimited by left and right parentheses. Associated with each
; facility name is a facility code.
;
; Facility codes occupy the low order 12 bits of the high order
; 16-bits of a 32-bit message code. Any facility code that does not
; fit in 12 bits is an error. This allows for 4,096 facility codes.
; The first 256 codes are reserved for use by the system software. The
; facility codes can be given symbolic names by following each value
; with :name

FacilityNames=(System=0x0:CTS_FACILITY_SYSTEM
               OS=0x1:CTS_FACILITY_OS
               Messaging=0x2:CTS_FACILITY_MESSAGING
               Runtime=0x3:CTS_FACILITY_RUNTIME
               BSA=0x4:CTS_FACILITY_BSA
               RAID=0x5:CTS_FACILITY_RAID
               CMB=0x6:CTS_FACILITY_CMB
               SSAPI=0x7:CTS_FACILITY_SSAPI
               PTS=0x8:CTS_FACILITY_PTS
               VCM=0x9:CTS_FACILITY_VCM
               HotSwap=0xA:CTS_FACILITY_HOTSWAP
               Alarm=0xB:CTS_FACILITY_ALARM
               CHAOS=0xC:CTS_FACILITY_CHAOS
               Upgrade=0xD:CTS_FACILITY_UPGRADE
               FileSys=0xE:CTS_FACILITY_FILESYS
               Utility=0xF:CTS_FACILITY_UTILITY
               FlashStorage=0x10:CTS_FACILITY_FLASH
			   FCM=0x11:CTS_FACILITY_FCM
               Facilities=0xFFF:CTS_FACILITY_FACIL_NAMES
              )

; *** If you add a facility code, please be sure to also add an entry for it
;     in the "Facilities" string section later in this file (near line 280).  ***

; Presently defined facility codes are:
;       CTS_FACILITY_SYSTEM - Really common things like "success" and heap
;       CTS_FACILITY_OS     - OS (i.e., Nucleus) error codes
;       CTS_FACILITY_MESSAGING - I2O error codes
;       CTS_FACILITY_RUNTIME - Runtime library error codes (if any)
;       CTS_FACILITY_BSA    - BSA driver codes
;       CTS_FACILITY_RAID   - RAID driver codes
;       CTS_FACILITY_CMB    - CMB interface [DDM] codes
;       CTS_FACILITY_SSAPI  - SSAPI layer codes
;       CTS_FACILITY_PTS  - Persistant table Sevice status codes
;       CTS_FACILITY_VCM  - Virtual Circuit Master status codes
;       CTS_FACILITY_HOTSWAP - Hot Swap Master status codes
;       CTS_FACILITY_ALARM - Alarm Master status codes
;       CTS_FACILITY_CHAOS  - CHAOS status codes
;       CTS_FACILITY_UPGRADE - Upgrade Master status codes
;       CTS_FACILITY_FILESYS - File System Master status codes
;       CTS_FACILITY_UTILITY - Returned by code in Odyssey\Util\.
;       CTS_FACILITY_FCM  - Fibre channel master status/event codes
;       CTS_FACILITY_FACIL_NAMES - Special section for defining textual names
;                                  of facility codes themselves.


;
; The LanguageNames keyword defines the set of names that are allowed
; as the value of the Language keyword in the message definition. The
; set is delimited by left and right parentheses. Associated with each
; language name is a number and a file name that are used to name the
; generated resource file that contains the messages for that
; language. The number corresponds to the language identifier to use
; in the resource table. The number is separated from the file name
; with a colon.
;
; The language numbers used here are adapted from NT's "locale" scheme,
; which concatentates two bit fields to form a 16 bit number.  Bits zero
; through nine are the primary language ID, and bits ten through fifteen
; are the sublanguage (or dialect) ID.  Full language IDs may be formed
; using NT's MAKELANGID macro (see WinNt.h).  Note that the sublanguage
; code is normally set to SUBLANG_DEFAULT ("user default"), which results
; in an apparent upper byte value of 0x04.
;
; The alphabetic language codes used for file extensions come from
; ISO 639-2/T, the ISO's "terminological" language coding scheme.
;

; English:  k_eLangEnglish   / MAKELANGID(LANG_ENGLISH,  SUBLANG_DEFAULT)
LanguageNames=(English=0x0409:msg_eng)                  
                                                        
; German:   k_eLangGerman    / MAKELANGID(LANG_GERMAN,   SUBLANG_DEFAULT)
LanguageNames=(German=0x0407:msg_deu)                   
                                                        
; French:   k_eLangFrench    / MAKELANGID(LANG_FRENCH,   SUBLANG_DEFAULT)
LanguageNames=(French=0x040C:msg_fra)                   
                                                        
; Italian:   k_eLangItalian  / MAKELANGID(LANG_ITALIAN,  SUBLANG_DEFAULT)
LanguageNames=(Italian=0x0410:msg_ita)                  
                                                        
; Spanish:   k_eLangSpanish  / MAKELANGID(LANG_SPANISH,  SUBLANG_DEFAULT)
LanguageNames=(Spanish=0x040A:msg_esp)

; Japanese:  k_eLangJapanese / MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT)
LanguageNames=(Japanese=0x0411:msg_jpn)

;
; Any new names in the source file which don't override the built-in
; names are added to the list of valid languages. This allows an
; application to support private languages with descriptive names.
;*/
;


;
;//  BEGIN COMMON EVENT CODE DEFINITIONS
;

;//  standard "success" code - shared by all facilities
MessageId=0x0
Severity=Informational
Facility=System
SymbolicName=CTS_SUCCESS
Language=English
Success
.
Language=German
Die Operation erfolgreich durchgefuehrt.
.

;//  standard "not implemented" code
MessageId=
Severity=Error
Facility=System
SymbolicName=CTS_NOT_IMPLEMENTED
Language=English
Function not implemented.
.
Language=German
Funktion nicht eingefuehrt.
.

;//  standard "out of memory" code (do we want a separate one for each heap?)
MessageId=
Severity=Error
Facility=System
SymbolicName=CTS_OUT_OF_MEMORY
Language=English
Out of memory.
.
Language=German
Aus Speicher heraus.
.

;//  event message renderer requested an unsupported language
MessageId=
Severity=Error
Facility=System
SymbolicName=CTS_UNSUPPORTED_LANGUAGE
Language=English
Unsupported language.
.
Language=German
Ungestützte Sprache.
.

;//  event message renderer encountered an invalid message code
MessageId=
Severity=Error
Facility=System
SymbolicName=CTS_INVALID_MESSAGE_ID
Language=English
Unknown event code.
.
Language=German
Unbekannter Fallcode.
.

;//  IOP Failed to change state indicating power up
MessageId=
Severity=Error
Facility=System
SymbolicName=CTS_IOP_POWER_TIMEOUT
Language=English
The IOP Failed to change state indicating power up.
.
Language=German
Unbekannter Fallcode.
.

;//  IOP Failed to change state indicating power up
MessageId=
Severity=Error
Facility=System
SymbolicName=CTS_IOP_ACTIVE_TIMEOUT
Language=English
The IOP Failed to change state indicating IOP active.
.
Language=German
Unbekannter Fallcode.
.

;//  IOP type unknown
MessageId=
Severity=Error
Facility=System
SymbolicName=CTS_IOP_TYPE_UNKNOWN
Language=English
An unknown IOP type was discovered by the BootMgr.
.
Language=German
Unbekannter Fallcode.
.

MessageId=
Severity=Error
Facility=System
SymbolicName=CTS_IOP_OOS_STATE_NONO
Language=English
An IOP cannot as requested be taken out of service; it is in state %d.
.
Language=German
Unbekannter Fallcode.
.

MessageId=
Severity=Error
Facility=System
SymbolicName=CTS_IOP_BAD_IMAGE_SIGNATURE
Language=English
IOP image has bad signature.
.
Language=German
Unbekannter Fallcode.
.

MessageId=
Severity=Error
Facility=System
SymbolicName=CTS_IOP_NO_SUCH_SLOT
Language=English
Requested action cannot be taken on  IOP.  Invalid slot.
.
Language=German
Unbekannter Fallcode.
.

MessageId=
Severity=Error
Facility=System
SymbolicName=CTS_IOP_POWERON_TIMED_OUT
Language=English
Request to power on Iop in slot %1 timed out.
.
Language=German
Unbekannter Fallcode.
.

MessageId=
Severity=Error
Facility=System
SymbolicName=CTS_IOP_UNABLE_TO_STOP_TRANSPORT
Language=English
Failure stopping transport from slot %1.
.
Language=German
Unbekannter Fallcode.
.

MessageId=
Severity=Error
Facility=System
SymbolicName=CTS_IOP_UNABLE_TO_RESTART_TRANSPORT
Language=English
Failure restarting transport from slot %1.
.
Language=German
Unbekannter Fallcode.
.

MessageId=
Severity=Error
Facility=System
SymbolicName=CTS_IOP_POWERON_STATE_NONO
Language=English
An IOP cannot as requested be powered on; it is in state %d.
.
Language=German
Unbekannter Fallcode.
.

MessageId=
Severity=Error
Facility=System
SymbolicName=CTS_IOP_ITS_STATE_NONO
Language=English
An IOP cannot as requested be put into service; it is in state %d.
.
Language=German
Unbekannter Fallcode.
.

MessageId=
Severity=Error
Facility=System
SymbolicName=CTS_IOP_LOCK_STATE_NONO
Language=English
An IOP cannot as requested be locked; it is in state %d.
.
Language=German
Unbekannter Fallcode.
.

MessageId=
Severity=Error
Facility=System
SymbolicName=CTS_IOP_UNLOCK_STATE_NONO
Language=English
An IOP cannot as requested be unlocked; it is in state %d.
.
Language=German
Unbekannter Fallcode.
.

;//  here's a message which will probably never be logged.  It exists
;//  to exercise the message compiler's escape character handling.
MessageId=
Severity=Error
Facility=System
SymbolicName=CTS_MESSAGE_COMPILER_TEST1
Language=English
"This" is at least 7%% of the %1%% solution; it's conceivable that we won't
catch all possible {likely} escaping problems, as in %2.%.
.
Language=German
???
.

;//  "invalid parameter" would be a logical global candidate, but it would
;//  be almost useless -- having one per facility code allows at least some
;//  means for identifying where in a stack trace the problem happened.

;//  Returned from Ddm::DoWork, must be numeric 13 to match old OsStatus.h
MessageId=0xD
Severity=Informational
Facility=System
SymbolicName=CTS_CHAOS_INAPPROPRIATE_FUNCTION
Language=English
The service does not process this message function code.
.
Language=German
Unbekannter Fallcode.
.

;// * * *  Facility code display strings.
;//        Note that the message IDs are exactly the (unshifted) facility
;//        codes being described (e.g., facility "OS" has ID 1).

;//  common system-wide codes, not associated with any facility
MessageId=0
Severity=Informational
Facility=Facilities
SymbolicName=CTS_FACIL_SYSTEM
Language=English
Odyssey system
.
Language=German
?
.

;//  for status reported by the operating system
MessageId=1
Severity=Informational
Facility=Facilities
SymbolicName=CTS_FACIL_OS
Language=English
Operating System
.
Language=German
?
.

;//  the CHAOS transport/messaging layer (used to be I2O)
MessageId=2
Severity=Informational
Facility=Facilities
SymbolicName=CTS_FACIL_MESSAGING
Language=English
Transport
.
Language=German
?
.

;//  the compiler / Odyssey runtime library support, excluding CHAOS
MessageId=3
Severity=Informational
Facility=Facilities
SymbolicName=CTS_FACIL_RUNTIME
Language=English
Runtime library
.
Language=German
?
.

;//  the BSA disk driver family (??)
MessageId=4
Severity=Informational
Facility=Facilities
SymbolicName=CTS_FACIL_BSA
Language=English
Disk driver
.
Language=German
?
.

;//  the RAID driver family
MessageId=5
Severity=Informational
Facility=Facilities
SymbolicName=CTS_FACIL_RAID
Language=English
RAID driver
.
Language=German
?
.

;//  the Card Management Bus facility (does this have *any* user-visible name?)
MessageId=6
Severity=Informational
Facility=Facilities
SymbolicName=CTS_FACIL_CMB
Language=English
CMB subsystem
.
Language=German
?
.

;//  the SSAPI events and localized strings
MessageId=7
Severity=Informational
Facility=Facilities
SymbolicName=CTS_FACIL_SSAPI
Language=English
SSAPI layer
.
Language=German
?
.

;//  the Persistent Table Service facility
MessageId=8
Severity=Informational
Facility=Facilities
SymbolicName=CTS_FACIL_PTS
Language=English
Persistent Table Service
.
Language=German
?
.

;//  the Virtual Circuit Master
MessageId=9
Severity=Informational
Facility=Facilities
SymbolicName=CTS_FACIL_VCM
Language=English
Virtual Circuit Master
.
Language=German
?
.

;//  the Hot Swap Master
MessageId=0xA
Severity=Informational
Facility=Facilities
SymbolicName=CTS_FACIL_HOTSWAP
Language=English
Hot Swap Master
.
Language=German
?
.

;//  the Alarm Master
MessageId=0xB
Severity=Informational
Facility=Facilities
SymbolicName=CTS_FACIL_ALARM
Language=English
Alarm Master
.
Language=German
?
.

;//  the Upgrade Master
MessageId=0xC
Severity=Informational
Facility=Facilities
SymbolicName=CTS_FACIL_UPGRADE
Language=English
Image Upgrade Master
.
Language=German
?
.

;//  the CHAOS events and localized strings
MessageId=0xD
Severity=Informational
Facility=Facilities
SymbolicName=CTS_FACIL_CHAOS
Language=English
CHAOS services
.
Language=German
?
.

;//  the (Flash) File System
MessageId=0xE
Severity=Informational
Facility=Facilities
SymbolicName=CTS_FACIL_FILESYS
Language=English
Flash File System
.
Language=German
?
.

;//  utility code in Odyssey\Util\.
MessageId=0xF
Severity=Informational
Facility=Facilities
SymbolicName=CTS_FACIL_UTILITY
Language=English
Utility services
.
Language=German
?
.

;//  the Flash Storage System and driver
MessageId=0x10
Severity=Informational
Facility=Facilities
SymbolicName=CTS_FACIL_FLASH
Language=English
FlashStorage
.
Language=German
?
.

;//  the Fibre Channel Master
MessageId=0x11
Severity=Informational
Facility=Facilities
SymbolicName=CTS_FACIL_FCM
Language=English
Fibre Circuit Master
.
Language=German
?
.

;
;//  BEGIN EVENT CODE DEFINITION FILE INCLUDES
;

;//  CTS_FACILITY_OS:  OS facility codes
include  "CTEvtKernel.mc"

;//  CTS_FACILITY_MESSAGING:  Messaging facility codes
include  "CTEvtI2O.mc"

;//  CTS_FACILITY_RUNTIME:  Standard compiler runtime codes
;//*** none presently defined ***
;

;//  CTS_FACILITY_BSA:  Block Storage Access codes (raw disk driver)
include  "CTEvtBSA.mc"

;//  CTS_FACILITY_RAID:  RAID DDM codes
include  "CTEvtRAID.mc"

;//  CTS_FACILITY_CMB:  CMB interface DDM codes
include  "CTEvtCMB.mc"

;//  CTS_FACILITY_SSAPI:  SSAPI localized string and event codes
include  "CTEvtSSAPI.mc"

;//  CTS_FACILITY_PTS:  Persistant Table Service localized string and event codes
include  "CTEvtPTS.mc"

;//  CTS_FACILITY_VCM:  Virtual Circuit Master localized string and event codes
include  "CTEvtVCM.mc"

;//  CTS_FACILITY_HOTSWAP:  Hot Swap Master localized string & event codes
include  "CTEvtHotSwap.mc"

;//  Some event codes used for the System Log test driver.
include  "CTEvtTest.mc"

;// Event codes for the Alarm Master
include "CTEvtAlarmMaster.mc"

;//  CTS_FACILITY_TRANSPORT
;// include  "CTEvtTransport.mc"

;//  CTS_FACILITY_DMA
;// include  "CTEvtDma.mc"

;//  CTS_FACILITY_CHAOS
include  "CTEvtCHAOS.mc"

;// CTS_FACILITY_UPGRADE
include "CTEvtUpgrade.mc"

;//CTS_FACILITY_FILESYS
include "CTEvtFileSys.mc"

;//CTS_FACILITY_UTILITY
include "CtEvtUtils.mc"

;//CTS_FACILITY_FLASH
include "CtEvtFlash.mc"

;//  CTS_FACILITY_FCM
include  "CTEvtFCM.mc"
