#ifndef CONNECTINGSOCKET_HPP
# define CONNECTINGSOCKET_HPP

# include "sockets/SimpleSocket.hpp"
# include <iostream>

class ConnectingSocket : public SimpleSocket
{
public:
	ConnectingSocket(int domain, int service, int protocol, int port, u_long interface);
	int connectToNetwork(int sock, struct sockaddr_in address);
};

#endif