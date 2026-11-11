#ifndef LISTENINGSOCKET_HPP
# define LISTENINGSOCKET_HPP

# include "sockets/BindingSocket.hpp"
# include <iostream>

class ListeningSocket : public BindingSocket
{
private:
	int	_backlog;
	int	_listening;

public:
	ListeningSocket(int domain, int service, int protocol, int port,
		u_long interface, int backlog);
	void	startListening(void);
};

#endif