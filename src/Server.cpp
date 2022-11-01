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

    /* create the queue */
    kq = kqueue();  
    if (kq == -1)
        return ft_return("kqueue failed");


    /* initialize kevent events structs - uses EVFILT_READ/WRITE so it returns when there is data available to read/write */
    /* EV_ADD adds it to the queue and EV_CLEAR resets the events state on return */
    for (i = 0; i < 3; ++i)
        EV_SET(&chevents[i], this->_sockets[i]->socket_fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, 0);
    for (j = 3, i = 0; i < 3; ++j, ++i)
        EV_SET(&chevents[j], this->_sockets[i]->socket_fd, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, 0);
    while(true)
    {
        /* use kevent to wait for an event */
        std::cout << "--waiting on connection...--\n";
        new_event = kevent(kq, chevents, 6, tevents, 6, NULL);
        if (new_event < 0)
            ft_return("kevent failed:\n");
        /* kevent returned with n new events */
        else if (new_event > 0)
        {
            for (i = 0; i < new_event; i++)
            {
                if (tevents[i].flags & EVFILT_READ)
                {
                    j = 0;
                    while (tevents[i].ident != (unsigned long)this->_sockets[j]->socket_fd && j < 3)
                        j++;
                    // std::cout << "j:" << j << "  i:" << i  << std::endl;
                    if (this->_sockets[j]->accept_Request() == 0)
                        if (this->_sockets[j]->receive_ClientRequest(kq) == 0)
                            this->_sockets[j]->respond_to_Client();
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

	// this->_sockets.push_back(new Socket("localhost", 8093, "port8093.log"));
	// this->_sockets.push_back(new Socket("localhost", 8094, "port8094.log"));
	// this->_sockets.push_back(new Socket("localhost", 8095, "port8095.log"));
	this->_sockets.push_back(new Socket("localhost", 8096, "port8093.log"));
	this->_sockets.push_back(new Socket("localhost", 8097, "port8094.log"));
	this->_sockets.push_back(new Socket("localhost", 8098, "port8095.log"));

    std::cout << "opened sockets:" << this->_sockets[0]->socket_fd << " " << this->_sockets[1]->socket_fd << " " << this->_sockets[2]->socket_fd << std::endl;
    std::cout << "listening to ports:" << this->_sockets[0]->port << " " << this->_sockets[1]->port << " " << this->_sockets[2]->port << std::endl;
	status = this->monitor_fd();
	if (status == -1)
		return ft_return("monitor failed:\n");
	close(this->_sockets[0]->socket_fd);
	close(this->_sockets[1]->socket_fd);
	close(this->_sockets[2]->socket_fd);
    delete this->_sockets[0];
    delete this->_sockets[1];
    delete this->_sockets[2];

	return 0;
}

int ft_return(std::string str)
{
    std::cerr << str << strerror(errno) << std::endl;
    return (errno);
}
