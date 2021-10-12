/**
  ******************************************************************************
  * File Name          : SEwalletClient.h
  * Description        :
  ******************************************************************************
  *
  * Copyright © 2021-present Blu5 Group <https://www.blu5group.com>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Lesser General Public
  * License as published by the Free Software Foundation; either
  * version 3 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  * Lesser General Public License for more details.
  *
  * You should have received a copy of the GNU Lesser General Public
  * License along with this library; if not, see <https://www.gnu.org/licenses/>.
  *
  ******************************************************************************
  */

#ifndef SEWALLETCLIENT_H
#define SEWALLETCLIENT_H

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fstream>
#include <memory>
#include <ctime>
#include <iostream>

// Cereal library
#include "archives/binary.hpp"
#include "types/array.hpp"
#include "types/vector.hpp"
#include "types/polymorphic.hpp"
#include "cereal.hpp"

// SEwalletInterafce
#include "../../SEwalletBackend/src/SEwalletInterface.h"

// Dimension of input/output buffer for communicating with the SEcube SDK Server (1 MB)
#define BUFLEN 1024*1024

// Offset and size of fields in requests and responses */
#define T_OFFSET    0
#define T_SIZE      1
#define L_OFFSET    1
#define L_SIZE      4
#define V_OFFSET    5

// Server address
#define SERVER_ADDRESS "127.0.0.1"

// Server port
#define SERVER_PORT 2000

// Number of attempts to connect to the backend
#define ATTEMPTS 10

// Logger
#define LOGGER_

// Log file
extern std::ofstream logfile;

// List of commands that can be sent to the backend
typedef enum class SEwalletCommand {
    LOGIN = 1,
    INITIALIZE_DEVICE,
    GET_DEVICE_LIST,
    GET_FILE_LIST,
    OPEN_WALLET,
    SAVE_WALLET,
    DELETE_WALLET,
    LOGOUT
} SEwalletCommand;

// Object managing the interaction with the SEcube SDK Server
class SEwalletClient {

private:

    int sock, valread;
    struct sockaddr_in serv_addr;
    uint8_t buffer[BUFLEN];

    int login(void *params);
    int initializeDevice(void *params);
    int getDeviceList(void *params);
    int getFileList(void *params);
    int openWallet(void *params);
    int saveWallet(void *params);
    int deleteWallet(void *params);
    int logout(void *params);

public:

    explicit SEwalletClient();
    int init();
    int connectToBackend();
    int txrx(SEwalletCommand command, void* params);
    int disconnectFromBackend();
    void closeSocket();

};

void send_text(int s1, const std::string& msg);

void send_cereal(int s1, uint8_t* reply, uint8_t T, std::stringstream& ss, std::string& altmsg);

void logger(const std::string& msg);

#endif // SEWALLETCLIENT_H
