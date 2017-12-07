/**
 * @file	UDP\TxUDPSocket.cpp
 *
 * @brief	Declares the Transmit UDP Socket class. 
 */

#include "TxUDPSocket.h"
#include <string> // strcpy def'n

/**
 * @fn	TxUDPSocket:
 *
 * @brief	Constructor. 
 *
 * @param [in,out]	String IP to send UDP datagrams to.
 * @param	ulPort		port number. 
 * @param	ulOptions	bitwise options for socket.  Set bit 0 to send
 * 						broadcast frames. 
 */

TxUDPSocket::TxUDPSocket(	const char * sIP,
							unsigned long ulPort,
							unsigned long ulOptions)
{
	m_ulOptions = ulOptions;
	
	m_txsocketFile = -1;

	strcpy(m_remoteIPString,sIP);
	
	m_port = ulPort;
}

/**
 * @fn	TxUDPSocket::~TxUDPSocket()
 *
 * @brief	Destructor. 
 */

TxUDPSocket::~TxUDPSocket()
{
	if(m_txsocketFile != -1)
		close(m_txsocketFile);
	m_txsocketFile = -1;
}

/**
 * @fn	int TxUDPSocket::CreateTxSocket(void)
 *
 * @brief	Creates and binds the transmit socket. 
 *
 * @return	zero iff socket created successfully.
 */

int TxUDPSocket::CreateTxSocket(void)
{
	struct sockaddr_in	sin;

	if ((m_txsocketFile = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		m_txsocketFile = -1;
		return (errno);
	}

	sin.sin_family = AF_INET;
	sin.sin_port = htons(1217);
	sin.sin_addr.s_addr = INADDR_ANY;

	if (bind(m_txsocketFile, (struct sockaddr*)&sin, sizeof(sin)) < 0)
	{
		close(m_txsocketFile);
		m_txsocketFile = -1;
		return (errno);
	}
	
	if(m_ulOptions & 1)
	{
		BOOL bOptVal = 1;
		int bOptLen = sizeof(BOOL);
		if(setsockopt(m_txsocketFile,SOL_SOCKET,SO_BROADCAST,(char*)&bOptVal,bOptLen) < 0)
		{
			close(m_txsocketFile);
			m_txsocketFile = -1;
			return (errno);
		}
	}
	return (0);
}

/**
 * @fn	int TxUDPSocket::Send(char * data, unsigned long len)
 *
 * @brief	Sends one UDP datagram. 
 *
 * @param [in,out]	data	Data to transmit.  Be mindful not to exceed UDP MTU.
 * @param	len				Number of bytes to transmit in datagram. 
 *
 * @return	0 iff successful. 
 */

int TxUDPSocket::Send(char * data, unsigned long len)
{
	struct sockaddr_in      serverSockAddr;
	int						bytes;

	struct in_addr remoteIP;

	if(m_txsocketFile <= 0)
		CreateTxSocket();
	
	if(m_txsocketFile <= 0)
		return -10;
	
	if (!inet_aton(m_remoteIPString, &remoteIP))
	{
		//return ;
	}

	memset(&serverSockAddr, 0, sizeof(serverSockAddr));
	serverSockAddr.sin_family = AF_INET;
	serverSockAddr.sin_addr.s_addr = remoteIP.s_addr;
	serverSockAddr.sin_port = htons(m_port);

	bytes = sendto(m_txsocketFile, data, len, 0,
		(struct sockaddr*)&serverSockAddr, sizeof(serverSockAddr));
	if(bytes < 0)
	{
		close(m_txsocketFile);
		m_txsocketFile = -1;
		return -2;
	}
	if( (unsigned long)bytes != len)
	{
		close(m_txsocketFile);
		m_txsocketFile = -1;
		return -3;
	}

	return 0;
}

	
