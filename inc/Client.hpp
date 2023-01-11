#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "Server.hpp"

class Config;
class Client
{
	public:
    	Client( int, int );
    	~Client();
    	Client( const Client& );
    	Client  &operator=( const Client& );

		int			getConnectionFD();
		int			getPort();
		std::string	getLocation();
		std::string	getBody();
		std::string	getHeader();
		std::string	getMethod();
		std::time_t	getTimeStamp();
		bool		requestIsRead();
		bool		headerIsSet();
		bool		bodyTooLarge();
		bool		illegalRequest();
	
		void		setIllegalRequest( bool );
		void		setHeaderIsSet( bool );
		void		setRequestIsRead( bool );
		void		setBodyTooLarge( bool );
		void		setHeader( std::string, int );
		void		setBody( std::string );
		void		setMethod( std::string );
		void		setLocation( std::string );
	    void		update_client_timestamp();

	private:
	    std::time_t	_timestamp;
		std::string	_requestHeader;
		std::string _requestBody;
		std::string	_requestMethod;
		std::string	_requestLocation;
    	int			_conn_fd;
    	int			_port;
		bool		_headerSet;
    	bool		_request_is_read;
		bool		_client_body_too_large;
		bool		_illegal_request;
    	Client();
}; 

#endif
