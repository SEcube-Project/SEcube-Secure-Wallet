#ifndef MAIN_H
#define MAIN_H

#include "MainWindow.h"
#include "myapplication.h"
#include "SEwalletClient.h"

#include <iostream>
#include <thread>
#include <cstdlib>
#include <fstream>

#include <QApplication>
#include <QMessageBox>
#include <QDebug>

#define BACKEND_PATH "../../SEwalletBackend/Release/SEwalletBackend"

std::ofstream logfile("GUI_log.txt");

void runBackend();

void killBackend();

int connectToBackend(SEwalletClient *client);

int showCriticalMessage();

#endif // MAIN_H
