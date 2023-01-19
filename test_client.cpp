#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>
#include <time.h>

#define PORT 8007

int main(int argc, char const *argv[])
{
    int sock = 0;
    struct sockaddr_in serv_addr;
	char hello[500] = "DELETE /uploads/abc.txt HTTP/1.1\r\nHost: localhost\r\n\r\n";
    
    char buffer[6000] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }   
    send(sock, hello, sizeof(hello) , 0);
    printf("Hello message sent\n");
	recv(sock, buffer, 6000, 0);
    return 0;
}
