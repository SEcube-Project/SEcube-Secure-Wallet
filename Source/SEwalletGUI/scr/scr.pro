QT += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    LoginDialog.cpp \
    MainWindow.cpp \
    SEwalletClient.cpp \
    Wallet.cpp \
    addentry.cpp \
    clickablelabel.cpp \
    dictlist.cpp \
    edituserdict.cpp \
    environmentdialog.cpp \
    filtersaligned.cpp \
    helpwindow.cpp \
    main.cpp \
    myapplication.cpp \
    myqtableview.cpp \
    mysortfilterproxymodel.cpp \
    passphrasegen.cpp \
    passwordInfo.cpp \
    passworditemdelegate.cpp \
    preferencesdialog.cpp \
    saveconfirmation.cpp \
    securefiledialog.cpp \
    userwordslist.cpp

HEADERS += \
    LoginDialog.h \
    MainWindow.h \
    SEwalletClient.h \
    Wallet.h \
    addentry.h \
    clickablelabel.h \
    dictlist.h \
    edituserdict.h \
    environmentdialog.h \
    filtersaligned.h \
    helpwindow.h \
    main.h \
    myapplication.h \
    myqtableview.h \
    mysortfilterproxymodel.h \
    passphrasegen.h \
    passwordInfo.h \
    passworditemdelegate.h \
    preferencesdialog.h \
    saveconfirmation.h \
    securefiledialog.h \
    userwordslist.h

FORMS += \
    LoginDialog.ui \
    MainWindow.ui \
    addentry.ui \
    dictlist.ui \
    edituserdict.ui \
    helpwindow.ui \
    passwordInfo.ui \
    preferencesdialog.ui \
    saveconfirmation.ui \
    userwordslist.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Include
INCLUDEPATH += $$PWD/../../SEwalletBackend/src/cereal/
INCLUDEPATH += $$PWD/../../SEwalletBackend/Release/
INCLUDEPATH += $$PWD/../pwgen
INCLUDEPATH += $$PWD/../zxcvbn


RESOURCES += \
    resources.qrc
