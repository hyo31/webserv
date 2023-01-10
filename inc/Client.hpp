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
	    std::time_t	timestamp;
		std::string	requestHeader;
		std::string requestBody;
		std::string	requestMethod;
		std::string	requestLocation;
		std::string	current_route;
    	int			conn_fd;
    	int			port;
		bool		headerSet;
    	bool		request_is_read;
		bool		client_body_too_large;
		bool		illegal_request;
    	Client();
}; 

#endif
