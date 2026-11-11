#ifndef HTTP_EXCEPTION_HPP
# define HTTP_EXCEPTION_HPP

# include "WebservException.hpp"

using namespace std;

class HttpException : public WebservException
{
public:
	explicit HttpException(const string& msg) : WebservException(msg) {}
};

class BadRequestException : public HttpException
{
public:
	BadRequestException(void) : HttpException("400 Bad Request") {}
};

class NotFoundException : public HttpException
{
public:
	NotFoundException(void) : HttpException("404 Not Found") {}
};

class PayloadTooLargeException : public HttpException
{
public:
	PayloadTooLargeException(void) : HttpException("413 Payload too large") {}
};

#endif