;//
;//  CTEvtCMB.mc - Error codes unique to the CMB interface DDM.
;//                Included by CTEvent.mc.  See that file for more info.
;//
;//


;//  the DDM received an invalid parameter in a request message
MessageId=0x1
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_INVALID_PARAMETER
Language=English
Invalid parameter.
.
Language=German
?
.

;//  here's a special case of invalid parameter:  the IOP slot we're
;//  supposed to do something to doesn't have an IOP in it.
MessageId=
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_REQUESTED_IOP_SLOT_EMPTY
Language=English
Cannot control an empty IOP slot.
.
Language=German
?
.

;//  CMB communication failure - couldn't ask it to do something
MessageId=
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_CMA_REQ_FAILED
Language=English
Request to CMB microcontroller failed.
.
Language=German
?
.

;//  the IOP we're trying to deal with has "unknown" state: it might
;//  not be there at all, or it might be busted.
MessageId=
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_REQUESTED_IOP_UNRESPONSIVE
Language=English
IOP slot is not responding, and may be empty.
.
Language=German
?
.

;//  tried to send a message to an IOP's MIPS CPU, but the MIPS
;//  is presently powered down.
MessageId=
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_REQUESTED_IOP_POWERED_DOWN
Language=English
IOP is presently powered down.
.
Language=German
?
.

;//  IOP's MIPS CPU is not ready to receive unsolicited CMB requests
MessageId=
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_IOP_MIPS_NOT_READY
Language=English
IOP's MIPS CPU is not ready to receive unsolicited CMB requests.
.
Language=German
?
.

;//  CMA didn't recognize command in request packet
MessageId=
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_CMA_UNKNOWN_CMD
Language=English
CMB request not understood by destination microcontroller.
.
Language=German
?
.

;//  CMA reports bad parameter in request packet
MessageId=
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_CMA_BAD_PARAM
Language=English
CMB microcontroller reports bad parameter in request packet.
.
Language=German
?
.

;//  CMA failed to forward request on to its local MIPS CPU - timeout
MessageId=
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_CMA_MIPS_TIMEOUT
Language=English
CMB microcontroller reports target MIPS CPU unreachable (timeout).
.
Language=German
?
.

;//  Unrecognized NAK reason code returned in CMB response packet
MessageId=
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_CMA_UNKNOWN_NAK
Language=English
CMB microcontroller returned an unknown NAK reason code.
.
Language=German
?
.

;//  CMB transmit hardware interface is busy (try again later)
MessageId=
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_XMTR_BUSY
Language=English
CMB microcontroller transmit interface is busy.
.
Language=German
?
.

;//  CMB hw interface class has no unsolicited messages to return
MessageId=
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_NO_UNSOLICITED_MSG
Language=English
No unsolicited messages pending from CMB microcontroller.
.
Language=German
?
.

;//  Supplied buffer is too small for given CMB message
MessageId=
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_BUFFER_TOO_SMALL
Language=English
Supplied buffer is too small for given CMB message.
.
Language=German
?
.

;//  CMB transmit hardware interface failed to send
MessageId=
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_XMTR_ERROR
Language=English
CMB microcontroller transmit error.
.
Language=German
?
.

;//  tried to unlock an IOP's solenoid, to enable IOP removal,
;//  but IOP is presently flagged as running OS-level code.
MessageId=
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_REQUESTED_IOP_IN_USE
Language=English
IOP is presently running OS-level code.
.
Language=German
?
.

;//  wanted to talk to a DDH which appears to not be present in system
MessageId=
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_REQUESTED_DDH_NOT_PRESENT
Language=English
Requested drive bay's DDH is not present in the system.
.
Language=German
?
.

;//  wanted to talk to a DDH which appears to be unusable (not responding, etc.)
MessageId=
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_REQUESTED_DDH_NOT_USABLE
Language=English
Requested drive bay's DDH is not usable at this time.
.
Language=German
?
.

;//  PTS insert op failed in strange and wondrous way
MessageId=
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_PTS_INSERT_FAILED
Language=English
PTS insert attempt failed with no error return.
.
Language=German
?
.

;//  CMA reports it's busy awaiting response to unsolicited command
MessageId=
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_CMA_BUSY_UNSOL
Language=English
CMB microcontroller refused send; is awaiting reply to earlier command.
.
Language=German
?
.

;//  here's a special case of invalid parameter:  the DDH slot we're
;//  supposed to do something to doesn't have an DDH in it.
MessageId=
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_REQUESTED_DDH_SLOT_EMPTY
Language=English
Cannot control an empty DDH slot.
.
Language=German
?
.

;//  This one happens when a message gets lost somewhere between the
;//  CMB DDM and its local CMA.  This is a "should never happen" error.
MessageId=
Severity=Error
Facility=CMB
SymbolicName=CTS_CMB_LOCAL_AVR_TIMEOUT
Language=English
Communication with local CMB microcontroller timed out.
.
Language=German
?
.


