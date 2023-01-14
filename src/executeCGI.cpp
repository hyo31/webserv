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
        return ( env );
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
        if (body.size() > 6)
            env["FILE_NAME"] = body.substr(0, 6);
        else
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
    pid_t		                        pid, pid2, timeout_pid;
    std::map<std::string, std::string>  env;
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
        // set a time-out process so that the executed script can't hang indefinitely
        timeout_pid = fork();
        if ( !timeout_pid )
        {
            sleep( 5 );
            exit( 0 );
        }
        else
        {
            pid2 = fork();
            if ( pid2 == -1 )
                exit( printerror( "fork failed: " ) );
            // child process to execute script
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
                std::ofstream   ofs;
                ofs.open("response/responseCGI");
                ofs.close();
                int fd = open("response/responseCGI", O_WRONLY);
                if ( !fd )
                    exit (printerror("failed to open file: "));
                dup2(fd, 1);
                execve(pathCGI.c_str(), NULL, c_env);
                exit( printerror( "execve failed: " ) );
            }
            else
            {
                // check which child exits first and terminating the other one
                pid_t exited_pid = wait(NULL);
                if (exited_pid == timeout_pid)
                {
                    kill(pid2, SIGKILL);
                    return ( 1 );
                }
                kill(timeout_pid, SIGKILL);
                return ( 0 );
            }
        }
    }
    return ( -1 );
}
