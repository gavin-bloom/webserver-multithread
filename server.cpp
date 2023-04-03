#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <list>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

const int BUFFER_SIZE = 1024;

list<string> server_messages; //shared message list
shared_mutex rw_mutex; //shared read/write lock

void handle_client(int client_socket) {
    cout << "Client connected. Thread ID: " << this_thread::get_id() << endl;
	
	// Acquire the read lock to allow concurrent read access
	shared_lock<shared_mutex> read_lock(rw_mutex);
    while (true) {
        

        char buffer[BUFFER_SIZE];
        int bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read < 0) {
            cerr << "Error reading from client socket." << endl;
            break;
        } else if (bytes_read == 0) {
            cout << "Client disconnected." << endl;
            break;
        } else {
            buffer[bytes_read] = '\0';
            cout << "Received message: " << buffer << endl;
	    string cur = buffer;

	    //send list of messages back
	    if (cur == "get") {
		string response = "";
		for (const auto& msg : server_messages) {
	    	    response += msg + "\n";
                }
		send(client_socket, response.c_str(), response.length(), 0);
	    } else {
	        // Release the read lock before acquiring the write lock
	        read_lock.unlock();

	        // Acquire the write lock to modify the shared list of messages
	        unique_lock<shared_mutex> write_lock(rw_mutex);

	        server_messages.push_back(buffer);
	        cout << "Added message to list." << endl;

		//simulate work
	        sleep(1);
	        // Release the write lock
	        write_lock.unlock();


	        // Acquire the read lock again to allow concurrent read access
	        read_lock.lock();
	        string response = "Message received.\n";
	        send(client_socket, response.c_str(), response.length(), 0);
	    }
        }
    }

    // Release the read lock before closing the client socket
    read_lock.unlock();
    close(client_socket);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <port>" << endl;
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
	
    // Create the server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        cerr << "Error creating socket." << endl;
        exit(EXIT_FAILURE);
    }
    
    // Bind server socket to server address
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        cerr << "Error binding socket." << endl;
        exit(EXIT_FAILURE);
    }

    // Start listnening for connections
    if (listen(server_socket, SOMAXCONN) < 0) {
        cerr << "Error listening on socket." << endl;
        exit(EXIT_FAILURE);
    }

    cout << "Server listening on port " << port << endl;

    // Accept client connections and handle them in separate threads
    while (true) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            cerr << "Error accepting client connection." << endl;
            continue;
        }

        thread t(handle_client, client_socket);
        t.detach();
    }

    close(server_socket);
    return 0;
}
