#include "clients/Client.hpp"

using namespace std;

Client::Client(int fd, const ServerConfig& conf) : _fd(fd), _inactiveTicks(0),
	_writeOffset(0), _request(conf), _conf(conf), _activeCgi(NULL), _state(READING_REQUEST) {}

Client::~Client(void)
{
	if (this->_activeCgi != NULL)
		delete this->_activeCgi;
}

int	Client::getFd(void) const
{
	return (this->_fd);
}

int		Client::getInactiveTicks(void) const
{
	return (this->_inactiveTicks);
}

void	Client::incrementInactiveTicks(void)
{
	this->_inactiveTicks++;
}

void	Client::resetInactiveTicks(void)
{
	this->_inactiveTicks = 0;
}

void	Client::setWriteBuffer(const string& data)
{
	this->_writeBuffer = data;
	this->_writeOffset = 0;
}

const string&	Client::getWriteBuffer(void) const
{
	return (this->_writeBuffer);
}

size_t	Client::getWriteOffset(void) const
{
	return (this->_writeOffset);
}

void	Client::increaseWriteOffset(const size_t n)
{
	this->_writeOffset += n;
}

void	Client::advanceWriteOffset(size_t n)
{
	this->_writeOffset += n;
}

bool	Client::isWriteComplete(void) const
{
	return (this->_writeOffset >= this->_writeBuffer.size());
}

HttpRequest	&Client::getRequest(void)
{
	return (this->_request);
}

HttpResponse	&Client::getResponse(void)
{
	return (this->_response);
}

const ServerConfig	&Client::getConf(void)
{
	return (this->_conf);
}

void		Client::setActiveCgi(CgiHandler* cgi)
{
	this->_activeCgi = cgi;
}

CgiHandler*	Client::getActiveCgi() const
{
	return this->_activeCgi;
}

void		Client::setState(ClientState s)
{
	this->_state = s;
}

ClientState	Client::getState() const
{
	return this->_state;
}