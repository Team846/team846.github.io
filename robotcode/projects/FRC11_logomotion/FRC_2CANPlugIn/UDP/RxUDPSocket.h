/**
 * @file	UDP\RxUDPSocket.h
 *
 * @brief	Declares the Receive UDP Socket class. 
 */

#ifndef RXUDPSockets__h_
#define RXUDPSockets__h_

#include "sockLib.h" 
#include "hostLib.h" 
#include "inetLib.h" 

class RxUDPSocket
{
	public:
		RxUDPSocket(const char * sIP,unsigned long ulPort,unsigned long ulOptions);
		
		~RxUDPSocket();
		
		int GetRxIP()
		{
			return m_RxIp;
		}
		
		int Read(char * data, int len);
		
	private:

		int CreateRxSocket();
		
		int	m_rxsocketFile;

		char m_remoteIPString[20];
		
		unsigned long m_port;
		
		unsigned long m_ulOptions;
		
		int m_RxIp;
};


#endif // RXUDPSockets__h_


