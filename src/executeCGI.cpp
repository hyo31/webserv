#include "../inc/Server.hpp"

std::vector<std::string>	readFile( std::string header, std::string body )
{
    size_t                      pos;
    std::vector<std::string>    vars;
    std::string                 boundary, filename, fileContent;

    pos = header.find( "boundary=" );
    if ( pos == std::string::npos )
    {
        printerror( "request has no boundary: " );
        return ( vars );
    }
    boundary = header.substr( header.find( "=", pos ) + 1, header.find( "\r\n", pos ) - ( header.find( "=", pos ) + 1 ) );
    pos = body.find( "--" + boundary );
    pos = body.find( "filename=", pos ) + 10;
    filename = body.substr( pos, body.find( "\r\n", pos ) - ( pos + 1 ) );
	if ( filename.empty() )
		return (vars);
    pos = body.find( "\r\n\r\n", pos );
    fileContent = body.substr( pos + 4, body.find( "--" + boundary, pos ) - ( pos + 6) );
    vars.push_back( filename );
    vars.push_back( fileContent );
    return ( vars );
}

// set the environment for CGI
char	**setupEnv( std::string page, int port, std::string path, std::string root, std::string body, std::string header, std::string uploaddir )
{
    std::map<std::string, std::string>	env;
    std::vector<std::string>			vars;
    size_t								pos;
    std::string							temp;

    // find the content type and set the environment accordingly
    pos = header.find( "Content-Type: " );
    if ( pos == std::string::npos )
    {
        printerror( "request has no Content-Type: " );
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
		env["BODY_LEN"] = std::to_string( vars[1].size() );
    }
    env["HTTP_HOST"] =  "localhost:" + std::to_string( port );
    env["REQUEST_URI"] = page;
    env["REMOTE_PORT"] = std::to_string( port );
    env["REQUEST_METHOD"] = "POST";
    env["SERVER_PORT"] = std::to_string( port );
    env["RESPONSE_FILE"] = "response/responseCGI.html";
    env["UPLOAD_DIR"] = path + "/" + root + uploaddir;

    // copy env to a c_str
    char    **c_env = new char*[env.size() + 1];
    int     i = 0;
    for ( std::map<std::string, std::string>::const_iterator it = env.begin(); it != env.end(); it++ )
    {
        temp = it->first + "=" + it->second;
        c_env[i] = new char[temp.size() + 1];
		for ( size_t j = 0; j < temp.size(); ++j ) {
			c_env[i][j] = temp[j];
		}
		c_env[i][temp.size()] = '\0';
        i++;
    }
    c_env[i] = NULL;
    return ( c_env );
}

// execute the CGI
int	executeCGI( std::string page, int port, std::string path, std::string root, std::string body, std::string header, std::string uploaddir )
{
    pid_t		pid;
    char		**env;
    int			status;
    std::string	pathCGI;

    // setup the environmental variables for execve
	env = setupEnv( page, port, path, root, body, header, uploaddir );
    pathCGI = path + page;
    if ( !env )
        return printerror( "failed setting up the environment: " );
    pid = fork();
    if ( pid == -1 )
        return printerror( "fork failed: " );
    
    // execute the script
    if ( !pid )
    {
        execve( pathCGI.c_str(), NULL, env );
        exit ( printerror( "execve failed: " ) );
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
