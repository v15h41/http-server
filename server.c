#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main(int argc, char** argv) {
    int sockfd, newsockfd, portno;// clilen;
	char buffer[1024];
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;
	int n;

	portno = 3000;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); //create socket

    memset(&serv_addr, '0', sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET; //Type of address – internet IP
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //Listen on ANY IP Addr
    serv_addr.sin_port = htons(portno); //Listen on port 

    bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
    listen(sockfd,5);

    newsockfd = accept(sockfd, (struct sockaddr*)NULL, NULL); 

    memset(buffer, '0', sizeof(buffer)); 

    n = read(newsockfd,buffer,1023);

    printf("%s", buffer);


    close(sockfd);

    return 0;
}