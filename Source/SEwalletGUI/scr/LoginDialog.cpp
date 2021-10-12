#include "LoginDialog.h"
#include "ui_LoginDialog.h"

/** Constructor
 *
 * @brief LoginDialog::LoginDialog
 * @param parent
 * @param client
 */
LoginDialog::LoginDialog(QWidget *parent, SEwalletClient *client) : QDialog(parent) , ui(new Ui::LoginDialog) {
    ui->setupUi(this);
    this->client = client;
    //connect(ui->initializeDeviceButton, SIGNAL(clicked()), this, SLOT(initializeDevice()));
    connect(ui->loginButton, SIGNAL(clicked()), this, SLOT(login()));
    connect(ui->getDeviceListButton, SIGNAL(clicked()), this, SLOT(getDeviceList()));
    connect(ui->quitButton, SIGNAL(clicked()), this, SLOT(quit()));
    dateTime = dateTime.currentDateTime();
    ui->currentDateTime->setStyleSheet("color: white");
    ui->currentDateTime->setText(dateTime.toString("[yyyy-MM-dd hh-mm-ss]"));
}

/** Destructor
 *
 * @brief LoginDialog::~LoginDialog
 */
LoginDialog::~LoginDialog() {
    delete ui;
}

/** The user wants to initialize a SEcube device.
 *
 * @brief LoginDialog::initializeDevice
 */
//void LoginDialog::initializeDevice() {

//    QString device = ui->deviceComboBox->currentText();
//    QString password = ui->passwordEdit->text();

//    // Check the user data
//    if(device.isEmpty()) {
//        QMessageBox::warning(this, "Initialization Error", "No device selected.");
//        ui->passwordEdit->clear();
//        return;
//    } else if (password.isEmpty()) {
//        QMessageBox::warning(this, "Initialization Error", "No password provided.");
//        return;
//    }

//    // Create params to initialize
//    SEwallet_Device_Initialization_Params param;
//    param.pin = {0};
//    param.serial_number = {'S', 'E', 'c', 'u', 'b', 'e', 'W', 'A', 'L', 'L','E','T','1'};
//    param.device_path = device.toStdString();
//    memcpy(&(param.pin), password.toLocal8Bit(), sizeof(uint8_t) * password.size());

//    // Try to initialize
//    if (client->txrx(SEwalletCommand::INITIALIZE_DEVICE, &param) == 0) {
//        if (param.retvalue == WALLET_OK) {
//            logger("Device initialization succeded.");
//            ui->messageLabel->setStyleSheet("color: rgb(0, 255, 0)");
//            ui->messageLabel->setText(tr("Device initialization succeded."));
//            qDebug() << "Device initialization succeded.";
//        } else {
//            ui->passwordEdit->clear();
//            dateTime = dateTime.currentDateTime();
//            ui->currentDateTime->setText(dateTime.toString("[yyyy-MM-dd hh-mm-ss]"));
//            ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
//            ui->messageLabel->setText(QString::fromStdString(param.errmessage));
//            logger(param.errmessage);
//        }
//    } else {
//        ui->passwordEdit->clear();
//        dateTime = dateTime.currentDateTime();
//        ui->currentDateTime->setText(dateTime.toString("[yyyy-MM-dd hh-mm-ss]"));
//        ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
//        ui->messageLabel->setText(tr("Protocol transmission error. It was not possible to initialize the selected device."));
//        logger("Protocol transmission error. It was not possible to initialize the selected device.");
//    }

//}

/** The user wants to login to a SEcube device
 *
 * @brief LoginDialog::login
 */
void LoginDialog::login() {

    QString device = ui->deviceComboBox->currentText();
    QString password = ui->passwordEdit->text();

    // Get the selected device serial number
    for (pair<string, string> current : devices) {
        if (current.first == device.toStdString()) {
            selected_device = current;
            break;
        }
    }

    // Check the user data
    if(device.isEmpty()) {
        QMessageBox::warning(this, "Login Error", "No device selected.");
        ui->passwordEdit->clear();
        return;
    } else if (password.isEmpty()) {
        QMessageBox::warning(this, "Login Error", "No password provided.");
        return;
    }

    // Create params to login
    SEwallet_User_Login_Params param;
    param.pin = {0};
    param.type = SE3_ACCESS_ADMIN;
    param.force = true;
    param.selected_device = selected_device;
    memcpy(&(param.pin), password.toLocal8Bit(), sizeof(uint8_t) * password.size());

    // Try to login
    if (client->txrx(SEwalletCommand::LOGIN, &param) == 0) {
        if (param.retvalue == WALLET_OK) {
            selected_device = param.selected_device;
            logger("Logged in.");
            accept();
        } else {
            ui->passwordEdit->clear();
            dateTime = dateTime.currentDateTime();
            ui->currentDateTime->setText(dateTime.toString("[yyyy-MM-dd hh-mm-ss]"));
            ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
            ui->messageLabel->setText(QString::fromStdString(param.errmessage));
            logger(param.errmessage);
        }
    } else {
        ui->passwordEdit->clear();
        dateTime = dateTime.currentDateTime();
        ui->currentDateTime->setText(dateTime.toString("[yyyy-MM-dd hh-mm-ss]"));
        ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
        ui->messageLabel->setText(tr("Protocol transmission error. It was not possible to login."));
        logger("Protocol transmission error. It was not possible to login.");
    }

}

/** The user wants to get the list of available SEcube devices
 *
 * @brief LoginDialog::getDeviceList
 */
void LoginDialog::getDeviceList() {

    SEwallet_Device_GetList_Params param;

    if (client->txrx(SEwalletCommand::GET_DEVICE_LIST, &param) == 0) {           
            if (param.retvalue == WALLET_OK) {
            // Save the list
            devices.clear();
            devices = param.devices;

            //  Delete the old list from QComboBox
            while(ui->deviceComboBox->count()) {
                ui->deviceComboBox->removeItem(0);
            }

            // Update the QComboBox with the retrieved list
            for (auto i = param.devices.begin(); i != param.devices.end(); ++i) {
                ui->deviceComboBox->addItem(QString::fromStdString(i->first) + " - " + QString::fromStdString(i->second));
            }

            dateTime = dateTime.currentDateTime();
            ui->currentDateTime->setText(dateTime.toString("[yyyy-MM-dd hh-mm-ss]"));
            ui->messageLabel->setStyleSheet("color: rgb(0, 255, 0)");
            ui->messageLabel->setText(tr("Device list correctly retrieved."));
            ui->passwordEdit->clear();
            logger("Device list correctly retrieved.");
        } else {
            dateTime = dateTime.currentDateTime();
            ui->currentDateTime->setText(dateTime.toString("[yyyy-MM-dd hh-mm-ss]"));
            ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
            ui->messageLabel->setText((QString::fromStdString(param.errmessage)));
            logger(param.errmessage);
        }
    } else {
        dateTime = dateTime.currentDateTime();
        ui->currentDateTime->setText(dateTime.toString("[yyyy-MM-dd hh-mm-ss]"));
        ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
        ui->messageLabel->setText(tr("Protocol transmission error. Device list not available."));
        logger("Protocol transmission error. Device list not available.");
    }

}

/** The user wants to abort the login operation and quit the application
 *
 * @brief LoginDialog::quit
 */
void LoginDialog::quit() {
    reject();
}

/** Get the selected device
 *
 * @brief LoginDialog::getSelectedDevice
 * @return
 */
std::pair<std::string, std::string> LoginDialog::getSelectedDevice() {
    return selected_device;
}

