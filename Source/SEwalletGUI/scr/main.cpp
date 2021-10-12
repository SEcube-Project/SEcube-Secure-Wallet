/**
  ******************************************************************************
  * File Name          : main.cpp
  * Description        : main of SEwalletGUI
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

#include "main.h"

/** main
 *
 * Is the entry point to the program. Almost always when using Qt, main() only
 * needs to perform some  kind  of initialization  before passing  the control
 * to the Qt library, which  then  tells the program  about the user's actions
 * via events.
 *
 * "argc" is the number of command-line  arguments and "argv" is the array  of
 * command-line arguments. This is a C/C++ feature.  It is not specific to Qt;
 * however, Qt needs to process these arguments.
 *
 * @brief main
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {

    /*
     * Create this program's QApplication. Here  it is  created and  processes
     * some of the command-line arguments (such as  -display under  X Window).
     * Note that all command-line arguments recognized by Qt are removed  from
     * argv (and argc is decremented accordingly). It is  essential  that  the
     * QApplication object be created before any window-system parts of Qt are
     * used.
     */
    QApplication a(argc, argv);
    QApplication::setDesktopSettingsAware(false);
    QApplication::setStyle("Fusion");

    /*
     * Create the client object in order to connect and exchange data with the
     * backend. This object works as a client of a TCP connection whereas  the
     * backend works as a server capable of accepting connections  on  a given
     * SERVER_PORT (see SEwalletClient.h).
     */
    SEwalletClient client;

    /*
     * Run the backend in order to allow it to accept connections on  a  given
     * SERVER_PORT. This opearation is mandatory because this  program   works
     * as a client which has to connect to it.
     */
    runBackend();

    /* Wait for the backend to run */
    this_thread::sleep_for(chrono::milliseconds(1000));

    /*
     * Connect to the backend. This operation is crucial in order to make  the
     * whole application working.
     */
    if (connectToBackend(&client) < 0) {
        return -1;
    }

    /* Create the main window */
    MainWindow mainWindow(nullptr, &client);

    /*
     * A widget is never visible when you create it. You must call  show()  to
     * make it visible; showMaximized() will maximize the window.
     */
    mainWindow.showMaximized();

    /*
     * Here is where main() passes control to Qt, and exec() will  return when
     * the application exits.
     */
    return a.exec();
}

/** Run the backend
 *
 * @brief runBackend
 */
void runBackend() {

    // Kill the instance of the backend if exists
    //killBackend();

    // Run a new instance of the backend
    logger("Start the backend");
    QProcess *myProcess = new QProcess();
    myProcess->start(BACKEND_PATH, nullptr);
//    std::thread server = std::thread([]() {
//        std::system(BACKEND_PATH);
//    });

    /*
     * Don't care about joining the backend  because we
     * will send asynchronous quit message to terminate
     */
    //server.detach();
}

/** Kill the backend
 *
 * @brief killBackend
 */
void killBackend() {
    logger("Kill the backend");
    QProcess *myProcess = new QProcess();
    QString str1 = "kill $(ps -aux | grep " ;
    QString str2 = " | awk '{print $2}')";
    QString program = str1.append(BACKEND_PATH).append(str2);
    myProcess->start(program, nullptr);
    //std::system("kill $(ps -aux | grep //BACKEND_PATH | awk '{print $2}')");
}

/** Connect to the backend
 *
 * @brief connectToBackend
 * @param client
 * @return
 */
int connectToBackend(SEwalletClient *client) {

    /*
     * Initialize the client. This operation might fail if the operating system
     * is not able to create the socket or the SERVER_ADDRESS is not valid.
     */
    if (client->init() < 0)
        return -1;

    /*
     * Try to connect to the backend. The connectToBackend() function  will  do
     * several  ATTEMPTS  (see SEWalletClient.h) in  order  to connect  to  the
     * backend. The user will be able  to  choose  if trying again or exit from
     * the application.
     */
    for (;;) {
        if (!client->connectToBackend()) {
            return 0;
        } else {
            switch (showCriticalMessage()) {
            case QMessageBox::No:
                client->closeSocket();
                return -1;
            case QMessageBox::Yes:
                continue;
            }
        }
    }
}

/** Show a critical message
 *
 * @brief showCriticalMessage
 * @return
 */
int showCriticalMessage() {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowTitle("SEwallet");
    msgBox.setText("Unable to run the application. Retry?");
    msgBox.setDetailedText("It is possible that a critical component for running this application is missing.\nCheck if the file SEwalletBackEnd.exe exists in the parent folder of this file.");
    msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    msgBox.setDefaultButton(QMessageBox::Yes);
    return msgBox.exec();
}
