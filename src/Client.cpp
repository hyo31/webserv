#include "../inc/Client.hpp"

//CONSTRUCTOR + DESTRUCTOR
Client::Client( int fd, int port ) : _conn_fd( fd ), _port( port )
{ 
    this->_timestamp = std::time(nullptr);
    this->_request_is_read = true;
	this->_client_body_too_large = false;
	this->_requestHeader = "";
	this->_requestBody = "";
	this->_headerSet = false;
}
Client::~Client() { std::cout << "Client removed\n"; }

//COPY CONSTRUCTOR
Client::Client( const Client& src ) { *this = src; }

//ASSIGNMENT OPERATOR
Client & Client::operator=( const Client& src )
{
    this->_conn_fd = src._conn_fd;
    this->_timestamp = src._timestamp;
    this->_port = src._port;
	this->_request_is_read = src._request_is_read;
	this->_requestHeader = src._requestHeader;
	this->_requestBody = src._requestBody;
	this->_headerSet = src._headerSet;
	this->_current_route = src._current_route;
	this->_requestMethod = src._requestMethod;
	this->_requestLocation = src._requestLocation;
	this->_current_route = src._current_route;
	this->_client_body_too_large = src._client_body_too_large;
	this->_illegal_request = src._illegal_request;
    return *this;
}

//CLIENT FUNCTIONS

//UTILS
void    Client::update_client_timestamp()
{
    this->_timestamp = std::time( nullptr );
}

//GETTERS
std::string	Client::getLocation()		{ return this->_requestLocation; }
std::string	Client::getBody()			{ return this->_requestBody; }
std::string	Client::getHeader()			{ return this->_requestHeader; }
std::string	Client::getMethod()			{ return this->_requestMethod; }
int			Client::getConnectionFD()	{ return this->_conn_fd; }
int			Client::getPort()			{ return this->_port; }
bool		Client::requestIsRead()		{ return this->_request_is_read; }
bool		Client::headerIsSet()		{ return this->_headerSet; }
bool		Client::bodyTooLarge()		{ return this->_client_body_too_large; }
bool		Client::illegalRequest()	{ return this->_illegal_request; }
std::time_t	Client::getTimeStamp()		{ return this->_timestamp; }

//SETTERS
void	Client::setBody( std::string body )					{ this->_requestBody = body; }
void	Client::setHeader( std::string header, int end )	{ this->_requestHeader = header; this->_requestHeader[end] = '\0'; }
void	Client::setMethod( std::string method )				{ this->_requestMethod = method; }
void	Client::setLocation( std::string location )			{ this->_requestLocation = location; }
void	Client::setHeaderIsSet( bool status )				{ this->_headerSet = status; }
void	Client::setRequestIsRead( bool status )				{ this->_request_is_read = status; }
void	Client::setBodyTooLarge( bool status )				{ this->_client_body_too_large = status; }
void	Client::setIllegalRequest( bool status )			{ this->_illegal_request = status; }
