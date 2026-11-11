#ifndef WEBSERV_EXCEPTION_HPP
# define WEBSERV_EXCEPTION_HPP

# include <exception>
# include <string>

using namespace std;

class WebservException : public exception
{
public:
	explicit WebservException(const string& msg) : _msg(msg) {}
	virtual ~WebservException() throw() {}

	virtual const char  *what() const throw() { return _msg.c_str();  }

protected:
	string _msg;

};

#endif