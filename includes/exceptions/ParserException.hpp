#ifndef PARSER_EXCEPTION_HPP
# define PARSER_EXCEPTION_HPP

# include "WebservException.hpp"

using namespace std;

class ParserException : public WebservException
{
public: 
	explicit ParserException(const string& msg) : WebservException(msg) {}
};

class FailToOpenFile : public ParserException
{
public:
	FailToOpenFile(void) : ParserException("Fail to open configuration file") {}
};

class MissingServerKeyWord : public ParserException
{
public:
	MissingServerKeyWord(void) : ParserException("Keyword \"server\" is missing") {}
};

class WrongKeyWord : public ParserException
{
public:
	WrongKeyWord(string keyword) : ParserException("Use of wrong keyword : " + keyword) {}
};

class MissingOpeningCurlyBracket : public ParserException
{
public:
	MissingOpeningCurlyBracket(void) : ParserException("Missing a \"{\" in conf file") {}
};

class MissingClosingCurlyBracket : public ParserException
{
public:
	MissingClosingCurlyBracket(void) : ParserException("Missing a \"}\" in conf file") {}
};

class MissingSemiColon : public ParserException
{
public:
	MissingSemiColon(void) : ParserException("Missing \";\" in conf file") {}
};

class EmptyConfigFile : public ParserException
{
public:
	EmptyConfigFile(void) : ParserException("Config file is empty") {}
};

class EmptyPortException : public ParserException
{
public:
	EmptyPortException(void) : ParserException("Empty port in configuration file") {}
};

class NonNumericPortValue : public ParserException
{
public:
	NonNumericPortValue(void) : ParserException("Port contains non numeric values in conf file") {}
};

class PortNotInPortRange : public ParserException
{
public:
	PortNotInPortRange(void) : ParserException("Port value outsite port range (1 - 65535)") {}
};

class InvalidPortFormat : public ParserException
{
public:
	InvalidPortFormat(void) : ParserException("Port format is invalid") {}
};

class IpFortmatNotValid : public ParserException
{
public:
	IpFortmatNotValid(void) : ParserException("Ip adress format is not valid") {}
};

class EmptyDomainName : public ParserException
{
public:
	EmptyDomainName(void) : ParserException("Domain name is empty") {}
};

class DomainNameToLong : public ParserException
{
public:
	DomainNameToLong(void) : ParserException("Domain name is too long") {}
};

class WrongDomainNameFormat : public ParserException
{
public:
	WrongDomainNameFormat(void) : ParserException("Domain name format is wrong") {}
};

class RootPathError : public ParserException
{
public:
	RootPathError(void) : ParserException("There's a problem in the path root in conf file") {}
};

class UploadStorePathError : public ParserException
{
public:
	UploadStorePathError(void) : ParserException("There's a problem in the \"upload_store\" in conf file") {}
};

class IndexNameError : public ParserException
{
public:
	IndexNameError(void) : ParserException("There's a problem in the index name in conf file") {}
};

class WrongMaxBodySizeFormat : public ParserException
{
public:
	WrongMaxBodySizeFormat(void) : ParserException("client_max_body_size has format error in conf file") {}
};

class MaxBodySizeToBig : public ParserException
{
public:
	MaxBodySizeToBig(void) : ParserException("client_max_body_size must not be over 10m") {}
};

class InvalidLocationPath : public ParserException
{
public:
	InvalidLocationPath(void) : ParserException("Config file contains a invalid location path") {}
};

class AutoIndexFormatError : public ParserException
{
public:
	AutoIndexFormatError(void) : ParserException("Format error in \"autoindex\" in location block") {}
};

class AllowUploadFormatError : public ParserException
{
public:
	AllowUploadFormatError(void) : ParserException("Format error in \"allow_upload\" in location block") {}
};

class CGIExtansionFormatError : public ParserException
{
public:
	CGIExtansionFormatError(void) : ParserException("Format error in \"cgi_extension\" in location block") {}
};

class CGIPathFormaError : public ParserException
{
public:
	CGIPathFormaError(void) : ParserException("Format error in \"cgi_path\" in location block") {}
};

class CGIPathAccessError : public ParserException
{
public:
	CGIPathAccessError(void) : ParserException("Can't access \"cgi_path\" in location block") {}
};

class InvalidReturnFormat : public ParserException
{
public:
	InvalidReturnFormat(void) : ParserException("Format error in \"return\" in location block") {}
};

class InvalidReturnCode : public ParserException
{
public:
	InvalidReturnCode(void) : ParserException("\"return\" in location block must be >= 100 and <= 599") {}
};

class MethodNotSupported : public ParserException
{
public:
	MethodNotSupported(string method) : ParserException(method + " is not supported by the server") {}
};

class LocationSamePath : public ParserException
{
public:
	LocationSamePath(string path) : ParserException("Location with path \"" + path + "\" already exists") {}
};

class ServerAlreadyExists : public ParserException
{
public:
	ServerAlreadyExists() : ParserException("Two servers with same port in conf file") {}
};

class RootNotAbsolut : public ParserException
{
public:
	RootNotAbsolut() : ParserException("The server root path needs to be an absolut path") {}
};

#endif
