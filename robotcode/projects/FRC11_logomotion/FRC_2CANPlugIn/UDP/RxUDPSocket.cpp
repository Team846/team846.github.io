/**
 * @file	UDP\RxUDPSocket.cpp
 *
 * @brief	Declares the Receive UDP Socket class. 
 */

#include "RxUDPSocket.h"
#include <string> // strcpy def'n

/**
 * @fn	RxUDPSocket:
 *
 * @brief	Constructor. 
 *
 * @param [in,out]	String IP to send UDP datagrams to.
 * @param	ulPort		port number. 
 * @param	ulOptions	bitwise options for socket.  Set bit 0 to send
 * 						broadcast frames. Set bit 1 to make Receive()
 *						non blocing.
 */

RxUDPSocket::RxUDPSocket(	const char * sIP,
							unsigned long ulPort,
							unsigned long ulOptions)
{
	m_ulOptions = ulOptions;

	m_rxsocketFile = -1	;

	strcpy(m_remoteIPString,sIP);
	
	m_port = ulPort;
}

/**
 * @fn	RxUDPSocket::~RxUDPSocket()
 *
 * @brief	Destructor. 
 */

RxUDPSocket::~RxUDPSocket()
{
	if(m_rxsocketFile != -1)
		close(m_rxsocketFile);
	m_rxsocketFile = -1;
}

/**
 * @fn	int RxUDPSocket::CreateRxSocket()
 *
 * @brief	Creates and binds the receive socket. 
 *
 * @return	zero iff socket created successfully.
 */

int RxUDPSocket::CreateRxSocket()
{
	struct sockaddr_in	sin;

	if ((m_rxsocketFile = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		m_rxsocketFile = -1;
		return (errno);
	}
	sin.sin_family = AF_INET;
	sin.sin_port = htons(m_port);
	sin.sin_addr.s_addr = INADDR_ANY;

	if (bind(m_rxsocketFile, (struct sockaddr*)&sin, sizeof(sin)) < 0)
	{
		close(m_rxsocketFile);
		m_rxsocketFile = -1;
		return (errno);
	}
	return (0);
}

/**
 * @fn	int RxUDPSocket::Read(char * data, int len)
 *
 * @brief	Receive one UDP datagram. 
 *
 * @param [in,out]	data	Data to fill with receive datagram.
 * @param	len				Maximun fill size of data pointer. 
 *
 * @return	number of bytes received.
 */

int RxUDPSocket::Read(char * data, int len)
{
	int flags = 0;
	
	if(m_rxsocketFile <= 0)
		CreateRxSocket();

	if(m_rxsocketFile <= 0)
		return -1;

	struct in_addr remoteIP;
	
	struct sockaddr_in      serverSockAddr;
	int				sin_len;

	if (!inet_aton(m_remoteIPString, &remoteIP))
	{
		//return ;
	}

	sin_len = sizeof(serverSockAddr);

	if(m_ulOptions&2)
		flags |= MSG_DONTWAIT;
	
	len = recvfrom(	m_rxsocketFile,
					data, 
					len,
					flags,
					(struct sockaddr*)&serverSockAddr,
					&sin_len);

	if ( (m_ulOptions&1) || (serverSockAddr.sin_addr.s_addr == remoteIP.s_addr))
	{
		if(len >= 0)
		{
			m_RxIp = inet_netof(remoteIP);
		}
		return len;
	}
	return 0;
}


	
