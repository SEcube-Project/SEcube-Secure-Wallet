/**
  ******************************************************************************
  * File Name          : main.cpp
  * Description        : main of SEwalletBackend
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

/*
 *****************************************************************************
 * APPLICATION PROTOCOL DESCRIPTION
 *****************************************************************************
 * GUI REQUESTS
 *
 * GUI sends requests in TLV format.
 * T -> 1 byte, uint8_t value from 0 to 255 to identify each  specific request
 * L -> 4 bytes, uint32_t value to tell how many bytes is V field
 * V -> L bytes, uint8_t* to be casted to stringstream  and deserialized  with
 *      Cereal library depending on T value
 *
 *****************************************************************************
 * SERVER RESPONSES
 *
 * The server or Backend (this program) sends  responses  to  the   GUI  using
 * the same TLV approach.
 * T -> 1 byte (uint8_t), it is the same value that the GUI sent
 * L -> 4 bytes (uint32_t), it is the dimension of the V field
 * V -> L bytes (uint8_t*), to be casted to stringstream and deserialized with
 *      Cereal library
 *
 *****************************************************************************
 * WARNING!
 *
 * T = 0 is reserved for  requests  and  responses  that  contain  exclusively
 * "string-like" data, so Cereal is not really required in this case (i.e. the
 * GUI sends "quit" to the server).
 * If T == 0, the GUI must not use Cereal... it  simply  needs  to process the
 * response in terms of the text that the server has sent (text-only responses
 * are used to signal  errors, i.e.  the  server could  not  read the  request
 * coming from the GUI).
 *
 *****************************************************************************
 */

#include "main.h"

/*
 * Application entry point
 */
int main() {

	// Create the server
	SEwalletServer sewalletServer;

	/*
	 * Initialize the server
	 *
	 * This init()  method perform the classic sequence of steps
	 * (socket, bind, listen, accept)  in order to run a welcome
	 * socket capable  of  accepting  connections  on a specific
	 * SERVER_ADDRESS and SERVER_PORT (see "main.h").
	 *
	 * It returns the file descriptor which refers to the client.
	 */
	#ifndef AUTO_TEST_MODE
	int fd;
	if ((fd = sewalletServer.init()) < 0) {
		cout << "Something went wrong." << endl;
		return 1;
	}

	// Create the client handler
	SEwalletClientHandler sewalletClientHandler(fd);
	sewalletClientHandler.run();

	cout << "End backend Self Test"<<endl;

	#else
	cout << "Run backend Self Test" << endl;
	SEwalletClientHandler sewalletClientHandler(0);
	sewalletClientHandler.auto_test();
	#endif

	return 0;
}

