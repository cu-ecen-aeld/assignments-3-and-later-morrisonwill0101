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
#include <sys/stat.h>


#define ERROR_CODE -1
void sigHandler(int signum);
void handleCleanup(int fdsock, int fdcli, FILE *tmpFile, int err);

int main(int argc, char const *argv[]) {

    struct addrinfo *sockResult, *hostAddr;
    struct addrinfo hints;
    struct sockaddr sockAddr, cliAddr;
    socklen_t cliLen;
    FILE *fptr = NULL;
    int fdSock, fdCli;
    char str[INET6_ADDRSTRLEN];

    cliLen = sizeof(cliAddr);
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // setup log
    openlog("aesdserver", LOG_CONS | LOG_NDELAY, LOG_USER);
    syslog(LOG_DEBUG, "Start aesdserver");
    if( argc > 1) {
        syslog(LOG_DEBUG, "arg 1: %s", argv[1]);
    }

    // setup signals
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);

    // open stream socket on port 9000
    int result = getaddrinfo(NULL, "9000", &hints, &sockResult);
    syslog(LOG_DEBUG, "getaddrinfo result: %d\n",result);
    if (result != 0){
        syslog(LOG_ERR, "Failed getaddrinfo: %d", result);
        freeaddrinfo(sockResult);
        handleCleanup(fdSock, fdCli, fptr,1);
    }


    // fork if -d argument
    if (argc > 1) {
        if (strcmp(argv[1], "-d") == 0 ){
            syslog(LOG_USER, "forking: %s", argv[1]);
            pid_t pid = fork();

            if (pid < 0){
                handleCleanup(fdSock, fdCli, fptr,1);
                exit(ERROR_CODE);
            }
            if (pid > 0) {
                exit(0);
            }
        }
    }
   
    // setup server
    fdSock = socket(AF_INET, SOCK_STREAM, 0);
    syslog(LOG_DEBUG, "fdSock: %d\n",fdSock);
    if(fdSock < 0){
        syslog(LOG_ERR, "Failed to create socket: %d", fdSock);
        handleCleanup(fdSock, fdSock, fptr, 1);
    }

    if (setsockopt(fdSock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0){

    }

    // bind 
    hostAddr = sockResult;

    result = bind(fdSock, hostAddr->ai_addr, hostAddr->ai_addrlen);
    printf("%s\n", inet_ntop(AF_INET, sockResult->ai_addr, str, sizeof(str)));

    freeaddrinfo(sockResult);
    if(result < 0){
        printf("Bind Failure: %d\n",errno);
        syslog(LOG_ERR, "Failed bind: %d", result);
        handleCleanup(fdSock, fdCli, fptr, 1); 
    }

    // Listen and accept connection
    result = listen(fdSock,50);
    if(result == -1){
        syslog(LOG_ERR, "listen failure: %d\n",errno);
        handleCleanup(fdSock, fdCli, fptr, 1); 
    }

    // Keep accepting new clients until SIGINT or SIGTERM
    for(;;) {
        fdCli = accept(fdSock, (struct sockaddr *) &cliAddr, &cliLen);

        if (fdCli < 0) {
            syslog(LOG_ERR, "Accept failure: %d\n",errno);
            handleCleanup(fdSock, fdCli, fptr, 1); 
        }

        // log to syslog
        syslog(LOG_USER, "Accepted connection from %s", inet_ntop(AF_INET, cliAddr.sa_data, str, sizeof(str)));

        // Open File for Writing
        fptr = fopen("/var/tmp/aesdsocketdata", "a");
        if (fptr == NULL){
            handleCleanup(fdSock, fdCli, fptr, 1); 
        }

        char receiveBuf[1023];

        do {
            result = recv(fdCli, receiveBuf, 1023, 0); 
            syslog(LOG_USER, "Recv Len: %d\n",result);


            if (result == -1) {
                syslog(LOG_ERR, "$$$ recv failure: %d\n",errno);
                handleCleanup(fdSock, fdCli, fptr, 1); 
            }


            size_t written = fwrite(receiveBuf, sizeof(char), result, fptr);
            if( written != result) {
                handleCleanup(fdSock, fdCli, fptr, 1); 
            }
        } while( result == 1023);

        fclose(fptr);

        // send full content of /var/tmp/aesdsocketdata to client
        fptr = fopen("/var/tmp/aesdsocketdata", "r");
        if (fptr == NULL){
            handleCleanup(fdSock, fdCli, fptr, 1); 
        }

        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fptr) != NULL) {
//            syslog(LOG_USER,"sending: %s", buffer);
            if(send(fdCli, buffer, strlen(buffer), 0) == -1){
                syslog(LOG_ERR, "send failure: %d\n",errno);
                handleCleanup(fdSock, fdCli, fptr, 1); 
            } 
        }

        fclose(fptr);
        close(fdCli);

        syslog(LOG_USER, "Closed connection from %s", inet_ntop(AF_INET, cliAddr.sa_data, str, sizeof(str)));
    } 

    return 1;
}

void sigHandler(int signum){
    syslog(LOG_USER, "Caught signal, exiting");
    printf("%c", '\n');
    remove("/var/tmp/aesdsocketdata");
    _exit(0);
}

void handleCleanup(int fdsock, int fdcli, FILE *tmpFile, int err){
    if (fdsock > 0) {
        printf("Closing socket: %d", fdsock);
        close(fdsock);
    }

    if (fdcli > 0) {
        printf("Closing socket: %d", fdcli);
        close(fdcli);
    }

    if ( tmpFile != NULL) {
        fclose(tmpFile);
    }

    remove("/var/tmp/aesdsocketdata");

    if(err) {
        exit(ERROR_CODE);
    }
}

