FROM ubuntu:16.04

RUN apt-get update
RUN apt-get install -y ca-certificates
RUN apt-get install -y rsyslog 

ADD ./sim-host /sim-host

EXPOSE 8080

ENTRYPOINT service rsyslog start && ./sim-host --config /configs/config.json --secrets /secrets/secrets.json