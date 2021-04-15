#!/bin/bash

# Check for -b args
while getopts b: option
do
 case "${option}"
 in
 b) BRANCH_NAME=${OPTARG};;
 esac
done

if [ -z $BRANCH_NAME ]; then
  # If no argument is provided, get the current branch name
  BRANCH_NAME=$(git symbolic-ref HEAD 2>/dev/null | cut -d"/" -f 3);
fi

echo "Pulling from branch ${BRANCH_NAME}"
# Pull the docker image
docker pull server1.zippy:5000/teleop-build:${BRANCH_NAME}
# Run and expose port 8000
docker run --entrypoint ./teleop-server -p 8000:8000 server1.zippy:5000/teleop-build:${BRANCH_NAME}
