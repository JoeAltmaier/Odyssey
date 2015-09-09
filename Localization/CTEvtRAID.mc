;//
;//  CTErrRAID.mc - Error codes unique to the RAID HDM.
;//                 Included by CTEvent.mc.  See that file for more info.
;//
;//


;//
;//		ARRAY RELATED EVENTS
;//

;//  New Array [name] created.
MessageId=1
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_ARRAY_ADDED
Language=English
Array %1 created successfully.
.
Language=German
?
.


;//  Array [name] deleted from the system.
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_ARRAY_DELETED
Language=English
Array %1 deleted.
.
Language=German
?
.

;//  Array [name] is now fault tolerant.
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_ARRAY_FAULT_TOLERANT
Language=English
Array %1 is now fault tolerant.
.
Language=German
?
.


;//  Array [name] is critical (non-redundant configuration)
MessageId=
Severity=Error
Facility=RAID
SymbolicName=CTS_RMSTR_ARRAY_CRITICAL
Language=English
Array %1 is critical (non-redundant configuration)
.
Language=German
?
.

;//  Array [name] is offline
MessageId=
Severity=Error
Facility=RAID
SymbolicName=CTS_RMSTR_ARRAY_OFFLINE
Language=English
Array %1 is off-line.
.
Language=German
?
.


;//
;//		SPARE RELATED EVENTS
;//

;//  Dedicated Spare [name] added to array [name]
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_DEDICATED_SPARE_ADDED
Language=English
Dedicated spare %1 added to Array %2.
.
Language=German
?
.


;//  Dedicated Spare [disk] added to array [name]
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_DEDICATED_SPARE_DISK_ADDED
Language=English
Dedicated spare [Slot=%1] added to Array %2.
.
Language=German
?
.


;//  Pool Spare [name] added to system spare pool
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_SYSTEM_POOL_SPARE_ADDED
Language=English
Pool Spare %1 added to system pool spare.
.
Language=German
?
.


;//  Pool Spare [Disk] added to system spare pool
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_SYSTEM_POOL_SPARE_DISK_ADDED
Language=English
Pool Spare [Slot=%1] added to system pool spare.
.
Language=German
?
.


;//  Pool Spare [name] added to for host
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_HOST_POOL_SPARE_ADDED
Language=English
Pool Spare %1 added to Host %2
.
Language=German
?
.


;//  Pool Spare [disk] added for host
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_HOST_POOL_SPARE_DISK_ADDED
Language=English
Host Pool Spare [Slot=%1] added to Host %2
.
Language=German
?
.


;//  Dedicated Spare [name] for array [name] deleted 
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_DEDICATED_SPARE_DELETED
Language=English
Dedicated spare %1 for array %2 deleted.
.
Language=German
?
.

;//  Dedicated Spare [disk] for array [name] deleted 
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_DEDICATED_SPARE_DISK_DELETED
Language=English
Dedicated spare [Slot=%1] for array %2 deleted.
.
Language=German
?
.


;//  System Pool Spare [name] deleted
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_SYSTEM_POOL_SPARE_DELETED
Language=English
System Pool Spare %1 deleted.
.
Language=German
?
.

;//  System Pool Spare [disk] deleted
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_SYSTEM_POOL_SPARE_DISK_DELETED
Language=English
System Pool Spare [Slot=%1] deleted.
.
Language=German
?
.


;//  Host Pool Spare [name] deleted from host [name]
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_HOST_POOL_SPARE_DELETED
Language=English
Host Pool Spare %1 deleted for Host %2.
.
Language=German
?
.

;//  Host Pool Spare [disk] deleted from host [name]
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_HOST_POOL_SPARE_DISK_DELETED
Language=English
Host Pool Spare [Slot=%1] deleted for host %2.
.
Language=German
?
.


;//  Dedicated spare [name] used to regenerate for array [name]
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_DEDICATED_SPARE_ACTIVATED
Language=English
Dedicated spare %1 used to regenerate array %2.
.
Language=German
?
.

;//  Dedicated spare [disk] used to regenerate for array [name]
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_DEDICATED_SPARE_DISK_ACTIVATED
Language=English
Dedicated spare [Slot=%1] used to regenerate array %2.
.
Language=German
?
.

;//  System Pool spare [name] used to regenerate for array [name]
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_SYSTEM_POOL_SPARE_ACTIVATED
Language=English
System Pool spare %1 used to regenerate array %2.
.
Language=German
?
.


;//  System Pool spare [disk] used to regenerate for array [name]
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_SYSTEM_POOL_SPARE_DISK_ACTIVATED
Language=English
System Pool spare [Slot=%1] used to regenerate array %2.
.
Language=German
?
.


;//  Host Pool spare [name] for host [name] used to regenerate for array [name]
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_HOST_POOL_SPARE_ACTIVATED
Language=English
Host Pool spare %1 for host %2 used to regenerate array %3
.
Language=German
?
.

;//  Host Pool spare [disk] for host [name] used to regenerate for array [name]
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_HOST_POOL_SPARE_DISK_ACTIVATED
Language=English
Host Pool spare [Slot=%1] for host %2 used to regenerate array %3.
.
Language=German
?
.


;//  No more valid spares available for array [name]
MessageId=
Severity=Warning
Facility=RAID
SymbolicName=CTS_RMSTR_NO_MORE_SPARES
Language=English
No more valid spares available for array %1.
.
Language=German
?
.


;//  No valid spare found for array [name]. Regenerate could not be started
MessageId=
Severity=Warning
Facility=RAID
SymbolicName=CTS_RMSTR_REGENERATE_NOT_STARTED
Language=English
No valid spare found for array %1. Regenerate could not be started.
.
Language=German
?
.



;//
;//		UTILITY RELATED EVENTS
;//

;//  Initialize started on array [name]
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_BKGD_INIT_STARTED
Language=English
Initialize started on array %1.
.
Language=German
?
.

;//  Initialize completed on array [name]
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_BKGD_INIT_COMPLETED
Language=English
Initialize completed on array %1.
.
Language=German
?
.


;//  Initialize aborted on array [name] due to I/O error.
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_BKGD_INIT_ABORTED_IOERROR
Language=English
Initialize aborted on array %1 due to I/O Error.
.
Language=German
?
.


;//  Verify started
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_VERIFY_STARTED
Language=English
Verify started on array %1.
.
Language=German
?
.

;//  Verify Priority Changed to High
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_VERIFY_PRIORITY_CHANGED_HIGH
Language=English
Verify running on array %1 has its priority changed to High.
.
Language=German
?
.


;//  Verify Priority Changed to Low
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_VERIFY_PRIORITY_CHANGED_LOW
Language=English
Verify running on array %1 has its priority changed to Low.
.
Language=German
?
.


;//  Verify Priority Changed to Medium
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_VERIFY_PRIORITY_CHANGED_MEDIUM
Language=English
Verify running on array %1 has its priority changed to Medium.
.
Language=German
?
.

;//  Verify completed
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_VERIFY_COMPLETED
Language=English
Verify completed for array %1, %2 miscompares found.
.
Language=German
?
.

;//  Verify aborted by user
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_VERIFY_ABORTED_BY_USER
Language=English
Verify on array %1 was aborted by user, %2 miscompares found.
.
Language=German
?
.

;//  Verify aborted due to I/O Error
MessageId=
Severity=Warning
Facility=RAID
SymbolicName=CTS_RMSTR_VERIFY_ABORTED_IOERROR
Language=English
Verify on array %1 was aborted due to I/O error, %2 miscompares found.
.
Language=German
?
.


;//  Regenerate started
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_REGENERATE_STARTED
Language=English
Regenerate started on array %1.
.
Language=German
?
.

;//  Regenerate Priority Changed High
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_REGENERATE_PRIORITY_CHANGED_HIGH
Language=English
Regenerate running on array %1 has its priority changed to High.
.
Language=German
?
.


;//  Regenerate Priority Changed Low
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_REGENERATE_PRIORITY_CHANGED_LOW
Language=English
Regenerate running on array %1 has its priority changed to Low.
.
Language=German
?
.


;//  Regenerate Priority Changed Medium
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_REGENERATE_PRIORITY_CHANGED_MEDIUM
Language=English
Regenerate running on array %1 has its priority changed to Medium.
.
Language=German
?
.


;//  Regenerate completed
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_REGENERATE_COMPLETED
Language=English
Regenerate completed on array %1.
.
Language=German
?
.


;//  Regenerate aborted by user
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_REGENERATE_ABORTED_BY_USER
Language=English
Regenerate on array %1 was aborted by user.
.
Language=German
?
.

;//  Regenerate aborted due to I/O Error
MessageId=
Severity=Warning
Facility=RAID
SymbolicName=CTS_RMSTR_REGENERATE_ABORTED_IOERROR
Language=English
Regenerate on array %1 was aborted due to I/O error.
.
Language=German
?
.


;//  Hot copy started
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_HOTCOPY_STARTED
Language=English
Hot copy started on array %1.
.
Language=German
?
.

;//  Hotcopy Priority Changed High
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_HOTCOPY_PRIORITY_CHANGED_HIGH
Language=English
Hotcopy running on array %1 has its priority changed to High.
.
Language=German
?
.


;//  Hotcopy Priority Changed Low
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_HOTCOPY_PRIORITY_CHANGED_LOW
Language=English
Hotcopy running on array %1 has its priority changed to Low.
.
Language=German
?
.


;//  Hotcopy Priority Changed Medium
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_HOTCOPY_PRIORITY_CHANGED_MEDIUM
Language=English
Hotcopy running on array %1 has its priority changed to Medium.
.
Language=German
?
.



;//  Hot copy completed
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_HOTCOPY_COMPLETED
Language=English
Hot copy completed on array %1.
.
Language=German
?
.

;//  Hot copy aborted by user
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_HOTCOPY_ABORTED_BY_USER
Language=English
Hot copy on array %1 was aborted by user.
.
Language=German
?
.

;//  Hot copy aborted due to I/O Error
MessageId=
Severity=Warning
Facility=RAID
SymbolicName=CTS_RMSTR_HOTCOPY_ABORTED_IOERROR
Language=English
Hot copy on array %1 was aborted due to I/O error.
.
Language=German
?
.


;//
;//		MEMBER RELATED EVENTS
;//

;//  Member [name] downed by user
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_MEMBER_DOWN_BY_USER
Language=English
Member %2 of Array %1 was downed by user.
.
Language=German
?
.

;//  Member [disk] downed by user
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_MEMBER_DISK_DOWN_BY_USER
Language=English
Member [Slot=%2] of Array %1 was downed by user.
.
Language=German
?
.


;//  Member [name] on array [name] down due to IO error
MessageId=
Severity=Warning
Facility=RAID
SymbolicName=CTS_RMSTR_MEMBER_DOWN_IOERROR
Language=English
Member %2 on array %1 is down due to I/O Error.
.
Language=German
?
.

;//  Member [disk] on array [name] down due to IO error
MessageId=
Severity=Warning
Facility=RAID
SymbolicName=CTS_RMSTR_MEMBER_DISK_DOWN_IOERROR
Language=English
Member [Slot=%2] on array %1 is down due to I/O Error.
.
Language=German
?
.


;//  New Member [name] added to array [name]
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_MEMBER_ADDED
Language=English
New member %2 added to Array %1.
.
Language=German
?
.

;//  New Member [disk] added to array [name]
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_MEMBER_DISK_ADDED
Language=English
New member [Slot=%2] added to Array %1.
.
Language=German
?
.


;//  Member [name] Removed from array [name]
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_MEMBER_REMOVED
Language=English
Member %2 removed from Array %1.
.
Language=German
?
.


;//  Member [disk] removed from array [name]
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_MEMBER_DISK_REMOVED
Language=English
Member [slot=%2] removed from Array %1.
.
Language=German
?
.


;//  New source member for array [name] is [name]
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_SOURCE_MEMBER_CHANGED
Language=English
New source member for array %1 is %2
.
Language=German
?
.


;//  New preferred member for array [name] is [name]
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_PREFERRED_MEMBER_CHANGED
Language=English
New preferred member for array %1 is %2
.
Language=German
?
.


;//  New source member for array [name] is [disk]
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_SOURCE_MEMBER_DISK_CHANGED
Language=English
New source member for array %1 is [Slot=%2]
.
Language=German
?
.


;//  New preferred member for array [name] is [disk]
MessageId=
Severity=Informational
Facility=RAID
SymbolicName=CTS_RMSTR_PREFERRED_MEMBER_DISK_CHANGED
Language=English
New preferred member for array %1 is [Slot=%2]
.
Language=German
?
.
