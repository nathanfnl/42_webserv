#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

# include <string>
# include <map>
# include <sstream>
# include <iostream>
# include <fstream>
# include <sys/stat.h>
# include <dirent.h>
# include <cstdio>
# include "HttpRequest.hpp"
# include "../config/ServerConfig.hpp"
# include "../config/LocationConfig.hpp"

class CgiHandler;

class HttpResponse {
private:
	int						_statusCode;
	string					_statusMessage;
	map<string, string>		_headers;
	string					_body;
	string					_fullResponse;

	const HttpRequest*	_request;
	const ServerConfig*	_serverConfig;

	void	_setStatusMessage(void);
	void	_buildHeaders(void);
	void	_formatResponse(void);
	void	_makeBodyListHTML(const string& path, const string& url);

	void	_handleGet(const string& path);
	void	_handlePost(const LocationConfig& location);
	void	_handleDelete(const string& path);

	string	_handleErrorPage(int code);

	string	_getContentType(const std::string& path) const;

	bool 	_isCgiRequest(const string& url, const LocationConfig& location) const;

public:
	HttpResponse();
	~HttpResponse();

	void	setRequest(const HttpRequest* req);
	void	setConfig(const ServerConfig* conf);
	void	setHeader(const string &key, const string &value);
	void	setBody(const string &value);
	void	setStatusCode(const int value);

	CgiHandler	*buildResponse(void);

	const string& getBuf(void) const;

	void	finalizeResponse(void);

	void	swapBody(string& other);
};

#endif
