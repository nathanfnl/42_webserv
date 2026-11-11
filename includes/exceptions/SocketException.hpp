#ifndef SOCKET_EXCEPTION_HPP
# define SOCKET_EXCEPTION_HPP

# include "WebservException.hpp"

using namespace std;

class SocketException : public WebservException
{
public:
    explicit SocketException(const string& msg) : WebservException(msg) {}
};

class SocketConnectionException : public SocketException
{
public:
    SocketConnectionException(void) : SocketException("Socket failed to connect") {}
};

#endif