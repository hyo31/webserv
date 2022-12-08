#include "../inc/Server.hpp"

std::vector<std::string>    readFile(std::string request)
{
    size_t                      pos;
    std::vector<std::string>    vars;
    std::string                 boundary, filename, fileContent;

    pos = request.find("boundary=");
    if (pos == std::string::npos)
    {
        ft_return("request has no boundary: ");
        return (vars);
    }
    boundary = request.substr(request.find("=", pos) + 1, request.find("\r\n", pos) - (request.find("=", pos) + 1));
    pos = request.find("--" + boundary);
    pos = request.find("filename=", pos) + 10;
    filename = request.substr(pos, request.find("\r\n", pos) - (pos + 1));
    pos = request.find("\r\n\r\n", pos);
    fileContent = request.substr(pos + 4, request.find("--" + boundary, pos) - (pos + 5));
    vars.push_back(filename);
    vars.push_back(fileContent);
    return (vars);
}

std::string readForm(std::string request)
{
    size_t  pos;

    pos = request.find("\r\n\r\n");
    if (pos == std::string::npos)
    {
        ft_return("request has no body: ");
        return ("");
    }
    return (request.substr(pos + 4, request.length() - pos - 4));
}

char        **setupEnv(std::string page, Socket *socket, std::string path, std::string root)
{
    std::map<std::string, std::string>  env;
    std::vector<std::string>            vars;
    std::fstream                        receivedMessage;
    std::stringstream                   buff;
    std::string                         request, content;
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
    pos = request.find("Content-Type: ");
    if (pos == std::string::npos)
    {
        ft_return("request has no Content-Type: ");
        return (NULL);
    }
    if (request.substr(request.find(" ", pos) + 1, request.find("\r\n", pos) - (request.find(" ", pos) + 1)) == "application/x-www-form-urlencoded")
    {
        env["FILE_NAME"] = "form.log";
        env["QUERY_STRING"] = readForm(request);
    }
    else
    {
        vars = readFile(request);
        if (vars.empty())
            return (NULL);
        env["FILE_NAME"] = vars[0];
        env["FILE_BODY"] = vars[1];
    }
    env["HTTP_HOST"] =  "localhost:" + std::to_string(socket->port);
    env["REQUEST_URI"] = page;
    env["REMOTE_PORT"] = std::to_string(socket->port);
    env["REQUEST_METHOD"] = "POST";
    env["SERVER_PORT"] = std::to_string(socket->port);
    env["RESPONSE_FILE"] = "response/responseCGI.html";
    env["PATH"] = path + "/" + root;
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

int         executeCGI(std::string page, Socket *socket, std::string path, std::string root)
{
    pid_t           pid;
    char            **env;
    int             status;
    std::string     pathCGI;

    env = setupEnv(page, socket, path, root);
    pathCGI = path + page;
    std::cout << pathCGI << std::endl;
    if (!env)
        return ft_return("failed setting up the environment: ");
    pid = fork();
    if (pid == -1)
        return ft_return("fork faield: ");
    if (!pid)
    {
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
	Config							*config;
    std::fstream                    fstr;

    for(; it != end; ++it)
        if (c_fd == (*it)->conn_fd)
            break ;
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
	config = this->_sockets[(*it)->port]->getConfig(head[1]);
	if ((*it)->client_body_too_large == true)
	{
	    _responseHeader = "HTTP/1.1 413 Request Entity Too Large";
        return (config->errorpages + "413.html");
	}
    if (head[1].size() > config->extension.size() && head[1].substr(head[1].size() - 3, head[1].size() - 1) == config->extension)
	{
        int expression = checkMaxClientBodySize(it);
        switch (expression)
        {
            case 1:
                _responseHeader = "HTTP/1.1 413 Request Entity Too Large";
                return (config->errorpages + "413.html");
            default:
                if (!executeCGI("/" + config->cgi + head[1], this->_sockets[(*it)->port], this->_path, config->root))
                {
                    _responseHeader = "HTTP/1.1 200 OK";
                    return ("response/responseCGI.html");
                }
        }
	}
    
    if (std::find(config->methods.begin(), config->methods.end(), head[0]) == config->methods.end())
    {
        _responseHeader = "HTTP/1.1 405 Method Not Allowed";
        return (config->errorpages + "405.html");
    }
	/* if request GET = directory */
    ret = this->_sockets[(*it)->port]->getLocationPage(head[1]);
	if (ret == "Directory")
	{
		_responseHeader = "HTTP/1.1 200 OK";
		if (config->directoryRequest != "")
			return (config->root + config->directoryRequest);
		ret = this->_sockets[(*it)->port]->getLocationPage(head[1] + "index.html");
		if (ret != "")
        {
            (*it)->current_route = head[1];
		    return (ret);
        }
        std::cout << config->autoindex << std::endl;
        if (config->autoindex)
            return (this->createAutoIndex(config->root, head[1]));
        _responseHeader = "HTTP/1.1 403 Forbidden";
        return (config->errorpages + "403.html");
	}
    if (ret != "")
    {
        _responseHeader = "HTTP/1.1 200 OK";
         (*it)->current_route = head[1];
        return (ret);
    }
	ret = this->_sockets[(*it)->port]->getRedirectPage(head[1]);
	if (ret != "")
	{
		_responseHeader = "HTTP/1.1 301 Moved Permanently\r\nLocation: ";
		_responseHeader.append(ret);
		return ( config->errorpages + "301.html" );
	}
    head.clear();
    _responseHeader = "HTTP/1.1 404 Not Found";
    return (config->errorpages + "404.html");
}
