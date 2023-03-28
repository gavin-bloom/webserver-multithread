# webserver-multithread
Multithreaded webserver and a simple client to connect to it. Clients can add to a shared list of "messages" and read from the list with thread safety.

# usage
Server takes in the port to use as a command line argument, client takes in an ip address and a port as well. 
Sending "get" to the server will respond with the contents of the shared list of messages.
