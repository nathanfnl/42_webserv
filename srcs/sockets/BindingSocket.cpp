#include "sockets/BindingSocket.hpp"

BindingSocket::BindingSocket(int domain, int service, int protocol, int port,
     u_long interface) : SimpleSocket(domain, service, protocol, port, interface)
{
    setConnection(connectToNetwork(getSock(), getAddress()));
    testConnection(getConnection());
}

int BindingSocket::connectToNetwork(int sock, struct sockaddr_in address)
{
    return bind(sock, (struct sockaddr *)&address, sizeof(address));
}