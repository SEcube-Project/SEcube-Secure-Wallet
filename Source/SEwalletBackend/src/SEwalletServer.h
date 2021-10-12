#ifndef SEWALLETSERVER_H_
#define SEWALLETSERVER_H_

#include <fstream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ctime>
#include <string.h>

// Server address
#define SERVER_ADDRESS "127.0.0.1"

// Server port
#define SERVER_PORT 2000

class SEwalletServer {

private:

	int s0; // welcome socket
    int s1; // client socket

public:

    explicit SEwalletServer();
    ~SEwalletServer();
    int init();

};

#endif /* SEWALLETSERVER_H_ */
