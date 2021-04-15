#!/bin/bash

ssh ec2-user@54.201.181.154 'docker ps -qf "name=teleop-server" | xargs docker logs -f'
