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
    std::map<int, int>::iterator it;
    it = this->_conn_fd.find(fd);
    if (it == this->_conn_fd.end())
        ft_return("attempted erase unknown socket_pair: ");
    else
        this->_conn_fd.erase(it);
    close(fd);
    std::cout << "disconnected from socket:" << fd << std::endl;
    return 0;
}

int    Server::is_connection_open(int c_fd)
{
    std::map<int,int>::iterator it;
    it = this->_conn_fd.find(c_fd);
    if (it == this->_conn_fd.end())
        return -1;
    return true;
}
