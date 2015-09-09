# Odyssey
Message-based embedded service platform
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
