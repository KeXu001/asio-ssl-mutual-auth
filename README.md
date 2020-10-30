Simple echo server using Asio and ssl to do client and server authentification.

This is a fork of AdamMagaluk's asio-ssl-mutual-auth, with the following changes:
- standalone Asio instead of Boost.Asio
- fixed certificates so that server and client have separate keys, and certs are signed by a separate CA

# How To

## Building
```
make
```

## Testing
```
./server <port>
./client <host> <port> 
```
