;//
;//  CTEvtHotSwap.mc - Error codes unique to the Hot Swap Master DDM.
;//                Included by CTEvent.mc.  See that file for more info.
;//
;//


;//  the DDM received an invalid parameter in a request message
MessageId=0x1
Severity=Error
Facility=HotSwap
SymbolicName=CTS_HSW_INVALID_PARAMETER
Language=English
Invalid parameter.
.
Language=German
?
.

;//  the specified drive is in use, and can't be released from its bay
MessageId=
Severity=Error
Facility=HotSwap
SymbolicName=CTS_HSW_DRIVE_IN_USE
Language=English
Drive is part of a storage configuration, and cannot be released.
.
Language=German
?
.

