#include "../inc/Server.hpp"

// create an automatically generated index for the requested directory
std::string Server::createAutoIndex( std::string root, std::string folder )
{
    std::ofstream   autoindexFile;
    DIR             *directory;

    // std::cout << folder[folder.size() - 1] << std::endl;
    if (folder[folder.size() - 1] != '/')
        folder += "/";
	std::string		page = root + folder;

    autoindexFile.open( "response/autoindex.html", std::ofstream::out | std::ofstream::trunc );
    if ( !autoindexFile.is_open() )
    {
        printerror( "failed to create autoindex: " );
        return root + "/pages/errorpages/404.html";
    }
    directory = opendir( page.c_str() );
    if ( !directory )
    {
        printerror( "can not open directory(createAutoIndex): " );
        return root + "/pages/errorpages/404.html";
    }

    // content of the autoindex file
    autoindexFile <<
    "<!DOCTYPE html>\n\
    <html lang=\"en\">\n\
    <head>\n\
    <title>Webserv</title>\n\
    <link rel=\"icon\" type=\"image/png\" sizes=\"16x16\" href=\"data:image/png;base64,\
    iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAIAAAAlC+aJAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFn\
    ZVJlYWR5ccllPAAABORJREFUeNrsWltIY1cUNbljKtGMqKXVzOhIRRttg6kKqcVKfQVHHNCPBKpYfKCoUCTG+KHEnyi0xRcK+iODocRnPgQ/\
    6gs0qOD7hSKNSDVSEpWI0mhjNKYbLYMMc09uzE1yB7K+9Obuc/bae5+z99nnenl54IEHHzVopI8YExOTkpLC4/EiIyMjIiKe3wOe39zcXF5e6\
    vV6jUazt7e3sLCgVqtPT0+pYomEhISOjg6dTme1B+vr6xKJJDg42G160+l0kUi0trZmdQC3t7cDAwNcLtfV2mdkZOzs7FjJA9AIDQ11herg9O\
    HhYasTAOukurqaRqM5UXuBQABr0epMTE5OhoSEOEX7mpqau7s7q/Oh1WpjY2NJ3XFptLa2NqsLsbq6SiaB9vZ2V2pvMBiioqJsavWMoPZ1dXVV\
    VVV2Ed7Y2FhZWYGcBdnKYrH4+Piw2WxIc0lJSTbjG7JeTk4OpDxybJ+VlUXccvv7++jcBKEYHx/f1dVlNBrxBsnLyyMa2H/yv7fho+DPX/3+Fm\
    OxbI4FlgZH9fb2QmIiuBfLZLLy8nLIho+fNzQ0yOVy0gi8aPvNN/FbmwOpVCpQBQLXXvfy+XylUglV08O/CoWiqKgInEC0FED/zEr9gYj2UqlUK\
    BQ+QXvA4uIi1FHj4+Pw98zMTFlZGXHtAdjPL1/hegfD2K2/Yn5+6CEKCwu7u7sdWWMmk2loaIjFYlVWVkImtksWtQuxMgXetupEyGvgdMf3CbPZ\
    LBaLn1JNIvJWUNFPaGEwW0tLi3vLeFwCzG943i9fICRPTk4gXt1+DsEl8Dz7NVqytrb24uKCokdKDMNgS/H398cTg2wFeR6qOpsTHHUmu8EDcX\
    FxCO0BUNUR0d5tIZSamoreMfr7+ylyFqfjdRYQMnNzc2dnZ5QmwOFwEDJqtZo6faEPEwgPD0fIbG9vU52AH7J8ODw8pDoBJpOJkDk/P6cOgWc\
    /vrU7Ht788sfxP2aCL4cWf+0GD1zfovZ4JoNO9RBCE/jUj0F1AqfGG4QM2/8TqhPQXVwjZL5i+1KdwN/nKAJffsZkMjBKE9jRoc513hjtuy/8\
    KU3gL4Pp3xvUOn4dE+jU/jEJUCqV6AZWQUEBpQmkp6ejCeh0uofLL4qCTqcfHBygOYCX3K4n7mYC+plMpuzsbIQwl8uFg8HS0hJFncBgMLRaLd\
    oJFoslPz+fBENiWFNTU0BAAMkcRCKRzXY0cCgtLXVkFl9f35GRERhqenoarEYyh7GxMSJd9Z6eHj9bTUi84+vW1ta7cRQKBck3fGFhYRDoRDjA\
    os/NzSU+PYvFksvl19fX740jk8kc7Qu9B1jKo6OjBEfc3Nzs7OwcHBw0Go1478CRtaSkpKKiIigo6IMvwLrq6+t7/ATv3ELUWvX19Y2NjcQNYza\
    b5+fnl5eXNRqNwWAAM0Ogs9ns6Ojo5ORkdNfD6/6KKS0tbXZ2ljQCXveXfPZekzkCiNvExMR312R4BOw4W4nFYuDgMgKBgYFE2md2EIDlBRykU\
    qldNyhPxtHRUXFxMZkEHtDc3JyZmXl8fOxU7aempvh8PuwH5BMATExM8Hg8lUrlDNWvrq4kEolAIIBi8ennAZvQ6/VCoRCm2d3dJVF72Hw5HE5\
    ra6trovT/ovUj/uDpMdz1yZnnoz8PPPDAvfhPgAEAeYUvyGcO9EMAAAAASUVORK5CYII=\" />\
    </head>\n\
    <body\">\n\
    <h1>" + folder + "</h1><br>" << std::endl;

    // find all files and directories in the requested direcotry
    for ( struct dirent *dirEntry = readdir( directory ); dirEntry; dirEntry = readdir( directory ) )
    {
        std::string link = std::string( dirEntry->d_name );
        if ( link != "." && link != ".." )
        {
            std::string	temp = page + link;
			DIR* 		tempDir = opendir( temp.c_str() );

            // add a file to the autoindex
			if ( !tempDir )
                autoindexFile << "<a href=\"" << folder + link << "\">/" << link << "</a><br>" << std::endl;
            // add a directory to the autoindex
            else
            {
                autoindexFile << "<a href=\"" << folder + link << "/\">/" << link << "/</a><br>" << std::endl;
                closedir( tempDir );
            }
        }
    }
    autoindexFile <<\
    "</body>\n\
    </html>" << std::endl;
	closedir( directory );
    return "response/autoindex.html";
}
