#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include <QApplication>


class MyApplication : public QApplication
{
public:
    MyApplication(int &argc, char ** argv);
private:
    bool notify(QObject *receiver, QEvent *event);
};
#endif // MYAPPLICATION_H
