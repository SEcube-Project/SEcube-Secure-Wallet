#include "SEwalletServer.h"

/*
 * Costructor
 */
SEwalletServer::SEwalletServer() {
    this->s0 = -1;
    this->s1 = -1;
}

/*
 * Destructor
 */
SEwalletServer::~SEwalletServer() {
}

/*
 * Initialize the server in order to accept connections
 */
int SEwalletServer::init() {

	// Create an endpoint for communication (welcome socket)
	if ((s0 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	    //logger("Socket creation error");
	    return -1;
	}

	// Setup server address and port
	struct sockaddr_in address;
	memset(&address, 0, sizeof(struct sockaddr_in));
	address.sin_family = AF_INET;
	address.sin_port = htons(SERVER_PORT);
	address.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);

	// Bind a name to the welcome socket
	if ((bind(s0, (struct sockaddr*) &address, sizeof(address))) < 0) {
		//std::string s("Error binding: ");
	    //logger(s.append(strerror(errno)));
	    return -1;
    }

	// Linger active, timeout 0 (close the listen socket immediately at program termination)
	struct linger linger_opt = {1, 0};
	setsockopt(s0, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));

	// Listen for connections on the welcome socket; // "1" is the maximal length of the queue
	if ((listen(s0, 1)) < 0) {
		//std::string s("Error listening: ");
		//logger(s.append(strerror(errno)));
		return -1;
	}

	// Create the client socket
	struct sockaddr_in peer_address;
	socklen_t peer_address_len;
	//logger("Waiting for connection");
	if ((s1 = accept(s0, (struct sockaddr*) &peer_address, &peer_address_len)) < 0) {
		//std::string s("Error accepting connection: ");
		//logger(s.append(strerror(errno)));
		return -1;
	}

	// Close the welcome socket
	//close(s0);

	// Return the client socket
	return s1;
}
