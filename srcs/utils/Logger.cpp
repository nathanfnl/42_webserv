#include "utils/Logger.hpp"

using namespace std;

ofstream	Logger::_logFile;
bool		Logger::_initialized = false;

void Logger::init(void)
{
	if (_initialized)
		return ;
	_logFile.open("logfile.txt", ios::app);
	if (!_logFile.is_open())
		cerr << "Error opening log file." << endl;
	_initialized = true;
}

string	Logger::levelToString(LogLevel level)
{
	switch (level) {
		case DEBUG:
			return ("DEBUG");
		case INFO:
			return ("INFO");
		case WARNING:
			return ("WARNING");
		case ERROR:
			return ("ERROR");
		case CRITICAL:
			return ("CRITICAL");
		default:
			return ("UNKNOWN");
	}
}

string Logger::levelToColor(LogLevel level)
{
    switch (level)
    {
        case DEBUG:
			return ("\033[36m");
		case INFO:
			return ("\033[32m");
		case WARNING:
			return ("\033[33m");
		case ERROR:
			return ("\033[31m");
		case CRITICAL:
			return ("\033[1;31m");
		default:
			return "\033[0m";
    }
}

void	Logger::log(LogLevel level, const string& msg)
{
	init();

	time_t	now = time(0);
	tm		*timeinfo = localtime(&now);
	char	timestamp[20];
	strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

	const	string reset = "\033[0m";
	const	string	color = levelToColor(level);

	ostringstream  consoleEntry;
	consoleEntry << "[" << timestamp << "] "
					<< color << levelToString(level)
					<< ": " << msg << reset << endl;

	ostringstream  fileEntry;
	fileEntry << "[" << timestamp << "] "
				<< levelToString(level)
				<< ": " << msg << endl;

	cout << consoleEntry.str();

	if (_logFile.is_open())
	{
		_logFile << fileEntry.str();
		_logFile.flush();
		_logFile.close();
	}
}