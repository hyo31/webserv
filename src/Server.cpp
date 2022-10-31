#include "../inc/Server.hpp"


Server::Server() {}
Server::~Server() { std::cout << "Closing server...\n";	}



//using kqueue() in order to monitor the 3 fds
//then accepts this with socket::acceptSocket() when a client sends a request
int	Server::monitor_fd()
{
    int             i, j, new_event, kq;
	struct  kevent  chevents[6];   /* Events to monitor */
	struct  kevent  tevents[6];	 /* Triggered event*/


    kq = kqueue();  /* creates queue */
    if (kq == -1)
        return ft_return("kqueue failed");

//   EV_SET(kev, ident,	filter,	flags, fflags, data, udata);
    for (i = 0; i < 3; ++i) /* initialize kevent events struct - uses EVFILT_READ so it returns when there is data available to read */
        EV_SET(&chevents[i], this->_sockets[i]->_socket, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, 0);
    for (j = 3, i = 0; j < 6; ++j, ++i) /* initialize kevent events struct - uses EVFILT_WRITE so it returns when there is data available to write */
        EV_SET(&chevents[j], this->_sockets[i]->_socket, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, 0);
    while(true)
    {
        //kevent(kq, *changelist, nchanges, *eventlist, nevents, timespec *timeout);
        std::cout << "--waiting on connection...--\n";
        new_event = kevent(kq, chevents, 6, tevents, 6, NULL);
        if (new_event == -1)
            ft_return("kevent failed");
        else if (new_event > 0)
        {
            for (i = 0; i < new_event; ++i)
            {
                if (tevents[i].flags & EV_EOF)
                    close(tevents[i].ident);
                else if (tevents[i].flags & EVFILT_READ)
                {
                    j = 0;
                    while (tevents[i].ident != (unsigned long)this->_sockets[j]->_socket && j < 3)
                        j++;
                    // std::cout << "j:" << j << "  i:" << i  << std::endl;
                    if (this->_sockets[j]->acceptSocket() == 0)
                        if (this->_sockets[j]->receive_from_client(kq) == 0)
                            this->_sockets[j]->respond_to_client();
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
	// this->_sockets.push_back(new Socket("localhost", 8096, "port8093.log"));
	// this->_sockets.push_back(new Socket("localhost", 8097, "port8094.log"));
	// this->_sockets.push_back(new Socket("localhost", 8098, "port8095.log"));

    std::cout << "sockets:" << this->_sockets[0]->_socket << " " << this->_sockets[1]->_socket << " " << this->_sockets[2]->_socket << std::endl;
	status = this->monitor_fd();
	if (status == -1)
		return ft_return("invalid fd:");
	close(this->_sockets[0]->_socket);
	close(this->_sockets[1]->_socket);
	close(this->_sockets[2]->_socket);

	return 0;
}
