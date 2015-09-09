;//
;//  CTEvtUtils.mc - Error codes returned by utility routines found in
;//                Odyssey\Util\. source code.
;//
;//                Included by CTEvent.mc.  See that file for more info.
;//
;//


;//  attempted to start an already running watchdog timer
MessageId=0x1
Severity=Error
Facility=Utility
SymbolicName=CTS_UTIL_WATCHDOG_ALREADY_RUNNING
Language=English
Attempted to start an already-running watchdog timer.
.
Language=German
?
.

;//  attempted to reset a stopped (or unstarted) watchdog timer
MessageId=
Severity=Error
Facility=Utility
SymbolicName=CTS_UTIL_WATCHDOG_NOT_RUNNING
Language=English
Watchdog timer operation requires that timer be running.
.
Language=German
?
.


