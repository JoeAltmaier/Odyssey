/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This class abstracts network service.
// 
// Update Log: 
// 8/16/99 Joe Altmaier: Create file
/*************************************************************************/

#define _TRACEF
#define	TRACE_INDEX		TRACE_TRANSPORT
#include "Odyssey_Trace.h"

#include "Socket.h"
#include "Kernel.h"
#include "HbcSubnet.h"

	Socket::Socket() {}
	
	void Socket::Configure(U32 ipMe_, U32 ipHim_, U32 port_) {
		// Set up address structures
		addrMe.family = NU_FAMILY_IP;
		addrMe.port = port_;
		addrMe.id.is_ip_addrs[0] = IPa(ipMe_);
		addrMe.id.is_ip_addrs[1] = IPb(ipMe_);
		addrMe.id.is_ip_addrs[2] = IPc(ipMe_);
		addrMe.id.is_ip_addrs[3] = IPd(ipMe_);
		addrMe.name = "internal";

		addrHim.family = NU_FAMILY_IP;
		addrHim.port = port_;
		addrHim.id.is_ip_addrs[0] = IPa(ipHim_);
		addrHim.id.is_ip_addrs[1] = IPb(ipHim_);
		addrHim.id.is_ip_addrs[2] = IPc(ipHim_);
		addrHim.id.is_ip_addrs[3] = IPd(ipHim_);
		addrHim.name = "internal";

		Close();
		}
	
	// Open socket, as bind/listen or connect
	Status Socket::Connect(BOOL fHost_) {
		Tracef("Socket::Connect ");

		STATUS status_=OK;
		
		int socketd_ = NU_Socket(addrMe.family, NU_TYPE_STREAM, NU_NONE);

		if (socketd_ < 0)
			return socketd_;

		if (fHost_) { // Server
			Tracef("as Host %08lx\n", *(U32*)&addrMe.id);

			// Set the address we will be responding to
			status_ = NU_Bind(socketd_, &addrMe, 0);
			if (status_ < 0) {
				NU_Close_Socket(socketd_);
				return status_;
				}

			// This sets the max # connections
			status_ = NU_Listen(socketd_, 1);
			if (status_ < 0) {
				NU_Close_Socket(socketd_);
				return status_;
				}

			addr_struct addrClient;

			// Wait for client to connect
			status_ = NU_Accept(socketd_, &addrClient, 0);

			// Throw away original socket; we only accept one connection
			NU_Close_Socket(socketd_);

			if (status_ < 0)
				return status_;

			socketDescriptor = status_;
			NU_Fcntl(socketDescriptor, NU_SETFLAG, NU_BLOCK);
			NU_Push(socketDescriptor);
			status_ = OK;
			}

		else { // Client
			Tracef("as client to %08lx\n", *(U32*)&addrHim.id);
			status_ = NU_Connect(socketd_, &addrHim, 0);
			if (status_ < 0) {
				NU_Close_Socket(socketd_);
				return status_;
				}

			socketDescriptor = status_;
			NU_Fcntl(socketDescriptor, NU_SETFLAG, NU_BLOCK);
			NU_Push(socketDescriptor);
			status_ = OK;
			}

		return status_;
		}

	Status Socket::Close() {
		Tracef("Socket::Close\n");

		if (socketDescriptor >= 0)
			NU_Close_Socket(socketDescriptor);

		socketDescriptor = -1;

		return OK;
		}
	
	// Transmit
	Status Socket::Write(void *pData_, U32 cbData_) {
		if (socketDescriptor < 0)
			return socketDescriptor;

//		TRACE_DUMP_HEX(8, "Socket::Write", (unsigned char*)pData_, cbData_);

		int cbXfer_;
		while (cbData_) {
			cbXfer_=cbData_;
			if (cbXfer_ > (U32)0x7F00)
				cbXfer_ = (U32)0x7F00;

//Tracef("Socket::Write attempt cb=%x\n", cbXfer_);
			cbXfer_ = NU_Send(socketDescriptor, (char*)pData_, cbXfer_, 0);
//Tracef("Socket::Write sent cb=%x\n", cbXfer_);
			if (cbXfer_ <= 0)
				return cbXfer_;
				
			(char*)pData_ += cbXfer_;
			cbData_ -= cbXfer_;
			}

		return OK;
		}

	// Block on receive
	Status Socket::Read(void *pData_, U32 cbRead_, U32 *pCbRead_) {
//		Tracef("Socket::Read %d\n", cbRead_);
		STATUS status_ = OK;

		*pCbRead_=0; // so far
		while (cbRead_ && status_ == OK) {

			U32 cbReadNow_ = cbRead_;
			if (cbReadNow_ > (U32)0xFF00)
				cbReadNow_ = (U32)0xFF00;

//		Tracef("NU_Recv %x\n", cbReadNow_);
			status_ = NU_Recv(socketDescriptor, (char*)pData_, cbReadNow_, 0);

			if (status_ >= 0) {
//Tracef("Socket::Read cb=%x\n", status_);
				cbRead_ -= status_;
				*pCbRead_ += status_;
				((char*)pData_) += status_;
				status_ = OK;
				}
			else
{Tracef("Error %d\n", status_);
				cbRead_ = 0;
}
			}

//Tracef("status=%08lx\n", status_);
		return status_;
		}

