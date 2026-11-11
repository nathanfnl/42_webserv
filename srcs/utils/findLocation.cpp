#include "utils/findLocation.hpp"

string	cleanUrl(const string& url)
{
	string result;
	bool lastWasSlash = false;
	
	for (size_t i = 0; i < url.length(); i++)
	{
		if (url[i] == '?') 
		{
			result = url.substr(0, i);
			break;
		}
		
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

LocationConfig	findLocation(const string& raw_url, const ServerConfig& serverConfig)
{
	size_t maxNbMatchChars = 0;
	LocationConfig ret;
	string url = cleanUrl(raw_url);

	for (size_t i = 0; i < serverConfig.getLocations().size(); i++)
	{
		LocationConfig current = serverConfig.getLocations().at(i);
		string path = current.getPath();

		if (path.length() > 0 && path[0] == '*')
		{
			string suffix = path.substr(1);
			if (url.length() >= suffix.length() && url.compare(url.length() - suffix.length(), suffix.length(), suffix) == 0)
			{
				if (path.length() > maxNbMatchChars)
				{
					maxNbMatchChars = path.length();
					ret = current;
				}
			}
		}
		else if (url.find(path) == 0 || path == url + "/")
		{
			if (path.length() > maxNbMatchChars)
			{
				maxNbMatchChars = path.length();
				ret = current;
			}
		}
	}
	return (ret);
}