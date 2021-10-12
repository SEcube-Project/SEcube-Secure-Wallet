/**
  ******************************************************************************
  * File Name          : SEwalletClient.cpp
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

#include "SEwalletClient.h"

/** Template to read serialized data by using Cereal library */
template <class T1>
static void read_serial(uint8_t* request, T1& inputparams) {
    std::stringstream ss;
    uint32_t vsize = 0;
    memcpy(&vsize, request+L_OFFSET, L_SIZE);
    ss.write((char*)request+V_OFFSET, vsize);
    cereal::BinaryInputArchive iarchive(ss);
    iarchive(inputparams);
}

/** Template to serialize data with Cereal library and send them to the backend */
template <class T2>
void write_serial(int s1, uint8_t T, uint8_t* reply, T2& inputparams) {
    std::stringstream ss;
    {
        cereal::BinaryOutputArchive oarchive(ss);
        oarchive(inputparams);
    }
    send_cereal(s1, reply, T, ss, inputparams.errmessage);
}

/** Constructor
 *
 * @brief SEwalletClient::SEwalletClient
 */
SEwalletClient::SEwalletClient() {
    this->sock = -1;
    this->valread = 0;
    memset((void*)(this->buffer), 0, BUFLEN);
}

/** Initialize the socket
 *
 * @brief SEwalletClient::init
 * @return
 */
int SEwalletClient::init() {

    // Create an endpoint for communication (socket)
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        logger("Socket creation error");
        return -1;
    }

    // Setup connection parameters and convert IPv4 and IPv6 addresses from text to binary form
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_ADDRESS, &serv_addr.sin_addr) <= 0) {
        logger("Invalid address or address not supported");
        return -1;
    }

    return 0;
}

/** Connect to the backend
 *
 * @brief SEwalletClient::connectToBackend
 * @return
 */
int SEwalletClient::connectToBackend() {

    // Connect to the backend with several attempts
    for (int i = 1; i <= ATTEMPTS; i++) {
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            logger("Failed to connect to backend");
            usleep(200 * 1000);
        } else {
            logger("Connected to backend");
            return 0;
        }
    }

    // Initialization failed after several attempts
    return -1;
}

/** Send data to and receive data from the backend
 *
 * @brief SEwalletClient::txrx
 * @param command
 * @param params
 * @return
 */
int SEwalletClient::txrx(SEwalletCommand command, void *params) {

    switch (command) {

    case SEwalletCommand::LOGIN:
        return login(params);

    case SEwalletCommand::INITIALIZE_DEVICE:
        return initializeDevice(params);

    case SEwalletCommand::GET_DEVICE_LIST:
        return getDeviceList(params);

    case SEwalletCommand::GET_FILE_LIST:
        return getFileList(params);

    case SEwalletCommand::OPEN_WALLET:
        return openWallet(params);

    case SEwalletCommand::SAVE_WALLET:
        return saveWallet(params);

    case SEwalletCommand::DELETE_WALLET:
        return deleteWallet(params);

    case SEwalletCommand::LOGOUT:
        return logout(params);

    default:
        return -1;

    }
}

/** Login the user to SEcube device
 *
 * @brief SEwalletClient::login
 * @param params
 * @return
 */
int SEwalletClient::login(void *params) {
      SEwallet_User_Login_Params *inputparams = (SEwallet_User_Login_Params*)params;
      write_serial<SEwallet_User_Login_Params>(sock, 1, buffer, *inputparams);
      memset(buffer, 0, BUFLEN);
      int valread = read(sock, buffer, BUFLEN);
      if (valread <= 5) {
          return -1;
      } else {
          if (buffer[T_OFFSET] == 1) {
              read_serial<SEwallet_User_Login_Params>(buffer, *inputparams);
          }
      }
      return 0;
}

/** Initilaize a SEcube device
 *
 * @brief SEwalletClient::initializeDevice
 * @param params
 * @return
 */
int SEwalletClient::initializeDevice(void *params) {
    SEwallet_Device_Initialization_Params *inputparams = (SEwallet_Device_Initialization_Params*)params;
    write_serial<SEwallet_Device_Initialization_Params>(sock, 2, buffer, *inputparams);
    memset(buffer, 0, BUFLEN);
    int valread = read(sock, buffer, BUFLEN);
    if (valread <= 5) {
        return -1;
    } else {
        if (buffer[T_OFFSET] == 2) {
            read_serial<SEwallet_Device_Initialization_Params>(buffer, *inputparams);
        }
    }
    return 0;
}

/** Get the list of available devices connected to host
 *
 * @brief SEwalletClient::getDeviceList
 * @param params
 * @return
 */
int SEwalletClient::getDeviceList(void *params) {
    SEwallet_Device_GetList_Params *inputparams = (SEwallet_Device_GetList_Params*)params;
    write_serial<SEwallet_Device_GetList_Params>(sock, 3, buffer, *inputparams);
    memset(buffer, 0, BUFLEN);
    int valread = read(sock, buffer, BUFLEN);
    if (valread <= 5) {
        return -1;
    } else {
        if (buffer[T_OFFSET] == 3) {
            read_serial<SEwallet_Device_GetList_Params>(buffer, *inputparams);
        }
    }
    return 0;
}

/** Get the list of encrypted DBs
 *
 * @brief SEwalletClient::getFileList
 * @param params
 * @return
 */
int SEwalletClient::getFileList(void *params) {
    SEwallet_File_GetList_Params *inputparams = (SEwallet_File_GetList_Params*)params;
    write_serial<SEwallet_File_GetList_Params>(sock, 4, buffer, *inputparams);
    memset(buffer, 0, BUFLEN);
    int valread = read(sock, buffer, BUFLEN);
    if (valread <= 5) {
        return -1;
    } else {
        if (buffer[T_OFFSET] == 4) {
            read_serial<SEwallet_File_GetList_Params>(buffer, *inputparams);
        }
    }
    return 0;
}

/** Open a wallet
 *
 * @brief SEwalletClient::openWallet
 * @param params
 * @return
 */
int SEwalletClient::openWallet(void *params) {
    SEwallet_Wallet_Open_Params *inputparams = (SEwallet_Wallet_Open_Params*)params;
    write_serial<SEwallet_Wallet_Open_Params>(sock, 5, buffer, *inputparams);
    memset(buffer, 0, BUFLEN);
    int valread = read(sock, buffer, BUFLEN);
    if (valread <= 5) {
        return -1;
    } else {
        if (buffer[T_OFFSET] == 5) {
            read_serial<SEwallet_Wallet_Open_Params>(buffer, *inputparams);
        }
    }
    return 0;
}

/** Save the wallet
 *
 * @brief SEwalletClient::saveWallet
 * @param params
 * @return
 */
int SEwalletClient::saveWallet(void *params) {
    SEwallet_Wallet_Save_Params *inputparams = (SEwallet_Wallet_Save_Params*)params;
    write_serial<SEwallet_Wallet_Save_Params>(sock, 6, buffer, *inputparams);
    memset(buffer, 0, BUFLEN);
    int valread = read(sock, buffer, BUFLEN);
    if (valread <= 5) {
        return -1;
    } else {
        if (buffer[T_OFFSET] == 6) {
            read_serial<SEwallet_Wallet_Save_Params>(buffer, *inputparams);
        }
    }
    return 0;
}

/** Delete the wallet
 *
 * @brief SEwalletClient::deleteWallet
 * @param params
 * @return
 */
int SEwalletClient::deleteWallet(void *params) {
    SEwallet_Wallet_Delete_Params *inputparams = (SEwallet_Wallet_Delete_Params*)params;
    write_serial<SEwallet_Wallet_Delete_Params>(sock, 7, buffer, *inputparams);
    memset(buffer, 0, BUFLEN);
    int valread = read(sock, buffer, BUFLEN);
    if (valread <= 5) {
        return -1;
    } else {
        if (buffer[T_OFFSET] == 7) {
            read_serial<SEwallet_Wallet_Delete_Params>(buffer, *inputparams);
        }
    }
    return 0;
}

/** Send logout request to the backend
 *
 * @brief SEwalletClient::logout
 */
int SEwalletClient::logout(void *params) {
    SEwallet_User_Logout_Params *inputparams = (SEwallet_User_Logout_Params*)params;
    write_serial<SEwallet_User_Logout_Params>(sock, 8, buffer, *inputparams);
    memset(buffer, 0, BUFLEN);
    int valread = read(sock, buffer, BUFLEN);
    if (valread <= 5) {
        return -1;
    } else {
        if (buffer[T_OFFSET] == 8) {
            read_serial<SEwallet_User_Logout_Params>(buffer, *inputparams);
        }
    }
    return 0;
}

/** Send disconnect request to the backend
 *
 * @brief SEwalletClient::disconnectFromBackend
 * @return
 */
int SEwalletClient::disconnectFromBackend() {
    send_text(sock, "BYE");
    return close(sock);
}

/** Close the socket
 *
 * @brief SEwalletClient::closeSocket
 */
void SEwalletClient::closeSocket() {
    close(sock);
}

/** Send a text message to the backend
 *
 * @brief send_text
 * @param s1
 * @param msg
 */
void send_text(int s1, const std::string& msg) {
    uint32_t msgsize = msg.size();
    if (msgsize + T_SIZE + L_SIZE <= BUFLEN) {
        std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(msgsize);
        buffer[T_OFFSET] = 0;
        memcpy(buffer.get()+L_OFFSET, &msgsize, L_SIZE);
        memcpy(buffer.get()+V_OFFSET, msg.data(), msgsize);
        send(s1, buffer.get(), T_SIZE + L_SIZE + msgsize, 0);
    }
}

/** Send serialized data by using Cereal library to the backend
 *
 * @brief send_cereal
 * @param s1
 * @param reply
 * @param T
 * @param ss
 * @param altmsg
 */
void send_cereal(int s1, uint8_t* reply, uint8_t T, std::stringstream& ss, std::string& altmsg) {
    std::string s = ss.str();
    uint32_t replysize = s.size();
    if (replysize <= BUFLEN) {
        reply[T_OFFSET] = T;
        memcpy(reply+L_OFFSET, &replysize, L_SIZE);
        memcpy(reply+V_OFFSET, s.c_str(), replysize);
        send(s1, reply, T_SIZE + L_SIZE + replysize, 0);
    } else {
        send_text(s1, altmsg);
    }
}

/** Log messages
 *
 * @brief logger
 * @param msg
 */
void logger(const std::string& msg) {
#ifdef LOGGER_
    if (!msg.empty()) {
        std::time_t result = std::time(nullptr);
        std::string tmp = std::ctime(&result);
        logfile << tmp.substr(0, tmp.length()-1) << " | " << msg << std::endl;
    }
#endif
}
