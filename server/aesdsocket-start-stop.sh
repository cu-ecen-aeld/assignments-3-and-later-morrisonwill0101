#!/bin/sh

case "$1" in 
    start)
        echo "Starting aesdsocket"
        start-stop-daemon -S -n aesdsocket -a /usr/bin/aesdsocket -- -d
    ;;
    stop)
        echo "Stopping aesdsocket"
        start-stop-daemon -K --stop -n aesdsocket
    ;;
    *)
    echo "Unknown argument"
    exit 1
    ;;
esac

exit 0
