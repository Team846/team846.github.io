/**
 * @file	UDP\TxUDPSocket.h
 *
 * @brief	Declares the Transmit UDP Socket class. 
 */

#ifndef TXUDPSockets__h_
#define TXUDPSockets__h_

#include "sockLib.h" 
#include "hostLib.h" 
#include "inetLib.h" 

class TxUDPSocket
{
	public:
		TxUDPSocket(const char * sIP,unsigned long ulPort,unsigned long ulOptions);
		~TxUDPSocket();
		
		int Send(char * data, unsigned long len);

		bool WaitForConnect(void);

	private:
		void Run(void);

		int CreateTxSocket(void);
		
		bool IsConnected(void);

		int	m_txsocketFile;
		
		char m_remoteIPString[20];

		unsigned long m_port;

		unsigned long m_ulOptions;
};


#endif // TXUDPSockets__h_


