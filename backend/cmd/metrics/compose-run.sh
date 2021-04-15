#!/bin/sh

# values to replace tokens in the template with 
while getopts u:p:n: option
do
    case "${option}"
    in
    u) DB_USER=${OPTARG};;
    p) DB_PASS=${OPTARG};;
    n) DB_NAME=${OPTARG};;
esac
done

# varibles in template to replace
db_user_token="{{db_user}}"
db_pass_token="{{db_pass}}"
db_name_token="{{db_name}}"

sed -e "s/${db_user_token}/${DB_USER}/g" \
    -e "s/${db_pass_token}/${DB_PASS}/g" \
    -e "s/${db_name_token}/${DB_NAME}/g" \
    < docker-compose.tmpl.yml \
    > docker-compose.yml

docker-compose up -d