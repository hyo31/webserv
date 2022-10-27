#include "../inc/Server.hpp"


Server::Server() {}
Server::~Server() { std::cout << "Closing server...\n";	}



//using kqueue() in order to monitor the 3 fds
//then accepts this with socket::acceptSocket() when a client sends a request
int	Server::monitor_fd()
{
    int             i, j, new_event, kq;
	struct  kevent  events[3];   /* Events to monitor */
	struct  kevent  tevents[3];	 /* Triggered event*/


    kq = kqueue();  /* creates queue */
    if (kq == -1)
        return ft_return("kqueue failed");

    for (i = 0; i < 3; ++i) /* initualize kevent events struct - uses EVFILT_READ so it returns when there is data available to read */
        EV_SET(&events[i], this->_sockets[i]->_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
    while(true)
    {
        std::cout << "--waiting on kevent--\n";
        new_event = kevent(kq, events, 3, tevents, 3, NULL);
        std::cout << "--kevent triggered--\n";
        if (new_event == -1)
            ft_return("kevent failed");
        else if (new_event > 0)
        {
            for (i = 0; i < new_event; ++i)
            {
                int event_fd = tevents[i].ident;
                if (tevents[i].flags & EV_EOF)
                    close(event_fd);
                if (tevents[i].flags & EVFILT_READ)
                {
                    j = 0;
                    while (tevents[i].ident != (unsigned long)this->_sockets[j]->_socket && j < 3)
                        j++;
                    this->_sockets[j]->acceptSocket();
                }
            }
        }
    }
	return -1;
}

//making the server listen to three sockets, bound do three different ports, writing requests to logfiles
int	Server::startServer()
{
	int	status = 0;

	this->_sockets.push_back(new Socket("localhost", 8093, "port8093.log"));
	this->_sockets.push_back(new Socket("localhost", 8094, "port8094.log"));
	this->_sockets.push_back(new Socket("localhost", 8095, "port8095.log"));
    std::cout << "sockets:" << this->_sockets[0]->_socket << " " << this->_sockets[1]->_socket << " " << this->_sockets[2]->_socket << std::endl;
	status = this->monitor_fd();
	if (status == -1)
		return ft_return("invalid fd:");
	close(this->_sockets[0]->_socket);
	close(this->_sockets[1]->_socket);
	close(this->_sockets[2]->_socket);

	return 0;
}


//using select() in order to wait for one of the 3 fds to become readable (i.e. client sent a request)
//then accepts this with socket::acceptSocket()
// int	Server::select_fd()
// {
// 	fd_set	readfds;
// 	int		status = 1;
// 	int		maxfd = -1;

// 	FD_ZERO(&readfds);
// 	for(int i = 0; i < 3; ++i)
// 	{
// 		FD_SET(this->_sockets[i]->_socket, &readfds);
// 		if (this->_sockets[i]->_socket > maxfd)
// 			maxfd = this->_sockets[i]->_socket;
// 	}
// 	status = select(maxfd + 1, &readfds, NULL, NULL, NULL);
// 	if (status < 0)
// 		return -1;
// 	for(int i = 0; i < 3; ++i)
// 		if (FD_ISSET(this->_sockets[i]->_socket, &readfds))
// 			return this->_sockets[i]->acceptSocket();
// 	return -1;
// }
