/*
    file: writer.c
    Author: Will Morrison
 
    Description: write a file to specified directory
*/

#include <stdio.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>

#define ERROR_CODE 1
#define DEFAULT_PERMISSIONS 0644

int main(int argc, char *arg[]) {

    //setvbuf(stdout, NULL, _IOLBF, 0);
    printf("Hello World\n");

    // Setup Log
    openlog("writer", LOG_CONS, LOG_USER);

    printf("Opened log\n");
    printf("argc: %d\n", argc);
    printf("arg1: %s arg2: %s\n", arg[0], arg[1]);


    // Verify args
    if(argc != 3) {
        syslog(LOG_ERR, "Incorrect Number of arguments!");
        return 1; 
    }


    // write writefile with content writestr
    int fd;
    fd = creat(arg[1], DEFAULT_PERMISSIONS);
    if (fd == -1) {
        syslog(LOG_ERR, "Failed to create file!");
        return 1;
    }

    ssize_t nr;
    nr = write(fd, arg[2], sizeof(arg[2]));
    if (nr == -1) {
        //error
        syslog(LOG_ERR, "Failed to write file!");
        return 1;
    }

    // Close file when done writing
    if (close(fd) == -1){
        //syslog(LOG_ERR, "Failed to write file!");
    }

    return 0;
}
