#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include <string>
# include <vector>
# include <map>
# include <iostream>
# include "LocationConfig.hpp"

using namespace std;

class ServerConfig {
private:
	int						_port;
	string					_host;
	string					_serverName;

	map<int, string>		_errorPages;
	unsigned long			_clientMaxBodySize;

	string					_root;
	vector<string>			_index;

	vector<LocationConfig>	_locations;

public:
	ServerConfig();
	~ServerConfig();

	void	setServerName(const string& name);
	void	addErrorPage(int code, const string& path);
	void	addLocation(const LocationConfig& location);

	void	setPort(int port);
	void	setHost(string host);
	void	setRoot(string root);
	void	addIndex(string index);
	void	setMaxSize(unsigned long maxSize);

	int								getPort() const;
	unsigned long					getClientMaxBodySize() const;
	const string&					getHost() const;
	const string&					getRoot() const;
	const vector<string>&			getIndex() const;
	const string&					getServerName() const;
	const map<int, string>&			getErrorPages() const;
	const vector<LocationConfig>&	getLocations() const;
};

ostream& operator<<(ostream& os, const ServerConfig& config);

#endif