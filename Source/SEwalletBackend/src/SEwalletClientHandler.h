#ifndef SEWALLETCLIENTHANDLER_H_
#define SEWALLETCLIENTHANDLER_H_

#include <fstream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ctime>
#include <iostream>
#include <string>

#include "SEcubeLibraries/L0/L0.h"
#include "SEcubeLibraries/L1/L1.h"
#include "SEcubeLibraries/SEfile/environment.h"
#include "SEcubeLibraries/SEfile/SEcureDB.h"
#include "SEwalletInterface.h"

using namespace std;

//Debug option
//#define AUTO_TEST_MODE//comment this if not debguing

//Encryption Algorithm
#define WALLET_ENC_ALGORITHM  (L1Algorithms::Algorithms::AES_HMACSHA256)

// Default Key ID for Wallet
#define WALLET_DEFAULT_KEY_ID ((uint32_t)666)
#define WALLET_DEFAULT_KEY_SIZE ((uint16_t)32)


// Wallet Key status
#define	WALLET_KEY_ERROR	-1
#define WALLET_KEY_EXISTED 		1
#define WALLET_KEY_GENERATED	2

// Dimension of input/output buffer for communicating with the GUI (1 MB)
#define BUFLEN 	1024*1024

// Offset and size of fields in requests and responses
#define	T_OFFSET	0
#define T_SIZE 		1
#define L_OFFSET	1
#define L_SIZE		4
#define V_OFFSET	5



class SEwalletClientHandler {

private:

	int s1;
	unique_ptr<L0> l0 = make_unique<L0>();
	unique_ptr<L1> l1 = make_unique<L1>();

	static int callback_tableNames(void *list, int count, char **data, char **columns);
	static int callback_entries(void *list, int count, char **data, char **columns);

	void send_text(int s1, std::string msg);
	void send_cereal(int s1, uint8_t* reply, uint8_t T, std::stringstream& ss, std::string& altmsg);
	template <class T1>
	void read_serial(uint8_t* request, T1& inputparams);
	template <class T2>
	void write_serial(int s1, uint8_t T, uint8_t* reply, T2& inputparams);
    int get_key(uint32_t wallet_key_id);

public:

    explicit SEwalletClientHandler(int s1);
    ~SEwalletClientHandler();
    void run();
    void login(SEwallet_User_Login_Params &inputparams);
    void getDeviceList(SEwallet_Device_GetList_Params &inputparams);
    void getFileList(SEwallet_File_GetList_Params &inputparams);
    void openWallet(SEwallet_Wallet_Open_Params &inputparams);
    void saveWallet(SEwallet_Wallet_Save_Params &inputparams);
    void deleteWallet(SEwallet_Wallet_Delete_Params &inputparams);
    void logout(SEwallet_User_Logout_Params &inputparams);
	#ifdef AUTO_TEST_MODE
    void auto_test();
	#endif

};

#endif /* SEWALLETCLIENTHANDLER_H_ */
