#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>

#include "SEwalletClient.h"

namespace Ui {
    class LoginDialog;
}

class LoginDialog : public QDialog {

    Q_OBJECT

public:

    explicit LoginDialog(QWidget *parent = nullptr, SEwalletClient *client = nullptr);
    ~LoginDialog();

private:

    Ui::LoginDialog *ui;
    SEwalletClient *client;
    QDateTime dateTime;
    std::pair<std::string, std::string> selected_device;
    std::vector<std::pair<std::string, std::string>> devices;

public slots:

//    void initializeDevice();
    void login();
    void getDeviceList();
    void quit();
    std::pair<std::string, std::string> getSelectedDevice();

};

#endif // LOGIN_H
