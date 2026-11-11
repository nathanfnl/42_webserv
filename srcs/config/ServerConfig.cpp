#include "config/ServerConfig.hpp"

ServerConfig::ServerConfig()
: _port(8080),
	_host("0.0.0.0"),
	_clientMaxBodySize(104857600) {}

ServerConfig::~ServerConfig() {}

void ServerConfig::setServerName(const string& name)
{
	this->_serverName = name;
}

void ServerConfig::addErrorPage(int code, const string& path)
{
	this->_errorPages.insert(pair<int, const string>(code, path));
}

void ServerConfig::setPort(int port)
{
	this->_port = port;
}

void ServerConfig::setHost(string host)
{
	this->_host = host;
}

void ServerConfig::setRoot(string root)
{
	this->_root = root;
}

void ServerConfig::addIndex(string index)
{
	this->_index.push_back(index);
}

void ServerConfig::setMaxSize(unsigned long maxSize)
{
	this->_clientMaxBodySize = maxSize;
}

void ServerConfig::addLocation(const LocationConfig& location)
{
	this->_locations.push_back(location);
}

int ServerConfig::getPort() const
{
	return (this->_port);
}

const string& ServerConfig::getHost() const
{
	return (this->_host);
}

const string& ServerConfig::getServerName() const
{
	return (this->_serverName);
}

const string& ServerConfig::getRoot() const
{
	return (this->_root);
}

const vector<string>& ServerConfig::getIndex() const
{
	return (this->_index);
}

const map<int, string>& ServerConfig::getErrorPages() const
{
	return (this->_errorPages);
}

unsigned long ServerConfig::getClientMaxBodySize() const
{
	return (this->_clientMaxBodySize);
}

const vector<LocationConfig>& ServerConfig::getLocations() const
{
	return (this->_locations);
}

ostream& operator<<(ostream& os, const ServerConfig& config)
{
	os << "=== SERVER CONFIG ===" << "\n";
	os << "Listen      : " << config.getHost() << ":" << config.getPort() << "\n";

	os << "Server Name : " << config.getServerName() << "\n";

	os << "Max Body    : " << config.getClientMaxBodySize() << " bytes\n";
	os << "Root        : " << config.getRoot() << "\n";
	os << "Index       : ";
	const vector<string>& indexs = config.getIndex();
	for (vector<string>::const_iterator it = indexs.begin(); it != indexs.end(); ++it)
		os << *it << " ";
	os << "\n";

	os << "Error Pages :\n";
	const map<int, string>& errors = config.getErrorPages();
	for (map<int, string>::const_iterator it = errors.begin(); it != errors.end(); ++it)
	    os << "  " << it->first << " -> " << it->second << "\n";

	os << "Locations   : " << config.getLocations().size() << " bloc(s)\n";
	const vector<LocationConfig>& locs = config.getLocations();
	for (vector<LocationConfig>::const_iterator it = locs.begin(); it != locs.end(); ++it)
	{
	    os << "  ---\n" << *it;
	}

	os << "=====================" << endl;

	return os;
}
