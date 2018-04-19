#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

struct arg_struct {
    int newsockfd;
    char* file_dir;
};

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

void *respond(void* arguments) {
    struct arg_struct *args = arguments;

    int newsockfd = args->newsockfd;
    char *file_dir = args->file_dir;

	char buffer[1024];
    memset(buffer, '0', sizeof(buffer)); 

    read(newsockfd,buffer,1023);

    char file_location[1024];
    get_relative_file_location(file_location, buffer);
    
    char dir[1000];
    strcpy(dir, file_dir);
    char* absolute_file_location = strcat(dir, file_location);

    if (access(absolute_file_location, F_OK) != -1 
        && strcmp(get_mime(file_location), "") != 0) {
        
        char response[100]; 
        response[0] = '\0';
        strcat(response, "HTTP/1.0 200 OK\r\nContent-Type: ");
        strcat(response, get_mime(file_location));
        strcat(response, "\r\n\r\n");

        write(newsockfd,response,(int) strlen(response));

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

    return NULL;
}

int main(int argc, char** argv) {
    int sockfd, newsockfd, portno;
	struct sockaddr_in serv_addr;
    
	portno = atoi(argv[1]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0); //create socket

    memset(&serv_addr, '0', sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET; //Type of address – internet IP
    serv_addr.sin_addr.s_addr = INADDR_ANY; //Listen on ANY IP Addr
    serv_addr.sin_port = htons(portno); //Listen on port 

    bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
            
    while (1) {
        listen(sockfd,5);

        newsockfd = accept(sockfd, (struct sockaddr*)NULL, NULL); 

        struct arg_struct args;
        args.newsockfd = newsockfd;
        args.file_dir = argv[2];

        pthread_t tid;
	    pthread_create(&tid, NULL, &respond, (void*)&args);
    }
    
    return 0;
}