# Server-Client-Simple-HTTP-like-
A minimal multithreaded server + interactive client in C using a custom protocol (CHLP/1.0).

Features

GET → serve files from ./www/
POST → save body to file (overwrites)
ECHO → echo back the body
Persistent connections (many requests per client)
Multithreaded (handles many clients at once)
Case-insensitive input in client


Files

http.h → headers & structs
http.c → core logic (parsing, processing, sockets)
HTTP_Server.c → server main
HTTP_Client.c → interactive client
./www/ → folder for files (create it!)


Build

# Server
gcc HTTP_Server.c http.c -o server

# Client
gcc HTTP_Client.c http.c -o client



Run
./server # starts on port 8080
./client # in another terminal

Quick Test

1. Create a file:
mkdir www
echo "Hello!" > www/hello.txt

2. In client:
GET → /hello.txt → returns "Hello!"
POST → /hello.txt → type "Barev!" → file updated
GET again → now returns "Barev!"
ECHO → type anything → gets it back
