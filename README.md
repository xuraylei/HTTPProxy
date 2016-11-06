A Simple HTTP Proxy Program

How to build
>make

How to use
>./server <binding IP address> <binding port>
>./cliet <server IP address> <server port> <request URL>

Use Case
Step1: run proxy server
>./server 127.0.0.1 10000

Step2:run client
>./client 127.0.0.1 10000 stackoverflow.com/