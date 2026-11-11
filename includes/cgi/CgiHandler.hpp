#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include <iostream>
# include "http/HttpRequest.hpp"
# include "config/ServerConfig.hpp"
# include "config/LocationConfig.hpp"
# include <vector>
# include <map>
# include <algorithm>
# include <unistd.h>
# include <cstring>
# include <sys/wait.h>
# include <fcntl.h>

using namespace std;

class HttpResponse;

class CgiHandler
{
private:
	const HttpRequest&		_request;
	const LocationConfig&	_location;
	const ServerConfig&		_server;
	HttpResponse&			_response;

	string					_scriptPath;
	string					_responseBody;
	map<string, string>		_cgiHeaders;
	int						_statusCode;

	int						_fdIn;
	int						_fdOut;
	pid_t					_pid;
	size_t					_bodyWritten;
	string					_outputBuffer;
	bool					_inputDone;
	bool					_finished;
	time_t					_startTime;

	string					_getCleanUrl() const;
	string					_extractScriptName() const;
	string					_extractPathInfo() const;
	string					_getQueryString() const;
	string					_getServerName() const;
	vector<string>			_buildEnvStrings();
	size_t					_findHeaderEnd(const string& output) const;
	void					_parseOutput(string& output);
	char					*_stringToChar(const string &s);

public:
	CgiHandler(const HttpRequest& request, const LocationConfig& location, const ServerConfig& server, HttpResponse& response);
	~CgiHandler(void);

	void	start(void);
	void	handleWritable(void);
	void	handleReadable(void);
	void	finalize(void);

	bool	isFinished(void) const;
	int		getFdIn(void) const;
	int		getFdOut(void) const;
	time_t	getStartTime(void) const;
	bool	inputDone(void) const;
	bool	hasBodyToWrite(void) const;
	pid_t	getPid(void) const;
	void	closeInput(void);
	void	closeOutput(void);

};

#endif