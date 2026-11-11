#ifndef FIND_LOCATION_HPP
# define FIND_LOCATION_HPP

# include <iostream>
# include "config/ServerConfig.hpp"
# include "config/LocationConfig.hpp"

using namespace std;

string			cleanUrl(const string& url);
LocationConfig	findLocation(const string& raw_url, const ServerConfig& serverConfig);

#endif