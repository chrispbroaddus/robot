#!/usr/bin/env bash

function generate_key {
    openssl genrsa \
            -out certificates/CA/intermediate/private/$1.key.pem 4096
}

function generate_csr {
    openssl req \
            -config certificates/CA/intermediate/openssl.cnf \
            -key certificates/CA/intermediate/private/$1.key.pem \
            -new \
            -out certificates/CA/intermediate/csr/$1.csr.pem \
            -subj "/C=US/ST=California/L=Santa Clara/DC=dev/DC=zippy/DC=ai/O=Zippy.ai/OU=Developer Network/CN=$1"
}

function sign_server_certificate {
    yes y | \
    openssl ca \
            -config certificates/CA/intermediate/openssl.cnf \
            -extensions server_cert \
            -days 3654 \
            -notext \
            -md sha256 \
            -in certificates/CA/intermediate/csr/$1.csr.pem \
            -out certificates/CA/intermediate/certs/$1.cert.pem
}

function verify_server_certificate {
    openssl verify \
            -CAfile certificates/CA/intermediate/certs/ca-chain.cert.pem \
            certificates/CA/intermediate/certs/$1.cert.pem
}

function create_server_certificate {
    cat <<EOF
Generating server certificate for $1


EOF

    generate_key "$1"
    generate_csr "$1"
    sign_server_certificate "$1"
    openssl x509 -text -noout -in certificates/CA/intermediate/certs/$1.cert.pem
    verify_server_certificate "$1"
}

create_server_certificate ppa.zippy.ai
create_server_certificate docker.zippy.ai
create_server_certificate dns.zippy.ai