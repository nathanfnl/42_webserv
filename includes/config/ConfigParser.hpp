#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include <string>
# include <vector>
# include <stdexcept>
# include <fstream>
# include <sstream>
# include <iostream>
# include <cstdlib>
# include <sys/stat.h>
# include "config/ServerConfig.hpp"
# include "exceptions/ParserException.hpp"
# include <unistd.h>

using namespace std;

class ConfigParser {
private:
	string					_configFile;
	vector<ServerConfig>	_servers;
	vector<string>			_tokens;

	ConfigParser();

	void	_readFile(const string& filename);
	void	_parseServerBlock(vector<string>::iterator& it);
	void	_parseLocationBlock(ServerConfig& currentServer, vector<string>::iterator& it);
	void	_locationAlreadyExist(string locationPath, ServerConfig& currentServer);

public:
	ConfigParser(const string& filename);
	~ConfigParser();

	void						parse();
	void						addServer(const ServerConfig& server);
	const vector<ServerConfig>	&getServers() const;
};

#endif