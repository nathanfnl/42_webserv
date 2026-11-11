#ifndef CGI_EXCEPTION_HPP
# define CGI_EXCEPTION_HPP

# include "WebservException.hpp"

using namespace std;

class CgiException : public WebservException
{
public:
    explicit CgiException(const string& msg) : WebservException(msg) {}
};

// return 1
class GenericCgiException : public CgiException
{
public:
    GenericCgiException(void) : CgiException("Cgi internal error") {}
};

// return 127
class NoSuchFileOrDirectoryException : public CgiException
{
public:
    NoSuchFileOrDirectoryException(void) : CgiException("No such file or directory") {}
};

// return 126
class PermissionDeniedException : public CgiException
{
public:
    PermissionDeniedException(void) : CgiException("Permission denied") {}
};

class CgiParentException : public CgiException
{
public:
    CgiParentException(const string& msg) : CgiException(msg) {}
};

#endif