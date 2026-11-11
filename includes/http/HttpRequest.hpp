#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <iostream>
# include <iomanip>
# include <sstream>
# include <map>
# include "config/ServerConfig.hpp"
# include "exceptions/HttpException.hpp"
# include "config/ServerConfig.hpp"

using namespace std;

enum HttpStates { REQUEST_LINE, HEADERS, BODY, FINISH };

enum HttpMethods { GET, POST, DELETE };

enum ChunkedStates { WAITING_SIZE, WAITING_DATA };

class HttpRequest
{
private:
	HttpStates			_state;
	string				_method;
	string				_url;
	string				_version;
	map<string, string>	_headers;
	string				_body;
	string				_buf;
	size_t				_bufCursor;
	const ServerConfig&	_serverConfig;
	ChunkedStates		_chunkedStates;
	size_t				_currentChunkSize;
	size_t				_maxBodySize;
	size_t				_expectedBodySize;
	bool				_bodySizeKnown;


	HttpStates			_determineNextState(void);
	void				_waitChunkedData(void);

public:
	HttpRequest(const ServerConfig& conf);
	~HttpRequest(void);

	HttpStates					getState(void) const;
	string						getMethod(void) const;
	string						getUrl(void) const;
	string						getVersion(void) const;
	const string&				getBody(void) const;
	const map<string, string>&	getHeaders(void) const;
	void						setMaxBodySize(size_t max);

	void	parse(const string& str);
	void	print(void) const;
};

#endif
