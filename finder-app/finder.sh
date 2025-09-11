#!/bin/bash

# aesd assignment 1 writer.
#
# Will Morrison

filesDir=$1
if [ -z "${filesDir}" ]; then
    echo "File directory is missing!"
    exit 1
elif [ ! -d "${filesDir}" ]; then
    echo "Specified file directory does not exist!"
    exit 1
fi
    
searchStr=$2
if [ -z "${searchStr}" ]; then
    echo "Search String is missing!"
    exit 1
fi

fileCount=$(find "${filesDir}" -type f | wc -l)
lineCount=$(grep -r "${searchStr}" ${filesDir} | wc -l)
echo "The number of files are ${fileCount} and the number of matching lines are ${lineCount}"
