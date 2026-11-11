#ifndef LOGGER_HPP
# define LOGGER_HPP

# include <iostream>
# include <fstream>
# include <sstream>
# include <ctime>

using namespace std;

enum LogLevel { DEBUG, INFO, WARNING, ERROR, CRITICAL };

class Logger
{
private:
	static ofstream	_logFile;
	static bool		_initialized;

	Logger(void);
	~Logger(void);
	Logger(const Logger& other);
	Logger& operator=(const Logger& other);

	static void init(void);
	static string levelToString(LogLevel level);
	static string levelToColor(LogLevel level);

public:
	static void log(LogLevel level, const string& msg);
};

#endif
