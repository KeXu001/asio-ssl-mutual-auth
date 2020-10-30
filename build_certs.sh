#!/bin/bash

# create certificate authority private key
openssl genrsa -out ca_certs/ca.key 2048

# create server and client private keys
openssl genrsa -out server_certs/server.key 2048
openssl genrsa -out client_certs/client.key 2048

# create certificate authority certificate
openssl req -x509 -new -key ca_certs/ca.key -days 365 -out ca_certs/ca.crt -subj /CN=CA

# create signing requests
openssl req -new -key server_certs/server.key -out tmp/server.csr -subj /CN=Company
openssl req -new -key client_certs/client.key -out tmp/client.csr -subj /CN=Company

# create server and client certificates
openssl x509 -req -in tmp/server.csr -CA ca_certs/ca.crt -CAkey ca_certs/ca.key -CAcreateserial -days 365 -out server_certs/server.crt
openssl x509 -req -in tmp/client.csr -CA ca_certs/ca.crt -CAkey ca_certs/ca.key -days 365 -out client_certs/client.crt

# generate diffie-hellman parameters
openssl dhparam -out dh/dhparam.pem 2048

# below can be used to verify certificates
openssl verify -CAfile ca_certs/ca.crt server_certs/server.crt
openssl verify -CAfile ca_certs/ca.crt client_certs/client.crt
openssl verify -CAfile ca_certs/ca.crt ca_certs/ca.crt