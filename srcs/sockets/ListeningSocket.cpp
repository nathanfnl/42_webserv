#include "sockets/ListeningSocket.hpp"

ListeningSocket::ListeningSocket(int domain, int service,
	int protocol, int port, u_long interface, int backlog)
	: BindingSocket(domain, service, protocol, port, interface)
{
	this->_backlog = backlog;
	startListening();
	testConnection(this->_listening);
	setNonBlocking();
}

void	ListeningSocket::startListening(void)
{
	this->_listening = listen(getSock(), this->_backlog);
}