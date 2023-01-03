#include "../inc/Server.hpp"

std::vector<std::string>    readFile(std::string requestHeader, std::string requestBody)
{
    size_t                      pos;
    std::vector<std::string>    vars;
    std::string                 boundary, filename, fileContent;

    pos = requestHeader.find("boundary=");
    if (pos == std::string::npos)
    {
        ft_return("request has no boundary: ");
        return (vars);
    }
    boundary = requestHeader.substr(requestHeader.find("=", pos) + 1, requestHeader.find("\r\n", pos) - (requestHeader.find("=", pos) + 1));
    pos = requestBody.find("--" + boundary);
    pos = requestBody.find("filename=", pos) + 10;
    filename = requestBody.substr(pos, requestBody.find("\r\n", pos) - (pos + 1));
	if (filename.empty())
		return (vars);
    pos = requestBody.find("\r\n\r\n", pos);
    fileContent = requestBody.substr(pos + 4, requestBody.find("--" + boundary, pos) - (pos + 5));
    vars.push_back(filename);
    vars.push_back(fileContent);
    return (vars);
}

// set the environment for CGI
char        **setupEnv(std::string page, Socket *socket, std::string path, std::string root, std::string requestBody, std::string requestHeader )
{
    std::map<std::string, std::string>  env;
    std::vector<std::string>            vars;
    std::fstream                        receivedMessage;
    std::stringstream                   buff;
    std::string                         request, content;
    std::size_t                         pos;
    
    // find the content type and set the environment accordingly
    pos = requestHeader.find("Content-Type: ");
    if (pos == std::string::npos)
    {
        ft_return("request has no Content-Type: ");
        return (NULL);
    }

    // content type is a form
    if (requestHeader.substr(requestHeader.find(" ", pos) + 1, requestHeader.find("\r\n", pos) - (requestHeader.find(" ", pos) + 1)) == "application/x-www-form-urlencoded")
    {
        env["FILE_NAME"] = "form.log";
        env["QUERY_STRING"] = requestBody;
    }

    // content type is a file
    else
    {
        vars = readFile(requestHeader, requestBody);
        if (vars.empty())
            return (NULL);
		std::cout << "vars0:" <<vars[0] << "  vars1:" << vars[1] << std::endl;
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

    // cope env to a c_str
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

// execute the CGI
int         executeCGI(std::string page, Socket *socket, std::string path, std::string root, std::vector<Client*>::iterator	it)
{
    pid_t           pid;
    char            **env;
    int             status;
    std::string     pathCGI;

    // setup the environmental variables for execve
    env = setupEnv(page, socket, path, root, (*it)->requestBody, (*it)->requestHeader );
    pathCGI = path + page;
    if (!env)
        return ft_return("failed setting up the environment: ");
    pid = fork();
    if (pid == -1)
        return ft_return("fork failed: ");
    
    // execute the script
    if (!pid)
    {
        execve(pathCGI.c_str(), NULL, env);
        exit (ft_return("execve failed: "));
    }
    else
        waitpid(pid, &status, 0);
    
    // wait for the script to finish, then return
    if (WIFEXITED(status))
    {
        for (int i = 0; env[i] != NULL ; i++) {
		    delete env[i];
	    }
	    delete env;
        return (WEXITSTATUS(status));
    }
    return (0);
}

// find the right file to answer to the request
std::string Server::findHtmlFile(int c_fd)
{
    std::vector<Client*>::iterator	it = this->_clients.begin();
    std::vector<Client*>::iterator	end = this->_clients.end();
	std::string::iterator			strit;
	std::string						ret, location, line;
	Config							*config;
    std::fstream                    fstr;
    std::ifstream                   file;
    std::vector<std::string>        head;

    for(; it != end; ++it)
        if (c_fd == (*it)->conn_fd)
            break ;
    fstr.open(this->_sockets[(*it)->port]->logFile);
    if (!fstr.is_open())
    {
        ft_return("could not open logfile: ");
        return ("");
    }

    // set request method to head[0] and request page to head[1]
    for (int i = 0; i < 3 && fstr.peek() != '\n' && fstr >> line; i++)
        head.push_back(line);
	location = head[1];
    fstr.close();
    // get config file for route
	config = this->_sockets[(*it)->port]->getConfig(location);

    // respond to DELETE request
    if (head[0] == "DELETE")
    {
        ret = this->_sockets[(*it)->port]->getLocationPage(location);

        // if DELETE method is not allowed, return 405
		if (ret != "" && std::find(config->methods.begin(), config->methods.end(), head[0]) == config->methods.end())
        {
            _responseHeader = "HTTP/1.1 405 Method Not Allowed";
		    return (config->errorpages + "405.html");
        }

        // check if the requested file exists and delete it
        file.open(config->root + location);
        if (file)
            remove((config->root + location).c_str());
        _responseHeader = "HTTP/1.1 200 OK";
        return ("htmlFiles/index.html");
    }

    // check if method is allowed for route
	if (std::find(config->methods.begin(), config->methods.end(), head[0]) == config->methods.end())
	{
		_responseHeader = "HTTP/1.1 405 Method Not Allowed";
		return (config->errorpages + "405.html");
	}

    // respond to POST request
	if (head[0] == "POST")
	{
        // checks size of upload
		if ((*it)->client_body_too_large == true)
		{
			_responseHeader = "HTTP/1.1 413 Request Entity Too Large";
			return (config->errorpages + "413.html");
		}
        // execute the CGI on the requested file if it has the right extension
		if (location.size() > config->extension.size() && head[1].substr(location.size() - 3, location.size() - 1) == config->extension)
		{
			if (!executeCGI("/" + config->cgi + head[1], this->_sockets[(*it)->port], this->_path, config->root, it))
			{
				_responseHeader = "HTTP/1.1 200 OK";
				return ("response/responseCGI.html");
			}
		}
		else
			return (config->errorpages + "403.html");
	}
    ret = this->_sockets[(*it)->port]->getLocationPage(head[1]);
	
    // respond to a GET request that requests a directory
	if (ret == "Directory")
	{
        // responds the set directory request (if set)
		_responseHeader = "HTTP/1.1 200 OK";
		if (config->directoryRequest != "")
			return (config->root + config->directoryRequest);
		
        // if no directory request is set, search for an index
        location = head[1] + "index.html";
		ret = this->_sockets[(*it)->port]->getLocationPage(location);
		if (ret != "")
		    return (ret);
        
        // create an autoindex if enabled in config
        if (config->autoindex)
            return (this->createAutoIndex(config->root, head[1]));
        _responseHeader = "HTTP/1.1 403 Forbidden";
        return (config->errorpages + "403.html");
	}
    
    // return the requested file
    if (ret != "")
    {
        _responseHeader = "HTTP/1.1 200 OK";
        return (ret);
    }

    // check if requested page is a redirection
	ret = this->_sockets[(*it)->port]->getRedirectPage(head[1]);
	if (ret != "")
	{
		_responseHeader = "HTTP/1.1 301 Moved Permanently\r\nLocation: ";
		_responseHeader.append(ret);
		return (config->errorpages + "301.html" );
	}
    head.clear();
    
    // requested page isn't found
    _responseHeader = "HTTP/1.1 404 Not Found";
    return (config->errorpages + "404.html");
}
