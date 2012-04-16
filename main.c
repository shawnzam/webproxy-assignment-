/*
 Shawn Zamechek 
 Web Proxy
 */
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include "mysocket.h"
#include <time.h>
#include <netdb.h>
#include <string.h>

#define MAX_REQUEST 8192
#define MAX_URL 2048
#define WEBPORT 80
/*
 This struct contains the info that is passed to a thread.
 */
struct args 
{
    int id;
    int connfd;
    char buffer[MAX_REQUEST];
};
/*
 This struct is filled from data from the remote http request. It contains the whole remote request, the method, path, 
 version, contenttype(not implemented yet), the host and page or file.
 */
struct req
{
    char request[MAX_REQUEST];
    char method[16];
    char path[MAX_URL];
    char version[16];
    char contenttype[128];
    char host[MAX_URL];
    char page[MAX_URL];
};

void *handleConnection(void *args);
void getReqInfo(char *request, struct req *myreq);
void returnFile(int socket, char* filepath);
void sendRemoteReq(char filename[MAX_URL], char host[MAX_URL], int socket);
FILE *fp_log; //log

/*
 Main setup the socket for listening and enters an infinate while loop. 
 When a connection arrives it spawns a thread and passes the request to the handleConnection function
 */
int main(int argc, char** argv) {
    time_t result;
    result = time(NULL);
    struct tm* brokentime = localtime(&result);
    int port = atoi(argv[1]);
    int srvfd  = makeListener(port);
    int threadID = 0;
    fp_log = fopen("server.log", "a");
    fprintf(fp_log, "Server started at %s \nListening on Port %d\n",asctime(brokentime), port);	
    printf("Server started at %s \nListening on Port %d\n",asctime(brokentime), port);	
    fclose(fp_log);
    while (1)
    {
        struct args* myargs;
        myargs = (struct args*) malloc(sizeof(struct args));
        myargs->connfd = listenFor(srvfd);
        fp_log = fopen("server.log", "a");
        pthread_t thread;
        threadID++;
        myargs->id = threadID;
        pthread_create(&thread, NULL, handleConnection, myargs); //thread never joins
        fprintf(fp_log, "Thread %d created\n", myargs->id); 
        printf("Thread %d created\n", myargs->id);
    }
    close(srvfd);     
    return 0;
}
/*
 Handels the connection by passing the request to the getReqInfo function for parsing 
 and then retuns the request http page via the sendRemoteReq() function.
 */
void *handleConnection(void *args)
{
    struct args* myarg = (struct args*) args;
    struct req* myreq;
    myreq = (struct req*) malloc(sizeof(struct req));
    int bytesRead;
    bzero(myarg->buffer, MAX_REQUEST);
    bytesRead = read(myarg->connfd, myarg->buffer, sizeof(myarg->buffer));
    getReqInfo(myarg->buffer, myreq);
    sendRemoteReq(myreq->page, myreq->host, myarg->connfd);
    fprintf(fp_log, "Thread %d exits\n", myarg->id); 
    printf("Thread %d exits\n", myarg->id);
    pthread_exit(NULL);
}
/*
 parses the HTTP req and fills the struct req
 */
void getReqInfo(char *request, struct req *myreq)
{
    char host[MAX_URL], page[MAX_URL];
    strncpy(myreq->request, request, MAX_REQUEST-1);
    strncpy(myreq->method, strtok(request, " "), 16-1);
    strncpy(myreq->path, strtok(NULL, " "), MAX_URL-1);
    strncpy(myreq->version, strtok(NULL, "\r\n"), 16-1);
    sscanf(myreq->path, "http://%99[^/]%99[^\n]", host, page);
    strncpy(myreq->host, host, MAX_URL-1);
    strncpy(myreq->page, page, MAX_URL-1);
    fprintf (fp_log, "method: %s\nversion: %s\npath: %s\nhost: %s\npage: %s\n", myreq->method, myreq->version, myreq->path, myreq->host, myreq->page);
    printf("method: %s\nversion: %s\npath: %s\nhost: %s\npage: %s\n", myreq->method, myreq->version, myreq->path, myreq->host, myreq->page);
    
}
/*
 Sends the HTTP request to the remote site and retuns the HTTP payload to the connected client.
 */
void sendRemoteReq(char filename[MAX_URL], char host[MAX_URL], int socket)
{
    time_t result;
    result = time(NULL);
    struct tm* brokentime = localtime(&result);
    int chunkRead;
    int chunkWritten;
	size_t fd;
    char data[512];
	char reqBuffer[512];
    bzero(reqBuffer, 512);
    fd = connectTo(host, WEBPORT);
    strcat(reqBuffer, "GET ");
    strcat(reqBuffer, filename);
    strcat(reqBuffer, " HTTP/1.1\n");
    strcat(reqBuffer, "host: ");
    strcat(reqBuffer, host);
    strcat(reqBuffer, "\n\n");
    fprintf(fp_log, "request sent to %s :\n%s\nSent at: %s\n", host, reqBuffer, asctime(brokentime));
    printf("request sent to %s :\n%s\nSent at: %s\n", host, reqBuffer, asctime(brokentime));
    chunkRead = write(fd, reqBuffer, strlen(reqBuffer));
    int totalBytesRead = 0;
    int totalBytesWritten = 0;
    while ((chunkRead = read(fd, data, sizeof(data)))!= (size_t)NULL)//send file
    {
        chunkWritten = write(socket, data, chunkRead);
        totalBytesRead += chunkRead;
        totalBytesWritten += chunkWritten;
    }
    fprintf (fp_log, "completed sending %s at %d bytes at %s\n------------------------------------------------------------------------------------\n", filename, totalBytesRead, asctime(brokentime));
    printf("completed sending %s at %d bytes at %s\n------------------------------------------------------------------------------------\n", filename, totalBytesRead, asctime(brokentime));
    fclose(fp_log);
    close(fd);
    close(socket);
}




