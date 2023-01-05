#include "../inc/Server.hpp"

std::vector<std::string>	readFile( std::string header, std::string body )
{
    size_t                      pos;
    std::vector<std::string>    vars;
    std::string                 boundary, filename, fileContent;

    pos = header.find( "boundary=" );
    if ( pos == std::string::npos )
    {
        ft_return( "request has no boundary: " );
        return ( vars );
    }
    boundary = header.substr( header.find( "=", pos ) + 1, header.find( "\r\n", pos ) - ( header.find( "=", pos ) + 1 ) );
    pos = body.find( "--" + boundary );
    pos = body.find( "filename=", pos ) + 10;
    filename = body.substr( pos, body.find( "\r\n", pos ) - ( pos + 1 ) );
	if ( filename.empty() )
		return (vars);
    pos = body.find( "\r\n\r\n", pos );
    fileContent = body.substr( pos + 4, body.find( "--" + boundary, pos ) - ( pos + 5 ) );
    vars.push_back( filename );
    vars.push_back( fileContent );
    return ( vars );
}

// set the environment for CGI
char	**setupEnv( std::string page, int port, std::string path, std::string root, std::string body, std::string header )
{
    std::map<std::string, std::string>	env;
    std::vector<std::string>			vars;
    size_t								pos;
    std::string							temp;

    // find the content type and set the environment accordingly
    pos = header.find( "Content-Type: " );
    if ( pos == std::string::npos )
    {
        ft_return( "request has no Content-Type: " );
        return nullptr;
    }

    // content type is a form
    if ( header.substr( header.find( " ", pos ) + 1, header.find( "\r\n", pos ) - ( header.find( " ", pos ) + 1 ) ) == "application/x-www-form-urlencoded" )
    {
        env["FILE_NAME"] = "form.log";
        env["QUERY_STRING"] = body;
    }

    // content type is a file
    else
    {
        vars = readFile( header, body );
        if (vars.empty())
            return nullptr;
        env["FILE_NAME"] = vars[0];
        env["FILE_BODY"] = vars[1];
    }
    env["HTTP_HOST"] =  "localhost:" + std::to_string( port );
    env["REQUEST_URI"] = page;
    env["REMOTE_PORT"] = std::to_string( port );
    env["REQUEST_METHOD"] = "POST";
    env["SERVER_PORT"] = std::to_string( port );
    env["RESPONSE_FILE"] = "response/responseCGI.html";
    env["PATH"] = path + "/" + root;

    // copy env to a c_str
    char    **c_env = new char*[env.size() + 1];
    int     i = 0;
    for ( std::map<std::string, std::string>::const_iterator it = env.begin(); it != env.end(); it++ )
    {
        temp = it->first + "=" + it->second;
        c_env[i] = new char[temp.size() + 1];
        strcpy( c_env[i], temp.c_str() );
        i++;
    }
    c_env[i] = NULL;
    return ( c_env );
}

// execute the CGI
int	executeCGI( std::string page, int port, std::string path, std::string root, std::string body, std::string header )
{
    pid_t		pid;
    char		**env;
    int			status;
    std::string	pathCGI;

    // setup the environmental variables for execve
    env = setupEnv( page, port, path, root, body, header );
    pathCGI = path + page;
    if ( !env )
        return ft_return( "failed setting up the environment: " );
    pid = fork();
    if ( pid == -1 )
        return ft_return( "fork failed: " );
    
    // execute the script
    if ( !pid )
    {
        execve( pathCGI.c_str(), NULL, env );
        exit ( ft_return( "execve failed: " ) );
    }
    else
        waitpid( pid, &status, 0 );
    
    // wait for the script to finish, then return
    if ( WIFEXITED(status) )
    {
        for ( int i = 0; env[i] != NULL ; i++ ) {
		    delete env[i];
	    }
	    delete env;
        return WEXITSTATUS( status );
    }
    return 0;
}

// find the right file to answer to the request
std::string Server::findHtmlFile( Client* client )
{
	std::string				page, ret, line, location, index, method;
	int						port, c_fd;
	Config					*config;
	std::string::iterator	strit;
    std::fstream			fstr;
    std::ifstream			file;

    // get config file for route and clients request info
	c_fd = client->getConnectionFD();
	port = client->getPort();
	location = client->getLocation();
	method = client->getMethod();
	config = this->_sockets[port]->getConfig( location );

    // respond to DELETE request
    if ( method == "DELETE" )
    {
        page = this->_sockets[port]->getLocationPage( location );
        // if DELETE method is not allowed, return 405
		if ( page != "" && std::find( config->methods.begin(), config->methods.end(), method ) == config->methods.end() )
        {
            _responseHeader = "HTTP/1.1 405 Method Not Allowed";
		    return (config->errorpages + "405.html");
        }
        // check if the requested file exists and delete it
        file.open( config->root + location );
        if ( file )
            remove( ( config->root + location ).c_str() );
        _responseHeader = "HTTP/1.1 200 OK";
        return ( "htmlFiles/index.html" );
    }

    // check if method is allowed for route
	if ( std::find( config->methods.begin(), config->methods.end(), method ) == config->methods.end() )
	{
		_responseHeader = "HTTP/1.1 405 Method Not Allowed";
		return (config->errorpages + "405.html");
	}

    // respond to POST request
	if ( method == "POST" )
	{
        // checks size of upload
		if ( client->bodyTooLarge() == true )
		{
			_responseHeader = "HTTP/1.1 413 Request Entity Too Large";
			return ( config->errorpages + "413.html" );
		}
        // execute the CGI on the requested file if it has the right extension
		if ( location.size() > config->extension.size() && location.substr( location.size() - 3, location.size() - 1) == config->extension )
		{
			if ( !executeCGI("/" + config->cgi + location, port, this->_path, config->root, client->getBody(), client->getHeader() ) )
			{
				_responseHeader = "HTTP/1.1 200 OK";
				return ( "response/responseCGI.html" );
			}
		}
		else
			return ( config->errorpages + "403.html" );
	}
    page = this->_sockets[port]->getLocationPage( location );
	
    // respond to a GET request that requests a directory
	if ( page == "Directory" )
	{
        // responds the set directoryrequest (if set)
		_responseHeader = "HTTP/1.1 200 OK";
		if ( config->directoryRequest != "" )
			return ( config->root + config->directoryRequest );
		
        // if no directory request is set, search for an index
        index = location + "index.html";
		page = this->_sockets[port]->getLocationPage( index );
		if ( page != "" )
		    return ( page );

        // if there was no index -> create an autoindex (if enabled in config)
        if ( config->autoindex )
            return ( this->createAutoIndex( config->root, location ) );
        _responseHeader = "HTTP/1.1 403 Forbidden";
        return ( config->errorpages + "403.html" );
	}
    
    // return the requested file
    if ( page != "" )
    {
        _responseHeader = "HTTP/1.1 200 OK";
        return ( page );
    }

    // check if requested page is a redirection
	page = this->_sockets[port]->getRedirectPage( location );
	if ( page != "" )
	{
		_responseHeader = "HTTP/1.1 301 Moved Permanently\r\nLocation: ";
		_responseHeader.append( page );
		return ( config->errorpages + "301.html" );
	}
    
    // requested page isn't found
    _responseHeader = "HTTP/1.1 404 Not Found";
    return ( config->errorpages + "404.html" );
}
