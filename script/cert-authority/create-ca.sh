#!/usr/bin/env bash

# Following directions from https://networklessons.com/uncategorized/openssl-certification-authority-ca-ubuntu-server/

set -e

# Clean up the old stuff
rm -rf certificates
mkdir -p certificates/CA
mkdir -p certificates/CA/newcerts
mkdir -p certificates/CA/certs
mkdir -p certificates/CA/crl
mkdir -p certificates/CA/private
mkdir -p certificates/CA/requests

cat > certificates/CA/openssl.cnf <<EOF
# OpenSSL root CA configuration file.
# Copy to '/root/ca/openssl.cnf'.

[ ca ]
# 'man ca'
default_ca = CA_default

[ CA_default ]
# Directory and file locations.
dir               = $PWD/certificates/CA
certs             = \$dir/certs
crl_dir           = \$dir/crl
new_certs_dir     = \$dir/newcerts
database          = \$dir/index.txt
serial            = \$dir/serial
RANDFILE          = \$dir/private/.rand

# The root key and root certificate.
private_key       = \$dir/private/ZippyCA.key.pem
certificate       = \$dir/certs/ZippyCA.cert.pem

# For certificate revocation lists.
crlnumber         = \$dir/crlnumber
crl               = \$dir/crl/ZippyCA.crl.pem
crl_extensions    = crl_ext
default_crl_days  = 30

# SHA-1 is deprecated, so use SHA-2 instead.
default_md        = sha256

name_opt          = ca_default
cert_opt          = ca_default
default_days      = 375
preserve          = no
policy            = policy_strict

[ policy_strict ]
# The root CA should only sign intermediate certificates that match.
# See the POLICY FORMAT section of 'man ca'.
countryName             = match
stateOrProvinceName     = match
organizationName        = match
organizationalUnitName  = optional
commonName              = supplied
emailAddress            = optional

[ policy_loose ]
# Allow the intermediate CA to sign a more diverse range of certificates.
# See the POLICY FORMAT section of the 'ca' man page.
countryName             = optional
stateOrProvinceName     = optional
localityName            = optional
organizationName        = optional
organizationalUnitName  = optional
commonName              = supplied
emailAddress            = optional

[ req ]
# Options for the 'req' tool ('man req').
default_bits        = 2048
distinguished_name  = req_distinguished_name
string_mask         = utf8only

# SHA-1 is deprecated, so use SHA-2 instead.
default_md          = sha256

# Extension to add when the -x509 option is used.
x509_extensions     = v3_ca

[ req_distinguished_name ]
# See <https://en.wikipedia.org/wiki/Certificate_signing_request>.
countryName                     = Country Name (2 letter code)
stateOrProvinceName             = State or Province Name
localityName                    = Locality Name
0.organizationName              = Organization Name
organizationalUnitName          = Organizational Unit Name
commonName                      = Common Name
emailAddress                    = Email Address

# Optionally, specify some defaults.
countryName_default             = GB
stateOrProvinceName_default     = England
localityName_default            =
0.organizationName_default      = Alice Ltd
organizationalUnitName_default  =
emailAddress_default            =

[ v3_ca ]
# Extensions for a typical CA ('man x509v3_config').
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid:always,issuer
basicConstraints = critical, CA:true
keyUsage = critical, digitalSignature, cRLSign, keyCertSign

[ v3_intermediate_ca ]
# Extensions for a typical intermediate CA ('man x509v3_config').
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid:always,issuer
basicConstraints = critical, CA:true, pathlen:0
keyUsage = critical, digitalSignature, cRLSign, keyCertSign

[ usr_cert ]
# Extensions for client certificates ('man x509v3_config').
basicConstraints = CA:FALSE
nsCertType = client, email
nsComment = "OpenSSL Generated Client Certificate"
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid,issuer
keyUsage = critical, nonRepudiation, digitalSignature, keyEncipherment
extendedKeyUsage = clientAuth, emailProtection

[ server_cert ]
# Extensions for server certificates ('man x509v3_config').
basicConstraints = CA:FALSE
nsCertType = server
nsComment = "OpenSSL Generated Server Certificate"
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid,issuer:always
keyUsage = critical, digitalSignature, keyEncipherment
extendedKeyUsage = serverAuth

[ crl_ext ]
# Extension for CRLs ('man x509v3_config').
authorityKeyIdentifier=keyid:always

[ ocsp ]
# Extension for OCSP signing certificates ('man ocsp').
basicConstraints = CA:FALSE
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid,issuer
keyUsage = critical, digitalSignature
extendedKeyUsage = critical, OCSPSigning
EOF

# Build up our CA directory
touch certificates/CA/index.txt
echo 1000 > certificates/CA/serial

# Generate the root CA key
openssl genrsa -out certificates/CA/private/ZippyCA.key.pem 4096
chmod 400 certificates/CA/private/ZippyCA.key.pem

# Generate the root CA certificate
openssl req -new \
            -x509 \
            -key certificates/CA/private/ZippyCA.key.pem \
            -out certificates/CA/certs/ZippyCA.cert.pem \
            -config certificates/CA/openssl.cnf \
            -extensions v3_ca \
            -days 3650 \
            -set_serial 0 \
            -subj "/C=US/ST=California/L=Santa Clara/DC=dev/DC=zippy/DC=ai/O=Zippy.ai/OU=Developer Network/CN=Zippy Developer Network Root CA"
chmod 444 certificates/CA/certs/ZippyCA.cert.pem

# Verify the root CA certificate
openssl x509 -noout -text -in certificates/CA/certs/ZippyCA.cert.pem

# Intermediate CA
mkdir -p certificates/CA/intermediate/newcerts
mkdir -p certificates/CA/intermediate/crl
mkdir -p certificates/CA/intermediate/private
mkdir -p certificates/CA/intermediate/csr
mkdir -p certificates/CA/intermediate/certs

chmod 700 certificates/CA/intermediate/private
touch certificates/CA/intermediate/index.txt
echo 1000 > certificates/CA/intermediate/serial
echo 1000 > certificates/CA/intermediate/crlnumber

cat > certificates/CA/intermediate/openssl.cnf <<EOF
# OpenSSL intermediate CA configuration file.
# Copy to '/root/ca/intermediate/openssl.cnf'.

[ ca ]
# 'man ca'
default_ca = CA_default

[ CA_default ]
# Directory and file locations.
dir               = $PWD/certificates/CA/intermediate
certs             = \$dir/certs
crl_dir           = \$dir/crl
new_certs_dir     = \$dir/newcerts
database          = \$dir/index.txt
serial            = \$dir/serial
RANDFILE          = \$dir/private/.rand

# The root key and root certificate.
private_key       = \$dir/private/ZippyIntermediate.key.pem
certificate       = \$dir/certs/ZippyIntermediate.cert.pem

# For certificate revocation lists.
crlnumber         = \$dir/crlnumber
crl               = \$dir/crl/intermediate.crl.pem
crl_extensions    = crl_ext
default_crl_days  = 30

# SHA-1 is deprecated, so use SHA-2 instead.
default_md        = sha256

name_opt          = ca_default
cert_opt          = ca_default
default_days      = 375
preserve          = no
policy            = policy_loose

[ policy_strict ]
# The root CA should only sign intermediate certificates that match.
# See the POLICY FORMAT section of 'man ca'.
countryName             = match
stateOrProvinceName     = match
organizationName        = match
organizationalUnitName  = optional
commonName              = supplied
emailAddress            = optional

[ policy_loose ]
# Allow the intermediate CA to sign a more diverse range of certificates.
# See the POLICY FORMAT section of the 'ca' man page.
countryName             = optional
stateOrProvinceName     = optional
localityName            = optional
organizationName        = optional
organizationalUnitName  = optional
commonName              = supplied
emailAddress            = optional

[ req ]
# Options for the 'req' tool ('man req').
default_bits        = 2048
distinguished_name  = req_distinguished_name
string_mask         = utf8only

# SHA-1 is deprecated, so use SHA-2 instead.
default_md          = sha256

# Extension to add when the -x509 option is used.
x509_extensions     = v3_ca

[ req_distinguished_name ]
# See <https://en.wikipedia.org/wiki/Certificate_signing_request>.
countryName                     = Country Name (2 letter code)
stateOrProvinceName             = State or Province Name
localityName                    = Locality Name
0.organizationName              = Organization Name
organizationalUnitName          = Organizational Unit Name
commonName                      = Common Name
emailAddress                    = Email Address

# Optionally, specify some defaults.
countryName_default             = GB
stateOrProvinceName_default     = England
localityName_default            =
0.organizationName_default      = Alice Ltd
organizationalUnitName_default  =
emailAddress_default            =

[ v3_ca ]
# Extensions for a typical CA ('man x509v3_config').
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid:always,issuer
basicConstraints = critical, CA:true
keyUsage = critical, digitalSignature, cRLSign, keyCertSign

[ v3_intermediate_ca ]
# Extensions for a typical intermediate CA ('man x509v3_config').
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid:always,issuer
basicConstraints = critical, CA:true, pathlen:0
keyUsage = critical, digitalSignature, cRLSign, keyCertSign

[ usr_cert ]
# Extensions for client certificates ('man x509v3_config').
basicConstraints = CA:FALSE
nsCertType = client, email
nsComment = "OpenSSL Generated Client Certificate"
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid,issuer
keyUsage = critical, nonRepudiation, digitalSignature, keyEncipherment
extendedKeyUsage = clientAuth, emailProtection

[ server_cert ]
# Extensions for server certificates ('man x509v3_config').
basicConstraints = CA:FALSE
nsCertType = server
nsComment = "OpenSSL Generated Server Certificate"
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid,issuer:always
keyUsage = critical, digitalSignature, keyEncipherment
extendedKeyUsage = serverAuth

[ crl_ext ]
# Extension for CRLs ('man x509v3_config').
authorityKeyIdentifier=keyid:always

[ ocsp ]
# Extension for OCSP signing certificates ('man ocsp').
basicConstraints = CA:FALSE
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid,issuer
keyUsage = critical, digitalSignature
extendedKeyUsage = critical, OCSPSigning
EOF

# Generate the intermediate CA
openssl genrsa -out certificates/CA/intermediate/private/ZippyIntermediate.key.pem 4096
openssl req -config certificates/CA/intermediate/openssl.cnf -new -sha256 \
      -key certificates/CA/intermediate/private/ZippyIntermediate.key.pem \
      -out certificates/CA/intermediate/csr/ZippyIntermediate.csr.pem \
      -subj "/C=US/ST=California/L=Santa Clara/DC=dev/DC=zippy/DC=ai/O=Zippy.ai/OU=Developer Network/CN=Zippy Developer Network Intermediate CA"

yes y | \
openssl ca \
        -config certificates/CA/openssl.cnf \
        -extensions v3_intermediate_ca \
        -in certificates/CA/intermediate/csr/ZippyIntermediate.csr.pem \
        -out certificates/CA/intermediate/certs/ZippyIntermediate.cert.pem
chmod 444 certificates/CA/intermediate/certs/ZippyIntermediate.cert.pem
openssl x509 -noout -text -in certificates/CA/intermediate/certs/ZippyIntermediate.cert.pem


# This is *THE* CA certificate that we use for clients and servers to verify everyone's identity
cat certificates/CA/certs/ZippyCA.cert.pem \
    certificates/CA/intermediate/certs/ZippyIntermediate.cert.pem \
    > certificates/CA/intermediate/certs/ca-chain.cert.pem
chmod 444 certificates/CA/intermediate/certs/ca-chain.cert.pem

