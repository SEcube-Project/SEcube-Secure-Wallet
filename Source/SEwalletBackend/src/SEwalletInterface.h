/**
  ******************************************************************************
  * File Name          : GUI_Server_interface.h
  * Description        : data structures for communication with SEwalletGUI
  ******************************************************************************
  *
  * Copyright � 2021-present Blu5 Group <https://www.blu5group.com>
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

#ifndef SEWALLETINTERFACE_H_
#define SEWALLETINTERFACE_H_

#include <cstddef>

#include "cereal/archives/binary.hpp" // use "binary.hpp" instead of "portable_binary.hpp" if endianness is not important
#include "cereal/types/array.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/polymorphic.hpp"
#include "cereal/cereal.hpp"
#include "Wallet.h"
#include "cereal/types/utility.hpp"
#include "SEcubeLibraries/L1/L1 Base/L1_base.h"
#include "SEcubeLibraries/SQLite/sqlite3.h"

using namespace std;

/**
 * From Cereal library docs.
 *
 * When using a binary archive and a file stream (std::fstream), remember
 * to specify the binary  flag  (std::ios::binary) when constructing  the
 * stream.  This  prevents  the  stream from  interpreting  your data  as
 * ASCII  characters and altering them.
 */

#define WALLET_OK 0
#define WALLET_ERROR -1
#define WALLET_WARNING -2
//#define WALLET_NOT_INITIALIZED -3

// T = 1 - SEwallet_User_Login_Params
struct SEwallet_User_Login_Params {
    int retvalue;
    std::string errmessage;
    std::array<uint8_t, L1Parameters::Size::PIN> pin;
    se3_access_type type;
    bool force;
    std::pair<std::string, std::string> selected_device;
    void reset(){
        retvalue = 0;
        errmessage.clear();
        pin = {0};
        type = SE3_ACCESS_NONE;
        selected_device.first.clear();
        selected_device.second.clear();

    }
    template <class Archive>
    void serialize(Archive& archive){
        archive(retvalue, errmessage, pin, type, force, selected_device);
    }
 };

// T = 2 - SEwallet_Device_Initialization_Params
struct SEwallet_Device_Initialization_Params {
    int retvalue;
    std::string errmessage;
    std::array<uint8_t, L1Parameters::Size::PIN> pin;
    std::array<uint8_t, L1Parameters::Size::PIN> serial_number;
    //se3_access_type type;
    //bool force;
    bool force_serial_number;
    std::string device_path;
    void reset(){
        retvalue = 0;
        //force = false;
		force_serial_number = false;
        errmessage.clear();
        pin = {0};
        serial_number = {0};
        //type = SE3_ACCESS_NONE;
        device_path.clear();
    }
    template <class Archive>
    void serialize(Archive& archive){
        archive(retvalue, errmessage, pin, serial_number/*, type, force*/, force_serial_number, device_path);
    }
 };

// T = 3 - SEwallet_Device_GetList_Params
struct SEwallet_Device_GetList_Params {
    int retvalue;
    std::string errmessage;
    std::vector<std::pair<std::string, std::string>> devices;
    void reset() {
        retvalue = 0;
        errmessage.clear();
        devices.clear();
    }
    template <class Archive>
    void serialize(Archive& archive) {
        archive(retvalue, errmessage, devices);
    }
};

// T = 4 - SEwallet_File_GetList_Params
struct SEwallet_File_GetList_Params {
    int retvalue;
    std::string errmessage;
    std::string path;
    std::vector<std::string> list;
    void reset() {
        retvalue = 0;
        errmessage.clear();
        path.clear();
        list.clear();
    }
    template <class Archive>
    void serialize(Archive& archive) {
        archive(retvalue, errmessage, path,  list);
    }
};

// T = 5 - SEwallet_Wallet_Open_Params
struct SEwallet_Wallet_Open_Params {
    int retvalue;
    int sqlretvalue;
    int sefileretvalue;
    std::string errmessage;
    Wallet wallet;
    void reset() {
    	retvalue = WALLET_OK;
    	sqlretvalue =SQLITE_OK;
    	sefileretvalue = 0;
        errmessage.clear();
        //wallet = NULL;
    }
    template <class Archive>
    void serialize(Archive& archive) {
        archive(retvalue, sqlretvalue, sefileretvalue, errmessage, wallet);
    }
};

// T = 6 - SEwallet_Wallet_Save_Params
struct SEwallet_Wallet_Save_Params {
    int retvalue;
    int sqlretvalue;
    int sefileretvalue;
    std::string errmessage;
    Wallet wallet;
    void reset() {
        retvalue = WALLET_OK;
        sqlretvalue =SQLITE_OK;
        sefileretvalue = 0;
        errmessage.clear();
        //wallet = NULL;
    }
    template <class Archive>
    void serialize(Archive& archive) {
        archive(retvalue, sqlretvalue, sefileretvalue, errmessage, wallet);
    }
};

// T = 7 - SEwallet_Wallet_Delete_Params
struct SEwallet_Wallet_Delete_Params {
	int retvalue;
    std::string errmessage;
    Wallet wallet;
    int sefileretvalue;
    void reset() {
    	retvalue = WALLET_OK;
    	sefileretvalue =0;
        errmessage.clear();
        //wallet = NULL;
    }
    template <class Archive>
    void serialize(Archive& archive) {
        archive(retvalue, errmessage, sefileretvalue, wallet);
    }
};

// T = 8 - SEwallet_User_Logout_Params
struct SEwallet_User_Logout_Params {
    int retvalue;
    std::string errmessage;
    //bool force;
    void reset(){
        retvalue = 0;
        errmessage.clear();
    }
    template <class Archive>
    void serialize(Archive& archive){
        archive(retvalue, errmessage/*, force*/);
    }
 };

#endif /* SEWALLETINTERFACE_H_ */
