#include "../inc/Server.hpp"


Server::Server() {}
Server::~Server() { std::cout << "Closing server...\n";	}


//using select() in order to wait for one of the 3 fds to become readable (i.e. client sent a request)
//then accepts this with socket::acceptSocket()
int	Server::select_fd()
{
	fd_set	readfds;
	int		status = 1;
	int		maxfd = -1;

	FD_ZERO(&readfds);
	for(int i = 0; i < 3; ++i)
	{
		FD_SET(this->_sockets[i]->_socket, &readfds);
		if (this->_sockets[i]->_socket > maxfd)
			maxfd = this->_sockets[i]->_socket;
	}
	status = select(maxfd + 1, &readfds, NULL, NULL, NULL);
	if (status < 0)
		return -1;
	for(int i = 0; i < 3; ++i)
		if (FD_ISSET(this->_sockets[i]->_socket, &readfds))
			return this->_sockets[i]->acceptSocket();
	return -1;
}

//making the server listen to three sockets, bound do three different ports, writing requests to logfiles
int	Server::startServer()
{
	int	status = 0;

	this->_sockets.push_back(new Socket("localhost", 8080, "port8080.log"));
	this->_sockets.push_back(new Socket("localhost", 8081, "port8081.log"));
	this->_sockets.push_back(new Socket("localhost", 8082, "port8082.log"));
	while (status == 0)
		status = this->select_fd();
	if (status == -1)
		return this->_sockets[0]->ft_return("invalid fd:");
	close(this->_sockets[0]->_socket);
	close(this->_sockets[1]->_socket);
	close(this->_sockets[2]->_socket);

	return 0;
}

