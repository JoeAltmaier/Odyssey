;//
;//  CTEvtFlash.mc - Error codes unique to the FlashStorage Storage Manager and device driver.
;//                Included by CTEvent.mc.  See that file for more info.
;//
;//


;//  Invalid handle passed to flash storage method.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_INVALID_HANDLE
Language=English
Invalid handle passed to flash storage method.
.
Language=German
?
.

;//  No context could be allocated.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_NO_CONTEXT
Language=English
No context could be allocated.
.
Language=German
?
.

;//  Invalid handle passed to flash storage method.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_INVALID_CONFIG_VERSION
Language=English
Handle passed to flash storage method is invalid.
.
Language=German
?
.

;//  Invalid page state found in page table.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_INVALID_PAGE_STATE
Language=English
Invalid page state found in page table.
.
Language=German
?
.

;//  FlashStorage table of contents is too big.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_TOC_TOO_BIG
Language=English
The flash table of contents will not fit in one page.
.
Language=German
?
.

;//  No erased pages are available.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_NO_ERASED_PAGES
Language=English
No erased pages are available.
.
Language=German
?
.

;//  Invalid table of contents.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_INVALID_TOC
Language=English
Invalid flash table of contents.
.
Language=German
?
.

;//  Invalid table of contents version.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_INVALID_TOC_VERSION
Language=English
Invalid flash table of contents version.
.
Language=German
?
.

;//  Available replacement pages has fallen below threshold.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_REPLACEMENT_PAGE_THRESHOLD
Language=English
Available replacement pages has fallen below threshold.
.
Language=German
?
.

;//  No replacement pages are available.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_NO_REPLACEMENT_PAGES
Language=English
No replacement pages are available.
.
Language=German
?
.

;//  Table of contents could not be written.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_WRITE_TABLE_OF_CONTENTS
Language=English
Table of contents could not be written.
.
Language=German
?
.

;//  Timeout on flash operation.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_TIMEOUT
Language=English
Timeout occurred on a flash operation.
.
Language=German
?
.

;//  Timeout on erase.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_ERASE_TIMEOUT
Language=English
Timeout occurred on an erase operation.
.
Language=German
?
.

;//  Timeout on read.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_READ_TIMEOUT
Language=English
Timeout occurred on a read operation.
.
Language=German
?
.

;//  Timeout on write.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_WRITE_TIMEOUT
Language=English
Timeout occurred on a write operation.
.
Language=German
?
.

;//  Device error detected on erase operation.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_ERASE_ERROR
Language=English
Device error detected on erase operation.
.
Language=German
?
.

;//  Device error detected on flash operation.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_DEVICE_ERROR
Language=English
Device error detected on flash operation.
.
Language=German
?
.

;//  Verify error detected.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_VERIFY
Language=English
Miscompare on verify operation.
.
Language=German
?
.

;//  Verify error detected on write operation.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_VERIFY_WRITE
Language=English
Verify error detected on a write operation.
.
Language=German
?
.

;//  Verify error detected on erase operation.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_VERIFY_ERASE
Language=English
Verify error detected on an erase operation.
.
Language=German
?
.

;//  Page not erased.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_PAGE_NOT_ERASED
Language=English
Attempt to write to a page that is not erased -- internal error.
.
Language=German
?
.

;//  Data miscompare.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_DATA_MISCOMPARE
Language=English
Data miscompare -- data read does not match data expected.
.
Language=German
?
.

;//  Not enough memory.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_MEM_SIZE_TOO_SMALL
Language=English
Memory allocated for the flash service is too small.
.
Language=German
?
.

;//  Bad block table already exists.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_BAD_BLOCK_TABLE_ALREADY_EXISTS
Language=English
Bad block table cannot be created because it already exists.
.
Language=German
?
.

;//  Bad block table does not exist.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_BAD_BLOCK_TABLE_DOES_NOT_EXIST
Language=English
FlashStorage storage cannot be opened because the bad block table could not be found.
.
Language=German
?
.

;//  FlashStorage storage was not closed the last time it was opened.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_NEVER_CLOSED
Language=English
FlashStorage storage was not closed the last time it was opened.
.
Language=German
?
.

;//  Attempt was made to write over the bad block table -- internal error.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_WRITING_BAD_BLOCK_TABLE
Language=English
Attempt was made to write over the bad block table -- internal error.
.
Language=German
?
.

;//  Page map invalid.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_INVALID_PAGE_MAP_VIRTUAL_ADDRESS
Language=English
Page map read from flash storage is invalid.
Real address %1 has virtual address %2 > number of virtual pages %3
.
Language=German
?
.

;//  Page map invalid.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_INVALID_PAGE_MAP_DUPLICATE_VIRTUAL_ADDRESS
Language=English
Page map read from flash storage is invalid.
Virtual address %1 already mapped to real address %2
Real address %2 mapped to virtual address %3
.
Language=German
?
.

;//  Page map invalid.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_INVALID_PAGE_MAP_STATE
Language=English
Page map read from flash storage is invalid.
Real address %1 has invalid state %2.
.
Language=German
?
.

;//  FlashStorage storage not open.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_NOT_OPEN
Language=English
FlashStorage storage method was called, but flash storage is not open.
.
Language=German
?
.

;//  No replacement pages available.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_REPLACEMENT_PAGES_ERASING
Language=English
No replacement pages are available.
An erase operation is keeping the only available pages busy.
.
Language=German
?
.

;//  No good blocks to allocate system structure.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_NO_GOOD_BLOCKS
Language=English
Internal error -- there are not enough good blocks available to allocate a system block.
.
Language=German
?
.

;//  No good blocks to write page map.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_NO_GOOD_BLOCKS_PAGE_MAP
Language=English
The page map could not be written because there are not enough good blocks available.
.
Language=German
?
.

;//  No good blocks to write table of contents.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_NO_GOOD_BLOCKS_TOC
Language=English
The table of contents could not be written because there are not enough good blocks available.
.
Language=German
?
.

;//  No good blocks to allocate basic assurance test.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_NO_GOOD_BLOCKS_BAT
Language=English
The basic assurance test block could not be allocated because there are not enough good blocks.
.
Language=German
?
.

;//  Bad flash address detected.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_BAD_FLASH_ADDRESS
Language=English
Internal error -- bad flash address detected.
.
Language=German
?
.

;//  Bad flash address detected in page map table.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_BAD_PAGE_MAP_TABLE_ADDRESS
Language=English
Bad page map table address %1 at index %2.
.
Language=German
?
.

;//  Bad flash address detected in page map table.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_BAD_PAGE_MAP_ADDRESS
Language=English
Bad page map address %1 in page map table at index %2.
.
Language=German
?
.

;//  Invalid unit code returned by device.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_INVALID_UNIT
Language=English
Invalid unit code in array %1 column %2 bank %3,  register value = %4.
.
Language=German
?
.

;//  ECC error detected.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_ECC
Language=English
An ECC error was detected.
.
Language=German
?
.

;//  ECC error detected and corrected.
MessageId=
Severity=Warning
Facility=FlashStorage
SymbolicName=CTS_FLASH_ECC_CORRECTED
Language=English
An ECC error was detected and corrected.
.
Language=German
?
.

;//  Write data parity error.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_WRITE_DATA_PARITY
Language=English
Write data parity error detected.
.
Language=German
?
.

;//  Write data overflow error.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_WRITE_DATA_OVERFLOW
Language=English
Write data overflow error detected.
.
Language=German
?
.

;//  Read data underflow error detected.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_READ_DATA_UNDERFLOW
Language=English
Read data underflow error detected.
.
Language=German
?
.

;//  Command overwrite error detected.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_COMMAND_OVERWRITE
Language=English
Command overwrite error detected.
.
Language=German
?
.

;//  Invalid flash device detected.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_INVALID_FLASH_DEVICE
Language=English
Invalid flash device, unit ID = %1.
.
Language=German
?
.

;//  Invalid buffer alignment.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_BUFFER_NOT_64_BYTE_ALIGNED
Language=English
Buffer passed into flash storage method is not aligned on a 64-byte boundary.
.
Language=German
?
.

;//  Transfer length error.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_TRANSFER_LENGTH
Language=English
Transfer length error detected.
.
Language=German
?
.

;//  Verify error detected on read operation.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_VERIFY_READ
Language=English
Verify error detected on a read operation.
.
Language=German
?
.

;//  Invalid column.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_INVALID_COLUMN
Language=English
Invalid column in array %1 column %2.
.
Language=German
?
.

;//  Invalid maker code.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_INVALID_MAKER_CODE
Language=English
Invalid maker code in array %1 column %2 bank %3, register value = %4.
.
Language=German
?
.

;//  Invalid unit code.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_INVALID_UNIT_CODE
Language=English
Invalid unit code in array %1 column %2 bank %3, register value = %4.
.
Language=German
?
.

;//  Invalid unit and maker code.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_INVALID_UNIT_MAKER_CODE
Language=English
Invalid unit and maker code .
.
Language=German
?
.

;//  Too many bad spots.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_TOO_MANY_BAD_SPOTS
Language=English
Block has too many bad spots to be used.
.
Language=German
?
.

;//  No memory.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_NO_MEMORY
Language=English
No memory could be allocated from Mem object.
.
Language=German
?
.

;//  NFlash driver device write error.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_DEVICE_WRITE
Language=English
NFlash driver device write error..
.
Language=German
?
.

;//  NFlash driver device read error.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_DEVICE_READ
Language=English
NFlash driver device write error..
.
Language=German
?
.

;//  NFlash driver device busy error.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_DEVICE_BUSY
Language=English
NFlash driver device write error..
.
Language=German
?
.

;//  NFlash driver device semaphore error.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_DEVICE_SEMA
Language=English
NFlash driver device unable to create semaphore.
.
Language=German
?
.

;//  NFlash driver device reset error.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_DEVICE_RESET
Language=English
NFlash driver device reset error.
.
Language=German
?
.

;//  NFlash driver device erase error.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_DEVICE_ERASE
Language=English
NFlash driver device erase error.
.
Language=German
?
.

;//  No memory could be allocated for TyDma .
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_NO_MEMORY_TYDMA
Language=English
No memory could be allocated for TyDma.
.
Language=German
?
.

;//  End of SGL encountered before data.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_FLASH_SGL_OVERRUN
Language=English
End of SGL encountered before data.
.
Language=German
?
.

;//  No good blocks to write page map table.
MessageId=
Severity=Error
Facility=FlashStorage
SymbolicName=CTS_FLASH_NO_GOOD_BLOCKS_PAGE_MAP_TABLE
Language=English
The page map table could not be written because there are not enough good blocks available.
.
Language=German
?
.

