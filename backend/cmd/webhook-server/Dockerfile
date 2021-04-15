FROM ubuntu:16.04

RUN apt-get update
RUN apt-get install -y ca-certificates

ADD ./webhooks /webhooks
ADD ./conf/web_hooks/web_hook_server_config.json /config.json

EXPOSE 8181

ENTRYPOINT ./webhooks --config /config.json