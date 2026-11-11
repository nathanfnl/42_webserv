#include "sockets/SimpleSocket.hpp"
#include "exceptions/SocketException.hpp"
#include <cstring>
#include <fcntl.h>

using namespace std;

SimpleSocket::SimpleSocket(int domain, int service, int protocol, int port, u_long interface)
{
	// Define address structure
	this->_address.sin_family = domain;
	this->_address.sin_port = htons(port);
	this->_address.sin_addr.s_addr = htonl(interface);
	// Establish socket
    this->_sock = socket(domain, service, protocol);
	testConnection(this->_sock);
	
	int optval = 1;
	if (setsockopt(this->_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
		throw SocketException("setsockopt SO_REUSEADDR failed");
}

void	SimpleSocket::testConnection(int itemToTest)
{
	if (itemToTest < 0)
	{
		cerr << "Socket error: " << strerror(errno) << " (errno=" << errno << ")" << endl;
		throw SocketConnectionException();
	}
}

struct sockaddr_in	SimpleSocket::getAddress(void)
{
	return (this->_address);
}

int	SimpleSocket::getSock(void)
{
	return (this->_sock);
}

int	SimpleSocket::getConnection(void)
{
	return (this->_connection);
}

void	SimpleSocket::setConnection(int connection)
{
	this->_connection = connection;
}

void	SimpleSocket::setNonBlocking(void)
{
	int	flags = fcntl(this->_sock, F_GETFL, 0);
	if (flags < 0)
		throw SocketException("fcntl F_GETFL failed");
	if (fcntl(this->_sock, F_SETFL, flags | O_NONBLOCK) < 0)
		throw SocketException("fcntl F_SETFL failed");
}