#include "utils/Logger.hpp"
#include "exceptions/HttpException.hpp"
#include "exceptions/WebservException.hpp"
#include "exceptions/CgiException.hpp"
#include "config/ConfigParser.hpp"
#include "sockets/Socket.hpp"
#include "servers/ServerManager.hpp"
#include "csignal"
#include "exceptions/WebservException.hpp"

using namespace std;

int main(int ac, char **av)
{
	try
	{
		if (ac != 2)
			throw runtime_error("Wrong number of arg");
		ConfigParser	conf(av[1]);
		conf.parse();
		ServerManager	serv(&conf);

		struct sigaction	sa;
		sa.sa_handler = &handle_sig;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sigaction(SIGINT, &sa, NULL);
		sigaction(SIGQUIT, &sa, NULL);
		signal(SIGPIPE, SIG_IGN);

		Logger::log(INFO, "SERVER RUNNING !");
		serv.run();
	}
	catch (const GenericCgiException& e) {return (1);}
	catch (const NoSuchFileOrDirectoryException& e) {return (127);}
	catch (const PermissionDeniedException& e) {return (126);}
	catch (const std::exception& e)
	{
		Logger::log(CRITICAL, e.what());
		return (1);
	}
	return (0);
}