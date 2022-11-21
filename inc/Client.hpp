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
        std::time_t timestamp;
        int         port;
        bool        request_is_read;
        int         request_content_length;
        int         chunk_number;

    private:
        Client();

}; 

#endif
