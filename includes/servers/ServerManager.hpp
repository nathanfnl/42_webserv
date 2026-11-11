#ifndef SERVERMANAGER_HPP
# define SERVERMANAGER_HPP

# include <iostream>
# include <sys/epoll.h>
# include <unistd.h>
# include <sys/socket.h>
# include <map>
# include <vector>
# include <fcntl.h>
# include "clients/Client.hpp"
# include "config/ConfigParser.hpp"
# include "sockets/Socket.hpp"
# include <algorithm>

using namespace std;
class ServerManager
{
private:
	int 							_epollFd;
	vector<int>						_listenFds;
	map<int, Client*>				_clients;
	ConfigParser					*_config;
	map<int, const ServerConfig*>	_listenFdToConfig;
	map<int, int>					_cgiFdToClient;

	void	_handleNewConnection(int fd);
	void	_handleClientRead(int fd);
	void	_closeClient(int fd);
	bool	_writeToClient(Client *client);
	void	_checkClientsTimeout(void);
	bool	_isListenSocket(int fd) const;

	void	_handleCgiEvent(int pipeFd, uint32_t evMask);

public:
	ServerManager(ConfigParser *conf);
	~ServerManager(void);
	void	run(void);
};

void	handle_sig(int sig);

#endif