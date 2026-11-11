#include "../../includes/http/HttpResponse.hpp"
#include "../../includes/utils/Logger.hpp"
#include "cgi/CgiHandler.hpp"
#include "exceptions/CgiException.hpp"
#include <sys/stat.h>
#include "utils/findLocation.hpp"

using namespace std;

HttpResponse::HttpResponse() : _statusCode(200), _statusMessage("OK"), _request(NULL), _serverConfig(NULL) {}

HttpResponse::~HttpResponse() {}

void HttpResponse::setRequest(const HttpRequest* req) {
	this->_request = req;
}

void HttpResponse::setConfig(const ServerConfig* conf) {
	this->_serverConfig = conf;
}

void	HttpResponse::setHeader(const string &key, const string &value)
{
	this->_headers.insert(make_pair(key, value));
}

void	HttpResponse::setBody(const string &value)
{
	this->_body = value;
}

void	HttpResponse::setStatusCode(const int value)
{
	this->_statusCode = value;
}

const string& HttpResponse::getBuf(void) const {
	return this->_fullResponse;
}

void HttpResponse::_setStatusMessage(void) {
	switch (this->_statusCode)
	{
		case 200: this->_statusMessage = "OK"; break;
		case 201: this->_statusMessage = "Created"; break;
		case 204: this->_statusMessage = "No Content"; break;
		case 301: this->_statusMessage = "Moved Permanently"; break;
		case 400: this->_statusMessage = "Bad Request"; break;
		case 403: this->_statusMessage = "Forbidden"; break;
		case 404: this->_statusMessage = "Not Found"; break;
		case 405: this->_statusMessage = "Method Not Allowed"; break;
		case 409: this->_statusMessage = "Conflict"; break;
		case 413: this->_statusMessage = "Payload Too Large"; break;
		case 500: this->_statusMessage = "Internal Server Error"; break;
		default:  this->_statusMessage = "Unknown Error"; break;
	}
}

string HttpResponse::_getContentType(const string& path) const
{
	size_t dotPos = path.find_last_of('.');
	
	if (dotPos == string::npos)
		return "application/octet-stream"; 

	string ext = path.substr(dotPos);
	
	if (ext == ".html" || ext == ".htm") return "text/html";
	if (ext == ".css") return "text/css";
	if (ext == ".js") return "application/javascript";
	if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
	if (ext == ".png") return "image/png";
	if (ext == ".gif") return "image/gif";
	if (ext == ".ico") return "image/x-icon";
	if (ext == ".txt") return "text/plain";
	if (ext == ".json") return "application/json";
	if (ext == ".pdf") return "application/pdf";
	if (ext == ".xpm") return "image/x-xpixmap";

	return "application/octet-stream";
}

void HttpResponse::_buildHeaders(void) {
	ostringstream lengthStream;
	lengthStream << this->_body.length();

	this->_headers["Content-Length"] = lengthStream.str();

	if (this->_headers.find("Content-Type") == this->_headers.end())
		this->_headers["Content-Type"] = "text/html";

	this->_headers["Server"] = "Webserv/1.0";
	this->_headers["Connection"] = "close";
}

void HttpResponse::_formatResponse(void) {
	ostringstream headerStream;
	headerStream << "HTTP/1.1 " << this->_statusCode << " " << this->_statusMessage << "\r\n";
	for (map<string, string>::const_iterator it = this->_headers.begin(); it != this->_headers.end(); ++it)
		headerStream << it->first << ": " << it->second << "\r\n";
	headerStream << "\r\n";

	string headerStr = headerStream.str();

	this->_fullResponse.clear();
	this->_fullResponse.reserve(headerStr.size() + this->_body.size());
	this->_fullResponse = headerStr;
	this->_fullResponse.append(this->_body);
}

bool isMethodAllowed(const string& method, const LocationConfig& location)
{
	if (location.getAllowedMethods().empty())
		return (true);
		
	for (size_t i = 0; i < location.getAllowedMethods().size(); i++)
		if (method == location.getAllowedMethods().at(i))
			return (true);
	return (false);
}

string HttpResponse::_handleErrorPage(int code)
{
	const map<int, string>& errorPages = this->_serverConfig->getErrorPages();
	map<int, string>::const_iterator it = errorPages.find(code);

	if (it == errorPages.end())
		throw runtime_error("no error page");
		
	string path = it->second;
	
	ifstream file(path.c_str(), ios::in | ios::binary);
	if (!file.is_open())
		throw runtime_error("cannot open error page file");

	ostringstream ss;
	ss << file.rdbuf();
	
	return ss.str();
}

void HttpResponse::_makeBodyListHTML(const string& path, const string& url)
{
	DIR* dir;
	struct dirent* ent;

	if ((dir = opendir(path.c_str())) != NULL)
	{
		ostringstream html;
		
		html << "<html><head><title>Index of " << url << "</title></head>";
		html << "<body style=\"font-family: Arial, sans-serif;\">";
		html << "<h1>Index of " << url << "</h1><hr><ul>";

		while ((ent = readdir(dir)) != NULL)
		{
			string filename = ent->d_name;
			
			if (filename == "." || filename == "..")
				continue;

			string href = url;
			if (href.length() > 0 && href[href.length() - 1] != '/')
				href += "/";
			href += filename;

			html << "<li style=\"margin: 5px 0;\"><a href=\"" << href << "\">" << filename << "</a></li>";
		}
		
		html << "</ul><hr></body></html>";
		closedir(dir);
		
		this->_body = html.str();
	}
	else
	{
		this->_statusCode = 403;
		this->_body = "<html><body><h1>403 Forbidden</h1></body></html>";
	}
}

void HttpResponse::_handleGet(const string& path)
{
	ifstream file(path.c_str(), ios::in | ios::binary);
	
	if (!file.is_open())
	{
		this->_statusCode = 403;
		this->_body = "<html><body><h1>403 Forbidden</h1></body></html>";
		return;
	}

	ostringstream ss;
	ss << file.rdbuf();
	this->_body = ss.str();

	this->_headers["Content-Type"] = _getContentType(path);
}

void HttpResponse::_handlePost(const LocationConfig& location)
{
	if (!location.getAllowUpload())
	{
		this->_statusCode = 403;
		this->_body = "<html><body><h1>403 Forbidden</h1><p>File upload is not allowed for this location.</p></body></html>";
		return;
	}

	string uploadDir = location.getUploadStore();
	if (uploadDir.empty())
	{
		this->_statusCode = 500;
		this->_body = "<html><body><h1>500 Internal Server Error</h1><p>Upload directory is not configured.</p></body></html>";
		return;
	}

	struct stat info;
	if (stat(uploadDir.c_str(), &info) != 0) 
	{
		if (mkdir(uploadDir.c_str(), 0755) != 0) 
		{
			this->_statusCode = 500;
			this->_body = "<html><body><h1>500 Internal Server Error</h1><p>Could not create upload directory.</p></body></html>";
			return;
		}
	}
	else if (!S_ISDIR(info.st_mode)) 
	{
		this->_statusCode = 500;
		this->_body = "<html><body><h1>500 Internal Server Error</h1><p>Upload path is not a directory.</p></body></html>";
		return;
	}

	string url = cleanUrl(this->_request->getUrl());
	string filename;
	size_t lastSlash = url.find_last_of('/');
	if (lastSlash != string::npos)
		filename = url.substr(lastSlash + 1);
	else
		filename = url;

	if (filename.empty() || filename == "." || filename == "..")
	{
		this->_statusCode = 400;
		this->_body = "<html><body><h1>400 Bad Request</h1><p>Invalid filename.</p></body></html>";
		return;
	}

	string filePath = uploadDir;
	if (filePath.length() > 0 && filePath[filePath.length() - 1] != '/') 
		filePath += "/";
	filePath += filename;

	bool fileExists = (access(filePath.c_str(), F_OK) == 0);

	ofstream outFile(filePath.c_str(), ios::out | ios::binary | ios::trunc);
	if (!outFile.is_open())
	{
		this->_statusCode = 500;
		this->_body = "<html><body><h1>500 Internal Server Error</h1><p>Could not open file for writing.</p></body></html>";
		return;
	}

	outFile.write(this->_request->getBody().c_str(), this->_request->getBody().size());
	outFile.close();

	if (outFile.fail())
	{
		this->_statusCode = 500;
		this->_body = "<html><body><h1>500 Internal Server Error</h1><p>Error writing to file.</p></body></html>";
		return;
	}

	if (fileExists)
	{
		this->_statusCode = 200;
		this->_body = "<html><body><h1>200 OK</h1><p>File updated successfully.</p></body></html>";
	}
	else
	{
		this->_statusCode = 201;
		this->_body = "<html><body><h1>201 Created</h1><p>File uploaded successfully.</p></body></html>";
		this->_headers["Location"] = url;
	}
}

void HttpResponse::_handleDelete(const string& path)
{
	int status = remove(path.c_str());

	if (status != 0)
	{
		this->_statusCode = 403;
		this->_body = "<html><body><h1>403 Forbidden</h1></body></html>";
	}
	else
		this->_statusCode = 204;
}

CgiHandler	*HttpResponse::buildResponse(void) {
	if (!this->_request || !this->_serverConfig) 
	{
		this->_statusCode = 500;
		this->_body = "<h1>500 Internal Server Error</h1>";
	}
	else
	{
		string method = this->_request->getMethod();
		string raw_url = this->_request->getUrl();
		string url = cleanUrl(raw_url);
		
		LocationConfig location = findLocation(url, *this->_serverConfig); 

		if(!isMethodAllowed(method, location))
		{
			this->_statusCode = 405;
			finalizeResponse();
			return (NULL);
		}
		else if (location.getMaxBodySize() > 0 && _request->getBody().size() > location.getMaxBodySize())
		{
			this->_statusCode = 413;
			finalizeResponse();
			return (NULL);
		}

		if (_isCgiRequest(url, location))
		{
			CgiHandler	*cgi = new CgiHandler(*this->_request, location, *this->_serverConfig, *this);
			try {
				cgi->start();
			} catch (const CgiParentException& e)
			{
				delete cgi;
				this->_statusCode = 500;
				this->_body = "<html><body><h1>500 Internal Server Error</h1></body></html>";
				finalizeResponse();
			}
			return (cgi);
		}

		if (location.getReturnCode() != 0)
		{
			this->_statusCode = location.getReturnCode();
			_setStatusMessage();
			this->_headers["Location"] = location.getReturnUrl();
			_buildHeaders();
			_formatResponse();
			return (NULL);
		}
		if (this->_statusCode == 200)
		{
			if (method == "POST")
				_handlePost(location);
			else
			{
				string root = location.getRoot().empty() ? this->_serverConfig->getRoot() : location.getRoot();
				vector<string> indexs = location.getIndex().empty() ? this->_serverConfig->getIndex() : location.getIndex();
				string path;
				if (location.getRoot().empty())
					path = root + url;
				else
				{
					string url_remainder = "";
					
					if (url.find(location.getPath()) == 0)
						url_remainder = url.substr(location.getPath().length());
					else if (location.getPath() == url + "/")
						url_remainder = "";
					else
						url_remainder = url;

					if (root[root.length() - 1] != '/' && url_remainder.length() > 0 && url_remainder[0] != '/')
						path = root + "/" + url_remainder;
					else if (root[root.length() - 1] == '/' && url_remainder.length() > 0 && url_remainder[0] == '/')
						path = root + url_remainder.substr(1);
					else
						path = root + url_remainder;
				}
				struct stat info;
				if (stat(path.c_str(), &info) != 0)
				{
					this->_statusCode = 404;
					this->_body = "<html><body><h1>404 Not Found</h1></body></html>";
				}
				else
				{
					if (S_ISDIR(info.st_mode))
					{
						bool indexFound = false;
						if (!indexs.empty())
						{
							for (size_t i = 0; i < indexs.size(); i++)
							{
								string testPath = path;
								if (testPath[testPath.length() - 1] != '/')
									testPath += "/";
								testPath += indexs[i];

								struct stat indexInfo;
								if (stat(testPath.c_str(), &indexInfo) == 0 && S_ISREG(indexInfo.st_mode))
								{
									path = testPath;
									indexFound = true;
									break;
								}
							}
						}

						if (!indexFound && location.getAutoindex())
							_makeBodyListHTML(path, url);
						else if (!indexFound && !location.getAutoindex())
						{
							this->_statusCode = 404;
							this->_body = "<html><body><h1>404 Not Found</h1></body></html>";
						}
						
						if (indexFound)
						{
							if (method == "GET")
								_handleGet(path);
							else if (method == "DELETE")
								_handleDelete(path);
						}
					}
					else if (S_ISREG(info.st_mode))
					{
						if (method == "GET")
							_handleGet(path);
						else if (method == "DELETE")
							_handleDelete(path);
					}
				}
			}
		}
	}
	finalizeResponse();
	return (NULL);
}

bool HttpResponse::_isCgiRequest(const string& url, const LocationConfig& location) const
{
	string ext = location.getCgiExtension();
	if (ext.empty())
		return false;
	string clean = url;
	size_t q = clean.find('?');
	if (q != string::npos)
		clean = clean.substr(0, q);
	return clean.find(ext) != string::npos;
}

void	HttpResponse::finalizeResponse(void)
{
	if (this->_statusCode >= 400)
	{
		try {
			this->_body = _handleErrorPage(this->_statusCode);
			this->_headers["Content-Type"] = "text/html";
		} catch (const exception& e) { }
	}
	this->_setStatusMessage();
	this->_buildHeaders();
	this->_formatResponse();
}

void	HttpResponse::swapBody(string& other)
{
	this->_body.swap(other);
}
