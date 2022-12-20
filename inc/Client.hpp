#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "Server.hpp"

class Config;
class Client
{
	public:
    	Client(int, int, Config *);
    	~Client();
    	Client(const Client&);
    	Client  &operator=(const Client&);
    	void	update_client_timestamp();

    	std::time_t	timestamp;
    	int			conn_fd;
    	int			port;
    	int			requestContentLength;
		std::string	requestHeader;
		int			headerSize;
		std::string requestBody;
		std::string	current_route;
		bool		headerSet;
    	bool		request_is_read;
		bool		client_body_too_large;
		Config		*server_config;

	private:
    	Client();
}; 

#endif
