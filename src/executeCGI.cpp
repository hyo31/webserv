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
std::map<std::string, std::string>   setupEnv( std::string page, int port, std::string path, std::string root, std::string body, std::string header, std::string uploaddir, std::string method )
{
    std::map<std::string, std::string>	env;
    std::vector<std::string>			vars;
    size_t								pos;
    std::string							contentType;

    env["HTTP_HOST"] =  "localhost:" + std::to_string( port );
    env["REQUEST_URI"] = page;
    env["REMOTE_PORT"] = std::to_string( port );
    env["REQUEST_METHOD"] = method;
    env["SERVER_PORT"] = std::to_string( port );
    env["RESPONSE_FILE"] = "response/responseCGI.html";
    env["UPLOAD_DIR"] = path + "/" + root + uploaddir;
    
    // find the content type and set the environment accordingly
    if ( method == "GET" )
    {
        env["FILE_NAME"] = "form.log";
        env["QUERY_STRING"] = body;
    }
    pos = header.find( "Content-Type: " );
    if ( pos == std::string::npos )
    {
        printerror( "request has no Content-Type: " );
        return ( env);
    }
    contentType = header.substr( header.find( " ", pos ) + 1, header.find( "\r\n", pos ) - ( header.find( " ", pos ) + 1 ) );
    // content type is a form
    if ( contentType == "application/x-www-form-urlencoded" )
    {
        env["FILE_NAME"] = "form.log";
        env["QUERY_STRING"] = body;
    }
    // content type is a file
    else if ( contentType == "plain/text" )
    {
        env["FILE_NAME"] = body;
        env["FILE_BODY"] = body;
        if (body.find("=") != std::string::npos)
        {
            env["FILE_NAME"] = page + "/" + body.substr(0, body.find("="));
            env["FILE_BODY"] = body.substr(body.find("=") + 1, body.size() - (body.find("=") + 1));
        }
        env["BODY_LEN"] = std::to_string( env["FILE_BODY"].size() );
    }
    //content type is a file with a boundary
    else
    {
        vars = readFile( header, body );
        if (vars.empty())
            return ( env );
        env["FILE_NAME"] = vars[0];
        env["FILE_BODY"] = vars[1];
		env["BODY_LEN"] = std::to_string( vars[1].size() );
    }
    return ( env );
}

// execute the CGI
int	executeCGI( std::string page, int port, std::string path, std::string root, std::string body, std::string header, std::string uploaddir, std::string method )
{
    pid_t		                        pid, pid2;
    std::map<std::string, std::string>  env;
    int			                        status;
    std::string	                        pathCGI, temp;

    // setup the environmental variables for execve
	env = setupEnv( page, port, path, root, body, header, uploaddir, method );
    if ( !env.size() )
        return printerror( "failed setting up the environment: " );
    pathCGI = path + page;
    pid = fork();
    if ( pid == -1 )
        return printerror( "fork failed: " );
    
    // execute the script
    if ( !pid )
    {
        pid2 = fork();
        if ( pid2 == -1 )
            exit( printerror( "fork failed: " ) );
        if ( !pid2 )
        {
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
            execve( pathCGI.c_str(), NULL, c_env );
            exit( printerror( "execve failed: " ) );
        }
        else
            waitpid( pid, &status, 0 );
        // wait for the script to finish, then return
        if ( WIFEXITED( status ) )
        {
            // for ( int i = 0; c_env[i] != NULL ; i++ ) {
	    	//     delete c_env[i];
	        // }
	        // delete[] c_env;
            if (WEXITSTATUS( status ))
                return ( 1 );
            return ( 0 );
        }
    }
    return ( -1 );
}
