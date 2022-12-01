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
	char	buf[50000];
    ssize_t bytesRead = -1;
    std::vector<Client*>::iterator it = this->_clients.begin();
    std::vector<Client*>::iterator end = this->_clients.end();
    for(; it != end; ++it)
        if (c_fd == (*it)->conn_fd)
            break ;
    if (it == end)
        return ft_return("didn't find connection pair: ");
    bytesRead = recv(c_fd, buf, 50000, 0);
    buf[bytesRead] = '\0';
    update_client_timestamp(c_fd);
    if (bytesRead == -1)
    {
        closeConnection(c_fd);
        return ft_return("recv failed:\n");
    }
    else if (bytesRead == 0)
    {
        std::cout << "0 bytes read/stream socket peer shutdown (eof)\n";
        if (closeConnection(c_fd) == -1)
            return -1;
        return 1; 
    }
	else if (bytesRead == 50000)
		std::cout << "request is too big, didn't read it all\n";
    if ((*it)->request_is_read == true)
    {
        std::cout << "clearing content\n";
        std::ofstream ofs;
        ofs.open(this->_sockets[(*it)->port]->logFile, std::ofstream::out | std::ofstream::trunc);
        ofs.close();
		(*it)->headerSet = false;
		(*it)->requestBody = "";
		(*it)->requestHeader = "";
    }
    std::ofstream ofs;
    ofs.open(this->_sockets[(*it)->port]->logFile, std::fstream::out | std::fstream::app);
    ofs << buf;
    ofs.close();
    std::ofstream asd;
    asd.open("logs/check", std::fstream::out | std::fstream::app);
    asd << buf;
    /* check if request is full*/
    chunkedRequest(this->_sockets[(*it)->port]->logFile, it);
    if ((*it)->request_is_read == false)
        return 1;
    
    return 0;
}

char    **setupEnv(std::string page, Socket *socket, std::string path)
{
    std::map<std::string, std::string>  env;
    std::fstream                        receivedMessage;
    std::stringstream                   buff;
    std::string                         request;
    std::size_t                         pos;
    
    receivedMessage.open(socket->logFile);
    if (!receivedMessage.is_open())
    {
        ft_return("could not open file: ");
        return (NULL);
    }
    buff << receivedMessage.rdbuf();
    receivedMessage.close();
    request = buff.str();
    pos = request.find("\r\n\r\n");
    if (pos == std::string::npos)
    {
        ft_return("request has no body: ");
        return (NULL);
    }
    env["QUERY_STRING"] = request.substr(pos + 4, request.length() - pos - 4);
    env["HTTP_HOST"] =  "localhost:" + std::to_string(socket->port);
    env["REQUEST_URI"] = page;
    env["REMOTE_PORT"] = std::to_string(socket->port);
    env["REQUEST_METHOD"] = "POST";
    env["SCRIPT_NAME"] = "upload.php";
    env["SERVER_PORT"] = std::to_string(socket->port);
    env["RESPONSE_FILE"] = "response/responseCGI.txt";
    env["PATH"] = path + "/htmlFiles";
    env["FILE_NAME"] = "logs/form.log";
    char    **c_env = new char*[env.size() + 1];
    int     i = 0;
    for (std::map<std::string, std::string>::const_iterator it = env.begin(); it != env.end(); it++)
    {
        std::string temp = it->first + "=" + it->second;
        c_env[i] = new char[temp.size() + 1];
        strcpy(c_env[i], temp.c_str());
        i++;
    }
    c_env[i] = NULL;
    receivedMessage.close();
    return (c_env);
}

int    executeCGI(std::string page, Socket *socket, std::string path)
{
    pid_t           pid;
    char            **env;
    int             status;
    std::string     pathCGI;

    env = setupEnv(page, socket, path);
    pathCGI = path + page;
    if (!env)
        return ft_return("failed setting up the environment");
    pid = fork();
    if (pid == -1)
        return ft_return("fork faield: ");
    if (!pid)
    {
        freopen("response/responseCGI.html","w",stdout);
        execve(pathCGI.c_str(), NULL, env);
        exit (ft_return("execve failed: "));
    }
    else
        waitpid(pid, &status, 0);
    if (WIFEXITED(status))
        return (WEXITSTATUS(status));
    return (0);
}

std::string Server::findHtmlFile(int c_fd)
{
    std::vector<Client*>::iterator	it = this->_clients.begin();
    std::vector<Client*>::iterator	end = this->_clients.end();
	std::string::iterator			strit;
	std::string						ret;
    std::fstream                    fstr;

    for(; it != end; ++it)
        if (c_fd == (*it)->conn_fd)
            break ;
    if (it == end)
    {
        ft_return("didn't find connection pair: ");
        return ("");
    }
    fstr.open(this->_sockets[(*it)->port]->logFile);
    if (!fstr.is_open())
    {
        ft_return("could not open logfile: ");
        return ("");
    }
    std::vector<std::string> head;
    std::string line;
    for (int i = 0; i < 3 && fstr.peek() != '\n' && fstr >> line; i++)
        head.push_back(line);
    fstr.close();
    if (head[0] == "GET")
    {
		/* if request GET = directory */
        ret = this->_sockets[(*it)->port]->getLocationPage(head[1]);
		if (ret == "Directory")
		{
			_responseHeader = "HTTP/1.1 200 OK";
			ret = this->_sockets[(*it)->port]->getLocationPage(head[1] + "index.html");
			if (ret != "")
			    return (ret);
            if (this->_sockets[(*it)->port]->autoindex)
                return (this->createAutoIndex(this->_sockets[(*it)->port]->_root + head[1], head[1]));
            _responseHeader = "HTTP/1.1 403 Forbidden";
            return ("htmlFiles/Pages/errorPages/403.html");
            std::cout << "ret:" << ret << std::endl;
		}
        if (ret != "")
        {
            _responseHeader = "HTTP/1.1 200 OK";
            std::cout << ret << std::endl;
            return (ret);
        }
		ret = this->_sockets[(*it)->port]->getRedirectPage(head[1]);
		if (ret != "")
		{
			_responseHeader = "HTTP/1.1 301 Moved Permanently\r\nLocation: ";
			_responseHeader.append(ret);
			return ("htmlFiles/Pages/errorPages/301.html");
		}
        _responseHeader = "HTTP/1.1 404 Not Found";
        return ("htmlFiles/Pages/errorPages/404.html");
    }
    else if (head[0] == "POST")
	{
        if (checkMaxClientBodySize(it) == false)
        {
			_responseHeader = "HTTP/1.1 413 Request Entity Too Large";
            return ("htmlFiles/Pages/errorPages/413.html");
        }
        if (executeCGI(head[1], this->_sockets[(*it)->port], this->_path))
        {
            _responseHeader = "HTTP/1.1 404 Not Found";
            return ("htmlFiles/Pages/errorPages/404.html");
        }
        _responseHeader = "HTTP/1.1 200 OK";
        return ("response/responseCGI.html");

	}
	std::cout << "ILLEGAL METHOD\n";
    head.clear();
    return (NULL);
}

void    removeResponseFiles()
{
    DIR             *directory;
    directory = opendir("response");
    if (!directory)
        ft_return("can not open directory: ");
    for (struct dirent *dirEntry = readdir(directory); dirEntry; dirEntry = readdir(directory))
    {
        std::string link = std::string(dirEntry->d_name);
        if (link != "." && link != "..")
        {
            link = "response/" + link;
            std::cout << link << std::endl; 
            std::remove(link.c_str());
        }
    }
}

int Server::sendResponseToClient(int c_fd)
{
    int             fileSize;
    std::string     htmlFileName;
    std::ifstream   htmlFile;
    std::fstream    responseFile;
    std::ofstream 	ofs;
    std::vector<Client*>::iterator	it = this->_clients.begin();
    std::vector<Client*>::iterator	end = this->_clients.end();

    for(; it != end; ++it)
        if (c_fd == (*it)->conn_fd)
            break ;
    ofs.open("response/response.txt", std::ofstream::out | std::ofstream::trunc);
    ofs.close();
    //open streamfiles
    responseFile.open("response/response.txt", std::ios::in | std::ios::out | std::ios::binary);
    if (!responseFile.is_open())
        return ft_return("could not open response file ");
    htmlFileName = this->findHtmlFile(c_fd);
    if (!htmlFileName.size())
        htmlFileName = "htmlFiles/Pages/errorPages/500.html";
    htmlFile.open(htmlFileName, std::ios::in | std::ios::binary);
    if (!htmlFile.is_open())
    {
        ft_return("html file doesn't exist: ");
        htmlFile.open("htmlFiles/Pages/errorPages/403.html", std::ios::in | std::ios::binary);
    }

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

	//create response which is sent back to client
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
	std::cout << "\n\033[32m\033[1m" << "RESPONDED:\n\033[0m\033[32m" << std::endl << response << "\033[0m" << std::endl;
	this->_responseHeader.erase();
	htmlFile.close();
	responseFile.close();
	std::ifstream ifs("response/responseCGI.html");
	if (ifs.good())
		ifs.close();
	removeResponseFiles();
    return (0);
}
