#include "http/HttpRequest.hpp"
#include "utils/findLocation.hpp"
#include <sys/stat.h>

using namespace std;

HttpRequest::HttpRequest(const ServerConfig& conf): _state(REQUEST_LINE), _buf(""), _bufCursor(0),
		_serverConfig(conf), _chunkedStates(WAITING_SIZE), _currentChunkSize(0) , _maxBodySize(0),
		_expectedBodySize(0), _bodySizeKnown(false) {}

HttpRequest::~HttpRequest(void) {}

HttpStates	HttpRequest::getState(void) const
{
	return (this->_state);
}

string	HttpRequest::getVersion(void) const {
	return (this->_version);
}

const map<string, string>&	HttpRequest::getHeaders(void) const {
	return (this->_headers);
}

string	HttpRequest::getUrl(void) const {
	return (this->_url);
}

string	HttpRequest::getMethod(void) const {
	return (this->_method);
}

const string&	HttpRequest::getBody(void) const {
	return (this->_body);
}

void	HttpRequest::setMaxBodySize(size_t max) {
	this->_maxBodySize = max;
}

void HttpRequest::_waitChunkedData(void)
{
	while (true)
	{
		if (this->_chunkedStates == WAITING_SIZE)
		{
			size_t endLinePos = this->_buf.find("\r\n", this->_bufCursor);
			if (endLinePos == string::npos)
				return ;
			string line = this->_buf.substr(this->_bufCursor, endLinePos - this->_bufCursor);
			this->_bufCursor = endLinePos + 2;
			istringstream iss(line);
			if (!(iss >> std::hex >> this->_currentChunkSize))
				throw BadRequestException();
			if (this->_maxBodySize > 0 && 
				this->_body.size() + this->_currentChunkSize > this->_maxBodySize)
				throw PayloadTooLargeException();
			if (this->_currentChunkSize == 0)
			{
				this->_state = FINISH;
				return ;
			}
			this->_chunkedStates = WAITING_DATA;
		}
		else if (this->_chunkedStates == WAITING_DATA)
		{
			if (this->_buf.size() - this->_bufCursor < this->_currentChunkSize + 2)
				return ;
			this->_body += this->_buf.substr(this->_bufCursor, this->_currentChunkSize);
			this->_bufCursor += this->_currentChunkSize + 2;
			this->_chunkedStates = WAITING_SIZE;
		}
	}
}

HttpStates HttpRequest::_determineNextState(void)
{
	map<string, string>::const_iterator clIt = this->_headers.find("Content-Length");
	map<string, string>::const_iterator teIt = this->_headers.find("Transfer-Encoding");
	bool hasCL = (clIt != this->_headers.end());
	bool hasTE = (teIt != this->_headers.end());

	if (hasCL && hasTE)
		throw BadRequestException();

	if (hasTE)
	{
		this->_waitChunkedData();
		return (this->_state == FINISH) ? FINISH : BODY;
	}

	if (!hasCL)
		return (FINISH);

	if (!this->_bodySizeKnown)
	{
		size_t n;
		istringstream iss(clIt->second);
		if (!(iss >> n))
			throw BadRequestException();
		if (this->_maxBodySize > 0 && n > this->_maxBodySize)
			throw PayloadTooLargeException();
		this->_expectedBodySize = n;
		this->_bodySizeKnown = true;
		this->_body.reserve(n);
	}

	size_t available = this->_buf.size() - this->_bufCursor;
	size_t needed = this->_expectedBodySize - this->_body.size();
	size_t toAppend = min(available, needed);

	if (toAppend > 0)
	{
		this->_body.append(this->_buf, this->_bufCursor, toAppend);
		this->_bufCursor += toAppend;
	}

	return (this->_body.size() < this->_expectedBodySize) ? BODY : FINISH;
}

void HttpRequest::parse(const string& str)
{
	if (this->_state == FINISH)
		return ;

	this->_buf += str;

	while (this->_state == REQUEST_LINE || this->_state == HEADERS)
	{
		size_t endLinePos = this->_buf.find("\r\n", this->_bufCursor);
		if (endLinePos == string::npos)
			return ;

		string line = this->_buf.substr(this->_bufCursor, endLinePos - this->_bufCursor);
		this->_bufCursor = endLinePos + 2;

		if (this->_state == REQUEST_LINE)
		{
			istringstream iss(line);
			if (!(iss >> this->_method >> this->_url >> this->_version))
				throw BadRequestException();
			this->_state = HEADERS;
			size_t	maxBodySize = findLocation(this->_url, this->_serverConfig).getMaxBodySize();
			this->_maxBodySize = maxBodySize ? maxBodySize : this->_serverConfig.getClientMaxBodySize();
		}
		else if (this->_state == HEADERS)
		{
			if (line.empty())
			{
				this->_state = this->_determineNextState();
				break;
			}
			size_t colonPos = line.find(':');
			if (colonPos == string::npos)
				throw BadRequestException();
			string key = line.substr(0, colonPos);
			string value = line.substr(colonPos + 1);
			size_t firstNonSpace = value.find_first_not_of(' ');
			if (firstNonSpace != string::npos)
				value = value.substr(firstNonSpace);
			this->_headers[key] = value;
		}
	}

	if (this->_state == BODY)
		this->_state = this->_determineNextState();
	
	if (this->_bufCursor > 0)
	{
		this->_buf.erase(0, _bufCursor);
		this->_bufCursor = 0;
	}
}

void	HttpRequest::print(void) const
{
	cout << "STATE: " << this->_state << "\n";
	cout << "method: " << this->_method << "\nurl: " << this->_url << "\nversion: " << this->_version << "\n[HEADER]\n";
	for (map<string, string>::const_iterator it = this->_headers.begin(); it != this->_headers.end(); it++)
		cout << it->first << ": " << it->second << "\n";
	cout << "[BODY]\n";
	cout << this->_body;
}
