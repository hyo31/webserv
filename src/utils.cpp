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

// static int	ft_strlen(const char *str)
// {
// 	int i;

// 	i = 0;
// 	while (str[i] != '\0')
// 		i++;
// 	return (i);
// }
// static char	*ft_substr(char const *s, unsigned int start, size_t len)
// {
// 	char	*buff;
// 	size_t	i;

// 	i = 0;
// 	if (s == NULL)
// 		return (NULL);
// 	if (start >= (unsigned int)ft_strlen(s))
// 	{
// 		buff = (char*)malloc(1);
// 		buff[0] = '\0';
// 		return (buff);
// 	}
// 	buff = (char *)malloc(sizeof(char) * (len + 1));
// 	if (buff == NULL)
// 		return (NULL);
// 	while ((len > i) && (s[start] != '\0'))
// 	{
// 		buff[i] = s[start];
// 		i++;
// 		start++;
// 	}
// 	buff[i] = '\0';
// 	return (buff);
// }

// static char			**ft_freebuff(int j, char **buff)
// {
// 	int i;

// 	i = 0;
// 	while (j >= i)
// 	{
// 		free(buff[j]);
// 		j--;
// 	}
// 	free(buff);
// 	return (NULL);
// }

// static int			ft_wordlength(int j, const char *s, char c)
// {
// 	int i;
// 	int x;

// 	i = 0;
// 	while (j >= 0)
// 	{
// 		while ((s[i] == c) && (s[i] != '\0'))
// 			i++;
// 		x = i;
// 		while ((s[i] != c) && (s[i] != '\0'))
// 			i++;
// 		j--;
// 	}
// 	return (i - x);
// }

// static int			ft_wordcount(char const *s, char c, int x)
// {
// 	int i;
// 	int j;

// 	j = 0;
// 	i = 0;
// 	while (s[i] != '\0')
// 	{
// 		while (s[i] == c)
// 			i++;
// 		while (s[i] != c && s[i] != '\0')
// 		{
// 			if (x >= 0)
// 			{
// 				if (x == 0)
// 					return (i);
// 			}
// 			i++;
// 		}
// 		x--;
// 		j++;
// 	}
// 	if (s[i - 1] == c)
// 		j--;
// 	return (j);
// }

// char				**ft_split(std::string s, char c)
// {
// 	char	**buff;
// 	int		j;

// 	if (s.empty)
// 		return (NULL);
// 	buff = (char **)malloc(sizeof(char*) * (1 + ft_wordcount(s, c, -1)));
// 	if (buff == NULL)
// 		return (NULL);
// 	j = 0;
// 	while (j < ft_wordcount(s, c, -1))
// 	{
// 		buff[j] = ft_substr(s, ft_wordcount(s, c, j), ft_wordlength(j, s, c));
// 		if (buff[j] == NULL)
// 			return (ft_freebuff(j, buff));
// 		j++;
// 	}
// 	buff[j] = NULL;
// 	return (buff);
// }
