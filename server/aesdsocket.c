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
#include <signal.h>

#define ERROR_CODE -1
#define DEFAULT_PERMISSIONS 0644

void sigHandler(int signum);
void handleCleanup(int fdsock, int fdcli, FILE *tmpFile, int err);
volatile sig_atomic_t sigRec= 0;

int main(int argc, char const *argv[]) {

    struct addrinfo *sockResult, *hostAddr;
    struct addrinfo hints;
    struct sockaddr sockAddr, cliAddr;
    socklen_t cliLen;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    FILE *fptr = NULL;
    int fdSock, fdCli;

    // setup log
    openlog("aesdserver", LOG_CONS | LOG_NDELAY, LOG_USER);
    syslog(LOG_DEBUG, "Start aesdserver");

    // setup signals
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    // setup server
    fdSock = socket(AF_INET, SOCK_STREAM, 0);
    printf("$$$ fdSock: %d\n",fdSock);
    syslog(LOG_DEBUG, "fdSock: %d\n",fdSock);
    if(fdSock < 0){
        syslog(LOG_ERR, "Failed to create socket: %d", fdSock);
        handleCleanup(fdSock, fdSock, fptr, 1);
    }

    // open stream socket on port 9000
    int result = getaddrinfo(NULL, "9000", &hints, &sockResult);
    printf("$$$ getaddrinfo result: %d\n",result);
    syslog(LOG_DEBUG, "$$$ getaddrinfo result: %d\n",result);
    if (result != 0){
        syslog(LOG_ERR, "Failed getaddrinfo: %d", result);
        freeaddrinfo(sockResult);
        handleCleanup(fdSock, fdCli, fptr,1);
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
        handleCleanup(fdSock, fdCli, fptr, 1); 
    }

        // Listen and accept connection
        result = listen(fdSock,50);
        printf("$$$ listen result: %d\n",result);
        syslog(LOG_DEBUG, "$$$ listen result: %d\n",result);
        if(result == -1){
            syslog(LOG_ERR, "listen failure: %d\n",errno);
            handleCleanup(fdSock, fdCli, fptr, 1); 
        }

        cliLen = sizeof(cliAddr);

    // Keep accepting new clients until SIGINT or SIGTERM
    for(;;) {
        syslog(LOG_DEBUG, "$$$ accepting?: %d\n", fdSock);
        fdCli = accept(fdSock, (struct sockaddr *) &cliAddr, &cliLen);
        printf("$$$ accept fd: %d\n",fdCli);
        syslog(LOG_DEBUG, "$$$ accept fd: %d\n",fdCli);

        if (fdCli < 0) {
            syslog(LOG_ERR, "$$$ accept failure: %d\n",errno);
            handleCleanup(fdSock, fdCli, fptr, 1); 
        }

        // log to syslog
        syslog(LOG_USER, "Accepted connection from %s", inet_ntop(AF_INET, cliAddr.sa_data, str, sizeof(str)));

        char receiveBuf[500];
        result = recv(fdCli, receiveBuf, 500, 0); 
        printf("$$$ receive len: %d\n", result);

        if (result == -1) {
            syslog(LOG_ERR, "$$$ recv failure: %d\n",errno);
            handleCleanup(fdSock, fdCli, fptr, 1); 
        }

        receiveBuf[result] = '\0';

        syslog(LOG_USER, "Received: %s", receiveBuf);

        // write recieved data to /var/tmp/aesdsocketdata
        fptr = fopen("/var/tmp/aesdsocketdata", "a");
        if (fptr == NULL){
            handleCleanup(fdSock, fdCli, fptr, 1); 
        }

        fprintf(fptr, "%s", receiveBuf);
        fclose(fptr);

        // send full content of /var/tmp/aesdsocketdata to client
        fptr = fopen("/var/tmp/aesdsocketdata", "r");
        if (fptr == NULL){
            handleCleanup(fdSock, fdCli, fptr, 1); 
        }

        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fptr) != NULL) {
            syslog(LOG_USER,("%s", buffer));
            if(send(fdCli, buffer, strlen(buffer), 0) == -1){
                syslog(LOG_ERR, "send failure: %d\n",errno);
                handleCleanup(fdSock, fdCli, fptr, 1); 
            } 
        }

        fclose(fptr);
        close(fdCli);
//        if (close(fdSock) == -1){
 //           exit(ERROR_CODE);
  //      }

        syslog(LOG_USER, "Closed connection from %s", inet_ntop(AF_INET, cliAddr.sa_data, str, sizeof(str)));
    } 

    return 1;
}

void sigHandler(int signum){
    syslog(LOG_USER, "Caught signal, exiting");
    sigRec = 1;
}

void handleCleanup(int fdsock, int fdcli, FILE *tmpFile, int err){
    if (fdsock > 0) {
        close(fdsock);
    }

    if (fdcli > 0) {
        close(fdcli);
    }

    if ( tmpFile != NULL) {
        fclose(tmpFile);
    }

    remove("/var/tmp/aesdsocket");

    if(err) {
        exit(ERROR_CODE);
    }
}

