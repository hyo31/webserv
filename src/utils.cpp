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
        }
    }
    close(fd);
    std::cout << "disconnected from socket:" << fd << std::endl;
    return 0;
}

int    Server::is_connection_open(int c_fd)
{
    std::vector<Client*>::iterator it = this->_clients.begin();
    std::vector<Client*>::iterator end = this->_clients.end();

    for(; it != end; ++it)
    {
        if (c_fd == (*it)->conn_fd)
            return true;
    }
    return false;
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

static int	ft_checklong(int j)
{
	if (j == -1)
		return (0);
	else
		return (-1);
}

static int  ft_atoi(std::string str)
{
	long long unsigned int	i;
	int						j;
	long long unsigned int	n;

	i = 0;
	j = 1;
	n = 0;
	while (str[i] == '\v' || str[i] == '\f' || str[i] == '\t' || str[i] == '\n'
	|| str[i] == ' ' || str[i] == '\t' || str[i] == '\r')
		i++;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			j = -1;
		i++;
	}
	while (str[i] >= '0' && str[i] <= '9')
	{
		n = n * 10 + str[i] - 48;
		i++;
	}
	if (n > LONG_MAX)
		return (ft_checklong(j));
	return (n * j);
}

void    Server::chunkedRequest(char *buf, std::vector<Client*>::iterator it)
{
    //check if all headers read (end /r/n/r/n)
    // if not ; return
    std::string str=buf;
    std::string::size_type ret;
    std::string::iterator str_it;
    std::string::iterator str_it2;
    (*it)->request_is_read = false;

    if (str.find("\r\n\r\n") == std::string::npos)
    {
        std::cout << "ret 1\n";
        return ;
    }
    //check header for either content length: or Transfer-Encoding: chunked
    if ((ret = str.find("Content Length:")) != std::string::npos)
    {
        //find the number behind Content Length
        ret = str.find(" ", ret) + 1;
        std::string::size_type start = str.find(" ", ret) + 1;
        std::string::size_type end = str.find("\n", start) - 1;
        std::string substr = str.substr(start, (end - start + 1));
        (*it)->request_content_length = ft_atoi(substr);
        //check if the whole body is read
        ret = str.find("\r\n\r\n");
        start = ret + 4;
        for (end = 0; str[end] != '\0'; ++end);
        end--;
        if ((int)(start - end) != (*it)->request_content_length)
        {
            std::cout << "ret 2\n";
            return ;
        }
    }
    //then check if all is read or not. 


    //if not ; return
    //->true (or 'set client as 'ready to respond') ; return

    (*it)->request_is_read = true;
    std::cout << "ret 3\n";
    return ;
}
