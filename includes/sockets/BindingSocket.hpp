#ifndef BINDINGSOCKET_HPP
# define BINDINGSOCKET_HPP

# include "sockets/SimpleSocket.hpp"
# include <iostream>

class BindingSocket : public SimpleSocket
{
public:
	BindingSocket(int domain, int service, int protocol, int port, u_long interface);
	int connectToNetwork(int sock, struct sockaddr_in address);
};

#endif