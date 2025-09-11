#!/bin/bash

# Writer script for assignment 1
#
# Will Morrison

# Verify that arguments are present
writeFile=$1
if [ -z "${writeFile}" ]; then
    echo "Wirte file is missing!"
    exit 1
fi

writeStr=$2
if [ -z "${writeStr}" ]; then
    echo "File content is missing!"
    exit 1
fi

mkdir -p $(dirname ${writeFile})
echo ${writeStr} > ${writeFile}

