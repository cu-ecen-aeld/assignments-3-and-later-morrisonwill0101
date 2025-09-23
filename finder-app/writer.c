/*
    file: writer.c
    Author: Will Morrison
 
    Description: write a file to specified directory
*/

#include <stdio.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define ERROR_CODE 1
#define DEFAULT_PERMISSIONS 0644

int main(int argc, char *arg[]) {

    // Setup Log
    openlog("writer", LOG_CONS | LOG_NDELAY , LOG_USER);

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

    syslog(LOG_DEBUG, "Writing %s to %s", arg[1], arg[2]);

    ssize_t nr;

    nr = write(fd, arg[2], strlen(arg[2]));
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
