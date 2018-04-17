#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

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

void send_404(int sockfd) {
    write(sockfd,"HTTP/1.0 404 Not Found\r\n",23);
}

char *get_mime(char *file) {
    char *ext = strrchr(file, '.');
    if (!ext) {
        return "";
    }

    if (strcmp(ext+1, "html") == 0) {
        return"text/html";        
    } else if (strcmp(ext+1, "css") == 0) {
        return "text/css";        
    } else if (strcmp(ext+1, "jpg") == 0 || strcmp(ext+1, "jpeg") == 0) {
        return "image/jpeg";
    } else if (strcmp(ext+1, "js") == 0) {
        return "application/javascript";
    } else {
        return "";
    }
}

int main(int argc, char** argv) {
    int sockfd, newsockfd, portno;// clilen;
	char buffer[1024];
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;
	int n;
    
	portno = atoi(argv[1]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0); //create socket

    memset(&serv_addr, '0', sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET; //Type of address – internet IP
    serv_addr.sin_addr.s_addr = INADDR_ANY; //Listen on ANY IP Addr
    serv_addr.sin_port = htons(portno); //Listen on port 

    bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
    

    
        
    while (1) {
        listen(sockfd,1);

        newsockfd = accept(sockfd, (struct sockaddr*)NULL, NULL); 

        memset(buffer, '0', sizeof(buffer)); 

        n = read(newsockfd,buffer,1023);


        char file_location[1024];
        get_relative_file_location(file_location, buffer);
        
        char dir[1000];
        strcpy(dir, argv[2]);
        char* absolute_file_location = strcat(dir, file_location);
        printf("%s", absolute_file_location);

        if (access(absolute_file_location, F_OK) != -1 
            && strcmp(get_mime(file_location), "") != 0) {
            
            char response[100]; 
            response[0] = '\0';
            strcat(response, "HTTP/1.0 200 OK\nContent-Type: ");
            strcat(response, get_mime(file_location));
            strcat(response, "\n\n");

            printf("%s %d\n", response, (int) strlen(response));

            write(newsockfd,response,(int) strlen(response));


/*
            int bytes_read;
            void *progress = send_buffer;
            while ((bytes_read=read(fd, send_buffer, 1024))>0) {
                int bytes_written = write(newsockfd, progress, bytes_read);
                bytes_read -= bytes_written;
                progress += bytes_written;
            }
            */

            char send_buffer[1024];
            int fd = open(absolute_file_location, O_RDONLY);
            while (1) {
                int bytes_read = read(fd, send_buffer, sizeof(send_buffer));
                if (bytes_read == 0) 
                    break;
                void *p = send_buffer;
                while (bytes_read > 0) {
                    int bytes_written = write(newsockfd, p, bytes_read);
                    bytes_read -= bytes_written;
                    p += bytes_written;
                }
            }

        } else {
            send_404(newsockfd);
        }
        close(newsockfd);
    }
    

    

    return 0;
}