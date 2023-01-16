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
		int			getSockNum();
		std::string	getLocation();
		std::string	getBody();
		std::string	getHeader();
		std::string	getMethod();
		std::string	getHost();
		std::time_t	getTimeStamp();
		bool		requestIsRead();
		bool		headerIsSet();
		bool		bodyTooLarge();
		bool		illegalRequest();
		bool		unknownHost();
	
		void		setIllegalRequest( bool );
		void		setHeaderIsSet( bool );
		void		setRequestIsRead( bool );
		void		setBodyTooLarge( bool );
		void		setUnknownHost( bool );
		void		setHeader( std::string, int );
		void		setBody( std::string );
		void		setMethod( std::string );
		void		setLocation( std::string );
		void		setHost( std::string );
	    void		update_client_timestamp();

	private:
    	int			_conn_fd;
    	int			_sock_num;
	    std::time_t	_timestamp;
		std::string	_requestHeader;
		std::string _requestBody;
		std::string	_requestMethod;
		std::string	_requestLocation;
		std::string	_requestHost;
		bool		_headerSet;
    	bool		_request_is_read;
		bool		_client_body_too_large;
		bool		_illegal_request;
		bool		_unknownHost;
    	Client();
}; 

#endif
