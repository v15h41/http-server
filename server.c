#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

void get_relative_file_location(char *file_location, char *buffer) {
    int i;
    for (i = 4; i < 1023; i++) {
        if (buffer[i] == ' ') {
            file_location[i-4] = '\0';
            break;
        } else {
            file_location[i-4] = buffer[i];
        }
    }
}

int main(int argc, char** argv) {
    int sockfd, newsockfd, portno;// clilen;
	char buffer[1024];
    char *dir;
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;
	int n;
    
	portno = atoi(argv[2]);
    dir = argv[1];

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

    char file_location[1024];
    get_relative_file_location(file_location, buffer);
    
    char* absolute_file_location = strcat(dir, file_location);

    

    if (access(absolute_file_location, F_OK) != -1) {
        printf("found\n");
    }

    printf("%s\n", file_location);

    n = write(newsockfd,"HTTP/1.0 404 Not Found\n",24);

    close(sockfd);

    return 0;
}