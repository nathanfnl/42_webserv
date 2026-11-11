# include "config/ConfigParser.hpp"

using namespace std;

ConfigParser::ConfigParser(const string& filename) 
: _configFile(filename) {}

const vector<ServerConfig>	&ConfigParser::getServers() const 
{
	return (this->_servers);
}

void ConfigParser::addServer(const ServerConfig& server)
{
	for (size_t i = 0; i < _servers.size(); i++)
	{
		if (_servers.at(i).getPort() == server.getPort())
			throw ServerAlreadyExists();
	}
	
	this->_servers.push_back(server);
}

ConfigParser::~ConfigParser() {}

void ConfigParser::_readFile(const string& filename)
{
	string		line;
	string		text;
	ifstream	file;

	file.open(filename.c_str());
	if (!file.is_open())
		throw FailToOpenFile();

	while (getline(file, line))
	{
		size_t hashPos = line.find('#');
		if (hashPos != string::npos)
			line = line.substr(0, hashPos);
		if (line.empty())
			continue;
		stringstream ss(line);
		while (ss >> text)
			this->_tokens.push_back(text);
		string last = this->_tokens.back();
		if ((last != "{" && last != "}") && last[last.size() - 1] != ';')
			throw MissingSemiColon();
	}
}

/* -----------------------------
           VERIFICATORS
--------------------------------*/

void areCurlyBracketsOk(vector<string> tokens)
{
	int nbOpening = 0;
	int nbClosing = 0;
	for (size_t i = 0; i < tokens.size(); i++)
	{
		if (tokens.at(i) == "{")
			nbOpening++;
		else if (tokens.at(i) == "}")
			nbClosing++;
	}
	if (nbClosing > nbOpening)
		throw MissingOpeningCurlyBracket();
	else if (nbClosing < nbOpening)
		throw MissingClosingCurlyBracket();
}

bool isIPValid(string &s)
{
	int n = s.size();
	if (n < 7)
		return false;
	int lastPointPos = -10;
	for (int i = 0; i < (int)s.size(); i++)
	{
		if (s[i] == '.' && (lastPointPos == (i - 1) || lastPointPos == (i + 1)))
			return false;
		if (s[i] == '.')
			lastPointPos = i;
	}

	vector<string> v;
	stringstream ss(s);
	while (ss.good())
	{
		string substr;
		getline(ss, substr, '.');
		v.push_back(substr);
	}
	if (v.size() != 4)
		return false;
	for (int i = 0; i < (int)v.size(); i++)
	{
		string temp = v[i];
		if (temp.size() > 1)
			if (temp[0] == '0')
				return false;
		for (int j = 0; j < (int)temp.size(); j++)
			if (isalpha(temp[j]))
				return false;
		stringstream is(temp);
		int nb;
		is >> nb;
		if (nb > 255)
			return false;
	}
	return true;
}

bool isValidServerName(const string& name)
{
	if (name.empty())
		throw EmptyDomainName();
	if (name.length() > 253)
		throw DomainNameToLong();

	size_t start = 0;
	size_t end = name.find('.');
	while (start < name.length())
	{
		string label = name.substr(start, end - start);
		if (label.empty() || label.length() > 63)
			throw WrongDomainNameFormat();
		if (label[0] == '-' || label[label.length() - 1] == '-')
			throw WrongDomainNameFormat();

		for (size_t i = 0; i < label.length(); ++i)
		{
			char c = label[i];
			if (!isalnum(c) && c != '-')
				throw WrongDomainNameFormat();
		}

		if (end == string::npos)
			break;
		start = end + 1;
		end = name.find('.', start);
	}
	return true;
}

bool doesPathExist(const string& path)
{
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0);
}

bool isDirectory(const string& path)
{
	struct stat buffer;
	if (stat(path.c_str(), &buffer) != 0)
		return false;
	return S_ISDIR(buffer.st_mode);
}

bool isRegularFile(const string& path)
{
	struct stat buffer;
	if (stat(path.c_str(), &buffer) != 0)
		return false;
	return S_ISREG(buffer.st_mode);
}

bool isLocationPathOk(const string& path)
{
	if (path.empty())
		return false;
	if (path[0] != '/' && path[0] != '*')
		return false;
	for (size_t i = 0; i < path.length(); ++i) {
		char c = path[i];
		if (!isalnum(c) && c != '/' && c != '-' && c != '_' && c != '.' && c != '*')
			return false;
	}
	if (path.find("//") != string::npos)
		return false;
	return true;
}
/* -----------------------------
           EXTRACTORS
--------------------------------*/
void extractHostAndPort(string str, ServerConfig& serverConfig)
{
	if (str.length() < 2)
		throw InvalidPortFormat();
	if (str.at(str.size() - 1) != ';')
		throw MissingSemiColon();

	string data = str.substr(0, str.length() - 1);
	string hostStr = "0.0.0.0"; 
	string portStr = data;

	size_t colonPos = data.find(':');
	if (colonPos != string::npos)
	{
		if (colonPos > 0 && data[colonPos - 1] == '.')
			throw InvalidPortFormat();

		hostStr = data.substr(0, colonPos);
		portStr = data.substr(colonPos + 1);
	}
	else if (data.find('.') != string::npos || data == "localhost")
	{
		hostStr = data;
		portStr = "80";
	}

	for (size_t i = 0; i < portStr.length(); ++i)
		if (!isdigit(portStr[i]))
			throw NonNumericPortValue();

	char* endptr;
	long parsedPort = strtol(portStr.c_str(), &endptr, 10);
	if (parsedPort < 1 || parsedPort > 65535)
		throw PortNotInPortRange();

	if (!isIPValid(hostStr))
		throw IpFortmatNotValid();

	serverConfig.setHost(hostStr);
	serverConfig.setPort(static_cast<int>(parsedPort));
}

void extractServerNames(ServerConfig& serverConfig, vector<string>::iterator& it)
{
	string str = *it;
	if (str.at(str.size() - 1) != ';')
		throw MissingSemiColon();
	str = str.substr(0, str.length() - 1);
	if (isValidServerName(str))
		serverConfig.setServerName(str);
}

void extractRootServer(string str, ServerConfig& serverConfig)
{
	if (str.length() < 1)
		throw RootPathError();
	if (str.at(0) != '/')
		throw RootNotAbsolut();
	if (str.at(str.size() - 1) != ';')
		throw MissingSemiColon();
	string pathNoSemi = str.substr(0, str.length() - 1);
	if (doesPathExist(pathNoSemi) && isDirectory(pathNoSemi))
		serverConfig.setRoot(pathNoSemi);
	else
		throw RootPathError();
}

void extractRootLocation(LocationConfig& locationConfig, string str)
{
	if (str.length() < 1)
		throw RootPathError();
	if (str.at(0) != '/')
		throw RootNotAbsolut();
	if (str.at(str.size() - 1) != ';')
		throw MissingSemiColon();
	string pathNoSemi = str.substr(0, str.length() - 1);
	if (doesPathExist(pathNoSemi) && isDirectory(pathNoSemi))
		locationConfig.setRoot(pathNoSemi);
	else
		throw RootPathError();
}

void extractIndexServer(ServerConfig& serverConfig, vector<string>::iterator& it)
{
	string str = *it;
	while (str.at(str.size() - 1) != ';')
	{
		serverConfig.addIndex(str);
		it++;
		str = *it;
	}
	if (str.at(str.size() - 1) != ';')
		throw MissingSemiColon();
	str = str.substr(0, str.length() - 1);
	serverConfig.addIndex(str);
}

void extractIndexLocation(LocationConfig& locationConfig, vector<string>::iterator& it)
{
	string str = *it;
	while (str.at(str.size() - 1) != ';')
	{
		locationConfig.setIndex(str);
		it++;
		str = *it;
	}
	if (str.at(str.size() - 1) != ';')
		throw MissingSemiColon();
	str = str.substr(0, str.length() - 1);
	locationConfig.setIndex(str);
}

void extractErrorPages(ServerConfig& serverConfig, vector<string>::iterator& it)
{
	vector<string> lineTokens;
	while ((*it)[(*it).length() - 1] != ';')
		lineTokens.push_back(*(it++));
	string uri = (*it).substr(0, (*it).length() - 1);
	for (size_t i = 0; i < lineTokens.size(); i++)
	{
		stringstream snb(lineTokens[i]);
		int nb;
		snb >> nb;
		serverConfig.addErrorPage(nb, uri);
	}
}

unsigned long extractMaxBodySize(vector<string>::iterator& it)
{
	string str = *it;
	if (str.length() < 2)
		throw MissingSemiColon();
	if (str[str.length() - 1] != ';')
		throw MissingSemiColon();
	string sizeNoSemi = str.substr(0, str.length() - 1);
	string sizeStr;
	size_t mult = 1;
	char lastChar = sizeNoSemi[sizeNoSemi.length() - 1];

	if (lastChar == 'k' || lastChar == 'K')
	{
		mult = 1024;
		sizeStr = sizeNoSemi.substr(0, sizeNoSemi.length() - 1);
	}
	else if (lastChar == 'm' || lastChar == 'M')
	{
		mult = 1048576;
		sizeStr = sizeNoSemi.substr(0, sizeNoSemi.length() - 1);
	}
	else if (isdigit(lastChar))
	{
		mult = 1;
		sizeStr = sizeNoSemi;
	}
	else
		throw WrongMaxBodySizeFormat();

	if (sizeStr.empty())
		throw WrongMaxBodySizeFormat();
	for (size_t i = 0; i < sizeStr.length(); ++i)
		if (!isdigit(sizeStr[i]))
			throw WrongMaxBodySizeFormat();
	char* endptr;
	long sizenb = strtol(sizeStr.c_str(), &endptr, 10);
	size_t MAX_LIMIT_BYTES = 104857600;
	if (sizenb < 0 || static_cast<size_t>(sizenb) > (MAX_LIMIT_BYTES / mult))
		throw MaxBodySizeToBig();
	return (static_cast<size_t>(sizenb) * mult);
}

void extractAllowMethods(LocationConfig& locationConfig, vector<string>::iterator& it)
{
	vector<string> lineTokens;
	while ((*it)[(*it).length() - 1] != ';')
	{
		if (*it != "POST" && *it != "DELETE" && *it != "GET")
			throw MethodNotSupported(*it++);
		lineTokens.push_back(*(it++));
	}
	string method = (*it).substr(0, (*it).length() - 1);
	if (method != "POST" && method != "DELETE" && method != "GET")
		throw MethodNotSupported(method);
	lineTokens.push_back(method);
	locationConfig.setAllowedMethods(lineTokens);
}

void extractAutoIndex(LocationConfig& locationConfig, string s)
{
	if (s == "on;")
		locationConfig.setAutoindex(true);
	else if (s == "off;")
		locationConfig.setAutoindex(false);
	else
		throw AutoIndexFormatError();
}

void extractAllowUpload(LocationConfig& locationConfig, string s)
{
	if (s == "on;")
		locationConfig.setAllowUpload(true);
	else if (s == "off;")
		locationConfig.setAllowUpload(false);
	else
		throw AllowUploadFormatError();
}

void extractUploadStore(LocationConfig& locationConfig, string str)
{
	if (str.length() < 1)
		throw UploadStorePathError();
	if (str.at(str.size() - 1) != ';')
		throw MissingSemiColon();
	string pathNoSemi = str.substr(0, str.length() - 1);
	if (doesPathExist(pathNoSemi) && isDirectory(pathNoSemi))
		locationConfig.setUploadStore(pathNoSemi);
	else
		throw UploadStorePathError();
}

void extractCgiExtension(LocationConfig& locationConfig, string str)
{
	if (str.size() < 1 || str.at(str.size() - 1) != ';' || str.at(0) != '.')
		throw CGIExtansionFormatError();
	string noDotOrSemi = str.substr(1, str.size() - 2);
	for (size_t i = 0; i < noDotOrSemi.size(); i++)
		if (!isalnum(noDotOrSemi.at(i)))
			throw CGIExtansionFormatError();
	string noSemi = str.substr(0, str.size() - 1);
	locationConfig.setCgiExtension(noSemi);
}

void extractCgiPath(LocationConfig& locationConfig, string str)
{
	if (str.size() < 1 || str.at(str.size() - 1) != ';' || (str.at(0) != '/' && str.at(0) != '.'))
		throw CGIPathFormaError();
	string noSemi = str.substr(0, str.size() - 1);
	if (!isRegularFile(noSemi) || access(noSemi.c_str(), X_OK) < 0)
		throw CGIPathAccessError();
	locationConfig.setCgiPath(noSemi);
}

void extractReturn(LocationConfig& locationConfig, vector<string>::iterator& it)
{
	vector<string> args;
	string currentToken;

	while (true)
	{
		currentToken = *it;
		if (currentToken.empty())
			throw InvalidReturnFormat();
		if (currentToken[currentToken.length() - 1] == ';')
		{
			string cleanToken = currentToken.substr(0, currentToken.length() - 1);
			if (!cleanToken.empty())
				args.push_back(cleanToken);
			break;
		}
		else
		{
			args.push_back(currentToken);
			it++;
		}
	}

	if (args.empty() || args.size() > 2)
		throw InvalidReturnFormat();
	string codeStr = args[0];
	for (size_t i = 0; i < codeStr.length(); ++i)
		if (!isdigit(codeStr[i]))
			throw InvalidReturnCode();

	char* endptr;
	long parsedCode = strtol(codeStr.c_str(), &endptr, 10);

	if (parsedCode < 100 || parsedCode > 599)
		throw InvalidReturnCode();
	locationConfig.setReturnCode(static_cast<int>(parsedCode));
	if (args.size() == 2)
		locationConfig.setReturnUrl(args[1]);
	else
		locationConfig.setReturnUrl("");
}

void ConfigParser::_locationAlreadyExist(string locationPath, ServerConfig& currentServer)
{
	std::vector<LocationConfig> locations = currentServer.getLocations();
	for (size_t i = 0; i < locations.size(); i++)
		if (locations.at(i).getPath() == locationPath)
			throw LocationSamePath(locations.at(i).getPath());
}

void ConfigParser::_parseLocationBlock(ServerConfig& currentServer, vector<string>::iterator& it)
{
	LocationConfig locationConfig;
	if (!isLocationPathOk(*it))
		throw InvalidLocationPath();
	else
	{
		_locationAlreadyExist(*it, currentServer);
		locationConfig.setPath(*it);
	}
	if (*(++it) != "{")
		throw MissingOpeningCurlyBracket();
	while (*it != "}")
	{
		if (*it == "allow_methods")
			extractAllowMethods(locationConfig, ++it);
		else if (*it == "index")
			extractIndexLocation(locationConfig, ++it);
		else if (*it == "autoindex")
			extractAutoIndex(locationConfig, *(++it));
		else if (*it == "root")
			extractRootLocation(locationConfig, *(++it));
		else if (*it == "allow_upload")
			extractAllowUpload(locationConfig, *(++it));
		else if (*it == "upload_store")
			extractUploadStore(locationConfig, *(++it));
		else if (*it == "client_max_body_size")
			locationConfig.setMaxBodySize(extractMaxBodySize(++it));
		else if (*it == "cgi_extension")
			extractCgiExtension(locationConfig, *(++it));
		else if (*it == "cgi_path")
			extractCgiPath(locationConfig, *(++it));
		else if (*it == "return")
			extractReturn(locationConfig, ++it);
		else if (*it != "{")
			throw WrongKeyWord(*it);
		it++;
	}
	currentServer.addLocation(locationConfig);
}

void ConfigParser::_parseServerBlock(vector<string>::iterator& it)
{
	if (*(++it) != "{")
		throw MissingOpeningCurlyBracket();
	it++;
	ServerConfig serverConfig;
	while (*it != "}")
	{
		if (*it == "listen")
			extractHostAndPort(*(++it), serverConfig);
		else if (*it == "server_name")
			extractServerNames(serverConfig, ++it);
		else if (*it == "root")
			extractRootServer(*(++it), serverConfig);
		else if (*it == "index")
			extractIndexServer(serverConfig, ++it);
		else if (*it == "error_page")
			extractErrorPages(serverConfig, ++it);
		else if (*it == "client_max_body_size")
			serverConfig.setMaxSize(extractMaxBodySize(++it));
		else if (*it == "location")
			this->_parseLocationBlock(serverConfig, ++it);
		else
			throw WrongKeyWord(*it);
		it++;
	}
	this->addServer(serverConfig);
}

void printTokens(vector<string> tokens)
{
	cout << "=== CONF FILE TOKENS ===" << endl;
	for (int i = 0; i < (int)tokens.size(); i++)
	{
		cout << "[" << tokens[i] << "] ";
		if (tokens[i] == "{" || tokens[i] == "}" || tokens[i].at(tokens[i].size() - 1) == ';')
			cout << endl;
	}
	cout << "========================" << endl;
}

void ConfigParser::parse()
{
	_readFile(this->_configFile);
	areCurlyBracketsOk(this->_tokens);
	if (this->_tokens.empty())
		throw EmptyConfigFile();
	for (vector<string>::iterator it = this->_tokens.begin(); it != this->_tokens.end(); ++it)
	{
		if (*it == "server")
			_parseServerBlock(it);
		else
			throw MissingServerKeyWord();
	}
}
