#include "../inc/Server.hpp"

int Server::acceptRequest(int sock_num)
{
    int newfd = accept(this->_sockets[sock_num]->fd,
                        (struct sockaddr*)&this->_sockets[sock_num]->socketAddr,
                        (socklen_t *)&this->_sockets[sock_num]->socketAddrLen);
    if (newfd == -1)
        return (ft_return("error: accept\n"));
    this->_conn_fd.insert(std::make_pair(newfd, sock_num));
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
    std::map<int, int>::iterator it;
    it = this->_conn_fd.find(c_fd);
    if (it == this->_conn_fd.end())
        return ft_return("didn't find connection pair: ");
    this->_sockets[it->second]->logfile_fstream.open(this->_sockets[it->second]->logFile);
    this->_sockets[it->second]->logfile_fstream.clear();
    this->_sockets[it->second]->logfile_fstream.seekg(0);
    this->_sockets[it->second]->logfile_fstream << buf;
    this->_sockets[it->second]->logfile_fstream.close();

    std::cout << "\n\033[33m\033[1m" << "RECEIVED:\n\033[0m\033[33m" << buf << "\033[0m" << std::endl;
    return 0;
}

std::string Server::buildResponse(int c_fd)
{
    std::map<int, int>::iterator it;
    it = this->_conn_fd.find(c_fd);
    if (it == this->_conn_fd.end())
    {
        ft_return("didn't find connection pair: ");
        return (NULL);
    }
    this->_sockets[it->second]->logfile_fstream.open(this->_sockets[it->second]->logFile);
    if (!this->_sockets[it->second]->logfile_fstream.is_open())
    {
        ft_return("could not open logfile: ");
        return (NULL);
    }
    std::vector<std::string> head;
    std::string line;
    for (int i = 0; i < 3 && this->_sockets[it->second]->logfile_fstream.peek() != '\n' && this->_sockets[it->second]->logfile_fstream >> line; i++)
        head.push_back(line);
    if (head[0] == "GET" || head[0] == "POST")
    {
        // std::cout << head[1] << std::endl;
        std::string ret = this->_sockets[it->second]->getLocationPage(head[1]);
        if (ret != "")
        {
            responseHeader = "HTTP/1.1 200 OK";
            return (ret);
        }
        else
        {
            responseHeader = "HTTP/1.1 404 Not Found";
            return ("htmlFiles/404.html");
        }
    }
    head.clear();
    responseHeader = "HTTP/1.1 200 OK";
    return ("htmlFiles/button.html");
}

int Server::sendResponseToClient(int c_fd)
{
    int             fileSize;
    std::ofstream   temp("response.txt");
    temp.close();
    std::fstream    responseFile("response.txt");
    if (!responseFile.is_open())
        return ft_return("could not open response file ");
    std::ifstream htmlFile(this->buildResponse(c_fd));
    if (!htmlFile.is_open())
        return (ft_return("html file doesn't exist: "));
    htmlFile.seekg(0, std::ios::end);
    fileSize = htmlFile.tellg();
    htmlFile.clear();
    htmlFile.seekg(0);            
    responseFile << responseHeader << std::endl;
    responseFile << "Content-Type: text/html" << std::endl;
    responseFile << "Content-Length " << fileSize << std::endl << std::endl;
    char    html[fileSize];
    htmlFile.read(html, fileSize);
    responseFile << html << std::endl;
    responseFile.seekg(0, std::ios::end);
    fileSize = responseFile.tellg();
    responseFile.clear();
    responseFile.seekg(0);
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
    std::cout << "\n\033[32m\033[1m" << "RESPONDED:\n\033[0m\033[32m" << std::endl << response << "\033[0m" << std::endl;
    responseHeader.erase();
    htmlFile.close();
    responseFile.close();
    return (0);
}
