#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QFile>
#include <QDebug>
#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QScrollBar>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QUrl>
#include <QItemSelectionModel>
#include <QDate>
#include <QComboBox>
#include <QSettings>
#include <QMessageBox>
#include <QInputDialog>
#include <QDateTime>

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlTableModel>

#include "SEwalletClient.h"
#include "../../SEwalletBackend/src/SEwalletInterface.h"
#include "LoginDialog.h"
#include "securefiledialog.h"
#include "addentry.h"
//#include "environmentdialog.h"
#include "helpwindow.h"
#include "preferencesdialog.h"
#include "saveconfirmation.h"
#include "mysortfilterproxymodel.h"
#include "filtersaligned.h"
#include "passworditemdelegate.h"

#define DRIVER "QSQLITE"

#define IDENT_COL   0
#define USER_COL    1
#define DOM_COL     2
#define PASS_COL    3
#define DATE_COL    4
#define DESC_COL    5

#define UNUSED(expr) (void)(expr)
#define QD qDebug()


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {

    // This MACRO allows the class to define its own signals/slots through Qt's meta-object system
    Q_OBJECT

public:

    /** Constructor
     * @brief MainWindow
     * @param parent
     * @param client
     */
    explicit MainWindow(QWidget *parent = 0, SEwalletClient *client = nullptr);

    /** Destructor */
    ~MainWindow();

    int callback_createTableList(int argc, char **argv, char **azColName); //Build TableList from ciphered db
    int callback_populateTable(int argc, char **argv, char **azColName);   //Build TableList from ciphered db

private:

    // Allows to interact with C++ UI components
    Ui::MainWindow *ui;

    // TCP  client
    SEwalletClient *client;

    // Wallet object
    Wallet wallet;

    // Database related
    QSqlDatabase dbMem;                 // The database
    QSqlQuery query;
    QSqlTableModel *model;              // Model to handle tables in the database easily
    MySortFilterProxyModel *proxyModel;
    QStringList tables;
    QString currentTable;
    QString path, fileName;             // To store database filename
    QString saveAsAbort=NULL;
    QString walletName;

    FiltersAligned *filters;
    QComboBox *tableList;
    QLabel* displayWalletName;
    QDateTime dateTime;
    PasswordItemDelegate * passDelegate=new PasswordItemDelegate(this);
    int widths [DESC_COL-USER_COL+1] = {0};

    void init();                                     // Initialize the GUI and launch login dialog
    int createNewInMemoryDatabase();
    void closeCurrentInMemoryDatabase();
    bool OpenDataBase();                            //Create/Open Data base and create table, connections
    void createTableView(const QString &tableName); //Create the table model and display the data in the UI.
    void UpdateTableView(const QString &tableName);

    void setAllEnabled(bool enabled);

protected:

    void closeEvent(QCloseEvent *event) override;

private slots:

    void on_action_Logout_triggered();
    void on_action_New_Wallet_triggered();
    void on_action_Open_Wallet_triggered();
    bool on_action_Save_Wallet_triggered();
    void on_action_Save_Wallet_As_triggered();
    void on_action_Delete_Wallet_triggered();
    void on_action_Close_Wallet_triggered();

    void on_action_Add_Table_triggered();
    void on_action_Rename_Table_triggered();
    void on_action_Delete_Table_triggered();
    void on_action_Fit_Table_triggered();
    void on_tableView_doubleClicked(const QModelIndex &index);

    void on_action_Add_Entry_triggered();
    void on_action_Edit_Entry_triggered();
    void on_action_Delete_Entry_triggered();
    void on_action_Show_Passwords_toggled(bool show);

    void on_action_Launch_Domain_triggered();
    void on_action_Preferences_triggered();
//    void on_action_Set_Environment_triggered();
    void on_action_About_triggered();

    void on_action_Older_Than_Filter_toggled(bool arg1);
    void domainFilter_textChanged(const QString &arg1);
    void userFilter_textChanged(const QString &arg1);
    void passFilter_textChanged(const QString &arg1);
    void descFilter_textChanged(const QString &arg1);
    void dateUnit_currentIndexChanged(int index);
    void dateFilter_textChanged(const QString &arg1);
    void dateChoose_dateChanged(QDate date);
    void dateCheck_toggled(bool checked);

    void invalidateAlignedLayout();
    void tableList_currentIndexChanged(const QString &arg1);    

};

#endif // MAINWINDOW_H
