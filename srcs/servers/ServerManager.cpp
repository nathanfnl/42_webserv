#include "servers/ServerManager.hpp"
#include "exceptions/SocketException.hpp"
#include "utils/Logger.hpp"
#include "cgi/CgiHandler.hpp"

using namespace std;

volatile sig_atomic_t   g_stop = 0;

void	handle_sig(int sig)
{
	(void)sig;
    g_stop = 1;
}

ServerManager::ServerManager(ConfigParser *conf): _config(conf)
{
	this->_epollFd = epoll_create(1);
	if (this->_epollFd < 0)
		throw SocketException("epoll_create failed");

	const vector<ServerConfig>&	servers = this->_config->getServers();
	for (vector<ServerConfig>::const_iterator it = servers.begin(); it != servers.end(); it++)
	{
		ListeningSocket listenSock(AF_INET, SOCK_STREAM, 0, it->getPort(), INADDR_ANY, SOMAXCONN);
		listenSock.setNonBlocking();
		int	sockFd = listenSock.getSock();

		this->_listenFds.push_back(sockFd);
		this->_listenFdToConfig.insert(make_pair(sockFd, &(*it)));

		struct epoll_event	ev;
		ev.events = EPOLLIN;
		ev.data.fd = listenSock.getSock();
		if (epoll_ctl(this->_epollFd, EPOLL_CTL_ADD, listenSock.getSock(), &ev) < 0)
			throw SocketException("epoll_ctl ADD listen socket failed");
	}
}

ServerManager::~ServerManager(void)
{
	for (map<int, Client*>::iterator it = this->_clients.begin(); it != this->_clients.end(); it++)
		delete it->second;
}

void	ServerManager::_handleNewConnection(int fd)
{
	int	clientFd = accept(fd, NULL, NULL);
	if (clientFd < 0)
		return ;
	
	int	flags = fcntl(clientFd, F_GETFL, 0);
	fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);

	struct epoll_event	ev;
	ev.events = EPOLLIN;
	ev.data.fd = clientFd;
	if (epoll_ctl(this->_epollFd, EPOLL_CTL_ADD, clientFd, &ev) < 0)
	{
		close(clientFd);
		return ;
	}
	map<int, const ServerConfig*>::iterator confIt = this->_listenFdToConfig.find(fd);
	if (confIt == this->_listenFdToConfig.end())
	{
		Logger::log(CRITICAL, "No config found for listening fd");
		close(clientFd);
		return ;
	}
	this->_clients[clientFd] = new Client(clientFd, *(confIt->second));
	
	ostringstream	oss;
	oss << "New connection accepted, fd=" << clientFd;
	Logger::log(INFO, oss.str());
}

void	ServerManager::_handleClientRead(int fd)
{
	map<int, Client*>::iterator it = this->_clients.find(fd);
	if (it == this->_clients.end())
	{
		Logger::log(WARNING, "handleClientRead called on unknown fd");
		return ;
	}
	Client* client = it->second;

	char	buf[65536];
	ssize_t	n = recv(fd, buf, sizeof(buf), 0);

	if (n > 0)
	{
		client->resetInactiveTicks();
		ostringstream	oss;
		oss << "received " << n << " bytes from fd=" << fd;
		Logger::log(DEBUG, oss.str());
		try {
			client->getResponse().setRequest(&client->getRequest());
			client->getResponse().setConfig(&client->getConf());
			client->getRequest().parse(string(buf, n));
		}
		catch (const PayloadTooLargeException& e) {
			client->getResponse().setStatusCode(413);
			client->getResponse().setBody("<html><body><h1>413 Payload Too Large</h1></body></html>");
			client->getResponse().finalizeResponse();
			client->setWriteBuffer(client->getResponse().getBuf());
			client->setState(WRITING_RESPONSE);
			struct epoll_event ev;
			ev.events = EPOLLOUT;
			ev.data.fd = fd;
			epoll_ctl(this->_epollFd, EPOLL_CTL_MOD, fd, &ev);
			return;
		}
		catch (const BadRequestException& e) {
			client->getResponse().setStatusCode(400);
			client->getResponse().setBody("<html><body><h1>400 Bad Request</h1></body></html>");
			client->getResponse().finalizeResponse();
			client->setWriteBuffer(client->getResponse().getBuf());
			client->setState(WRITING_RESPONSE);
			struct epoll_event ev;
			ev.events = EPOLLOUT;
			ev.data.fd = fd;
			epoll_ctl(this->_epollFd, EPOLL_CTL_MOD, fd, &ev);
			return;
		}
		catch (const exception& e) {
			this->_closeClient(fd);
			return ;
		}
		if (client->getRequest().getState() == FINISH)
		{
			CgiHandler	*cgi = client->getResponse().buildResponse();

			if (cgi != NULL && cgi->isFinished())
			{
				client->getResponse().finalizeResponse();
				client->setState(WRITING_RESPONSE);
				client->setActiveCgi(NULL);
				delete cgi;
			}
			else if (cgi != NULL)
			{
				client->setActiveCgi(cgi);
				client->setState(RUNNING_CGI);
				struct epoll_event evOut;
				evOut.events = EPOLLIN;
				evOut.data.fd = cgi->getFdOut();
				epoll_ctl(this->_epollFd, EPOLL_CTL_ADD, cgi->getFdOut(), &evOut);
				this->_cgiFdToClient[cgi->getFdOut()] = fd;

				if (cgi->hasBodyToWrite())
				{
					epoll_event evIn;
					evIn.events = EPOLLOUT;
					evIn.data.fd = cgi->getFdIn();
					epoll_ctl(this->_epollFd, EPOLL_CTL_ADD, cgi->getFdIn(), &evIn);
					this->_cgiFdToClient[cgi->getFdIn()] = fd;
				}
				else
				{
					int	fdIn = cgi->getFdIn();
					cgi->closeInput();
					this->_cgiFdToClient.erase(fdIn);
				}
				return;
			}

			client->setWriteBuffer(client->getResponse().getBuf());
			struct epoll_event	ev;
			ev.events = EPOLLOUT;
			ev.data.fd = fd;
			if (epoll_ctl(this->_epollFd, EPOLL_CTL_MOD, fd, &ev) < 0)
			{
				close(fd);
				throw SocketException("epoll_ctl change fd state failed");
			}
		}
	}
	else
		this->_closeClient(fd);
}

void ServerManager::_closeClient(int fd)
{
	map<int, Client*>::iterator it = this->_clients.find(fd);
	if (it == this->_clients.end())
		return;
	Client* client = it->second;

	CgiHandler* cgi = client->getActiveCgi();
	if (cgi != NULL)
	{
		if (cgi->getFdIn() >= 0)
		{
			epoll_ctl(_epollFd, EPOLL_CTL_DEL, cgi->getFdIn(), NULL);
			this->_cgiFdToClient.erase(cgi->getFdIn());
			cgi->closeInput();
		}
		if (cgi->getFdOut() >= 0)
		{
			epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, cgi->getFdOut(), NULL);
			this->_cgiFdToClient.erase(cgi->getFdOut());
			cgi->closeOutput();
		}
		kill(cgi->getPid(), SIGKILL);
		waitpid(cgi->getPid(), NULL, 0);
		client->setActiveCgi(NULL);
		delete cgi;
	}

	ostringstream	oss;
	oss << "Client closed connection fd=" << fd;
	Logger::log(INFO, oss.str());
	epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, fd, NULL);
	close(fd);
	delete client;
	this->_clients.erase(it);
}

bool	ServerManager::_writeToClient(Client *client)
{
	const string& buf = client->getWriteBuffer();
	size_t remaining = buf.size() - client->getWriteOffset();

	ssize_t bytesSend = send(client->getFd(), buf.c_str() + client->getWriteOffset(), remaining, MSG_NOSIGNAL);

	if (bytesSend < 0)
		return true;

	client->increaseWriteOffset(bytesSend);
	return (client->getWriteOffset() >= buf.size());
}

void	ServerManager::_checkClientsTimeout(void)
{
	map<int, Client*>::iterator it = this->_clients.begin();
	while (it != this->_clients.end())
	{ 
		map<int, Client*>::iterator	curr = it;
		++it;
		if (curr->second->getInactiveTicks() >= 3000)
			this->_closeClient(curr->second->getFd());
		else
			curr->second->incrementInactiveTicks();
	}
}

bool	ServerManager::_isListenSocket(int fd) const
{
	return (find(this->_listenFds.begin(), this->_listenFds.end(), fd) != this->_listenFds.end());
}

void ServerManager::_handleCgiEvent(int pipeFd, uint32_t evMask)
{
	map<int, int>::iterator cgiit = this->_cgiFdToClient.find(pipeFd);
	if (cgiit == this->_cgiFdToClient.end())
		return;
	int clientFd = cgiit->second;
	map<int, Client*>::iterator clientit = this->_clients.find(clientFd);
	if (clientit == this->_clients.end())
		return;
	Client* client = clientit->second;
	CgiHandler* cgi = client->getActiveCgi();

	if (!cgi)
		return ;

	if (evMask & EPOLLOUT)
	{
		cgi->handleWritable();
		if (cgi->inputDone())
		{
			epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, cgi->getFdIn(), NULL);
			this->_cgiFdToClient.erase(cgi->getFdIn());
			cgi->closeInput();
		}
	}
	if (evMask & (EPOLLIN | EPOLLHUP | EPOLLERR))
	{
		cgi->handleReadable();
		if (cgi->isFinished())
		{
			epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, cgi->getFdOut(), NULL);
			this->_cgiFdToClient.erase(cgi->getFdOut());
			cgi->closeOutput();

			client->getResponse().finalizeResponse();
			client->setWriteBuffer(client->getResponse().getBuf());
			client->setState(WRITING_RESPONSE);

			client->setActiveCgi(NULL);
			delete cgi;

			epoll_event ev;
			ev.events = EPOLLOUT;
			ev.data.fd = clientFd;
			epoll_ctl(this->_epollFd, EPOLL_CTL_MOD, clientFd, &ev);

			return;
		}
	}
}

void	ServerManager::run(void)
{
	struct epoll_event	events[64];

	while (!g_stop)
	{
		int n = epoll_wait(this->_epollFd, events, 64, 10000);
		if (n < 0)
		{
			if (errno == EINTR)
				continue;
			throw SocketException("epoll wait failed");
		}
		for (int i = 0; i < n; ++i)
		{
			int	fd = events[i].data.fd;

			if (this->_isListenSocket(fd))
				this->_handleNewConnection(fd);
			else if (this->_cgiFdToClient.count(fd))
				this->_handleCgiEvent(fd, events[i].events);
			else if (events[i].events & EPOLLIN)
				this->_handleClientRead(fd);
			else if (events[i].events & EPOLLOUT)
			{
				map<int, Client*>::iterator wit = this->_clients.find(fd);
				if (wit == this->_clients.end())
					continue;

				if (this->_writeToClient(wit->second))
					this->_closeClient(fd);
			}
		}
	}
	map<int, Client*>::iterator it = this->_clients.begin();
	while (it != this->_clients.end())
	{
		map<int, Client*>::iterator	curr = it;
		++it;
		this->_closeClient(curr->second->getFd());
	}
	for (vector<int>::iterator lfit = this->_listenFds.begin(); lfit != this->_listenFds.end(); lfit++)
		close(*lfit);
	close(this->_epollFd);
}