#include "../inc/Server.hpp"

int ft_return(std::string str)
{
    std::cerr << str << strerror(errno) << std::endl;
    return (-1);
}

void Server::set_chlist(std::vector<struct kevent>& change_list, uintptr_t ident, int16_t filter,
        uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
    struct kevent temp_evt;

    EV_SET(&temp_evt, ident, filter, flags, fflags, data, udata);
    change_list.push_back(temp_evt);
}



int Server::closeConnection(int fd)
{
    std::vector<Client*>::iterator it = this->_clients.begin();
    std::vector<Client*>::iterator end = this->_clients.end();

    for(; it != end; ++it)
    {
        if (fd == (*it)->conn_fd)
        {
            delete *it;
            this->_clients.erase(it);
			break ;
        }
    }
    close(fd);
    std::cout << "disconnected from socket:" << fd << std::endl;
    return 0;
}

void    Server::update_client_timestamp(int fd)
{
    std::vector<Client*>::iterator it = this->_clients.begin();
    std::vector<Client*>::iterator end = this->_clients.end();

    for(; it != end; ++it)
        if (fd == (*it)->conn_fd)
            break ;
    if (it != end)
        (*it)->update_client_timestamp();
}

void    Server::bounceTimedOutClients()
{
    std::vector<Client*>::iterator it = this->_clients.begin();
    std::vector<Client*>::iterator end = this->_clients.end();
    time_t current_time = std::time(nullptr);

    for(; it != end; ++it)
    {
        if ((*it)->timestamp + TIMEOUT <=  current_time)
        {
            std::cout << "Bouncing client from:" << (*it)->conn_fd << std::endl;
			closeConnection((*it)->conn_fd);
        }
    }
}

int	Server::findSocket(int fd)
{
    for (size_t i = 0; i < this->_sockets.size(); i++)
        if (fd == this->_sockets[i]->fd)
            return i;
    return -1;
}
