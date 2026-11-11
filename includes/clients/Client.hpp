#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include "sockets/Socket.hpp"
# include "http/HttpRequest.hpp"
# include "http/HttpResponse.hpp"
# include "cgi/CgiHandler.hpp"

using namespace std;

enum ClientState { READING_REQUEST, RUNNING_CGI, WRITING_RESPONSE };

class Client
{
private:
	int					_fd;
	int					_inactiveTicks;
	string				_writeBuffer;
	size_t				_writeOffset;
	HttpRequest			_request;
	HttpResponse		_response;
	const ServerConfig	&_conf;

	CgiHandler			*_activeCgi;
	ClientState			_state;

	Client(const Client& other);
	Client	&operator=(const Client& other);

public:
	explicit Client(int fd, const ServerConfig& conf);
	~Client(void);

	int					getFd(void) const;

	int					getInactiveTicks(void) const;
	void				incrementInactiveTicks(void);
	void				resetInactiveTicks(void);

	void				setWriteBuffer(const string& data);
	const string&		getWriteBuffer(void) const;
	size_t				getWriteOffset(void) const;
	void				increaseWriteOffset(const size_t n);
	void				advanceWriteOffset(size_t n);
	bool				isWriteComplete(void) const;

	HttpRequest			&getRequest(void);
	HttpResponse		&getResponse(void);
	const ServerConfig	&getConf(void);

	void		setActiveCgi(CgiHandler* cgi);
	CgiHandler*	getActiveCgi() const;
	void		setState(ClientState s);
	ClientState	getState() const;
};

#endif