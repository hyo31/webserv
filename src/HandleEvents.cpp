#include "../inc/Server.hpp"

int Server::acceptRequest(int sock_num)
{
    int newfd = accept(this->_sockets[sock_num]->fd,
                        (struct sockaddr*)&this->_sockets[sock_num]->socketAddr,
                        (socklen_t *)&this->_sockets[sock_num]->socketAddrLen);
    if (newfd == -1)
        return (ft_return("error: accept\n"));
    Client *newclient = new Client(newfd, sock_num);
    this->_clients.push_back(newclient);
    int status = fcntl(newfd, F_SETFL, O_NONBLOCK);	
    if (status == -1)
        ft_return("fcntl failed");
    return newfd;
}

int Server::receiveClientRequest(int c_fd)
{
    ssize_t bytesRead = -1;
    char    buf[50000];

    bytesRead = recv(c_fd, buf, 50000, 0);
    update_client_timestamp(c_fd);
    if (bytesRead == -1)
    {
        closeConnection(c_fd);
        return ft_return("recv failed:\n");
    }
    if (bytesRead == 0)
    {
        if (closeConnection(c_fd) == -1)
            return -1;
        return 1; 
    }
    buf[bytesRead] = '\0';

    std::vector<Client*>::iterator it = this->_clients.begin();
    std::vector<Client*>::iterator end = this->_clients.end();
    for(; it != end; ++it)
        if (c_fd == (*it)->conn_fd)
            break ;
    if (it == end)
        return ft_return("didn't find connection pair: ");
    this->_sockets[(*it)->port]->logfile_fstream.open(this->_sockets[(*it)->port]->logFile, std::fstream::out);
    this->_sockets[(*it)->port]->logfile_fstream.clear();
    this->_sockets[(*it)->port]->logfile_fstream.seekg(0);
    this->_sockets[(*it)->port]->logfile_fstream << buf;
    this->_sockets[(*it)->port]->logfile_fstream.close();

    std::cout << "\n\033[33m\033[1m" << "RECEIVED:\n\033[0m\033[33m" << buf << "\033[0m" << std::endl;
    return 0;
}

std::string Server::findHtmlFile(int c_fd)
{
    std::vector<Client*>::iterator it = this->_clients.begin();
    std::vector<Client*>::iterator end = this->_clients.end();
    for(; it != end; ++it)
        if (c_fd == (*it)->conn_fd)
            break ;
    if (it == end)
    {
        ft_return("didn't find connection pair: ");
        return (NULL);
    }
    this->_sockets[(*it)->port]->logfile_fstream.open(this->_sockets[(*it)->port]->logFile);
    if (!this->_sockets[(*it)->port]->logfile_fstream.is_open())
    {
        ft_return("could not open logfile: ");
        return (NULL);
    }
    std::vector<std::string> head;
    std::string line;
    for (int i = 0; i < 3 && this->_sockets[(*it)->port]->logfile_fstream.peek() != '\n' && this->_sockets[(*it)->port]->logfile_fstream >> line; i++)
        head.push_back(line);
    if (head[0] == "GET" || head[0] == "POST")
    {
        // std::cout << head[1] << std::endl;
        std::string ret = this->_sockets[(*it)->port]->getLocationPage(head[1]);
        if (ret != "")
        {
            _responseHeader = "HTTP/1.1 200 OK";
            return (ret);
        }
        else
        {
            _responseHeader = "HTTP/1.1 404 Not Found";
            return ("htmlFiles/404.html");
        }
    }
    head.clear();
    _responseHeader = "HTTP/1.1 200 OK";
    return ("htmlFiles/button.html");
}

int Server::sendResponseToClient(int c_fd)
{
    int             fileSize;
    std::ifstream   htmlFile;
    std::fstream    responseFile;

    //open streamfiles
    responseFile.open("response.txt", std::ios::in | std::ios::out | std::ios::binary);
    if (!responseFile.is_open())
        return ft_return("could not open response file ");
    htmlFile.open(this->findHtmlFile(c_fd), std::ios::in | std::ios::binary);
    if (!htmlFile.is_open())
        return (ft_return("html file doesn't exist: "));
    
    //get length of htmlFile
    htmlFile.seekg(0, std::ios::end);
    fileSize = htmlFile.tellg();
    htmlFile.clear();
    htmlFile.seekg(0, std::ios::beg);   

    //read correct headers (first one set in 'findHtmlFile') into responseFile
    responseFile << this->_responseHeader << std::endl;
    responseFile << "Content-Type: text/html" << std::endl;
    responseFile << "Content-Length: " << fileSize << "\r\n\r\n"; //std::endl << std::endl;
    
    //create char string to read html into, which is then read into responseFile         
    char    html[fileSize];
    htmlFile.read(html, fileSize);
    responseFile << html << std::endl;

    //get length of full responseFile
    responseFile.seekg(0, std::ios::end);
    fileSize = responseFile.tellg();
    responseFile.clear();
    responseFile.seekg(0, std::ios::beg);

    //create respone which is sent back to client
    char    response[fileSize];
    responseFile.read(response, fileSize);
    ssize_t bytesSent = send(c_fd, response, fileSize, 0);
    if (bytesSent == -1)
    {
        htmlFile.close();
        responseFile.close();
        close(c_fd);
        return ft_return("error: send\n");
    }
    update_client_timestamp(c_fd);
    std::cout << "\n\033[32m\033[1m" << "RESPONDED:\n\033[0m\033[32m" << std::endl << "[" << response << "]\033[0m" << std::endl;
    this->_responseHeader.erase();
    htmlFile.close();
    responseFile.close();
    return (0);
}
