//SnmpInterface.h

#ifndef __SnmpInterface_H
#define __SnmpInterface_H

#define SNMP_TRAP_GENERAL_TYPE_ENTERPRISE   6

// One of the following two values is passed on the setInterface method as the iOperStatus variable.
// Down is basically dead. Up is good, warning, degraded or some semblance of life.
#define SNMP_OPER_STATUS_UP                 1
#define SNMP_OPER_STATUS_DOWN               2

// One of the following three values is passed on the SendTrap method as the iSeverity variable.
// They are passed in by DdmSnmp
#define SNMP_TRAP_SPECIFIC_TYPE_WARNING     1000
#define SNMP_TRAP_SPECIFIC_TYPE_MINOR       1001
#define SNMP_TRAP_SPECIFIC_TYPE_CRITICAL    1002

// One of the following three values is passed on the SendTrap method as the iSeverity variable.
// They are generated internally
#define SNMP_TRAP_SPECIFIC_TYPE_LINKDOWN    1003
#define SNMP_TRAP_SPECIFIC_TYPE_LINKUP      1004
#define SNMP_TRAP_SPECIFIC_TYPE_COLDSTART   1005

#include "UnicodeString.h"

class SnmpInterface {
public:

	SnmpInterface();
	~SnmpInterface();

    /* Called by DdmSnmp.
     * The description of this device is typically the identification of
     * HW level running in the device.
     * This method should be called as part of initialization. */
    void SetDescription(UnicodeString usVersion);

    /* Called by DdmSnmp.
     * The description of this device. For SNMP, this field is typically
     * the identification of HW and SW running in the device.
     * This method should be called as part of initialization. */
    void SetDescription(UnicodeString usVersion, UnicodeString usProdDate);

    /* The administrative contact for this device. For now, it will be compiled
     * into the SNMP code as no entry. There can be only one SNMP contact for the box. */
	void SetContact(UnicodeString usContact);

    /* Called by DdmSnmp.
     * The user-settable name of this device.
     * This method should be called as part of initialization. */
	void SetName(UnicodeString usName);

    /* Called by DdmSnmp.
     * The string describing the physical location of this device.
     * This method should be called as part of initialization. */
	void SetLocation(UnicodeString usLocation);

    /* Called by DdmSnmp.
     * This call is to set the relevant mib-2 interface data for a given index.
     * If the interface doesn't exist, this method will add it.
     * If the interface does exist, this method will update it.
     * This method will be called during initialization as many times as there
     * are components that we wish to expose as an SNMP interface.
     * The order of the interfaces in mib2 propogates through to the HP OpenView
     * display of the interface summary submap, so the interfaces should be grouped
     * in a logical way, ie. 1 to n are IOPs, n+1 to m are drives, m+1 to z are EVCs, etc.
     *
     * When the initialization of interfaces has completed, the SNMPInitComplete()
     * method should be called. This will do two things. It causes SNMPInterface to
     * send an enterprise cold start immediately to all addresses in the trap list.
     * Also, when the SetInterface method is subsequently called, if an existing
     * interface is being updated, an enterprise cold start will be sent to all
     * addresses in the trap list.
     *
     * iIndex - is an incrementing value, 1 to n. When SetInterface is called with
     * an index that already exists, the interface is updated. When SetInterface
     * is called with an index that does not exists, the interface is added.
     * This index value maps back to an object location.
     *
     * usName - comes from Device.getName() method. A change to this
     * value is most likely due to one type of IOP being pulled and another
     * being inserted in a slot. SNMPInterface will call SendColdStart to send
     * an enterprise colStart trap to all addresses in the trap list.
     *
     * iOperStatus - A change to this value is probably the most common reason this method
     * will be invoked after initialization. SetInterface will be responsible for interpreting
     * the value it receives as a state change or not. A state change will cause SNMPInterface
     * to call SendLinkUp or SendLinkDown, depending on the case, to send an enterprise linkUp
     * or linkDown trap to all addresses in the trap list.
     *  */
    void SetInterface(int iIndex, UnicodeString usName, int iOperStatus);

    /* Called by DdmSnmp when initialization processing complete. */
    void SNMPInitComplete();

    /* Called by DdmSnmp.
     * This method will send an enterprise trap to all addresses in the trap list.
     *
     * usName - Name of the component that has event occurring.
     *
     * usDescription - Message to be sent in trap.
     *
     * iSeverity - Passed by DdmSnmp.*/
    void SendTrap(UnicodeString usName, UnicodeString usDescription, int iSeverity);

    /* Called by DdmSnmp to add an address to the trap list if that address isn't found. */
	void AddTrapAddress(int iAddress);

    /* Called by DdmSnmp to remove all addresses from the trap list. */
    void ClearTrapAddresses();

private:
    /* Called by SNMPInterface to send an enterprise linkUp trap to all
     * addresses in the trap list. */
    void SendLinkUp(int iIndex, char *sName);

    /* Called by SNMPInterface to send an enterprise linkDown trap to all
     * addresses in the trap list. */
    void SendLinkDown(int iIndex, char *sName);

    /* Called by SNMPInterface to send an enterprise coldStart trap to all
     * addresses in the trap list. */
    void SendColdStart();

protected:
	char* ConvertString(UnicodeString usIN);

};

#endif

