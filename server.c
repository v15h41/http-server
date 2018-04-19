#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#define HTTP_404 "HTTP/1.0 404 Not Found\r\n"
#define HTTP_404_LENGTH 23
#define BUFFER 1024         //chosen buffer size for each socket operation
#define GET_CHAR_OFFSET 4   //characters offset after "GET " in the request
#define CONNECTIONS 5       //connections supported
#define HTTP_200 "HTTP/1.0 200 OK\r\n"
#define CONTENT_TYPE "Content-Type: "
#define BLANK_LINE "\r\n\r\n"   //double carriage return 

#define HTML_EXT "html"
#define JS_EXT "js"
#define CSS_EXT "css"
#define JPG_EXT "jpg"
#define JPEG_EXT "jpeg"
#define HTML_MIME "text/html"
#define CSS_MIME "text/css"
#define JS_MIME "application/javascript"
#define JPG_MIME "image/jpeg"

//struct that allows passing multiple arguments to a threaded function
struct arg_struct {
    int newsockfd;
    char* file_dir;
};

void get_relative_file_location(char *file_location, char *buffer);
void send_404(int sockfd);
char *get_mime(char *file);
void *respond(void* arguments);
void send_200(int newsockfd, char *absolute_file_location);

int main(int argc, char** argv) {
    int sockfd, newsockfd, portno;
	struct sockaddr_in serv_addr;
    
	portno = atoi(argv[1]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0); //create socket

    memset(&serv_addr, '0', sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET; //Type of address – internet IP
    serv_addr.sin_addr.s_addr = INADDR_ANY; //Listen on ANY IP Addr
    serv_addr.sin_port = htons(portno); //Listen on port 

    //bind socket to defined server address
    bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
            
    //listen for connections
    while (1) {
        //listen for incoming requests
        listen(sockfd, CONNECTIONS);

        //accept the request and pass it to a new socket
        newsockfd = accept(sockfd, (struct sockaddr*)NULL, NULL); 

        //put required values into the argument struct
        struct arg_struct args;
        args.newsockfd = newsockfd;
        args.file_dir = argv[2];

        //create a new thread and pass the arguments into the function
        pthread_t tid;
	    pthread_create(&tid, NULL, &respond, (void*)&args);
    }
    
    return 0;
}

//from the request buffer, get the relative location of the requested file
void get_relative_file_location(char *file_location, char *buffer) {
    //from the get char offset, read int the string until the next space char
    int i;
    for (i = GET_CHAR_OFFSET; i < BUFFER-1; i++) {
        if (buffer[i] == ' ') {
            //if found space, null terminate the string
            file_location[i-GET_CHAR_OFFSET] = '\0';
            break;
        } else {
            file_location[i-GET_CHAR_OFFSET] = buffer[i];
        }
    }
}

//wrtie a 404 to the socket
void send_404(int sockfd) {
    write(sockfd, HTTP_404, HTTP_404_LENGTH);
}

//based on the file extension from the file name, return the mime type
char *get_mime(char *file) {
    //search the string for the last dot
    char *ext = strrchr(file, '.');
    if (!ext) {
        return "";
    }

    //compare the found extension with the known extensions
    if (strcmp(ext+1, HTML_EXT) == 0) {
        return HTML_MIME;        
    } else if (strcmp(ext+1, CSS_EXT) == 0) {
        return CSS_MIME;        
    } else if (strcmp(ext+1, JPG_EXT) == 0 || strcmp(ext+1, JPEG_EXT) == 0) {
        return JPG_MIME;
    } else if (strcmp(ext+1, JS_EXT) == 0) {
        return JS_MIME;
    } else {
        //if extension not found, return nothing
        return "";
    }
}

//send HTTP 200 status along with requested file and its mime type
void send_200(int newsockfd, char *absolute_file_location) {
    //initialise the response buffer with a null termination
    char response[BUFFER]; 
    response[0] = '\0';

    //concatenate the status code
    strcat(response, HTTP_200);
    //concatenate the mime type
    strcat(response, CONTENT_TYPE);
    strcat(response, get_mime(absolute_file_location));
    //add a double CRLF
    strcat(response, BLANK_LINE);

    //write the response to the socket
    write(newsockfd,response,(int) strlen(response));

    //create send buffer
    char send_buffer[BUFFER];
    //open file
    int fd = open(absolute_file_location, O_RDONLY);
    //while file is sending
    while (1) {
        //fill buffer with bytes
        int bytes_read = read(fd, send_buffer, sizeof(send_buffer));
        //if no more bytes to be read, break
        if (bytes_read == 0) 
            break;

        //keep track of where we are in the buffer in case it fails
        void *progress = send_buffer;
        while (bytes_read > 0) {
            //write bytes to socket and return how many bytes successfully
            //written
            int bytes_written = write(newsockfd, progress, bytes_read);
            
            bytes_read -= bytes_written;
            progress += bytes_written;
        }
    }
}

void *respond(void* arguments) {
    //get arguments out of the argument struct
    struct arg_struct *args = arguments;
    int newsockfd = args->newsockfd;
    char *file_dir = args->file_dir;

    //initialize the receive buffer
	char buffer[BUFFER];
    memset(buffer, '0', sizeof(buffer)); 

    //read the data from the socket into the receive buffer
    read(newsockfd, buffer, BUFFER-1);

    //initialize the file location char array
    char file_location[BUFFER];
    //get the requested file location from the buffer and put it into the array
    get_relative_file_location(file_location, buffer);
    
    //concatenate the requested location to the servers working directory
    char dir[BUFFER];
    strcpy(dir, file_dir);
    char* absolute_file_location = strcat(dir, file_location);

    //check if the file exists and if we can serve its mime type
    if (access(absolute_file_location, F_OK) != -1 
        && strcmp(get_mime(file_location), "") != 0) {
        //send HTTP 200 status along with requested file and its mime type
        send_200(newsockfd, absolute_file_location);
    } else {
        //if file does not exist or mime type not supported send 404
        send_404(newsockfd);
    }
    
    close(newsockfd);

    return NULL;
}