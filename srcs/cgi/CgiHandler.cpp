#include "cgi/CgiHandler.hpp"
#include "utils/Logger.hpp"
#include "exceptions/CgiException.hpp"
#include "http/HttpResponse.hpp"

using namespace std;


CgiHandler::CgiHandler(const HttpRequest& request, const LocationConfig& location,
						const ServerConfig& server, HttpResponse& response)
	: _request(request), _location(location), _server(server), _response(response),
	_statusCode(0), _fdIn(-1),  _fdOut(-1), _bodyWritten(0), _inputDone(false) {}

CgiHandler::~CgiHandler(void)
{
	this->closeInput();
	this->closeOutput();
}

bool	CgiHandler::isFinished() const{ return this->_finished; }

int		CgiHandler::getFdIn()  const { return this->_fdIn; }

int		CgiHandler::getFdOut() const { return this->_fdOut; }

time_t	CgiHandler::getStartTime() const { return this->_startTime; }

bool	CgiHandler::inputDone() const { return this->_inputDone; }

bool	CgiHandler::hasBodyToWrite() const { return this->_request.getBody().size() > this->_bodyWritten; }

pid_t	CgiHandler::getPid() const { return this->_pid; }

static string cleanUrlSlash(const string& url)
{
	string result;
	bool lastWasSlash = false;
	
	for (size_t i = 0; i < url.length(); ++i)
	{
		if (url[i] == '/')
		{
			if (!lastWasSlash)
			{
				result += '/';
				lastWasSlash = true;
			}
		}
		else
		{
			result += url[i];
			lastWasSlash = false;
		}
	}
	return result;
}

void	CgiHandler::closeInput(void)
{
	if (this->_fdIn != -1)
	{
		close(this->_fdIn);
		this->_fdIn = -1;
	}
}

void	CgiHandler::closeOutput(void)
{
	if (this->_fdOut != -1)
	{
		close(this->_fdOut);
		this->_fdOut = -1;
	}
}

string	CgiHandler::_getCleanUrl() const
{
	string url = this->_request.getUrl();
	size_t query = url.find('?');
	if (query != string::npos)
		url = url.substr(0, query);
	url = cleanUrlSlash(url);
	return url;
}

string	CgiHandler::_extractScriptName() const
{
	string	url = this->_getCleanUrl();
	string	ext = this->_location.getCgiExtension();
	size_t	pos = url.find(ext);
	size_t	slashPos = url.rfind("/");
	if (pos == string::npos)
		throw CgiParentException("Error: CGI: extension is not corresponding with conf");
	if (slashPos == string::npos)
		slashPos = 0;
	else
		slashPos += 1;
	size_t end = pos + ext.size();
	if (end < url.size() && url[end] != '/')
		throw CgiParentException("Error: CGI: script name not found");
	return url.substr(slashPos, end);
}

string	CgiHandler::_extractPathInfo() const
{
	string url = this->_getCleanUrl();
	string ext = this->_location.getCgiExtension();
	size_t pos = url.find(ext);

	if (pos == string::npos)
		return "";

	size_t end = pos + ext.size();
	if (end >= url.size())
		return "";

	return url.substr(end);
}

string CgiHandler::_getQueryString() const
{
	string url = this->_request.getUrl();
	size_t pos = url.find('?');
	if (pos == string::npos)
		return "";
	return url.substr(pos + 1);
}


string CgiHandler::_getServerName() const
{
	map<string, string>::const_iterator it = this->_request.getHeaders().find("Host");
	if (it == this->_request.getHeaders().end())
		return "";
	string host = it->second;
	size_t colon = host.find(':');
	if (colon != string::npos)
		host = host.substr(0, colon);
	return host;
}

vector<string> CgiHandler::_buildEnvStrings()
{
	vector<string> env;

	env.push_back("REQUEST_METHOD=" + this->_request.getMethod());
	env.push_back("QUERY_STRING=" + this->_getQueryString());
	env.push_back("SERVER_PROTOCOL=" + this->_request.getVersion());
	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back("REQUEST_URI=" + this->_request.getUrl());
	env.push_back("SERVER_SOFTWARE=Webserv/1.0");
	env.push_back("REDIRECT_STATUS=200");

	map<string,string>::const_iterator type =  this->_request.getHeaders().find("Content-Type");
	if (type != this->_request.getHeaders().end())
		env.push_back("CONTENT_TYPE=" + type->second);

	if (this->_request.getMethod() == "POST")
	{
		ostringstream length;
		length << "CONTENT_LENGTH=" << this->_request.getBody().size();
		env.push_back(length.str());
	}

	string scriptName = this->_extractScriptName();

	string rawUrl = this->_request.getUrl();
	size_t qPos = rawUrl.find('?');
	this->_scriptPath = (qPos != string::npos) ? rawUrl.substr(1, qPos - 1) : rawUrl.substr(1);

	string pathInfo = this->_extractPathInfo();
	env.push_back("PATH_INFO=" + pathInfo);
	if (!pathInfo.empty())
		env.push_back("PATH_TRANSLATED=" + this->_location.getRoot() + pathInfo);

	env.push_back("SCRIPT_NAME=" + this->_scriptPath);
	env.push_back("SCRIPT_FILENAME=" + this->_scriptPath);
	env.push_back("SERVER_NAME=" + this->_getServerName());

	ostringstream port;
	port << "SERVER_PORT=" << this->_server.getPort();
	env.push_back(port.str());

	const map<string,string>& headers = this->_request.getHeaders();
	for (map<string,string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		if (it->first == "Content-Type" || it->first == "Content-Length")
			continue;
		string name = "HTTP_" + it->first;
		replace(name.begin(), name.end(), '-', '_');
		transform(name.begin(), name.end(), name.begin(), ::toupper);
		env.push_back(name + "=" + it->second);
	}
	
	return env;
}

size_t	CgiHandler::_findHeaderEnd(const string& output) const
{
	size_t	posCRLF = output.find("\r\n\r\n");
	size_t	posLF = output.find("\n\n");

	if (posCRLF == string::npos && posLF == string::npos)
		return (string::npos);
	if (posCRLF == string::npos)
		return (posLF);
	if (posLF == string::npos)
		return (posCRLF);
	return (min(posCRLF, posLF));
}

void	CgiHandler::_parseOutput(string& output)
{
	size_t	sepPos = this->_findHeaderEnd(output);

	if (sepPos == string::npos)
	{
		this->_response.setHeader("Content-Type", "text/plain");
		this->_response.swapBody(output);
		this->_response.setStatusCode(200);
		return;
	}

	size_t sepLen = (output.compare(sepPos, 4, "\r\n\r\n") == 0) ? 4 : 2;
	string header = output.substr(0, sepPos);

	output.erase(0, sepPos + sepLen);

	istringstream headerStream(header);
	string line;
	bool hasContentType = false;
	int statusCode = 200;
	string statusMsg = "OK";

	while (getline(headerStream, line))
	{
		if (!line.empty() && line.substr(line.size() - 1) == "\r")
			line.erase(line.size() - 1);
		if (line.empty())
			continue;

		size_t colon = line.find(':');
		if (colon == string::npos)
			continue;

		string key = line.substr(0, colon);
		size_t valStart = colon + 1;
		while (valStart < line.size() && line[valStart] == ' ')
			valStart++;
		string value = line.substr(valStart);

		if (key == "Status")
		{
			istringstream ss(value);
			ss >> statusCode;
			getline(ss, statusMsg);
			if (!statusMsg.empty() && statusMsg[0] == ' ')
				statusMsg = statusMsg.substr(1);
		}
		else if (key == "Location")
		{
			this->_response.setHeader("Location", value);
			if (statusCode == 200)
				statusCode = 302;
		}
		else
		{
			if (key == "Content-Type")
				hasContentType = true;
			this->_response.setHeader(key, value);
		}
	}

	if (!hasContentType)
		this->_response.setHeader("Content-Type", "text/html");

	this->_response.setStatusCode(statusCode);
	this->_response.swapBody(output);
}

char	*CgiHandler::_stringToChar(const string &s)
{
	char	*pc = new char[s.size() + 1];
	strcpy(pc, s.c_str());
	return (pc);
}

void CgiHandler::start()
{
	vector<string> env = this->_buildEnvStrings();
	string execPath = this->_location.getCgiPath();

	if (access(execPath.c_str(), X_OK) != 0)
	{
		this->_response.setStatusCode(403);
		this->_response.setBody("<html><body><h1>403 Forbidden</h1></body></html>");
		this->_finished = true;
		return ;
	}

	int fdsIn[2], fdsOut[2];
	if (pipe(fdsIn) == -1)
		throw CgiParentException("pipe failed");
	if (pipe(fdsOut) == -1)
	{
		close(fdsIn[0]); close(fdsIn[1]);
		throw CgiParentException("pipe failed");
	}
	cout << "exec path = " << execPath << "/ script path = " << _scriptPath << " / root = " << this->_location.getRoot() << endl;
	for (size_t i = 0; i < env.size(); ++i)
	{
		cout << env[i] << endl;
	}
	pid_t pid = fork();
	if (pid < 0)
	{
		close(fdsIn[0]); close(fdsIn[1]);
		close(fdsOut[0]); close(fdsOut[1]);
		throw CgiParentException("fork failed");
	}

	if (pid == 0)
	{
		chdir(this->_location.getRoot().c_str());
		dup2(fdsIn[0], STDIN_FILENO);
		dup2(fdsOut[1], STDOUT_FILENO);
		close(fdsIn[0]); close(fdsIn[1]);
		close(fdsOut[0]); close(fdsOut[1]);

		char	**envArr = new char*[env.size() + 1];
		for (size_t i = 0; i < env.size(); ++i)
			envArr[i] = _stringToChar(env[i]);
		envArr[env.size()] = NULL;

		char	*argv[3];
		argv[0] = _stringToChar(execPath);
		argv[1] = _stringToChar(this->_scriptPath);
		argv[2] = NULL;
		
		execve(argv[0], argv, envArr);

		Logger::log(CRITICAL, string("execve failed: ") + strerror(errno));
		delete [] argv[0];
		delete [] argv[1];
		for (size_t i = 0; i < env.size() + 1; i++)
			delete [] envArr[i];
		delete [] envArr;
		throw GenericCgiException();
	}

	close(fdsIn[0]);
	close(fdsOut[1]);

	fcntl(fdsIn[1], F_SETFL, fcntl(fdsIn[1], F_GETFL) | O_NONBLOCK);
	fcntl(fdsOut[0], F_SETFL, fcntl(fdsOut[0], F_GETFL) | O_NONBLOCK);

	this->_pid = pid;
	this->_fdIn = fdsIn[1];
	this->_fdOut = fdsOut[0];
	this->_bodyWritten = 0;
	this->_inputDone = this->_request.getBody().empty();
	this->_finished = false;
	this->_startTime = time(NULL);
}

void CgiHandler::handleWritable()
{
	const string& body = this->_request.getBody();

	if (this->_bodyWritten >= body.size())
	{
		this->_inputDone = true;
		return;
	}

	size_t remaining = body.size() - this->_bodyWritten;
	ssize_t n = write(this->_fdIn, body.c_str() + this->_bodyWritten, remaining);

	if (n == 0)
	{
		this->_inputDone = true;
		return ;
	}
	else if (n < 0)
		return;
	this->_bodyWritten += n;
	if (this->_bodyWritten == body.size())
		this->_inputDone = true;
}

void CgiHandler::handleReadable()
{
	if (this->_outputBuffer.capacity() == 0)
		this->_outputBuffer.reserve(65536);

	char buf[65536];
	ssize_t n = read(this->_fdOut, buf, sizeof(buf));

	if (n > 0)
		this->_outputBuffer.append(buf, n);
	else if (n == 0)
	{
		finalize();
		return ;
	}
}

void CgiHandler::finalize()
{
	int waitStatus = 0;
	waitpid(this->_pid, &waitStatus, 0);

	int exitCode = WIFEXITED(waitStatus) ? WEXITSTATUS(waitStatus) : -1;

	if (exitCode == 127)
	{
		this->_response.setStatusCode(404);
		this->_response.setBody("<html><body><h1>404 Not Found 4</h1></body></html>");
	}
	else if (exitCode == 126)
	{
		this->_response.setStatusCode(403);
		this->_response.setBody("<html><body><h1>403 Forbidden</h1></body></html>");
	}
	// else if (WIFSIGNALED(waitStatus) || exitCode != 0)
	// {
	// 	cout << "=== RAW CGI OUTPUT ===" << endl << this->_outputBuffer << endl << "=== END ===" << endl;
	// 	this->_response.setStatusCode(500);
	// 	this->_response.setBody("<html><body><h1>500 Internal Server Error CGI</h1></body></html>");
	// }
	else
	{
	cout << "=== RAW CGI OUTPUT ===" << endl << this->_outputBuffer << endl << "=== END ===" << endl;

		this->_parseOutput(this->_outputBuffer);
	}
		this->_finished = true;
}