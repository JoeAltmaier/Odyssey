# Odyssey
The Odyssey 2000 Network Attached Storage Peripheral is a highly reliable, high performance, high capacity storage peripheral.  The Odyssey is intended for use in installations where performance and reliability are a must.  It should never, ever fail!!! A cornerstone of the Odyssey’s design is that it shall have “no single point of failure.”  Commensurate with that design goal, the Odyssey may be configured so that within the chassis there is redundant everything, fans, power supplies, circuit boards, everything.  Note that this is one configuration and other less redundant configurations are also supported.
The performance and reliability to be delivered by Odyssey is no small engineering task.  In addition to the presence of fast and reliable hardware, the software that puts that hardware into action must be equally fast, robust and flexible - the work of gods.  This document describes that software.

CHAOS - a message-based embedded service platform

CHAOS is a message-based operating system built on a simple multi-processing kernel which is currently Nucleus.
Writing an application for CHAOS consists of writing one or more DDMs. An application DDM which is built on the DDM base class
allows the application to send and receive messages to/from other application DDMs.
Some of the CHAOS APIs include support for DDMs, Messages, Error Injection, Error Logging and macros to support the 
BuildSys module. Also included is a system Class library to support various APIs such as Critical Sections and Semaphores.

CHAOS applications are built as a single system image. 
The image linkage is specified in the BuildSys.cpp module which is described in a later section.
All images run CHAOS and the interboard transport service with the master image on the HBC also running the 
Partition Table Services (PTS).

All IOPs retrieve their configuration from the Master Image running on the HBC. 
All configurations are stored in the “VirtualDeviceTable” which is maintained by the Persistent Table Service. 
The Master Image Buildsys.cpp also contains a default configuration which is used if the Persistent Table Service 
has not yet been initialized.
