;//
;//  CTErrPTS.mc - Persistant Table Service status codes.
;//
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
;   Success=0x0
;   Informational=0x1
;   Warning=0x2
;   Error=0x3
;   )
;
; Severity values occupy the high two bits of a 32-bit message code.
; Any severity value that does not fit in two bits is an error. The
; severity codes can be given symbolic names by following each value
; with :name

SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
              )

;
; The FacilityNames keyword defines the set of names that are allowed
; as the value of the Facility keyword in the message definition. The
; set is delimited by left and right parentheses. Associated with each
; facility name is a number that, when shift it left by 16 bits, gives
; the bit pattern to logical-OR with the Severity value and MessageId
; value to form the full 32-bit message code. The default value of
; this keyword is:
;
; FacilityNames=(
;   System=0x0FF
;   Application=0xFFF
;   )
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
              )

; Presently defined facility codes are:
;       CTS_FACILITY_SYSTEM - Really common things like "success" and heap
;       CTS_FACILITY_OS     - OS (i.e., Nucleus) error codes
;       CTS_FACILITY_MESSAGING - I2O error codes
;       CTS_FACILITY_RUNTIME - Runtime library error codes (if any)
;       CTS_FACILITY_BSA    - BSA driver codes
;       CTS_FACILITY_RAID   - RAID driver codes
;       CTS_FACILITY_PTS   - Persistant Table Service codes
;       CTS_FACILITY_PHS   - Performance Health Status codes


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
; which concatentates two bytes.  The least significant byte is the
; primary language ID, and the most significant ID is the sublanguage
; (or dialect) ID.  Full language IDs may be formed using NT's MAKELANGID.
; [For some reason, the default English ID is 409, not 009.  Trying to
;  change it causes a compiler warning.]
;

LanguageNames=(English=0x0409:COM_0409)
LanguageNames=(German=0x0007:COM_0007)

;
; Any new names in the source file which don't override the built-in
; names are added to the list of valid languages. This allows an
; application to support private languages with descriptive names.
;*/
;




;//  standard "success" code - shared by all facilities
MessageId=0x0
Severity=Success
Facility=System
SymbolicName=CTS_SUCCESS
Language=English
Success
.
Language=German
Die Operation erfolgreich durchgefuehrt.
.

;//  Can't alloc Memory for ReadTable
MessageId=0x01
Severity=Error
Facility=PTS
SymbolicName=CTS_NO_MEM_FOR_READTABLE
Language=English
Cannot allocate memory for ReadTable operation on table:
.
Language=German
Sprechen sie Deutch.
.

;//  Next error goes here.  (Don't forget to copy this template first.)
MessageId=
Severity=Error
Facility=System
SymbolicName=CTS_NEXT_PTS_ERROR
Language=English
You Speak English?
.
Language=German
Sprechen sie Deutch?
.

