#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

const int BUFFER_SIZE = 1024;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <server_ip> <server_port>" << endl;
        exit(EXIT_FAILURE);
    }

    char* server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        cerr << "Error creating socket." << endl;
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(server_ip);
    server_address.sin_port = htons(server_port);

    if (connect(client_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        cerr << "Error connecting to server." << endl;
        exit(EXIT_FAILURE);
    }

    cout << "Connected to server." << endl;

    while (true) {
        cout << "Enter a message: ";
        string message;
        getline(cin, message);

        if (message.empty()) {
            continue;
        }

        if (send(client_socket, message.c_str(), message.length(), 0) < 0) {
            cerr << "Error sending message." << endl;
            continue;
        }

        char buffer[BUFFER_SIZE];
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
	
        if (bytes_received < 0) {
            cerr << "Error receiving response." << endl;
            continue;
        } else if (bytes_received == 0) {
            cout << "Server disconnected." << endl;
            break;
        } else {
            buffer[bytes_received] = '\0';
            cout << "Response: " << buffer;
        }
    }

    close(client_socket);
    return 0;
}
