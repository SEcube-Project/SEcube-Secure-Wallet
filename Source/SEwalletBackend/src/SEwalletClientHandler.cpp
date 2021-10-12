#include "SEwalletClientHandler.h"

L1 *SEcube = nullptr; // see environment.h

/** Constructor
 *
 * @brief
 *
 * @param s1
 */
SEwalletClientHandler::SEwalletClientHandler(int s1) {
    this->s1 = s1;
}

/** Destructor
 *
 * @brief
 *
 */
SEwalletClientHandler::~SEwalletClientHandler() {
}

/** Handle SEwalletGUI client requests
 *
 * @brief
 *
 */
void SEwalletClientHandler::run() {

	// Enter loop to handle SEwalletGUI requests
	bool quit = false;
	uint8_t request[BUFLEN] = {0};
	uint8_t reply[BUFLEN] = {0};
	int ret = 0;

	while (!quit) {

		memset(request, 0, BUFLEN);
		memset(reply, 0, BUFLEN);
		ret = read(s1, request, BUFLEN);

		// Four bytes is the minimum size of a request because we need at least 1B for T and 4B for L
		if (ret < 5) {
			std::string s("Error reading: ");
			send_text(s1, "Server error reading GUI request");
			continue;
		} else if (request[T_OFFSET] == 1) {
			SEwallet_User_Login_Params inputparams;
			read_serial<SEwallet_User_Login_Params>(request, inputparams);
			login(inputparams);
			write_serial<SEwallet_User_Login_Params>(s1, 1, reply, inputparams);
		} else if (request[T_OFFSET] == 3) {
			SEwallet_Device_GetList_Params inputparams;
			read_serial<SEwallet_Device_GetList_Params>(request, inputparams);
			getDeviceList(inputparams);
			write_serial<SEwallet_Device_GetList_Params>(s1, 3, reply, inputparams);
		} else if (request[T_OFFSET] == 4) {
			SEwallet_File_GetList_Params inputparams;
			read_serial<SEwallet_File_GetList_Params>(request, inputparams);
			getFileList(inputparams);
			write_serial<SEwallet_File_GetList_Params>(s1, 4, reply, inputparams);
		} else if (request[T_OFFSET] == 5) {
			SEwallet_Wallet_Open_Params inputparams;
			read_serial<SEwallet_Wallet_Open_Params>(request, inputparams);
			openWallet(inputparams);
			write_serial<SEwallet_Wallet_Open_Params>(s1, 5, reply, inputparams);
		} else if (request[T_OFFSET] == 6) {
			SEwallet_Wallet_Save_Params inputparams;
			read_serial<SEwallet_Wallet_Save_Params>(request, inputparams);
			saveWallet(inputparams);
			write_serial<SEwallet_Wallet_Save_Params>(s1, 6, reply, inputparams);
		} else if (request[T_OFFSET] == 7) {
			SEwallet_Wallet_Delete_Params inputparams;
			read_serial<SEwallet_Wallet_Delete_Params>(request, inputparams);
			deleteWallet(inputparams);
			write_serial<SEwallet_Wallet_Delete_Params>(s1, 7, reply, inputparams);
		} else if (request[T_OFFSET] == 8) {
			SEwallet_User_Logout_Params inputparams;
			read_serial<SEwallet_User_Logout_Params>(request, inputparams);
			logout(inputparams);
			write_serial<SEwallet_User_Logout_Params>(s1, 8, reply, inputparams);
		} else  {
			continue;
		}
	}

}

/** Login in a user to a SEcube device
 *
 * @brief
 *
 * @param inputparams
 */
void SEwalletClientHandler::login(SEwallet_User_Login_Params &inputparams) {

	// (path and serial number)  --> inputparams.selected_device
	std::array<uint8_t, L0Communication::Size::SERIAL> sn = {0};
	if (inputparams.selected_device.second.length() > L0Communication::Size::SERIAL) {
		inputparams.retvalue = WALLET_ERROR;
		inputparams.errmessage = "Unexpected error while checking the serial number (wrong serial number).";
		cout << "Unexpected error while checking the serial number (wrong serial number)." << endl;
		return;
	} else {
		memcpy(sn.data(), inputparams.selected_device.second.data(), inputparams.selected_device.second.length());
	}
	this->l1->L1SelectSEcube(sn);
	cout << "Device " << inputparams.selected_device.first << " - " << inputparams.selected_device.second << " selected." << endl;

    // Try to login
	try {
		/* Provide pin, access privilege and boolean to force logout before login (in  case
		 * there was already an active login on the device) */
		this->l1->L1Login(inputparams.pin, inputparams.type, inputparams.force);
		/*
		 * Notice that when the login is executed, the SEcube device enters a specific loop
		 * to handle requests coming from the host. If you  close  your   program   without
		 * performing the logout, the  SEcube  device  will  still be in that loop the next
		 * time you will attempt the login (provided that you do not unplug the SEcube from
		 * the PC, removing power). This is the reason why you have to provide the  boolean
		 * as third parameter of the L1Login function, because you need to force the logout
		 * from previous pending sessions. If  you  do  not force the logout, you won't  be
		 * able to communicate with the SEcube (because the session  is  still  open on the
		 * SEcube side, but it doesn't exist anymore on your host).
		 */
	} catch (...) {
		/* Catch any kind of exception (login will throw if the password is wrong or if any
		 * error happens) */
		inputparams.retvalue = WALLET_ERROR;
		inputparams.errmessage = "Login error. Wrong password or device not initialized.";
		cout << "Login error. Wrong password or device not initialized." << endl;
		return;
	}

	// Check if login was ok
	if (!this->l1->L1GetSessionLoggedIn()) {
		inputparams.retvalue = WALLET_ERROR;
		inputparams.errmessage = "Not Logged in error.";
		cout << "Not Logged in error." << endl;
		return;
	}
	cout << "User logged in." << endl;

	// Operation successfully completed
	SEcube = this->l1.get();
	inputparams.retvalue = WALLET_OK;
}

/** Get the list of available SEcube devices
 * @brief
 *
 * @param inputparams
 */
void SEwalletClientHandler::getDeviceList(SEwallet_Device_GetList_Params &inputparams) {

	// Check how many SEcube devices are connected to the PC
	int numdevices = this->l0->GetNumberDevices();
	if (numdevices == 0) {
		inputparams.retvalue = WALLET_ERROR;
		inputparams.errmessage = "No SEcube devices found.";
		return;
	}
	//cout << "Number of SEcube devices found: " << numdevices << endl;

	// Fill a vector with pairs including details about the devices (path and serial number)
	vector<pair<string, string>> devices;
	int ret = this->l0->GetDeviceList(inputparams.devices);
	if (ret) {
		inputparams.retvalue = WALLET_ERROR;
	    inputparams.errmessage = "Error while searching for SEcube devices.";
		//cout << "Error while searching for SEcube devices." << endl;
	    return;
	}

	// Operation successfully completed
	inputparams.retvalue = WALLET_OK;
}

/** Get the list of encrypted wallets
 *
 * @brief
 *
 * @param inputparams
 */
void SEwalletClientHandler::getFileList(SEwallet_File_GetList_Params &inputparams) {

	vector<pair<string, string>> list;

	/** Same as secure_ls() but for encrypted SQLite databases.
	*
	* @brief This function identifies which encrypted files and encrypted directories are present
	*        in the directory pointed by path and writes them in list.
	* @param [in] path Absolute or relative path to the directory to browse.
	* @param [out] list List of pairs containing the encrypted name and the decrypted name.
	* @param [in] SEcubeptr Pointer to the L1 object used to communicate with the SEcube.
	* @return The function returns a 0 in case of success. See \ref errorValues for error list.
	* @details Notice that, if the name of a file or of a directory belonging to the path  is not
	*          associated to SEfile, then it is copied as it is in the list. This function is not
	*          recursive.
	**/
	int ret = securedb_ls(inputparams.path, list, this->l1.get());
	if (ret) {
		inputparams.retvalue = WALLET_ERROR;
		inputparams.errmessage = "Error while reading the list of encrypted files.";
		return;
	}

	// Save the list
	for (pair<string, string> p : list) {
		inputparams.list.insert(inputparams.list.begin(), p.second);
		//cout << "encrypted: " << p.first << " - " << "decrypted: " << p.second << endl;
	}

	// Operation successfully completed
    inputparams.retvalue = WALLET_OK;
}

/** Open a wallet
 *
 * @brief
 *
 * @param inputparams
 */
void SEwalletClientHandler::openWallet(SEwallet_Wallet_Open_Params &inputparams) {

	Wallet wallet = inputparams.wallet;

	// Database pointer
	sqlite3 *db;

	// Standard SEfile object also for encrypted databases
	unique_ptr<SEfile> walletdatabase = make_unique<SEfile>();
	SEcube = l1.get();

	uint16_t sefile_status;
	sefile_status = walletdatabase->secure_init(l1.get(), /*WALLET_DEFAULT_KEY_ID*/ 1, WALLET_ENC_ALGORITHM); // no key needed for reading file
	if(sefile_status != 0){
		inputparams.reset();
		inputparams.sefileretvalue = sefile_status;
		inputparams.retvalue = WALLET_ERROR;
		inputparams.errmessage = "Error setting SEfile Environment.";
		cout << "Error sefile code: "<< sefile_status<< endl;
		return;
	}

	// Set the database name
	int n = wallet.getFileName().length();
	int m = wallet.getPath().length();
	char dbfullname[m + n + 1];
	strcpy((&dbfullname[0]), wallet.getPath().c_str());
	strcpy((&dbfullname[m]), wallet.getFileName().c_str());

	// Copy the name of the database into the name field of the handleptr attribute of the SEfile object
	memcpy(walletdatabase->handleptr->name, dbfullname, strlen(dbfullname));

	// Move the SEfile object to the databases vector
	databases.push_back(std::move(walletdatabase));

	// Open the database
	int rc = sqlite3_open_v2(dbfullname, &db, SQLITE_OPEN_READONLY, NULL);
	if(rc != SQLITE_OK) {
		sqlite3_close_v2(db);
		inputparams.reset();
		inputparams.retvalue = WALLET_ERROR;
		inputparams.errmessage = "Error opening the encrypted database.";
		return;
	} else {
		std::string query;

		// Get table names from database
		std::vector<std::string> tableNames;
		query="SELECT name FROM sqlite_master WHERE type='table' ORDER BY name;";
		rc = sqlite3_exec(db, query.c_str(), callback_tableNames, &tableNames, NULL);
		if(rc != SQLITE_OK) {
			inputparams.reset();
			inputparams.sefileretvalue = sqlite3_extended_errcode(db);
			inputparams.sqlretvalue = rc;
			inputparams.retvalue = WALLET_ERROR;
			inputparams.errmessage = "Error reading table names.";
			sqlite3_close_v2(db);
			return;
		}

		// Retrieve data and save in the wallet object
		for (std::string tableName : tableNames) {
			Wallet::Table table;
			table.setName(tableName);
			cout << tableName << endl;

			// Get entries of current table
			vector<Wallet::Table::Entry> entries;
			query="SELECT * FROM " +  tableName + ";";
			rc = sqlite3_exec(db, query.c_str(), callback_entries, &entries, NULL);
			if (rc != SQLITE_OK) {
				sqlite3_close_v2(db);
				inputparams.reset();
				inputparams.retvalue = WALLET_ERROR;
				inputparams.errmessage = "Error reading entries.";
				return;
			}
            table.setEntries(entries);

            // Save table data in the wallet
			wallet.insertTable(table);
		}
	}

	// Close the database
    sqlite3_close_v2(db);
	
	// Operation successfully completed
	inputparams.retvalue = WALLET_OK;
	inputparams.wallet = wallet;
}

/** Save a wallet
 *
 * @brief
 *
 * @param inputparams
 */
void SEwalletClientHandler::saveWallet(SEwallet_Wallet_Save_Params &inputparams) {

	Wallet wallet = inputparams.wallet;

	// Database pointer
	sqlite3 *db;

	// Standard SEfile object also for encrypted databases
	unique_ptr<SEfile> walletdatabase = make_unique<SEfile>();
	SEcube = l1.get();

	// Initialize the SEfile object with the parameters we want (we could have done this using the specific constructor)
	int key_status = get_key(WALLET_DEFAULT_KEY_ID);
	if (key_status == WALLET_KEY_ERROR){
		inputparams.reset();
		inputparams.retvalue = WALLET_ERROR;
		inputparams.errmessage = "No keys available and Error creating the keys on the SEcube.";
		return;
	} else if (key_status == WALLET_KEY_EXISTED){
		cout << "The Key already existed." << endl;
	} else if (key_status == WALLET_KEY_GENERATED){
		cout << "A new Key has been generated and added to SEcube." << endl;
	} else {
		cout << "Shouldn't reach here!" << endl;
	}

	uint16_t sefile_status;
	sefile_status = walletdatabase->secure_init(l1.get(), WALLET_DEFAULT_KEY_ID, WALLET_ENC_ALGORITHM);
	if(sefile_status != 0){
		inputparams.reset();
		inputparams.sefileretvalue = sefile_status;
		inputparams.retvalue = WALLET_ERROR;
		inputparams.errmessage = "Error setting SEfile Environment!";
		//cout << "Error sefile code: "<< sefile_status<< endl;
		return;
	}

	// Set the database name
	int n = wallet.getFileName().length();
	int m = wallet.getPath().length();
	char dbfullname[m + n + 1];
	strcpy((&dbfullname[0]), wallet.getPath().c_str());
	strcpy((&dbfullname[m]), wallet.getFileName().c_str());

	// Copy the name of the database into the name field of the handleptr attribute of the SEfile object
	memcpy(walletdatabase->handleptr->name, dbfullname, strlen(dbfullname));

	// Move the SEfile object to the databases vector
	databases.push_back(std::move(walletdatabase));

	// Save the wallet as an encrypted database
	int rc = sqlite3_open_v2(dbfullname, &db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, NULL);
	if(rc != SQLITE_OK) {
		sqlite3_close_v2(db);
		inputparams.reset();
		inputparams.sefileretvalue = sqlite3_extended_errcode(db);
		inputparams.sqlretvalue = rc;
		inputparams.retvalue = WALLET_ERROR;
		inputparams.errmessage = "Error opening/creating the encrypted database.";
		return;
	} else {
		std::string query;
		for (Wallet::Table table : wallet.getTables()) {

			// Create the table
			query = "CREATE TABLE '" + table.getName() +
			                      "'(ID INTEGER PRIMARY KEY, "
			                      "Username TEXT, "
			                      "Domain TEXT, "
			                      "Password TEXT, "
		                          "Date TEXT, "
			                      "Description TEXT );";
			rc = sqlite3_exec(db, query.c_str(), NULL, NULL, NULL);
			if (rc != SQLITE_OK) {
				inputparams.reset();
				inputparams.sefileretvalue = sqlite3_extended_errcode(db);
				inputparams.sqlretvalue = rc;
				inputparams.retvalue = WALLET_ERROR;
				inputparams.errmessage = "Error saving the encrypted database. Create table failed.";
				sqlite3_close_v2(db);
				return;
			}
			cout << table.getName() << " created" << endl;

			// Create entries in the table
			for (Wallet::Table::Entry entry : table.getEntries()) {

				// Create the entry
				query = "INSERT INTO '" + table.getName() +
				      "'('Username', "
				      "  'Domain', "
				      "  'Password', "
				      "  'Date', "
				      "  'Description') "
				      "VALUES ('" + entry.getUsername()    + "','" +
					                entry.getDomain()    			  + "','" +
									entry.getPassword().c_str()   + "','" +
									entry.getDate()        			  + "','" +
									entry.getDescription() + "');";
				rc = sqlite3_exec(db, query.c_str(), NULL, NULL, NULL);
				if (rc != SQLITE_OK) {
					inputparams.reset();
					inputparams.sefileretvalue = sqlite3_extended_errcode(db);
					inputparams.sqlretvalue = rc;
					inputparams.retvalue = WALLET_ERROR;
					inputparams.errmessage = "Error saving the encrypted database. Insert into entry failed.";
					sqlite3_close_v2(db);
					return;
				}
				cout << "("
				     << entry.getUsername()    << ","
		             << entry.getDomain()      << ","
					 << entry.getPassword() .c_str()   << ","
					 << entry.getDate()        << ","
					 << entry.getDescription() << ")"
					 <<" entry created"        << endl;
			}
		}
	}

	// Close the database
	sqlite3_close_v2(db);

	// Operation successfully completed
	inputparams.retvalue = WALLET_OK;
}

/** Delete a wallet
 *
 * @brief
 *
 * @param inputparams
 */
void SEwalletClientHandler::deleteWallet(SEwallet_Wallet_Delete_Params &inputparams) {

	Wallet wallet = inputparams.wallet;
	int n = wallet.getFileName().length();
	int m = wallet.getPath().length();
	char dbfullname[m + n + 1];
	strcpy((&dbfullname[0]), wallet.getPath().c_str());
	strcpy((&dbfullname[m]), wallet.getFileName().c_str());

	// Encrypt wallet
    uint16_t lenc=0;
    char enc_filename[MAX_PATHNAME];
    memset(enc_filename, 0, MAX_PATHNAME*sizeof(char));
    if (crypto_filename(dbfullname, enc_filename, ((uint16_t*)&lenc))) {
    	inputparams.reset();
        inputparams.sefileretvalue = SEFILE_CREATE_ERROR;
    	inputparams.retvalue = WALLET_ERROR;
    	inputparams.errmessage = "Error encrypting database filename.";
    	return;
    }

    // Delete wallet
	int removeres;
	removeres = remove(enc_filename);
	if (removeres!= 0) {
		inputparams.reset();
		inputparams.sefileretvalue = removeres;
		inputparams.retvalue = WALLET_ERROR;
		inputparams.errmessage = "Error removing encrypted database.";
		return;
	}

	// Operation successfully completed
	inputparams.retvalue= WALLET_OK;
}

/** Logout a user from the SEcube device
 *
 * @param inputparams
 */
void SEwalletClientHandler::logout(SEwallet_User_Logout_Params &inputparams) {

	// Try to logout
	try {
		this->l1->L1Logout();
	} catch (...) {
		inputparams.retvalue  = WALLET_ERROR;
		inputparams.errmessage = "Logout try error.";
		cout << "Logout error." << endl;
		return;
	}

	// Check if logout was ok//
	if (l1->L1GetSessionLoggedIn()) {
		inputparams.retvalue  = WALLET_ERROR;
	    inputparams.errmessage = "Logout was not successfully. Still logged in.";
		cout << "Logout error." << endl;
		return;
	}
	cout << "User logged out." << endl;

	// Operation successfully completed
	inputparams.retvalue = WALLET_OK;
}

/** Cereal send operation
 *
 * @brief
 *
 * @param s1
 * @param reply
 * @param T
 * @param ss
 * @param altmsg
 */
void SEwalletClientHandler::send_cereal(int s1, uint8_t* reply, uint8_t T, std::stringstream& ss, std::string& altmsg) {
	std::string s = ss.str();
	uint32_t replysize = s.size();
	if(replysize <= BUFLEN){
		reply[T_OFFSET] = T;
		memcpy(reply+L_OFFSET, &replysize, L_SIZE);
		memcpy(reply+V_OFFSET, s.c_str(), replysize);
		send(s1, reply, T_SIZE + L_SIZE + replysize, 0);
	} else {
		send_text(s1, altmsg);
	}
}

/** Send a text over the TCP connection
 *
 * @brief
 *
 * @param s1
 * @param msg
 */
void SEwalletClientHandler::send_text(int s1, std::string msg)  {
	uint32_t msgsize = msg.size();
	if(msgsize + T_SIZE + L_SIZE <= BUFLEN) {
		std::unique_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(msgsize);
		buf[T_OFFSET] = 0;
		memcpy(buf.get()+L_OFFSET, &msgsize, L_SIZE);
		memcpy(buf.get()+V_OFFSET, msg.data(), msgsize);
		send(s1, buf.get(), T_SIZE + L_SIZE + msgsize, 0);
	}
}

/** Callback tableNames
 *
 * @param list
 * @param count
 * @param data
 * @param columns
 * @return
 */
int SEwalletClientHandler::callback_tableNames(void *list, int count, char **data, char **columns) {
	std::vector<std::string> *names = (std::vector<std::string> *)list;

	names->insert(names->end(), data[0]);
	cout << " table read: "<< data[0];
	return 0;
}

/** Callback entries
 *
 * @param list
 * @param count
 * @param data
 * @param columns
 * @return
 */
int SEwalletClientHandler::callback_entries(void *list, int count, char **data, char **columns) {
    std::vector<Wallet::Table::Entry> *entries = (std::vector<Wallet::Table::Entry>*)list;
    Wallet::Table::Entry entry;
    //data[0] is column's index
    entry.setUsername(data[1]);
    entry.setDomain(data[2]);
    entry.setPassword(data[3]);
    entry.setDate(data[4]);
    entry.setDescription(data[5]);
    entries->insert(entries->end(), entry);
    cout << "("
    				     << entry.getUsername()    << ","
    		             << entry.getDomain()      << ","
    					 << entry.getPassword()     << ","
    					 << entry.getDate()        << ","
    					 << entry.getDescription() << ")"
    					 <<" entry read";
    return 0;
}

/** Read serial from received data
 *
 * @param request
 * @param inputparams
 */
template <class T1>
void SEwalletClientHandler::read_serial(uint8_t* request, T1& inputparams) {
		std::stringstream ss;
		uint32_t vsize = 0;
		memcpy(&vsize, request+L_OFFSET, L_SIZE);
		ss.write((char*)request+V_OFFSET, vsize);
		cereal::BinaryInputArchive iarchive(ss);
		iarchive(inputparams);
}

/** Write serial to transmit
 *
 * @tparam T2
 * @param s1
 * @param T
 * @param reply
 * @param inputparams
 */
template <class T2>
void SEwalletClientHandler::write_serial(int s1, uint8_t T, uint8_t* reply, T2& inputparams) {
	std::stringstream ss;
	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(inputparams);
	}
	send_cereal(s1, reply, T, ss, inputparams.errmessage);
	//logger(inputparams.errmessage);
}

/** Get a key
 *
 * @param wallet_key_id
 * @return
 */
int SEwalletClientHandler::get_key(uint32_t wallet_key_id) {

	// Generate new key or check if it is available
	bool wallet_key_available = false;
	vector<pair<uint32_t, uint16_t>> keys;

	l1->L1KeyList(keys); // read ID of keys in the SEcube
	if(wallet_key_id < L1Key::Id::MANUAL_ID_END && wallet_key_id > L1Key::Id::MANUAL_ID_BEGIN) { // valid key id
		//rndkeyid = rand() % L1Key::Id::MANUAL_ID_END + L1Key::Id::MANUAL_ID_BEGIN; // generate random number in range [1, 999]
		for(pair<uint32_t, uint16_t> p : keys) {
			if(p.first == wallet_key_id) {
				wallet_key_available = true;
				break;
			}
		}

		if (wallet_key_available == false) {
			// Generate and flash key (256 bit) to the device
			se3Key wallet_key;
			wallet_key.id = wallet_key_id;
			wallet_key.dataSize = WALLET_DEFAULT_KEY_SIZE;
			wallet_key.data = nullptr; // no key data in this case
			try {
				l1->L1KeyEdit(wallet_key, L1Commands::KeyOpEdit::SE3_KEY_OP_ADD_TRNG);
			} catch (...) {
				cout << "Error creating the keys on the SEcube. Quit." << endl;
				l1->L1Logout();
				return WALLET_KEY_ERROR;
			}
			return WALLET_KEY_GENERATED;
			cout << "\n256 bit key created correctly." << endl;
		} else {
			// Key is already existed in secube
			return WALLET_KEY_EXISTED;
		}
	} else {
		// Not valid key id
		return WALLET_KEY_ERROR;
	}
	return WALLET_KEY_ERROR;
}
#ifdef AUTO_TEST_MODE
void SEwalletClientHandler::auto_test(){
	// Get Device List
	SEwallet_Device_GetList_Params DGL_param;
	DGL_param.reset();
	getDeviceList(DGL_param);
	DGL_param.retvalue !=0 ? (cout<<"status: " << DGL_param.retvalue << " ,error massage: " << DGL_param.errmessage <<endl ):(cout<< "SEwallet_Device_GetList_OK"<<endl);
	if(DGL_param.retvalue == WALLET_ERROR)
		return;

	//Try to initialize
	SEwallet_Device_Initialization_Params DI_param;
	DI_param.reset();
	DI_param.force_serial_number = false;
	DI_param.pin= {'t','e','s','t'};
	DI_param.serial_number={'S', 'E', 'c', 'u', 'b', 'e', 'W', 'A', 'L', 'L','E','T'};
	DI_param.device_path=DGL_param.devices.back().first;
	initializeDevice(DI_param);
	DI_param.retvalue !=0 ? (cout<<"status: " << DI_param.retvalue << " ,error massage: " << DI_param.errmessage <<endl ):(cout<< "SEwallet_Device_Initialization_OK"<<endl);
	if(DI_param.retvalue == WALLET_WARNING)
		cout<< "SEwallet_Device_Initialization_OK"<<endl;
	else if(DI_param.retvalue == WALLET_ERROR)
		return;

	//try to login
	SEwallet_User_Login_Params UL_param;
	if (!DGL_param.devices.empty())
		UL_param.selected_device = DGL_param.devices.back();
	UL_param.pin = {'t','e','s','t'}; // tmp=atoi(array[i].c_str());
	UL_param.type = SE3_ACCESS_ADMIN;
	UL_param.force= true;
	login(UL_param);
	UL_param.retvalue !=0 ? (cout<<"status: " << UL_param.retvalue << " ,error massage: " << UL_param.errmessage <<endl ):(cout<< "SEwallet_User_Login_OK"<<endl);
	if(UL_param.retvalue == WALLET_ERROR)
		return;

	//set test file path and name
	bool use_sdcard = true;
	string microsd;
	string dummy_path;
	string dummy_db_name = "dummy wallet.sql";

	if (use_sdcard){
		if(get_microsd_path(*l0, microsd) != 0){//
			SEcube = nullptr;
			cout<< "error retrieving microSD path"<<", Use linux Path instead: "<<dummy_path<<endl;
			dummy_path = "/home/user/Desktop/CS_repo/SEWallet-code/Database/";
			//return SEKEY_ERR;
		}else{
			cout<< "microsd path: "<<microsd<<endl;
			dummy_path = microsd;
		}
	} else {
		dummy_path = "/home/user/Desktop/CS_repo/SEWallet-code/Database/";
		//string dummy_path = "/home/ufox/Ingegneria/IngegneriaInformaticaSpecialistica/CybersecurityForEmbeddedSystems/Project/DevIman/SEWallet-code/";
		cout<< "use local path: "<< dummy_path <<endl;
	}


	//generate dummy wallet
	Wallet::Table::Entry dummy_enty1, dummy_enty2;
	Wallet::Table dummy_table1, dummy_table2;
	Wallet dummy_wallet;
	dummy_enty1.setUsername("imanuser");
	dummy_enty1.setPassword("imanpass");
	dummy_enty1.setDescription("dummy");
	dummy_enty1.setDomain("dummy.com");
	dummy_enty1.setDate("15/11/1995");
	dummy_enty2.setUsername("imanuser2");
	dummy_enty2.setPassword("imanpass2");
	dummy_enty2.setDescription("dummy2");
	dummy_enty2.setDomain("dummy.com2");
	dummy_enty1.setDate("1/1/2021");
	dummy_table1.setName("iman1");
	dummy_table1.setEntries({dummy_enty1, dummy_enty1});
	dummy_table2.setName("iman2");
	dummy_table2.setEntries({dummy_enty1, dummy_enty2, dummy_enty1, dummy_enty2});
	dummy_wallet.setTables({dummy_table1, dummy_table2});
	dummy_wallet.setPath(dummy_path);
	dummy_wallet.setFileName(dummy_db_name);

	//save Dummy Wallet
	SEwallet_Wallet_Save_Params SW_param;
	SW_param.wallet= dummy_wallet;
	saveWallet(SW_param);
	SW_param.retvalue !=0 ? (cout<<"status: " << SW_param.retvalue << " ,error massage: " << SW_param.errmessage <<" ,sqlite error code: " << SW_param.sqlretvalue  <<" ,sefile error code: " << SW_param.sefileretvalue  <<endl ):(cout<< "SEwallet_Wallet_Save_OK"<<endl);
	if(SW_param.retvalue == WALLET_ERROR)
		return;

	//Get Encrypted File names
	SEwallet_File_GetList_Params GL_param;
	GL_param.path = dummy_path;
	getFileList(GL_param);
	GL_param.retvalue !=0 ? (cout<<"status: " << GL_param.retvalue << " ,error massage: " << GL_param.errmessage <<endl ):(cout<< "SEwallet_File_GetList_Params_OK"<<endl);
	if(UL_param.retvalue == WALLET_ERROR)
		return;
	else{
		cout << "file lists:" <<endl;
		for (string p : GL_param.list) {
			cout << "\tfname: " << p <<endl;
		}

	}

	//open encrypted dummy file
 	SEwallet_Wallet_Open_Params OW_param;
	Wallet empty_wallet;
	empty_wallet.setPath(dummy_path);
	empty_wallet.setFileName(GL_param.list.at(0)); // use first file
	OW_param.wallet = empty_wallet;
	openWallet(OW_param);
	OW_param.retvalue !=0 ? (cout<<"status: " << OW_param.retvalue << " ,error massage: " << OW_param.errmessage <<" ,sqlite error code: " << OW_param.sqlretvalue  <<" ,sefile error code: " << OW_param.sefileretvalue  <<endl ):(cout<< "SEwallet_Wallet_Open_OK"<<endl);
	if(OW_param.retvalue == WALLET_ERROR)
			return;

	//check that open wallet is same as saved one
	if(OW_param.wallet == SW_param.wallet)
		cout << "Database has been saved and retrieved correctly." << endl;
	else
		cout << "Database has been either saved or retrieved Wrongly!" << endl;

	//delete saved file
	SEwallet_Wallet_Delete_Params DW_param;
	Wallet to_delete_wallet;
	to_delete_wallet.setPath(dummy_path);
	to_delete_wallet.setFileName(GL_param.list.at(0)); // use first file
	DW_param.wallet = to_delete_wallet;
	deleteWallet(DW_param);
	DW_param.retvalue !=0 ? (cout<<"status: " << DW_param.retvalue << " ,error massage: " << DW_param.errmessage <<" ,sefile error code: " << DW_param.sefileretvalue  <<endl ):(cout<< "SEwallet_Wallet_Delete_OK"<<endl);
	if(DW_param.retvalue == WALLET_ERROR)
		return;


	//logout
	SEwallet_User_Logout_Params LO_param;
	logout(LO_param);
	LO_param.retvalue !=0 ? (cout<<"status: " << LO_param.retvalue << " ,error massage: " << LO_param.errmessage <<endl ):(cout<< "SEwallet_User_Logout_OK"<<endl);
	if(LO_param.retvalue == WALLET_ERROR)
		return;

	return;
}
#endif
