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
        void    update_client_timestamp();

        int         conn_fd;
        std::time_t is_connected;
        int         port;

    private:
        Client();

}; 

#endif
