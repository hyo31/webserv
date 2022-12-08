#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "Server.hpp"

class Client
{
	public:
    	Client(int, int);
    	~Client();
    	Client(const Client&);
    	Client  &operator=(const Client&);
    	void	update_client_timestamp();

    	int			conn_fd;
    	std::time_t	timestamp;
    	int			port;
    	bool		request_is_read;
    	int			requestContentLength;
		std::string	requestHeader;
		std::string requestBody;
		bool		headerSet;
		std::string	current_route;

	private:
    	Client();
}; 

#endif
