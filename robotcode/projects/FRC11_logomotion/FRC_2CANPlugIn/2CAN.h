/**
 * @file	2CANPlugin\2CAN.h
 *
 * @brief	Implements the 2CAN communication class.  The CANInterfacePlugin base class contains
 *			the interface required for a CAN gateway to function with FRC Robot. 
 */

#ifndef C2CAN__H_
#define C2CAN__H_


#include "CANInterfacePlugin.h" // CANInterfacePlugin def'n
#include "2CANDataObjects.h"	// defs for comm structures
#include "UDP/TxUDPSocket.h"	//UDP Tx Socket
#include "UDP/RxUDPSocket.h"	//UDP Rx Socket
#include "Task.h"				//Task

#include <VxWorks.h>	// SEM_ID defs

class C2CAN : public CANInterfacePlugin
{
	public:

		C2CAN();
		~C2CAN();

		INT32 sendMessage(UINT32 messageID, const UINT8 *data, UINT8 dataSize);

		INT32 receiveMessage(UINT32 &messageID, UINT8 *data, UINT8 &dataSize);

		/**
		 * @brief	Terminates all network communications and perorms cleanup. 
		 */
		void Close();

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
		bool HasBeenClosed();

		/**
		 * @brief	Sets a robot state and updates 2CAN green LED status. 
		 *
		 * @param	robotState	State of the robot. 0 - disabled, 1 - teleop, 2-
		 * 						auton. 
		 */
		void SetRobotState(int robotState)
		{
			_robotState = robotState;
		}
	private:
		
		static void InitTask(C2CAN * pThis);
		void Run();
		bool Find(int port);
		unsigned short CheckSum(unsigned short * data,unsigned long len);

		void Swap(STo2CAN_TxCANPacket & Packet);
		void Swap(STo2CAN_RxCANPacket & Packet);
		void Swap(STo2CAN_EnablePacket & Packet);

		void ProcessHeartBeat();

		SEM_ID m_CriticalRegion;	///< Semaphore for making send API reentrant. 
		
		unsigned long m_ulBadFrame;	///< counter for bad UDP frames. Usefull for debugging. 

		unsigned long m_ulHeartBeatTimeout;	///< Counter for sending heartbeats only after so much time from last transmission to 2CAN. 
		
		char m_sIP[20];	///< temp cache for holding IP of 2CAN that was found. 

		TxUDPSocket *m_p2CANTxSocket;	///< TX Socket pointer.  Initially set to 0, then later constructed after finding a 2CAN. 
		RxUDPSocket *m_p2CANRxSocket;	///< RX Socket pointer.  Initially set to 0, then later constructed after finding a 2CAN. 

		/**
		 * @enum	eState
		 *
		 * @brief	State of the background task.  First we find the 2CAN using broadcast UDP frames, then we send periodic heartbeats to update LED status. 
		 */
		enum
		{
			eFind2CAN,
			eProcess,
		}eState;

		SEM_ID _blockSend;
		SEM_ID _blockReceive;
		
		BL_COMM_RxPacketInfo _info;

		CTask m_obTask;	///< Task object for background thread. 
		
		STo2CAN_TxCANPacket m_To2CAN_TxCANPacket;	///< Temp cache for holding UDP frame intended for transmission.
		
		int _robotState;
};



#endif // C2CAN__H_


