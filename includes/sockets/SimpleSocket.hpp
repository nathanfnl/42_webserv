#ifndef SIMPLESOCKET_HPP
# define SIMPLESOCKET_HPP

# include <iostream>
# include <cerrno>
# include <sys/socket.h>
# include <netinet/in.h>

class SimpleSocket
{
private:
	struct sockaddr_in	_address;
	int					_sock;
	int					_connection;

public:
	SimpleSocket(int domain, int service, int protocol, int port, u_long interface);

	virtual int	connectToNetwork(int sock, struct sockaddr_in address) = 0;
	void		testConnection(int);

	struct sockaddr_in	getAddress(void);
	int					getSock(void);
	int					getConnection(void);

	void				setConnection(int connection);
	void				setNonBlocking(void);
};

#endif