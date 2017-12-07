/**
 * @file	2CANPlugin\2CAN.cpp
 *
 * @brief	Implements the 2CAN communication class.  The CANInterfacePlugin base class contains
 *			the interface required for a CAN gateway to function with FRC Robot. 
 */

#include "2CAN.H"
#include "2CANDataObjects.H"

#include "CSynchronized.h"
#include "Utility.h"
#include <sysLib.h>	// sysClkRateGet
#include <string> // strcpy

/**
 * @def	ENABLE_LOGGING
 *
 * @brief	set to 1 to log all API send and receive calls to a log file.
 */
#define ENABLE_LOGGING (0) 
#if ENABLE_LOGGING
	#include "APILogger.h"
	APILogger g_ApiLogger("FRC_2CANPlugin.log");
#endif // ENABLE_LOGGING


//function proto's
void Swap(uint8_t & var);
void Swap(int16 & var);
void Swap(int32 & var);
void Swap(uint64_t & var);


/**
 * @fn	C2CAN()
 *
 * @brief	Default constructor. 
 */

C2CAN::C2CAN() 
: m_obTask("C2CAN", (FUNCPTR)InitTask)
{
	//setup m_To2CAN_TxCANPacket signature and byte len
	m_To2CAN_TxCANPacket.iSig = SIG_TX_CAN_FRAMES;
	m_To2CAN_TxCANPacket.iByteLen = sizeof(m_To2CAN_TxCANPacket);

	//clear socket pointers, these are filled in after we find the 2can
	m_p2CANTxSocket = 0;
	m_p2CANRxSocket = 0;

	_robotState = 0;
	
	//clear bad frame counter
	m_ulBadFrame = 0;

	//create a semaphore so that transmitting frames is reentrant
	m_CriticalRegion = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE | SEM_DELETE_SAFE);

	//initial state is finding 2can
	eState = eFind2CAN;
	_blockSend = semBCreate (SEM_Q_PRIORITY, SEM_EMPTY);
	_blockReceive = semBCreate (SEM_Q_PRIORITY, SEM_EMPTY);
	//clear heart beat timeout
	m_ulHeartBeatTimeout = 0;

	//start our task
	if (!m_obTask.Start((INT32)this)) {
		printf("2CAN Task did not start successfully\n");
	}
}

/**
 * @fn	~C2CAN()
 *
 * @brief	Destructor. 
 */

C2CAN::~C2CAN() 
{
	Close();
}
/**
 * @brief	Terminates all network communications and perorms cleanup. 
 */
void C2CAN::Close() 
{
	m_obTask.Stop(); // stop task so we can delete sockets
	if(m_p2CANTxSocket)
		delete m_p2CANTxSocket;
	m_p2CANTxSocket = 0;
	if(m_p2CANRxSocket)
		delete m_p2CANRxSocket;
	m_p2CANRxSocket = 0;
}
/**
 * @brief	Checks if C2CAN task has stopped running.  If you are frequently
 * starting and stopping this downloadble kernal module in Wind River
 * Workbench you will find that the udp ports will fail to open after a
 * while because the system resources that get opened (udp handles) do not
 * get freed. One solution is to call cleanup functions from Wind River's
 * console but that can be annoying after several sessions.  Instead you can
 * terminate the C2CAN task in the Debug view.  A monitor task can wait for
 * this by calling this function.  When it returns true just free the 2CAN
 * class instance and it will clean up socket handles and allocated memory. 
 *
 * @return	true C2CAN iff task has terminated or exited, false otherwise.
 */
bool C2CAN::HasBeenClosed()
{
	INT32 id = taskNameToId("FRC_C2CAN");
	if(id == -1)
	{
		return true;
	}
	return false;
}
/**
 * @fn	void C2CAN::InitTask(C2CAN * pThis)
 *
 * @brief	Static function thats is entry point for our background task. 
 *
 * @param [int]	pThis	Pointer to class instance
 */

void C2CAN::InitTask(C2CAN * pThis) 
{
	pThis->Run();
}

/**
 * @fn	void C2CAN::Swap(STo2CAN_TxCANPacket & Packet)
 *
 * @brief	Swaps the byte in each 16bit word in a 2CAN Tx packet.  2CAN is a
 * 			little endian device and cRIO Processor is big endian. 
 */

void C2CAN::Swap(STo2CAN_TxCANPacket & Packet) 
{
	for (unsigned int i=0; i<sizeof(Packet)/2; ++i)
		::Swap(Packet.Words[i]);
}

/**  
 * @fn	void C2CAN::Swap(STo2CAN_EnablePacket & Packet)
 *
 * @brief	Swaps the byte in each 16bit word in a 2CAN Enable packet.  2CAN is a
 * 			little endian device and cRIO Processor is big endian. 
 */

void C2CAN::Swap(STo2CAN_EnablePacket & Packet) 
{
	::Swap(Packet.iSig);
	::Swap(Packet.iByteLen);
	::Swap(Packet.enableState);
	::Swap(Packet.outputEnables);
	::Swap(Packet.sequence);
	::Swap(Packet.iCRC);
}

/**
 * @fn	void C2CAN::Swap(STo2CAN_TxCANPacket & Packet)
 *
 * @brief	Swaps the byte in each 16bit word in a 2CAN Rx packet.  2CAN is a
 * 			little endian device and cRIO Processor is big endian. 
 */

void C2CAN::Swap(STo2CAN_RxCANPacket & Packet) 
{
	for (unsigned int i=0; i<sizeof(Packet)/2; ++i)
		::Swap(Packet.Words[i]);
}

/**
 * @fn	unsigned short C2CAN::CheckSum(unsigned short * data,
 * 		unsigned long byte_len)
 *
 * @brief	Creates checksum for 2CAN packet.
 *
 * @param [in]	data	Packet to checksum.  All packets lengths are even so that packet can be iterated as a word array.
 * @param	byte_len	Length of the packet in bytes (should always be even).
 *
 * @return	16 bit checksum. 
 */

unsigned short C2CAN::CheckSum(unsigned short * data, unsigned long byte_len) 
{
	unsigned int i=0;
	unsigned short c=0;
	c = 0;
	for (; i<byte_len/2; ++i) {
		c += data[i];
	}
	return ~c + 1;
}

/**
 * @fn	void C2CAN::Run()
 *
 * @brief	Runs the background task.
 * 			
 * 			First it finds the 2can regardless of what cRIO port it's on. The
 * 			second port is the only legal port currently but using port one
 * 			can be useful for practice as it allows wireless access to the
 * 			web dashboard.
 * 			
 * 			Secondly this task will send heartbeat frames to the 2CAN after
 * 			it's found.  This is how the 2CAN knows to change it's led status
 * 			to indicate that the plugin is working.  Web dashboard can also
 * 			be used to check plugin status. 
 */

void C2CAN::Run()
{
	int port = 1;
	while(true) 
	{
		switch (eState) 
		{
			case eFind2CAN:
				if (Find(port))
				{
					eState = eProcess; // we found port move on to heartbeat
					semGive(_blockSend); // signal sendMessage to stop blocking
					semGive(_blockReceive); // signal receiveMessage to stop blocking
					printf("2CAN found at IP %s\n",m_sIP);
				}
				else
					port = (port==1) ? 2 : 1; // go to next port.
				break;
				
			case eProcess:
				ProcessHeartBeat();
				break;
		}
		taskDelay(10);
	}
}

/**
 * @fn	bool C2CAN::Find(int port)
 *
 * @brief	Trys to find a 2can on a particular cRIO Ethernet port. If the
 * 			poll was successful then m_p2CANTxSocket and m_p2CANRxSocket are
 * 			constructed.  Otherwise they remain null so it is important for
 * 			send/receive functions to check our state to ensure we've found
 * 			the 2CAN before using these socket pointers. 
 *
 * @param	port	The cRIO Ethernet port number (1 or 2). 
 *
 * @return	true if 2CAN was found on port. 
 */

bool C2CAN::Find(int port)
{
	char ip[20];
	//Pick an IP that will send a broadcast UDP Frame on a given port.
	if (port == 1)
		strcpy(ip, "255.255.255.255");
	else
		strcpy(ip, "192.168.0.255"); // network broadcast

	//create sockets
	TxUDPSocket g_BroadcastSocketTx(ip, 32000, 1);
	RxUDPSocket g_BroadcastSocketRx(ip, 32001, 3);

	//UDP data payload for 'who's there'.  2CAN will respond with an 'I'm here' response.
	char whos_there[] = { 0xcc, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x8a, 0x80, 0x00,
			0x00, 0xaa, 0xd4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	//Container for response frame
	BL_COMM_Resp rx;

	int len=0;
	
	//send the who's there
	g_BroadcastSocketTx.Send(whos_there, sizeof(whos_there));

	//wait for response, g_BroadcastSocketRx was configured to be non-blocking.
	for(int i=0;i<50;++i)
	{
		len = g_BroadcastSocketRx.Read((char*)&rx, sizeof(rx));
		taskDelay(1);
		if(len > 0) // if we got response then leave for loop
			break;
	}

	//is the response the right size
	if (len != 72)
		return 0;

	::Swap(rx.iHeader); // the received data is little endian so flip the bytes in each word.
	::Swap(rx.iLen);
	::Swap(rx.iCommand);
	::Swap(rx.iChipID);

	if((rx.iHeader) != 0xaacc) // check header
		return 0;
	if((rx.iLen) != 0x34)		// check payload length
		return 0;
	if((rx.iCommand) != 0x8000) // check command
		return 0;
	if((rx.iChipID) != 0)		//check chip id
		return 0;

	//2CAN Comm frames contain a payload holding data unique to the response type
	// for this particular response, the 2CAN stuffs it's configured IP in the packet
	BL_COMM_RxPacketInfo *packet = (BL_COMM_RxPacketInfo *)rx.iAppPacketBytes;
	//swap the words (little endian => big endian)
	uint16_t *packetWords = (uint16_t *)rx.iAppPacketBytes;
	uint16_t packetWordSize = sizeof(BL_COMM_RxPacketInfo)/2;
	for(uint16_t i=0;i<packetWordSize;++i)
		::Swap(*packetWords++);
	_info = *packet;
	//create string ip
	sprintf(m_sIP,"%d.%d.%d.%d", (_info.uiIP[0]), // stuff the 2CAN's IP into m_sIP
			(_info.uiIP[1]),
			(_info.uiIP[2]),
			(_info.uiIP[3]) );
	//create the sockets we will use from now on
	m_p2CANTxSocket = new TxUDPSocket(m_sIP,1217,0);
	m_p2CANRxSocket = new RxUDPSocket(m_sIP,1218,0);
	return 1;
}

/**
 * @fn	void C2CAN::ProcessHeartBeat()
 *
 * @brief	Process function for the heart beat logic.
 */

void C2CAN::ProcessHeartBeat()
{
	//get firmware version
	uint16_t firmwareVers;
	firmwareVers = _info.iFirmwareMajor & 0xFF;
	firmwareVers <<= 8;
	firmwareVers |= _info.iFirmwareMinor & 0xFF;
	
	//Since we will be transmitting frames lock out the send API.
	CRITICAL_REGION(m_CriticalRegion);
		
	if(m_ulHeartBeatTimeout)
	{
		//countdown heartbeat
		m_ulHeartBeatTimeout -= 10;
	}
	else
	{
		if(firmwareVers >= 0x0200)
		{
			// 2CAN firmware 2.0 and on uses 'robot state' to control LEDs
			//Send the enable datagram to the 2CAN.  This is what controls the LED status.
			STo2CAN_EnablePacket enableOut;
			memset((void*)&enableOut,0,sizeof(enableOut));
			enableOut.enableState = _robotState;
			enableOut.iSig = SIG_ENABLE;
			enableOut.iByteLen = sizeof(enableOut);
			enableOut.iCRC = 0;
			enableOut.iCRC = CheckSum(enableOut.Words,sizeof(enableOut));
			Swap(enableOut);
			if(m_p2CANTxSocket->Send((char*)&enableOut, sizeof(enableOut)) == 0)
			{
			}
		}
		else
		{	
			// 2CAN firmware 1.6 and before uses CAN tx requests to control transitions
			//from red to orange to green.
			//send a CAN Tx Frame containing NO CAN messages.  
			//This will have no effect on CAN bus but will maintain LED status in 2CAN (orange).
			m_To2CAN_TxCANPacket.iOptions = 0;
			m_To2CAN_TxCANPacket.iFrameCnt = 0;
			m_To2CAN_TxCANPacket.iCRC = 0;
			m_To2CAN_TxCANPacket.iCRC = CheckSum((unsigned short*)&m_To2CAN_TxCANPacket,sizeof(m_To2CAN_TxCANPacket));
			STo2CAN_TxCANPacket out = m_To2CAN_TxCANPacket;
			Swap(out);
			if(m_p2CANTxSocket->Send((char*)&out, sizeof(out)) == 0)
			{
			}
		}
		//we want to transmit if 50 ms have gone by and there has been no traffic
		//to maintain LED status.
		m_ulHeartBeatTimeout = 50;
	}
	//release semaphore
	END_REGION;
}

/**
 * This entry-point of the CANInterfacePlugin is passed a message that the driver needs to send to
 * a device on the CAN bus.
 * 
 * This function may be called from multiple contexts and must therefore be reentrant.
 * 
 * @param messageID The 29-bit CAN message ID in the lsbs.  Bit31 must be set if remote extended frames are desired.
 * @param data A pointer to a buffer containing between 0 and 8 bytes to send with the message.  May be NULL if dataSize is 0.
 * @param dataSize The number of bytes to send with the message.
 * @return Return any error code.  On success return 0.
 */
INT32 C2CAN::sendMessage(UINT32 messageID, const UINT8 *data, UINT8 dataSize)
{
#if ENABLE_LOGGING
	char line[100];
	sprintf(line,"%f) 2CAN::sendMessage(%x,%lx,%x)\n",GetTimeS(),messageID,*(long unsigned int*)data,dataSize);
	g_ApiLogger.Write(line);
#endif
	
	int ret; //return value

	if(eState != eProcess)
	{
		printf("C2CAN::sendMessage waiting for 2CAN to be found...\n");
		//wait for find task to release semaphore => that 2can was found
		if(semTake(_blockSend, (int)(sysClkRateGet()*0.300)) == OK)
		{
			// we just took semaphore, give it back...
			// Semaphore is used only for task synchonization, not mutual exlusion
			semGive(_blockSend); 
		}
	}
	
	//if we have yet to find 2CAN then this call must fail.
	if(eState != eProcess)
	{
		printf("C2CAN::sendMessage - 2CAN not found yet...\n");
#if ENABLE_LOGGING
		sprintf(line,"%f) return 2CAN::sendMessage() = %d\n",GetTimeS(),-10);
		g_ApiLogger.Write(line);
#endif
		return -10;
	}
	
	//sending must be reentrant so lock semaphore
	CRITICAL_REGION(m_CriticalRegion);
	
	//create a tx frame holding CAN frame to send.
	STo2CAN_CANFrame frame = { 0 };
	//len_options holds CAN DLC and options for extended/remote frames.
	//Assume caller wants extended frames
	frame.len_options = ((UINT16)dataSize)<<8 | STo2CAN_CANFrameOption_ExtendedID;
	//caller will set bit31 if remote extended frame is desired.
	if(messageID & 0x80000000) // check bit31
	{
		frame.len_options |= STo2CAN_CANFrameOption_Remote; // bit31 => remote frame
		messageID &= ~0x80000000; // clear bit31
	}
	//stuff CAN arbitration id.
	frame.arbid_h = messageID>>16;
	frame.arbid_l = messageID & 0xffff;
	//fill data bytes
	if (dataSize > 8)
		dataSize = 8;
	for (UINT8 i=0; i<dataSize; ++i)
		frame.data[i] = data[i];
	
	//copy frame into our comm structure and update checksum
	m_To2CAN_TxCANPacket.CANFrames[0] = frame;
	m_To2CAN_TxCANPacket.iOptions = 0;
	m_To2CAN_TxCANPacket.iFrameCnt = 1;
	m_To2CAN_TxCANPacket.iCRC = 0;
	m_To2CAN_TxCANPacket.iCRC = CheckSum((unsigned short*)&m_To2CAN_TxCANPacket,sizeof(m_To2CAN_TxCANPacket));
	STo2CAN_TxCANPacket out = m_To2CAN_TxCANPacket;
	Swap(out);
	
	//send 2CAN comm frame
	ret = m_p2CANTxSocket->Send((char*)&out, sizeof(out));

#if ENABLE_LOGGING
	sprintf(line,"%f) return 2CAN::sendMessage() = %d\n",GetTimeS(),ret);
	g_ApiLogger.Write(line);
#endif
	
	//release semaphore
	END_REGION;
	
	return ret;
}
/**
 * This entry-point of the CANInterfacePlugin is passed buffers which should be populated with
 * any received messages from devices on the CAN bus.
 * 
 * This function is always called by a single task in the Jaguar driver, so it need not be reentrant.
 * 
 * This function is expected to block for some period of time waiting for a message from the CAN bus.
 * It may timeout periodically (returning non-zero to indicate no message was populated) to allow for
 * shutdown and unloading of the plugin.
 * 
 * @param messageID A reference to be populated with a received 29-bit CAN message ID in the lsbs.
 * @param data A pointer to a buffer of 8 bytes to be populated with data received with the message.
 * @param dataSize A reference to be populated with the size of the data received (0 - 8 bytes).
 * @return This should return 0 if a message was populated, non-0 if no message was not populated.
 */
INT32 C2CAN::receiveMessage(UINT32 &messageID, UINT8 *data, UINT8 &dataSize)
{
#if ENABLE_LOGGING
	char line[100];
	sprintf(line,"%f, 2CAN::receiveMessage()\n",GetTimeS());
	g_ApiLogger.Write(line);
#endif
	
	int retval = -1; // return value, assume we get no CAN RX frames
	
	char m_ucRx[1500]; // data cache for recieved UDP datagrams
	int len = sizeof(m_ucRx);

	if(eState != eProcess)
	{
		printf("C2CAN::receiveMessage waiting for 2CAN to be found...\n");
		//wait for find task to release semaphore => that 2can was found
		if(semTake(_blockReceive, (int)(sysClkRateGet()*0.300)) == OK)
		{
			//we just took semaphore, give it back...
			// Semaphore is used only for task synchonization, not mutual exlusion
			semGive(_blockReceive);
		}
	}
	
	//if we have not yet found the 2CAN then return out of function.  
	if(eState != eProcess)
	{
		printf("C2CAN::receiveMessage - 2CAN not found yet...\n");
#if ENABLE_LOGGING
		sprintf(line,"%f) return 2CAN::receiveMessage() = %d,messageID = %x,data = %lx,dataSize = %x,\n",
					GetTimeS(),
					-10,
					messageID,*(long unsigned int*)data,dataSize);
		g_ApiLogger.Write(line);
#endif
		return -10;
	}

	//read UDP socket
	len = m_p2CANRxSocket->Read((char*)m_ucRx, len);
	
	//cast received data to 2CAN communication structure
	STo2CAN_RxCANPacket * pPack = (STo2CAN_RxCANPacket * )m_ucRx;

	//correct endianness of header
	uint16_t header = pPack->iSig;
	::Swap(header);

	if(len == 0)
	{
		//no data received
	}
	else if(len != sizeof(STo2CAN_RxCANPacket))
	{
		//bad frame received
		m_ulBadFrame++;
	}
	else if(header != SIG_RX_CAN_FRAMES)
	{
		//bad frame received
		m_ulBadFrame++;
	}
	else
	{
		//cast received data to the CAN RX communication structure
		STo2CAN_RxCANPacket *pSTo2CAN_RxCANPacket = (STo2CAN_RxCANPacket * )m_ucRx;
		Swap(*pSTo2CAN_RxCANPacket); // correct endianness
		if(CheckSum((unsigned short*)m_ucRx,sizeof(STo2CAN_RxCANPacket))) 
		{
			//bad checksum
			m_ulBadFrame++;
		}
		else
		{
			//copy received CAN frame into caller's parameters
			const STo2CAN_CANFrame & front = pSTo2CAN_RxCANPacket->CANFrames[0];
			
			messageID = ((INT32)front.arbid_h) << 16 | front.arbid_l; // arbid
			dataSize = front.len_options>>8; // dlc
			for(UINT8 i=0;i<dataSize;++i) // data bytes
				data[i] = front.data[i];
			retval = 0; // we received CAN frame so return 0
		}
	}

#if ENABLE_LOGGING
	sprintf(line,"%f) return 2CAN::receiveMessage() = %d,messageID = %x,data = %lx,dataSize = %x,\n",
			GetTimeS(),
			retval,
			messageID,*(long unsigned int*)data,dataSize);
	g_ApiLogger.Write(line);
#endif
	
	return retval;
}

/**
 * @fn	void Swap(char &a, char &b)
 *
 * @brief	Swaps two bytes
 */

void Swap(char &a, char &b)
{
	char temp(a);
	a = b;
	b = temp;
}

/**
 * @fn	void Swap(uint8_t & var)
 *
 * @brief	Does nothing, just a stub for members that are bytes.
 */

void Swap(uint8_t & var)
{
	//MT
}

/**
 * @fn	void Swap(int16 & var)
 *
 * @brief	Swaps two bytes in a word.
 */

void Swap(int16 & var)
{
	union {
		int16 u16;
		char bytes[2];
	} b;
	b.u16 = var;
	Swap(b.bytes[0], b.bytes[1]);
	var = b.u16;
}

/**
 * @fn	void Swap(int16 & var)
 *
 * @brief	Swaps two bytes in a dword.
 */
void Swap(int32 & var)
{
	union {
		int32 u32;
		char bytes[4];
	} b;
	b.u32 = var;
	Swap(b.bytes[0], b.bytes[3]);
	Swap(b.bytes[1], b.bytes[2]);
	var = b.u32;
}


/**
 * @fn	void Swap(int16 & var)
 *
 * @brief	Swaps two bytes in a dword.
 */
void Swap(uint64_t & var)
{
	union {
		uint64_t u64;
		char bytes[8];
	} b;
	b.u64 = var;
	Swap(b.bytes[0], b.bytes[7]);
	Swap(b.bytes[1], b.bytes[6]);
	Swap(b.bytes[2], b.bytes[5]);
	Swap(b.bytes[3], b.bytes[4]);
	var = b.u64;
}

