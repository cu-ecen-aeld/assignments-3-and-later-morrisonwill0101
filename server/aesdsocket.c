// assignment 5 server
// Will Morrison
// 10/8/25


#include <syslog.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>

#define ERROR_CODE -1
#define DEFAULT_PERMISSIONS 0644

int main(int argc, char const *argv[]) {

    struct addrinfo *sockResult, *hostAddr;
    struct addrinfo hints;
    struct sockaddr sockAddr, cliAddr;
    socklen_t cliLen;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // setup log
    openlog("aesdserver", LOG_CONS | LOG_NDELAY, LOG_USER);
    syslog(LOG_DEBUG, "Start aesdserver");

    // setup server
    int fdSock = socket(AF_INET, SOCK_STREAM, 0);
    printf("$$$ fdSock: %d\n",fdSock);
    syslog(LOG_DEBUG, "fdSock: %d\n",fdSock);
    if(fdSock < 0){
        syslog(LOG_ERR, "Failed to create socket: %d", fdSock);
        exit(ERROR_CODE);
    }

    // open stream socket on port 9000
    int result = getaddrinfo(NULL, "9000", &hints, &sockResult);
    printf("$$$ getaddrinfo result: %d\n",result);
    syslog(LOG_DEBUG, "$$$ getaddrinfo result: %d\n",result);
    if (result != 0){
        syslog(LOG_ERR, "Failed getaddrinfo: %d", result);
        close(fdSock);
        freeaddrinfo(sockResult);
        exit(ERROR_CODE);
    }


    // fork if -d argument

    // bind 

    hostAddr = sockResult;
    result = bind(fdSock, hostAddr->ai_addr, hostAddr->ai_addrlen);
    printf("$$$ bind result: %d\n",result);
    syslog(LOG_DEBUG, "$$$ bind result: %d\n",result);
    char str[INET6_ADDRSTRLEN];
    printf("%s\n", inet_ntop(AF_INET, sockResult->ai_addr, str, sizeof(str)));
    freeaddrinfo(sockResult);
    if(result < 0){
        printf("Bind Failure: %d\n",errno);
        syslog(LOG_ERR, "Failed bind: %d", result);
        close(fdSock);
        exit(ERROR_CODE);
    }

    // Listen and accept connection
    result = listen(fdSock,50);
    printf("$$$ listen result: %d\n",result);
    syslog(LOG_DEBUG, "$$$ listen result: %d\n",result);
    if(result == -1){
        syslog(LOG_ERR, "listen failure: %d\n",errno);
        close(fdSock);
        exit(ERROR_CODE);
    }

    cliLen = sizeof(cliAddr);
    syslog(LOG_DEBUG, "$$$ accepting?: %d\n", fdSock);
    int fdCli = accept(fdSock, (struct sockaddr *) &cliAddr, &cliLen);
    printf("$$$ accept fd: %d\n",fdCli);
    syslog(LOG_DEBUG, "$$$ accept fd: %d\n",fdCli);

    if (fdCli < 0) {
        syslog(LOG_ERR, "$$$ accept failure: %d\n",errno);
        close(fdSock);
        exit(ERROR_CODE);
    }

    // log to syslog
    syslog(LOG_USER, "Accepted connection from %s", inet_ntop(AF_INET, cliAddr.sa_data, str, sizeof(str)));

    char receiveBuf[500];
    result = recv(fdCli, receiveBuf, 500, 0); 
    printf("$$$ receive len: %d\n", result);

    if (result == -1) {
        syslog(LOG_ERR, "$$$ recv failure: %d\n",errno);
        exit(ERROR_CODE);
    }

    receiveBuf[result] = '\0';

    syslog(LOG_USER, "Received: %s", receiveBuf);

    // write recieved data to /var/tmp/aesdsocketdata
    FILE *fptr;
    fptr = fopen("/var/tmp/aesdsocketdata", "a");
    if (fptr == NULL){
        close(fdSock);
        exit(ERROR_CODE);
    }

    fprintf(fptr, "%s", receiveBuf);
    fclose(fptr);

    // send full content of /var/tmp/aesdsocketdata to client
    fptr = fopen("/var/tmp/aesdsocketdata", "r");
    if (fptr == NULL){
        exit(ERROR_CODE);
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fptr) != NULL) {
        syslog(LOG_USER,("%s", buffer));
        if(send(fdCli, buffer, strlen(buffer), 0) == -1){
            syslog(LOG_ERR, "send failure: %d\n",errno);
            fclose(fptr);
            close(fdSock);
            exit(ERROR_CODE);
        } 
    }

    fclose(fptr);
    if (close(fdSock) == -1){
        exit(ERROR_CODE);
    }

    syslog(LOG_USER, "Closed connection from %s", inet_ntop(AF_INET, cliAddr.sa_data, str, sizeof(str)));
    
    return 1;
}
