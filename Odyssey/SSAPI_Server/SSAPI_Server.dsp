# Microsoft Developer Studio Project File - Name="SSAPI_Server" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=SSAPI_Server - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SSAPI_Server.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SSAPI_Server.mak" CFG="SSAPI_Server - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SSAPI_Server - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "SSAPI_Server - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SSAPI_Server - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "SSAPI_Server - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /Zp4 /MTd /W3 /Gm /Gi /GR /GX /ZI /Od /I "..\DdmVCM" /I "..\..\include\CmdQueues" /I "..\..\include\AlarmManager" /I "..\..\include\Partition" /I "..\DdmPartitionMstr" /I "..\..\include\UpgradeMaster" /I "..\..\include\FileSystem" /I "..\..\include\Raid" /I ".\ObjectManagers" /I ".\ManagedObjects" /I ".\Utils" /I ".\\" /I "..\..\include" /I "..\..\localization" /I "..\..\include\oos" /I "..\..\include\cttables" /I "..\..\include\pts" /I "..\..\cmdqueues" /I "..\..\include\fcp" /I "..\..\include\drivers" /I "..\ssapi_server\\" /I "..\ssapi_server\utils" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR"Debug/browse.info/" /YX /FD /GZ /c
# SUBTRACT CPP /X
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 DdmVcm.lib PartitionMstr.lib RmstrLib.lib ddm_pts.lib oos.lib ChaosL2.lib oos_util.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /profile /debug /machine:I386 /libpath:"..\..\bin\win\\"

!ENDIF 

# Begin Target

# Name "SSAPI_Server - Win32 Release"
# Name "SSAPI_Server - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "transport"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DdmNwkMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\DdmSocketServer.cpp
# End Source File
# Begin Source File

SOURCE=.\MultiPacketMessageOrgainzer.cpp
# End Source File
# Begin Source File

SOURCE=.\PacketParser.cpp
# End Source File
# Begin Source File

SOURCE=.\SessionManager.cpp
# End Source File
# Begin Source File

SOURCE=.\SimpleQue.cpp
# End Source File
# Begin Source File

SOURCE=.\valueset.cpp
# End Source File
# Begin Source File

SOURCE=.\WriteQue.cpp
# End Source File
# End Group
# Begin Group "SSAPI"

# PROP Default_Filter ""
# Begin Group "ObjectManagers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ObjectManagers\AlarmManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\ConfigIdManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\ConnectionManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\DeviceManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\HostManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\ListenManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\LogManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\LunMapManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\ObjectManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\PHSDataManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\ProcessManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\ProfileManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\SoftwareImageManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\StorageManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\TableStuffManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\UserManager.cpp
# End Source File
# End Group
# Begin Group "ManagedObjects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ManagedObjects\Alarm.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\AlarmHistory.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\AssetInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Battery.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Board.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\BusSegment.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Chassis.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ChassisPowerSupply.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ClusteredDataPath.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Comparator.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ConfigId.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Connection.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\CordSet.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\CountFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\CtProcess.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\DataPath.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\DDH.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\DesignatorIdVectorFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Device.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\DeviceCollection.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\DiskPowerSupply.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\EvcDeviceCollection.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Fan.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\FcDeviceCollection.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\FcPort.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Filter.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\FloatVectorFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\HDDDevice.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Host.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\IntVectorFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Iop.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Listener.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\LogMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\LogMetaData.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\LunMapEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ManagedObject.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\MicroController.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Nac.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ObjectClassTypeFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\PciDeviceCollection.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\PHSData.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\PHSDataFloat.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\PHSDataInt.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\PHSDataInt64.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\PowerSupply.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ProcessRaidUtility.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ProcessRaidUtilityHotCopy.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ProcessRaidUtilityInitialize.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ProcessRaidUtilityRegenerate.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ProcessRaidUtilitySmartCopy.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ProcessRaidUtilityVerify.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\RedundantDataPath.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\SlotMap.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\SoftwareDescriptor.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\SoftwareImage.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils\SSAPIEvents.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StatusReporterInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageCollection.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageCollectionSparepool.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElement.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementArray.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementArray0.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementArray1.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementDisk.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementPartition.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementPassThru.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementPassThruSes.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementPassThruTape.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementSsd.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageIdInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\TableMetaData.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\TableStuff.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\TableStuffRow.cpp
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\User.cpp
# End Source File
# End Group
# Begin Group "utils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Utils\ClassTypeMap.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils\CmdSender.cpp
# End Source File
# Begin Source File

SOURCE=..\CmdQueues\CmdServer.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils\CoolVector.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils\DescriptorCollector.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils\DesignatorIdVector.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils\FilterSet.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils\Raid2SsapiErrorConvertor.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils\ShadowTable.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils\SList.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils\SListSorted.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils\SsapiAssert.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils\SsapiGateway.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils\SsapiLocalResponder.cpp
# End Source File
# Begin Source File

SOURCE=.\SsapiResponder.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils\StringClass.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils\StringResourceManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils\UnicodeString.cpp
# End Source File
# End Group
# Begin Group "Tables"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Include\CTTables\ArrayDescriptor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\ArrayPerformanceTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\ArrayStatusTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\ClassTableTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\CommandQueue.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\DeviceDescriptor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\DiskDescriptor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\DiskPerformanceTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\DiskStatusTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\EVCStatusRecord.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\ExportTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\ExportTableUserInfoTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\FCPortDatabaseTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\HostConnectionDescriptorTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\HostDescriptorTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\IopFiloverMapTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\IOPStatusTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\LoopDescriptor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\PartitionTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\PathDescriptor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\RAIDMemberDescriptor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\RaidSpareDescriptor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\RAIDUtilDescriptor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\SsdDescriptor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\SSDPerformanceTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\SSDStatusTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\StatusQueue.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\StorageRollCallTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\STSData.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\STSPerfTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\STSStatTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\SystemConfigSettingsTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\UnicodeString128Table.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\UnicodeString16Table.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\UnicodeString256Table.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\UnicodeString32Table.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\UnicodeString64Table.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\UserAccessTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\VirtualClassDescriptorTable..cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\VirtualDeviceTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\VirtualRouteTable.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\DdmSSAPI.cpp
# End Source File
# Begin Source File

SOURCE=.\SsapiRequestMessage.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\BuildSys.cpp
# End Source File
# Begin Source File

SOURCE=.\Utils\DdmSsapiTest.cpp
# End Source File
# Begin Source File

SOURCE=.\SSAPI_Server.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "transport_headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DdmNwkMgr.h
# End Source File
# Begin Source File

SOURCE=.\DdmSocketServer.h
# End Source File
# Begin Source File

SOURCE=.\NwkMsgs.h
# End Source File
# Begin Source File

SOURCE=.\PacketParser.h
# End Source File
# Begin Source File

SOURCE=.\SessionManager.h
# End Source File
# Begin Source File

SOURCE=.\valueset.h
# End Source File
# Begin Source File

SOURCE=.\WriteQue.h
# End Source File
# End Group
# Begin Group "SSAPI_headers"

# PROP Default_Filter ""
# Begin Group "ManagedObjects.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ManagedObjects\Alarm.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\AlarmHistory.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\AssetInterface.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Battery.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Board.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\BoardDevices.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\BusSegment.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Chassis.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ChassisPowerSupply.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ClusteredDataPath.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Comparator.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ConfigId.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Connection.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\CordSet.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\CountFilter.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\CtProcess.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\DataPath.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\DDH.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\DesignatorIdVectorFilter.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Device.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\DeviceCollection.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\DiskPowerSupply.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\DownstreamConnection.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\EvcDeviceCollection.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Fan.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\FcDeviceCollection.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\FcPort.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\FcPortInternal.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Filter.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\FloatVectorFilter.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\HDDDevice.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Host.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\IntVectorFilter.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Iop.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Listener.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\LockableInterface.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\LogMessage.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\LogMetaData.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\LunMapEntry.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ManagedObject.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\MicroController.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\Nac.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ObjectClassTypeFilter.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\PciDeviceCollection.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\PHSData.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\PHSDataFloat.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\PHSDataInt.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\PHSDataInt64.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\PHSDataSpecificObjects.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\PowerableInterface.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\PowerSupply.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ProcessRaidUtility.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ProcessRaidUtilityHotCopy.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ProcessRaidUtilityInitialize.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ProcessRaidUtilityRegenerate.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ProcessRaidUtilitySmartCopy.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ProcessRaidUtilityVerify.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\RedundantDataPath.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\ServiceableInterface.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\SlotMap.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\SNac.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\SoftwareDescriptor.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\SoftwareDescriptorObjects.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\SoftwareImage.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\SSAPIFilter.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StatusReporterInterface.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageCollection.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageCollectionSparepool.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElement.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementArray.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementArray0.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementArray1.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementArray1HotCopy.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementArray1HotCopyAuto.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementArray1HotCopyManual.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementBase.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementDisk.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementDiskExternal.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementDiskInternal.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementPartition.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementPassThru.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementPassThruSes.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementPassThruTape.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageElementSsd.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\StorageIdInfo.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\TableMetaData.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\TableStuff.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\TableStuffRow.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\UpstreamConnection.h
# End Source File
# Begin Source File

SOURCE=.\ManagedObjects\User.h
# End Source File
# End Group
# Begin Group "ObjectManager.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ObjectManagers\AlarmManager.h
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\ConfigIdManager.h
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\ConnectionManager.h
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\DeviceManager.h
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\HostManager.h
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\ListenManager.h
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\LogManager.h
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\LunMapManager.h
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\ObjectManager.h
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\PHSDataManager.h
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\ProcessManager.h
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\ProfileManager.h
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\SoftwareImageManager.h
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\StorageManager.h
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\TableStuffManager.h
# End Source File
# Begin Source File

SOURCE=.\ObjectManagers\UserManager.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\DdmSSAPI.h
# End Source File
# Begin Source File

SOURCE=.\SSAPI_Codes.h
# End Source File
# Begin Source File

SOURCE=.\SsapiRequestMessage.h
# End Source File
# Begin Source File

SOURCE=.\SSAPIServerVersion.h
# End Source File
# End Group
# Begin Group "utils.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Utils\ClassTypeMap.h
# End Source File
# Begin Source File

SOURCE=.\Utils\Container.h
# End Source File
# Begin Source File

SOURCE=.\Utils\DdmSsapiTest.h
# End Source File
# Begin Source File

SOURCE=.\Utils\DescriptorCollector.h
# End Source File
# Begin Source File

SOURCE=.\Utils\DesignatorId.h
# End Source File
# Begin Source File

SOURCE=.\Utils\DesignatorIdVector.h
# End Source File
# Begin Source File

SOURCE=.\Utils\FilterSet.h
# End Source File
# Begin Source File

SOURCE=.\Utils\Raid2SsapiErrorConvertor.h
# End Source File
# Begin Source File

SOURCE=.\Utils\RowId.h
# End Source File
# Begin Source File

SOURCE=.\Utils\ShadowTable.h
# End Source File
# Begin Source File

SOURCE=.\Utils\SList.h
# End Source File
# Begin Source File

SOURCE=.\Utils\SListSorted.h
# End Source File
# Begin Source File

SOURCE=.\Utils\SsapiAlarms.h
# End Source File
# Begin Source File

SOURCE=.\Utils\SSAPIAssert.h
# End Source File
# Begin Source File

SOURCE=.\Utils\SSAPIEvents.h
# End Source File
# Begin Source File

SOURCE=.\Utils\SsapiGateway.h
# End Source File
# Begin Source File

SOURCE=.\Utils\SsapiLocalResponder.h
# End Source File
# Begin Source File

SOURCE=.\SsapiResponder.h
# End Source File
# Begin Source File

SOURCE=.\Utils\StringClass.h
# End Source File
# Begin Source File

SOURCE=.\Utils\StringResourceManager.h
# End Source File
# Begin Source File

SOURCE=.\Utils\UnicodeString.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\Include\DdmCMBMsgs.h
# End Source File
# Begin Source File

SOURCE=.\SSAPIObjectStates.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
