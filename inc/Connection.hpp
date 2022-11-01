#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include "Server.hpp"

class Connection
{
    public:
        Connection(int);
        ~Connection();
        int  fd;

    private:
                        Connection(const Connection &);
        Connection &    operator=(const Connection &);

}; 

#endif
