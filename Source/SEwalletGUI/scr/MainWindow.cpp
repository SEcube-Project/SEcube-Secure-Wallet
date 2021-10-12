#include "MainWindow.h"
#include "ui_MainWindow.h"

/** Callback functions for sqlite3_exec. They need to be out of the class and static. They only job is to call the members who do all the job.
 *
 * @brief c_callback_createTableList
 * @param mainwindowP
 * @param argc
 * @param argv
 * @param azColName
 * @return
 */
static int c_callback_createTableList(void *mainwindowP, int argc, char **argv, char **azColName) { // Create Table List
    MainWindow* main = reinterpret_cast<MainWindow*>(mainwindowP);
    return main->callback_createTableList(argc, argv, azColName);
}

/**
 * @brief c_callback_populateTable
 * @param mainwindowP
 * @param argc
 * @param argv
 * @param azColName
 * @return
 */
static int c_callback_populateTable(void *mainwindowP, int argc, char **argv, char **azColName) { // Populate Table
    MainWindow* main = reinterpret_cast<MainWindow*>(mainwindowP);
    return main->callback_populateTable(argc, argv, azColName);
}

/**
 * @brief just_print
 * @param null
 * @param argc
 * @param argv
 * @param azColName
 * @return
 */
static int just_print(void *null, int argc, char **argv, char **azColName){ // Create Table List

    UNUSED (null);
    UNUSED (azColName);
    int i;
    for(i = 0; i<argc; i++)
        qDebug() << argv[i];
    return 0;
}

/** Constructor
 *
 * @brief MainWindow::MainWindow
 * @param parent
 * @param client
 */
MainWindow::MainWindow(QWidget *parent, SEwalletClient *client) : QMainWindow(parent), ui(new Ui::MainWindow) {
    this->client = client;
    init();
}

/** Destructor
 *
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow() {
    QD <<"destructor";

    // Close current in-memory database
    closeCurrentInMemoryDatabase();

    // Close client connection
    client->closeSocket();

    delete ui;
}

/** Initialize the GUI and launch the login dialog
 *
 * @brief MainWindow::init
 */
void MainWindow::init() {

    // Initialize all the widgets
    ui->setupUi(this);

    // Set the application title
    setWindowTitle(tr("SEwallet"));

    // Add table List to ToolBar before delete
    tableList = new QComboBox(this);
    //ui->tableTB->insertWidget(ui->action_Delete_Table, tableList);
    ui->entriesTB->addWidget(tableList);
    connect(tableList, SIGNAL(currentIndexChanged(QString)), SLOT(tableList_currentIndexChanged(QString)));

    // State of some actions as not clickable because there is no wallet to edit yet
    ui->action_New_Wallet->setEnabled(true);
    ui->action_Open_Wallet->setEnabled(true);
    ui->action_Delete_Wallet->setEnabled(true);
    ui->action_Save_Wallet->setEnabled(false);
    ui->action_Save_Wallet_As->setEnabled(false);
    ui->action_Close_Wallet->setEnabled(false);

    ui->action_Add_Table->setEnabled(false);
    ui->action_Delete_Table->setEnabled(false);
    ui->action_Rename_Table->setEnabled(false);

    ui->action_Add_Entry->setEnabled(false);
    ui->action_Edit_Entry->setEnabled(false);
    ui->action_Delete_Entry->setEnabled(false);

    ui->action_Show_Passwords->setEnabled(false);
    ui->action_Older_Than_Filter->setEnabled(false);
    ui->action_Launch_Domain->setEnabled(false);
    ui->action_Fit_Table->setEnabled(false);
    ui->filtersWidget->setVisible(false);

    // Set the wallet name in the status bar
    displayWalletName = new QLabel;
    displayWalletName->setStyleSheet("font-weight: bold; color: #000088");
    displayWalletName->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(displayWalletName,1);

    filters=NULL;
    model=NULL;
    //ui->WalletView->hide();

    QCoreApplication::setOrganizationName("WalterGallego");
    QCoreApplication::setApplicationName("SECubeWallet");
    QSettings settings;

    // ===============================================
    // Show login dialog
    // ===============================================
    LoginDialog *login = new LoginDialog(this, client);
    login->showMaximized();
    login->exec();
    if(login->result() == QDialog::Rejected){
        qDebug() << "User aborted, terminating";
        exit(1);
    }

    // Check if SQLite driver is installed on OS
    if(!(QSqlDatabase::isDriverAvailable(DRIVER))) {
        ui->action_New_Wallet->setEnabled(false);
        ui->action_Open_Wallet->setEnabled(false);
        ui->action_Delete_Wallet->setEnabled(false);
        qWarning() << "ERROR: no driver " << DRIVER << " for in-memory database available.";
        ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
        ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
        ui->messageLabel->setText("ERROR: no driver for in-memory database available.");
    }

    // Initialization succeeded
    ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
    ui->messageLabel->setStyleSheet("color: rgb(0, 255, 0)");
    ui->messageLabel->setText(QString::fromStdString(login->getSelectedDevice().first) + " - " + QString::fromStdString(login->getSelectedDevice().second));
    return;
}

/** The user wants to logout
 *
 * @brief MainWindow::on_action_Logout_triggered
 */
void MainWindow::on_action_Logout_triggered() {

    // Check if there are unsaved changes
    if (ui->action_Save_Wallet->isEnabled()) {
        SaveConfirmation *conf = new SaveConfirmation;
        conf->exec();
        if(conf->result()==QDialog::Rejected){
            QD << "cancel";
            return; // if error or cancel, do nothing
        }
        else if (conf->result()==QDialog::Accepted){
            if (conf->getResult()==SAVE){
                QD << "save";
                if(!on_action_Save_Wallet_triggered())// call save
                    return; // save gone wrong, do not do anything
            }
            else if(conf->getResult()==DISCARD) // continue without saving
                QD << "discard";
        }
    }

    // Try to logout
    SEwallet_User_Logout_Params param;
    if (client->txrx(SEwalletCommand::LOGOUT, &param) == 0) {
        if (param.retvalue == WALLET_OK) {
            logger("Logout.");
            this->hide();
            qDebug() << "Logout.";
            // ===============================================
            // Show login dialog
            // ===============================================
            LoginDialog *login = new LoginDialog(this, client);
            login->showMaximized();
            login->exec();
            if(login->result() == QDialog::Rejected){
                qDebug() << "User aborted, terminating";
                exit(1);
            }
            ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
            ui->messageLabel->setStyleSheet("color: rgb(0, 255, 0)");
            ui->messageLabel->setText(QString::fromStdString(login->getSelectedDevice().first) + " - " + QString::fromStdString(login->getSelectedDevice().second));
            this->showMaximized();
        } else {
            dateTime = dateTime.currentDateTime();
            ui->currentDateTimeLabel->setText(dateTime.toString("[yyyy-MM-dd hh-mm-ss]"));
            ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
            ui->messageLabel->setText(QString::fromStdString(param.errmessage));
            logger(param.errmessage);
        }
    } else {
        dateTime = dateTime.currentDateTime();
        ui->currentDateTimeLabel->setText(dateTime.toString("[yyyy-MM-dd hh-mm-ss]"));
        ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
        ui->messageLabel->setText(tr("Protocol transmission error. It was not possible to logout."));
        logger("Protocol transmission error. It was not possible to logout.");
    }
}

/** Check if there are unsaved changes on application closing event
 *
 * @brief MainWindow::closeEvent
 * @param event
 */
void MainWindow::closeEvent(QCloseEvent *event) {

    if (ui->action_Save_Wallet->isEnabled()) { // there are unsaved changes
        SaveConfirmation *conf = new SaveConfirmation;
        conf->exec();
        if(conf->result()==QDialog::Rejected) {
            QD << "cancel";
            event->ignore();
        }
        else if (conf->result()==QDialog::Accepted) {
            if (conf->getResult()==SAVE) {
                QD << "save";
                if(!on_action_Save_Wallet_triggered()) // call save
                    event->ignore();
            }
            else if(conf->getResult()==DISCARD) // continue without saving
                QD << "discard";
        }
    }
}

/** The user wants to create a new wallet. Start an in-memory database so the user can add tables/entries.
 *  Do not save in disk yet.
 *
 * @brief MainWindow::on_action_New_Wallet_triggered
 */
void MainWindow::on_action_New_Wallet_triggered() {

    // Check if there are unsaved changes
    if (ui->action_Save_Wallet->isEnabled()) {
        SaveConfirmation *conf = new SaveConfirmation;
        conf->exec();
        if(conf->result()==QDialog::Rejected){
            QD << "cancel";
            return; // if error or cancel, do nothing
        }
        else if (conf->result()==QDialog::Accepted){
            if (conf->getResult()==SAVE){
                QD << "save";
                if(!on_action_Save_Wallet_triggered())// call save
                    return; // save gone wrong, do not do anything
            }
            else if(conf->getResult()==DISCARD) // continue without saving
                QD << "discard";
        }
    }

    // Close current in-memory database
    closeCurrentInMemoryDatabase();

    // Create a new in-memory database
    if (createNewInMemoryDatabase() != 0) {
        ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
        ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
        ui->messageLabel->setText("New wallet creation failed.");
        return;
    }

    // If everything went ok, enable some actions
    ui->action_Add_Table->setEnabled(true);
    ui->action_Delete_Table->setEnabled(false);
    ui->action_Rename_Table->setEnabled(false);

    ui->action_Save_Wallet->setEnabled(true);
    ui->action_Save_Wallet_As->setEnabled(true);
    ui->action_Close_Wallet->setEnabled(true);

    ui->action_Add_Entry->setEnabled(false);
    ui->action_Edit_Entry->setEnabled(false);
    ui->action_Delete_Entry->setEnabled(false);
    ui->action_Show_Passwords->setEnabled(false);
    ui->action_Older_Than_Filter->setEnabled(false);
    ui->action_Launch_Domain->setEnabled(false);
    ui->action_Fit_Table->setEnabled(false);
    ui->filtersWidget->setVisible(false);

    tableList->clear();

    // Reset the filename
    fileName.clear();

    // Initialize wallet's name as "Unnamed"
    displayWalletName->setText("Unnamed");

    // New wallet creation succeeded
    ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
    ui->messageLabel->setStyleSheet("color: rgb(0, 255, 0)");
    ui->messageLabel->setText("New \"unnamed\" wallet successfully created.");
    return;
}

/** Create a new  in-memory database.
 *
 * @brief MainWindow::createNewInMemoryDatabase
 */
int MainWindow::createNewInMemoryDatabase() {

    // Adds a database to the list of database connections using the driver  type
    dbMem = QSqlDatabase::addDatabase(DRIVER);

    /* Sets the connection's database name to name. Additionally, name can be set
     * to  ":memory:"  which will create  a  temporary  database  which  is  only
     * available for the lifetime of the application. */
    dbMem.setDatabaseName(":memory:");

    // Check if it is possible to open the database
    if(!dbMem.open()) {
        qWarning() << "ERROR: " << dbMem.lastError();
        return -1;// Do nothing
    }

    qDebug() << "New in-memory database created.";
    return 0;
}

/** Close the current in-memory database.
 *
 * @brief MainWindow::closeCurrentInMemoryDatabase
 */
void MainWindow::closeCurrentInMemoryDatabase() {
    if (dbMem.isOpen()) {
        int col, aux;

        // Save table geometry before closing
        for (col = USER_COL; col <= DESC_COL; col++) {
            aux = ui->tableView->horizontalHeader()->sectionSize(col);
            widths[col] = (aux>0) ? aux : widths[col];
        }

        if (model != NULL) {
            model->clear();
            model = NULL;
            proxyModel->invalidate();
        }

        /* Close the database connection, freeing any resources acquired, and
         * invalidating any existing  QSqlQuery objects that are used with the
         * database. */
        dbMem.close();

        /* Remove the database  connection  connectionName  from  the list of
         * database connections.
         */
        QSqlDatabase::removeDatabase(DRIVER);

        /* Create an empty, invalid  QSqlDatabase  object.  Use addDatabase(),
         * removeDatabase(), and database() to get valid QSqlDatabase objects. */
        dbMem = QSqlDatabase();

        qDebug() << "The in-memory database was closed.";
    }
}

/** The user wants to add a new table. Create a new table in the in-memory database and add it to tableList.
 *
 * @brief MainWindow::on_action_Add_Table_triggered
 */
void MainWindow::on_action_Add_Table_triggered() {
    bool ok;
    QString tableName = QInputDialog::getText(this, "New Table", "Enter new table name", QLineEdit::Normal,"", &ok);
    if (!ok || tableName.isEmpty()) {
        ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
        ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
        ui->messageLabel->setText("Table creation error.");
        return;
    }

    // Preapare the query for creating the table
    QSqlQuery query(dbMem);
    query.prepare("create table '"+tableName+
                  "'(id integer primary key, "
                  "Username TEXT, "
                  "Domain TEXT, "
                  "Password TEXT, "
                  "Date TEXT, "
                  "Description TEXT )");

    if (!query.exec()){
        // TODO: prompt error dialog name may be invalid. see more. and link
        qWarning() << "Couldn't create the table " << tableName << query.lastError();
        ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
        ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
        ui->messageLabel->setText("Couldn't create the table.");
        return;
    }
    query.finish();

    QD << "table created";
    ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
    ui->messageLabel->setStyleSheet("color: rgb(0, 255, 0)");
    ui->messageLabel->setText("Table " + tableName + " created.");

    tableList->setEnabled(true);
    if(tableList->findText(tableName) == -1) //Check if item already in the list
        tableList->addItem(tableName); //if not, add it
    tableList->setCurrentText(tableName);

    // Finally, update the tableView to reflect the new/opened table
    ui->filtersWidget->setVisible(true);
    if (filters==NULL)
        createTableView(tableName);

    // As we now have a table, we can enable the buttons
    ui->action_Delete_Table->setEnabled(true);
    ui->action_Rename_Table->setEnabled(true);
    ui->action_Add_Entry->setEnabled(true);
    ui->action_Edit_Entry->setEnabled(true);
    ui->action_Delete_Entry->setEnabled(true);
    ui->action_Launch_Domain->setEnabled(true);
    ui->action_Show_Passwords->setEnabled(true);
    ui->action_Show_Passwords->setChecked(false);
    ui->action_Older_Than_Filter->setEnabled(true);
    ui->action_Older_Than_Filter->setChecked(false);
    ui->action_Fit_Table->setEnabled(true);
    ui->action_Save_Wallet->setEnabled(true);
    ui->action_Save_Wallet_As->setEnabled(true);

    //displayWalletName->setText(walletName.remove("*").append("*"));
}

/** After table is created in database, display it in a QTableView, and manage it using a Table Model.
 *
 * @brief MainWindow::createTableView
 * @param tableName
 */
void MainWindow::createTableView(const QString &tableName ) {

    // Create and configure model to access table easily
    model = new QSqlTableModel;
    model->setTable(tableName);
    model->select();
    model->setEditStrategy(QSqlTableModel::OnManualSubmit); // as the table will be not edditable, the changes are updated by calling submitAll()

    // Create ProxyModel for filtering
    proxyModel = new MySortFilterProxyModel(this);
    proxyModel->setSourceModel(model); //connect proxyModel to Model

    // Connect the model to the view table: Sqltable -> Model -> ProxyModel ->  TableView
    ui->tableView->setModel(proxyModel);

    // Configure the table
    //ui->WalletView->setSortingEnabled(true);//enable sorting
    ui->tableView->verticalHeader()->hide();
    ui->tableView->setColumnHidden(IDENT_COL, true);//Hide Identification column, not important for user, only for sqlite.
    ui->tableView->setItemDelegateForColumn(PASS_COL, passDelegate);//Hide passwords initially
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);// To make the table view not edditable
    ui->tableView->setSelectionBehavior( QAbstractItemView::SelectItems ); //To allow only one row selection...
    ui->tableView->setSelectionMode( QAbstractItemView::SingleSelection ); //So we can edit one entry per time
    ui->tableView->show();// show table, initially hidden

    // Resize table columns to fill available space
    on_action_Fit_Table_triggered();

    // Aligned Layout
    filters = new FiltersAligned();

    connect(filters->userFilter, SIGNAL(textChanged(QString)), SLOT(userFilter_textChanged(QString)));
    connect(filters->domFilter,  SIGNAL(textChanged(QString)), SLOT(domainFilter_textChanged(QString)));
    connect(filters->passFilter, SIGNAL(textChanged(QString)), SLOT(passFilter_textChanged(QString)));
    connect(filters->descFilter, SIGNAL(textChanged(QString)), SLOT(descFilter_textChanged(QString)));
    connect(filters->dateFilter, SIGNAL(textChanged(QString)), SLOT(dateFilter_textChanged(QString)));
    connect(filters->dateUnit,   SIGNAL(currentIndexChanged(int)), SLOT(dateUnit_currentIndexChanged(int)));
    connect(filters->dateChoose, SIGNAL(dateChanged(QDate)), SLOT(dateChoose_dateChanged(QDate)));
    connect(filters->dateCheck, SIGNAL(toggled(bool)), SLOT(dateCheck_toggled(bool)));

    ui->filtersWidget->setLayout(filters);
    filters->setTableColumnsToTrack(ui->tableView->horizontalHeader());
    filters->setParent(ui->filtersWidget);
    connect(ui->tableView->horizontalHeader(), SIGNAL(sectionResized(int,int,int)), SLOT(invalidateAlignedLayout()));
    connect(ui->tableView->horizontalScrollBar(), SIGNAL(valueChanged(int)), SLOT(invalidateAlignedLayout()));

    return;
}

/** Used by columnAlignedLayout in createViewTable.
 *
 * @brief MainWindow::invalidateAlignedLayout
 */
void MainWindow::invalidateAlignedLayout() {
    filters->invalidate();
}

/** Update Table view when user selects a different table.
 *
 * @brief MainWindow::UpdateTableView
 * @param tableName
 */
void MainWindow::UpdateTableView(const QString &tableName) {

    int col;
    if (model==NULL){ // if we are updating bcs a new wallet, therefore a new db connection
        model = new QSqlTableModel;
        model->setEditStrategy(QSqlTableModel::OnManualSubmit);// as the table will be not edditable, the changes are updated by calling submitAll()
        model->setTable(tableName);
        model->select();
        // Create ProxyModel for filtering
        proxyModel = new MySortFilterProxyModel(this);
        proxyModel->setSourceModel(model); //connect proxyModel to Model

        // Connect the model to the view table: Sqltable -> Model -> ProxyModel ->  TableView
        ui->tableView->setModel(proxyModel);
    } else{ //if we are updating bcs table change inside the same db
        int col;
        int aux;
        for (col=USER_COL;col<=DESC_COL; col++) { // save table geometry before closing
            aux=ui->tableView->horizontalHeader()->sectionSize(col);//for readability
            widths[col] = (aux>0) ? aux : widths[col];
        }
        model->setTable(tableName);
        model->select();
    }

    for (col=USER_COL;col<=DESC_COL; col++)
        ui->tableView->horizontalHeader()->resizeSection(col, widths[col]);
    ui->tableView->setColumnHidden(IDENT_COL, true);

}

/** When user changes table we need to modify the tableview. Just change the view to the selected table.
 *
 * @param arg1
 */
void MainWindow::tableList_currentIndexChanged(const QString &arg1) {
    QD << "in index change" <<arg1;
    if (!arg1.isEmpty() && filters!=NULL){
        QD << "calling update";
        UpdateTableView(arg1);
    }
}

/** The user wants to delete a table.
 *
 * @brief MainWindow::on_action_Delete_Table_triggered
 */
void MainWindow::on_action_Delete_Table_triggered() {
    QString tableName = tableList->currentText();

    if (!tableName.isEmpty()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      "Delete",
                                      "Are you sure you want to delete Table "+tableName,
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
            return;

        QSqlQuery query(dbMem);

        query.prepare("DROP TABLE '" + tableName+"'");
        if (!query.exec()) {
            qWarning() << "Couldn't delete table" << tableName;
            qWarning() << "ERROR: " << query.lastError();
            ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
            ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
            ui->messageLabel->setText("Couldn't delete table.");
            return;
        }
        query.finish();
        tableList->removeItem(tableList->currentIndex());
        ui->action_Save_Wallet->setEnabled(true);
        ui->action_Save_Wallet_As->setEnabled(true);
        //displayWalletName->setText(walletName.remove("*").append("*"));
        if (tableList->count()==0) {
            int col;
            int aux;
            for (col=USER_COL;col<=DESC_COL; col++){ // save table geometry before closing
                aux=ui->tableView->horizontalHeader()->sectionSize(col);//for readability
                widths[col] = (aux>0) ? aux : widths[col];
            }
            model->select();
            //As we do not have tables, we have to disable the buttons
            ui->action_Delete_Table->setEnabled(false);
            ui->action_Rename_Table->setEnabled(false);
            ui->filtersWidget->setVisible(false);
            ui->action_Add_Entry->setEnabled(false);
            ui->action_Edit_Entry->setEnabled(false);
            ui->action_Delete_Entry->setEnabled(false);
            ui->action_Launch_Domain->setEnabled(false);
            ui->action_Show_Passwords->setEnabled(false);
            ui->action_Older_Than_Filter->setEnabled(false);
            ui->action_Fit_Table->setEnabled(false);
        }
        QD << "Table deleted";
        ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
        ui->messageLabel->setStyleSheet("color: rgb(0, 255, 0)");
        ui->messageLabel->setText("Table " + tableName + " deleted.");
    }
    return;
}

/** The user wants to rename a table
 *
 * @brief MainWindow::on_action_Rename_Table_triggered
 */
void MainWindow::on_action_Rename_Table_triggered() {
    QString currentName = tableList->currentText();
    if (!currentName.isEmpty()){
        bool ok;
        QString newName = QInputDialog::getText(this, "Rename",
                                                "New name for Table "+currentName,
                                                QLineEdit::Normal,"", &ok);
        if (!ok || newName.isEmpty())
            return;

        QSqlQuery query(dbMem);
        query.prepare("ALTER TABLE '" + currentName + "' RENAME TO '"+newName+"'");
        if (!query.exec()){
            qWarning() << "Couldn't rename table" << currentName;
            qWarning() << "ERROR: " << query.lastError();
            ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
            ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
            ui->messageLabel->setText("Couldn't rename table.");
            return;
        }
        query.finish();
        tableList->removeItem(tableList->currentIndex());

        if(tableList->findText(newName) == -1) //Check if item already in the list
            tableList->addItem(newName); //if not, add it
        tableList->setCurrentText(newName);

        ui->action_Save_Wallet->setEnabled(true);
        ui->action_Save_Wallet_As->setEnabled(true);
        //displayWalletName->setText(walletName.remove("*").append("*"));

        ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
        ui->messageLabel->setStyleSheet("color: rgb(0, 255, 0)");
        ui->messageLabel->setText("Table " + currentName + " renamed as " + newName + ".");
    }
}

/** The user wants to add an entry to a table. Call dialog and populate Table with the aqcuired data.
 *
 * @brief MainWindow::on_action_Add_Entry_triggered
 */
void MainWindow::on_action_Add_Entry_triggered() {

    AddEntry *add = new AddEntry(this);
    add->exec();
    add->clean();
    if(add->result()==QDialog::Rejected) {
        return; // Error or cancel, do nothing
    }

    QSqlRecord rec = model->record(); // Temp entry
    rec.setGenerated("id", false); // The ID column is managed automatically by the model, not by the user
    rec.setValue("Username", add->getUser());
    rec.setValue("Password",add->getPassword());
    rec.setValue("Domain",add->getDomain());
    rec.setValue("Description",add->getDescription());
    rec.setValue("Date", QDate::currentDate());

    int newRecNo = model->rowCount(); //Where to insert the new record?: at the end of the table
    if (!model->insertRecord(newRecNo, rec)){
        return;
    }
    model->submitAll();// if insert ok, submit changes.

    ui->action_Save_Wallet->setEnabled(true);
    ui->action_Save_Wallet_As->setEnabled(true);
    //displayWalletName->setText(walletName.remove("*").append("*"));

    ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
    ui->messageLabel->setStyleSheet("color: rgb(0, 255, 0)");
    ui->messageLabel->setText("New entry created.");
    return;
}

/** Double click on the tableView
 *
 * @brief MainWindow::on_tableView_doubleClicked
 * @param index
 */
void MainWindow::on_tableView_doubleClicked(const QModelIndex &index) {
    MainWindow::on_action_Edit_Entry_triggered();
    UNUSED(index);
}

/** The user wants to edit an entry of a table. Call add entry dialog an pass values
 *  to constructior allowing users to edit the data.
 *
 * @brief MainWindow::on_action_Edit_Entry_triggered
 */
void MainWindow::on_action_Edit_Entry_triggered() {
   QModelIndexList selection = ui->tableView->selectionModel()->selectedIndexes();//Get Items selected in table
    model->submitAll();//Submit any pending changes before continue. (cannot be done before select bcs it deletes it)

    // If Multiple rows can be selected (Disabled for edit. Could be good for delete)
    for(int i=0; i< selection.count(); i++) {
        int row = selection.at(i).row();

        //Call add entry with second constructor, so we can pass it the values to edit
        AddEntry *add = new AddEntry(this, proxyModel->index(row,USER_COL).data().toString(),
                                     proxyModel->index(row,PASS_COL).data().toString(),
                                     proxyModel->index(row,DOM_COL).data().toString(),
                                     proxyModel->index(row,DESC_COL).data().toString());
        add->exec();
        add->clean();
        if(add->result()==QDialog::Rejected)
            return; // If error or cancel, do nothing

        //Change the values in proxy
        proxyModel->setData(proxyModel->index(row,USER_COL), add->getUser());
        proxyModel->setData(proxyModel->index(row,PASS_COL), add->getPassword());
        proxyModel->setData(proxyModel->index(row,DOM_COL),  add->getDomain());
        proxyModel->setData(proxyModel->index(row,DESC_COL), add->getDescription());
        proxyModel->setData(proxyModel->index(row,DATE_COL), QDate::currentDate());

        if(!model->submitAll()) //Submit changes to database
            qDebug() << "Error: " << model->lastError();
    }
    ui->action_Save_Wallet->setEnabled(true);
    ui->action_Save_Wallet_As->setEnabled(true);
    //displayWalletName->setText(walletName.remove("*").append("*"));
    return;
}

/** The user wants to delete an entry of a table. Call confirmation dialog,
 *  if confirmed, delete entry.
 *
 * @brief MainWindow::on_action_Delete_Entry_triggered
 */
void MainWindow::on_action_Delete_Entry_triggered() {
    //TODO: ask for password. change in settings.
    QModelIndexList selection = ui->tableView->selectionModel()->selectedIndexes();//Get Items selected in table
    model->submitAll();//Submit any pending changes before continue. (cannot be done before select bcs it deletes the selection)

    // If Multiple rows can be selected (disabled to allow edit)
    for(int i=0; i< selection.count(); i++){
        //Before continuing, promt confirmation dialog
        QModelIndex index = selection.at(i);
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      "Delete",
                                      "Are you sure you want to delete Entry "+
                                      proxyModel->index(index.row(),USER_COL).data().toString(),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
            return;

        proxyModel->removeRow(index.row());
        if(!model->submitAll()){ //Submit changes if no errors
            qDebug() << "Error: " << model->lastError();
            return;
        }
    }
    ui->action_Save_Wallet->setEnabled(true);
    ui->action_Save_Wallet_As->setEnabled(true);
    //displayWalletName->setText(walletName.remove("*").append("*"));
    ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
    ui->messageLabel->setStyleSheet("color: rgb(0, 255, 0)");
    ui->messageLabel->setText("Entry deleted.");
    return;
}

/** Check if url is written correctly and call browser.
 *
 * @brief MainWindow::on_action_Launch_Domain_triggered
 */
void MainWindow::on_action_Launch_Domain_triggered() {
    //TODO: autofill forms. check again url
    QModelIndexList selection = ui->tableView->selectionModel()->selectedIndexes();//Get Items selected in table
    model->submitAll();//Submit any pending changes before continue. (cannot be done before select bcs it deletes it)

    // If Multiple rows can be selected (Disabled for edit. Could be good for delete)
    for(int i=0; i< selection.count(); i++){
        int row = selection.at(i).row();
        QString domain=proxyModel->index(row,DOM_COL).data().toString();
        QString domainaux=domain;
        if (!domain.startsWith("http://") && !domain.startsWith("https://"))
            domain = "http://" + domain;

        if (domain.contains("www.")){//add .com just if no . are found. (from .it or .net etc)
            if (!domainaux.remove("www.").contains("."))
                domain = domain+".com";
        }else if (!domain.contains("."))
            domain = domain+".com";
        qDebug() <<domain;
        QDesktopServices::openUrl(QUrl(domain));
    }
}

/** The user wants to show or hide the password. Make passwords Not/visible by setting or not a column delegator.
 *
 * @brief MainWindow::on_action_Show_Passwords_toggled
 * @param show
 */
void MainWindow::on_action_Show_Passwords_toggled(bool show) {
    if (!show){
        ui->tableView->setItemDelegateForColumn(PASS_COL, passDelegate);
        filters->passFilter->clear();
        filters->passFilter->setEnabled(false);
    }
    else{
        //TODO: confirmation dialog, ask again for password? configurable from settings. How to ask SECUbE to check the password again. a new challenge function?
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      "Passwords",
                                      "Are you sure you want to show your passwords",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
            return;//if error or cancel, do nothing

        ui->tableView->setItemDelegateForColumn(PASS_COL, 0);
        filters->passFilter->clear();
        filters->passFilter->setEnabled(true);
    }
}

/** Fit table to available space.
 *
 * @brief MainWindow::on_action_Fit_Table_triggered
 */
void MainWindow::on_action_Fit_Table_triggered() {
    // Resize table columns to fill available space
    int col;
    int tableWidth=ui->tableView->width();

    int div = tableWidth / 5;
    int mod = tableWidth % 5;

    // Remainder too to the first column. (the -2 is for margins pixels(?))
    ui->tableView->horizontalHeader()->resizeSection(USER_COL, div + mod -2);

    // Add to each column the correspo. value
    for (col=DOM_COL;col<=DESC_COL; col++)
        ui->tableView->horizontalHeader()->resizeSection(col, div);
}

/** To change envirolment, key ID and encryption.
 *
 * @brief MainWindow::on_action_Set_Environment_triggered
 */
//void MainWindow::on_action_Set_Environment_triggered() {
//    /*EnvironmentDialog *envDialog = new EnvironmentDialog(this, &s);
//    envDialog->exec();*/
//    return;
//}

/** The user wants to save the wallet.
 *
 * @brief MainWindow::on_action_Save_Wallet_triggered
 * @return
 */
bool MainWindow::on_action_Save_Wallet_triggered() {

    ////  ask for a filename to save changes the first time the button is clicked after a new wallet
    // Get the filename and its path
    if (fileName.isEmpty()) {
        SecureFileDialog *fileDialog = new SecureFileDialog(this, 1, client); // option 1 means we are creating new file
        fileDialog->exec();
        if (fileDialog->result() == QDialog::Rejected) {
            if(!saveAsAbort.isEmpty()) {  // if it was a SaveAs, and was aborted, recover fileName
                fileName = saveAsAbort;
                saveAsAbort.clear();
            }
            return 0; // Error in Dialog or Cancel button, do nothing
        }
        path = fileDialog->getChosenPath();
        fileName = fileDialog->getChosenFile();

        if (fileName.isEmpty()) { // if no valid file name => do nothing
            if (!saveAsAbort.isEmpty()) { //if it was a SaveAs, and was aborted, recover fileName
                fileName = saveAsAbort;
                saveAsAbort.clear();
            }
            return 0;
        }
        qWarning() << "PATH: " << path;
        qWarning() << "FILE: " << fileName;

        //// Check if file already exists
        //Get name of file in disk (encrypted) and delete.
        //prepare vars
//        char enc_filename[65];
//        uint16_t enc_len = 0;
//        memset(enc_filename, 0, 65);
//        crypto_filename(fileName.toUtf8().data(), enc_filename, &enc_len );


//        if (QFile::exists(fileName)) {
//            QMessageBox::StandardButton reply; reply = QMessageBox::question(this,
//                                                                             "Overwrite",
//                                                                             "There already is a Wallet named "+fileName+
//                                                                             ", Do you want to overwrite it?",
//                                                                             QMessageBox::Yes|QMessageBox::No);
//            if (reply == QMessageBox::No) {
//                fileName.clear();
//                if(!saveAsAbort.isEmpty()) { //if it was a SaveAs, and was aborted, recover fileName
//                    fileName=saveAsAbort;
//                    saveAsAbort.clear();
//                }
//                return 0; //if error or cancel, do nothing
//            }
//        } else
//              fileName = QFileDialog::getSaveFileName(this, "select filename", QCoreApplication::applicationDirPath(), "Sqlite files(*.sqlite *.db)");

    } else {
//        //TODO: check what happens when overwrite, if file is deleted. and if it necessary. it is working, so maybe sqliteopen delets prev.
//        //Get name of file in disk (encrypted) and delete.
////        char enc_filename[65];
////        uint16_t enc_len = 0;
////        memset(enc_filename, 0, 65);
////        crypto_filename(fileName.toUtf8().data(), enc_filename, &enc_len );
////        QFile::remove(enc_filename);
//        QFile::remove(fileName);
    }
    saveAsAbort.clear();   // if save As was not aborted then clear for future.
    setAllEnabled(false);  // save process takes a while, so disable all buttons.
    qApp->processEvents(); // otherwise the repaint take place after this function finishes

    // Create a wallet object
    Wallet wallet;
    wallet.setPath(path.toStdString());
    wallet.setFileName(fileName.toStdString());

    // Dump data from in-memory database to wallet object
    QSqlQuery query(dbMem);
    QStringList values;
    QSqlRecord record;
    QStringList tables;

    // Get a list of the tables in dbMem
    tables << dbMem.tables(QSql::Tables);

    //BUG: workaround for first table always corrupted problem: have a dummy table, that ensures database is not empty
    /*tables.prepend("NoEmpty"); */
    // Get tables from in-memory database and fill the wallet object
    foreach (const QString currentTable, tables) {

        // Create a table object
        Wallet::Table table;
        table.setName(currentTable.toStdString());

        // Select * from current in-memory table
        query.prepare(QString("SELECT * FROM [%1]").arg(currentTable)); //prepare select
        if(!query.exec()){
            QD << "Error selecting * from" << currentTable << query.lastError();
            setAllEnabled(true);
            return 0;
         }

        // Get records from in-memory table
        while (query.next()) {
            values.clear();

            // Get record from in-memory table
            record = query.record();

            // Create an entry object
            Wallet::Table::Entry entry;
            values << record.value(0).toString();//the index does not goes in ''

            // Get fields
            for (int i = 1; i < record.count(); i++) {
                switch (i) {
                case 1:
                    entry.setUsername(record.value(i).toString().toStdString());
                    break;
                case 2:
                    entry.setDomain(record.value(i).toString().toStdString());
                    break;
                case 3:
                    entry.setPassword(record.value(i).toString().toStdString());
                    break;
                case 4:
                    entry.setDate(record.value(i).toString().toStdString());
                    break;
                case 5:
                    entry.setDescription(record.value(i).toString().toStdString());
                    break;
                default:
                    break;
                }     
            }
            // Add entry object to entries
            table.insertEntry(entry);
            qDebug() << table.getEntries().size();

        }
        // Add table object to tables
        wallet.insertTable(table);
        qDebug() << wallet.getTables().size();

    }
    query.finish();

    // Save the wallet
    SEwallet_Wallet_Save_Params params;
    params.wallet = wallet;
    if (client->txrx(SEwalletCommand::SAVE_WALLET, &params) == 0) {
        if (params.retvalue == 0) {
            qDebug() << "Wallet successfully saved.";
            ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
            ui->messageLabel->setStyleSheet("color: rgb(0, 255, 0)");
            ui->messageLabel->setText(fileName + " successfully saved.");
        } else {
            ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
            ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
            ui->messageLabel->setText(QString::fromStdString(params.errmessage));
            setAllEnabled(true);
            return 0;
        }
    } else {
        qDebug() << "Protocol comunication error. Error saving the wallet.";
        ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
        ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
        ui->messageLabel->setText("Protocol comunication error. Error saving the wallet.");
        setAllEnabled(true);
        return 0;
    }

    setAllEnabled(true);
    walletName=QFileInfo(QFile(fileName)).absoluteFilePath().remove(".sqlite");
    displayWalletName->setText(walletName);
    ui->action_Save_Wallet->setEnabled(false); //wallet is not dirty anymore, no save allowed until some change is made

    return 1;
}

/** The user wants to save the wallet.
 *
 * @brief MainWindow::on_action_Save_Wallet_As_triggered
 */
void MainWindow::on_action_Save_Wallet_As_triggered() {
    // In case SaveAs is aborted, we need to recover the fileName
    saveAsAbort = fileName;
    // It is enough to set filename to empty, for save_wallet to ask for a new fileName
    fileName.clear();
    // Save the wallet
    on_action_Save_Wallet_triggered();
}

/** The user wants to open a wallet. Open Cyphered database and display it.
 *
 * @brief MainWindow::on_action_Open_Wallet_triggered
 */
void MainWindow::on_action_Open_Wallet_triggered() {

    // Check if there are unsaved changes
    if(ui->action_Save_Wallet->isEnabled()) {
        SaveConfirmation * conf = new SaveConfirmation;
        conf->exec();
        if(conf->result()==QDialog::Rejected) {
            return; //if error or cancel, do nothing
        }
        else if (conf->result()==QDialog::Accepted) {
            if (conf->getResult()==SAVE){
                if( !on_action_Save_Wallet_triggered())// call save
                    return; // save gone wrong, dont do anything
            }
            else if(conf->getResult()==DISCARD){} // continue without saving
        }
    }

    // Call SecureFileDialog. Option 0 displays the cyphered files in the current directory
    SecureFileDialog *fileDialog = new SecureFileDialog(this, 0, client);
    fileDialog->exec();
    if (fileDialog->result()==QDialog::Rejected) {
        return;
    }
    path = fileDialog->getChosenPath();
    fileName = fileDialog->getChosenFile();
    // fileName = QFileDialog::getOpenFileName(this, "select filename", QCoreApplication::applicationDirPath(), "Sqlite files(*.sqlite *.db)");
    qWarning() << "PATH: " << path;
    qWarning() << "FILE: " << fileName;

    setAllEnabled(false); // open process takes a while, so disable all buttons.
    qApp->processEvents(); // otherwise the repaint take place after this function finishes

    // Open the wallet. If the operation succeded, the wallet object contains the ritrieved data.
    SEwallet_Wallet_Open_Params params;
    params.wallet.setPath(path.toStdString());
    params.wallet.setFileName(fileName.toStdString());
    if (client->txrx(SEwalletCommand::OPEN_WALLET, &params) == 0) {
        if (params.retvalue == 0) {
            qDebug() << "Wallet successfully opened.";
            ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
            ui->messageLabel->setStyleSheet("color: rgb(0, 255, 0)");
            ui->messageLabel->setText("Wallet successfully opened.");
        } else {
            qDebug() << QString::fromStdString(params.errmessage);
            qDebug() << params.sqlretvalue;
            ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
            ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
            ui->messageLabel->setText(QString::fromStdString(params.errmessage));
            setAllEnabled(true);
            return;
        }
    } else {
        qDebug() << "Protocol comunication error. Error opening the wallet.";
        ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
        ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
        ui->messageLabel->setText("Protocol comunication error. Error opening the wallet.");
        setAllEnabled(true);
        return;
    }

    // Close the current in-memory database
    closeCurrentInMemoryDatabase();

    // Create new in-merory database
    if (createNewInMemoryDatabase()) {
        qWarning() << "ERROR: " << dbMem.lastError();
        ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
        ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
        ui->messageLabel->setText("Error creating in memory wallet.");
        setAllEnabled(true);
        return; //TODO: in this return, and all of them, check if query.finish() and similar functions are beiing executed.
    }

    query = QSqlQuery(dbMem);

    // Fill in-memory database with retrieved data
    for (Wallet::Table table : params.wallet.getTables()) {
         qDebug() << params.wallet.getTables().size();

        // Create the table in the in-momery database
        QString tableName = QString::fromStdString(table.getName());
        QString sql = "create table '" + tableName +
                      "'(id integer primary key, "
                      "Username TEXT, "
                      "Domain TEXT, "
                      "Password TEXT, "
                      "Date TEXT, "
                      "Description TEXT );";
        if (tableName!="NoEmpty") {
            query.prepare(sql);
            if (!query.exec()){
                qWarning() << "Couldn't create the table" << tableName <<" one might already exist";
                setAllEnabled(true);
                return;
            }
            qDebug() << tableName << "created successfully";
        }

        // Insert entries in the current table of the in-memory database
        for (Wallet::Table::Entry entry : table.getEntries()) {

            QString username = QString::fromStdString(entry.getUsername());
            QString domain = QString::fromStdString(entry.getDomain());
            QString password = QString::fromStdString(entry.getPassword());
            QString date = QString::fromStdString(entry.getDate());
            QString description = QString::fromStdString(entry.getDescription());
            qDebug() << "(" << username << "," << domain << "," << password << "," << date << "," << description << ")";


            QString sql = "insert into '" + tableName +
                          "'('Username', "
                          "  'Domain', "
                          "  'Password', "
                          "  'Date', "
                          "  'Description') "
                          " values ('" + username    + "','" +
                                         domain      + "','" +
                                         password    + "','" +
                                         date        + "','" +
                                         description + "');";
            query.prepare(sql);
            if (!query.exec()){
                qWarning() << "Couldn't create the entry in " << tableName;
                setAllEnabled(true);
                return;
            }
            qDebug() << "Entry created successfully in " << tableName;
        }
    }

    query.finish();
    setAllEnabled(true);
    ui->action_Add_Table->setEnabled(true);
    ui->action_Save_Wallet_As->setEnabled(true);
    ui->action_Save_Wallet->setEnabled(false);
    ui->action_Close_Wallet->setEnabled(true);

    walletName=QFileInfo(QFile(fileName)).absoluteFilePath().remove(".sqlite");
    displayWalletName->setText(walletName);

    tableList->clear();

    QStringList tablesDBMem = dbMem.tables(QSql::Tables);
    //    tablesDBMem.removeAll("NoEmpty"); //not necessary anymore

    if (!tablesDBMem.isEmpty()){
        ui->filtersWidget->setVisible(true);
        if (filters==NULL)
            createTableView(tablesDBMem.last());
        tableList->setEnabled(true);
        tableList->addItems(tablesDBMem);

        ui->action_Delete_Table->setEnabled(true);
        ui->action_Rename_Table->setEnabled(true);

        ui->action_Add_Entry->setEnabled(true);
        ui->action_Edit_Entry->setEnabled(true);
        ui->action_Delete_Entry->setEnabled(true);
        ui->action_Launch_Domain->setEnabled(true);
        ui->action_Show_Passwords->setEnabled(true);
        ui->action_Show_Passwords->setChecked(false);
        ui->action_Older_Than_Filter->setEnabled(true);
        ui->action_Older_Than_Filter->setChecked(false);
        ui->action_Fit_Table->setEnabled(true);
    }
}

/**
 * @brief MainWindow::callback_createTableList
 * @param argc
 * @param argv
 * @param azColName
 * @return
 */
int MainWindow::callback_createTableList(int argc, char **argv, char **azColName) { //Build TableList from ciphered db
    qDebug() << "in callback_createTableList";
    tables << argv[0]; //only one arg, the table name.
    UNUSED(argc);
    UNUSED(azColName);
    return 0;
}

/**
 * @brief MainWindow::callback_populateTable
 * @param argc
 * @param argv
 * @param azColName
 * @return
 */
int MainWindow::callback_populateTable(int argc, char **argv, char **azColName)  { //fill Table in in-mem with data from ciphered db

    if (currentTable=="NoEmpty") // we dont want NoEmpty in the in-mem db
        return 0;
    int i;
    UNUSED(azColName);

    static const QString insert = QStringLiteral("INSERT INTO '%1' VALUES (%2);"); //Insert statement
    QStringList values;
    QString aux;

    for(i = 0; i<argc; i++){
        aux = argv[i];
        values << "'"+aux+"'";
    }

   query.prepare( insert.arg(currentTable).arg(values.join(", ")) );
   if (!query.exec()){
        qWarning() << "Couldn't insert values";
        qWarning() << "ERROR: " << query.lastError();
        return 0;
    }
    return 0;
}

/** The user wants to close a wallet.
 *
 * @brief MainWindow::on_action_Close_Wallet_triggered
 */
void MainWindow::on_action_Close_Wallet_triggered() {

    // Check if there are unsaved changes
    if(ui->action_Save_Wallet->isEnabled()) { // there are unsaved changes.
        SaveConfirmation * conf = new SaveConfirmation;
        conf->exec();
        if(conf->result()==QDialog::Rejected) {
            QD << "cancel";
            return; // if error or cancel, do nothing
        }
        else if (conf->result()==QDialog::Accepted) {
            if (conf->getResult()==SAVE) {
                QD << "save";
                if( !on_action_Save_Wallet_triggered()) // call save
                    return; // save gone wrong, dont do anything
            }
            else if(conf->getResult()==DISCARD) // continue without saving
                QD << "discard";
        }
    }

    // Close current in-memory database
    closeCurrentInMemoryDatabase();

    // State of some actions as not clickable because there is no wallet to edit yet
    ui->action_Save_Wallet->setEnabled(false);
    ui->action_Save_Wallet_As->setEnabled(false);
    ui->action_Close_Wallet->setEnabled(false);
    ui->filtersWidget->setVisible(false);
    ui->action_Add_Table->setEnabled(false);
    ui->action_Delete_Table->setEnabled(false);
    ui->action_Rename_Table->setEnabled(false);
    ui->action_Add_Entry->setEnabled(false);
    ui->action_Edit_Entry->setEnabled(false);
    ui->action_Delete_Entry->setEnabled(false);
    ui->action_Show_Passwords->setEnabled(false);
    ui->action_Older_Than_Filter->setEnabled(false);
    ui->action_Launch_Domain->setEnabled(false);
    ui->action_Fit_Table->setEnabled(false);

    tableList->clear();

    // Close wallet succeeded
    displayWalletName->setText("");
    ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
    ui->messageLabel->setStyleSheet("color: rgb(0, 255, 0)");
    ui->messageLabel->setText("The wallet was successfully closed.");
}

/** The user wants to delete a wallet. If there is an opened wallet, delete it, if not, then open select wallet dialog.
 *
 * @brief MainWindow::on_action_Delete_Wallet_triggered
 */
void MainWindow::on_action_Delete_Wallet_triggered() {

    if (ui->action_Add_Table->isEnabled()) { // this means there is a wallet opened, so delete the in-mem DB
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete", "Are you sure you want to delete this Wallet", QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        }
        // Close the current in-memory database
        closeCurrentInMemoryDatabase();
    } else {
        // Call securefileDialog. Option 0 displays the cyphered files in the current directory
        SecureFileDialog *fileDialog = new SecureFileDialog(this, 0, client);
        fileDialog->exec();
        if(fileDialog->result()==QDialog::Rejected) {
            return ;
        }
        // Set path and filename to be deleted
        path = fileDialog->getChosenPath();
        fileName = fileDialog->getChosenFile();
        qWarning() << "PATH: " << path;
        qWarning() << "FILE: " << fileName;
    }

    // If no in-memory table was deleted and no fileName to delete, nothing more to do
    if (fileName.isEmpty() && !ui->action_Add_Table->isEnabled())
        return;
    if (!fileName.isEmpty()){

        // Delete the wallet (in disk), either the current wallet or the one chosen
        SEwallet_Wallet_Delete_Params params;
        params.wallet.setPath(path.toStdString());
        params.wallet.setFileName(fileName.toStdString());

        if (client->txrx(SEwalletCommand::DELETE_WALLET, &params) == 0) {
            if (params.retvalue == 0) {
                statusBar()->showMessage(QFileInfo(QFile(fileName)).absoluteFilePath()+" Deleted successfully", 2000);
                ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
                ui->messageLabel->setStyleSheet("color: rgb(0, 255, 0)");
                ui->messageLabel->setText(fileName +  " was successfully deleted.");
            } else {
                statusBar()->showMessage(QFileInfo(QFile(fileName)).absoluteFilePath()+" not deleted", 2000);
                ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
                ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
                ui->messageLabel->setText(QString::fromStdString(params.errmessage));
            }
        } else {
            qDebug() << "Protocol comunication error. Error deleting the wallet.";
            ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
            ui->messageLabel->setStyleSheet("color: rgb(255, 102, 102)");
            ui->messageLabel->setText("Protocol comunication error. Error deleting the wallet.");
            //setAllEnabled(true);
            return;
        }

    } else { // Unnamed wallet, deleted only in the in-mem DB
        ui->currentDateTimeLabel->setText(dateTime.currentDateTime().toString("[yyyy-MM-dd hh-mm-ss] "));
        ui->messageLabel->setStyleSheet("color: rgb(0, 255, 0)");
        ui->messageLabel->setText("Unnamed wallet deleted successfully.");
    }
    displayWalletName->clear();

    ui->action_Save_Wallet->setEnabled(false);
    ui->action_Save_Wallet_As->setEnabled(false);
    ui->action_Close_Wallet->setEnabled(false);
    ui->filtersWidget->setVisible(false);

    ui->action_Add_Table->setEnabled(false);
    ui->action_Delete_Table->setEnabled(false);
    ui->action_Rename_Table->setEnabled(false);

    ui->action_Add_Entry->setEnabled(false);
    ui->action_Edit_Entry->setEnabled(false);
    ui->action_Delete_Entry->setEnabled(false);
    ui->action_Show_Passwords->setEnabled(false);
    ui->action_Older_Than_Filter->setEnabled(false);
    ui->action_Launch_Domain->setEnabled(false);
    ui->action_Fit_Table->setEnabled(false);
    tableList->clear();
}

/** Update user filter.
 *
 * @brief MainWindow::userFilter_textChanged
 * @param arg1
 */
void MainWindow::userFilter_textChanged(const QString &arg1) {
    proxyModel->setFilterUser(arg1);
}

/** Update domain filter.
 *
 * @brief MainWindow::domainFilter_textChanged
 * @param arg1
 */
void MainWindow::domainFilter_textChanged(const QString &arg1) {
    proxyModel->setFilterDomain(arg1);
}

/** Update password filter.
 *
 * @brief MainWindow::passFilter_textChanged
 * @param arg1
 */
void MainWindow::passFilter_textChanged(const QString &arg1) {
    proxyModel->setFilterPass(arg1);
}

/** Update description filter.
 *
 * @brief MainWindow::descFilter_textChanged
 * @param arg1
 */
void MainWindow::descFilter_textChanged(const QString &arg1) {
    proxyModel->setFilterDesc(arg1);
}

/**
 * @brief MainWindow::dateUnit_currentIndexChanged
 * @param index
 */
void MainWindow::dateUnit_currentIndexChanged(int index) {
    proxyModel->setFilterOlder(index, filters->dateFilter->text(), QDate::fromString("-1"));
}

/** Update date filter.
 *
 * @brief MainWindow::dateFilter_textChanged
 * @param arg1
 */
void MainWindow::dateFilter_textChanged(const QString &arg1) {
    proxyModel->setFilterOlder(filters->dateUnit->currentIndex(), arg1, QDate::fromString("-1"));
}

/**
 * @brief MainWindow::dateChoose_dateChanged
 * @param date
 */
void MainWindow::dateChoose_dateChanged(QDate date) {
    proxyModel->setFilterOlder(-1, "", date);
}

/**
 * @brief MainWindow::dateCheck_toggled
 * @param checked
 */
void MainWindow::dateCheck_toggled(bool checked) {
    filters->dateChoose->setEnabled(checked);
    if (checked)
        proxyModel->setFilterOlder(-1, "", filters->dateChoose->date());
    else
        proxyModel->setFilterOlder(0, "", QDate::fromString("-1"));
}

/**
 * @brief MainWindow::on_action_Older_Than_Filter_toggled
 * @param older
 */
void MainWindow::on_action_Older_Than_Filter_toggled(bool older) {
    if (older) {
        filters->dateChoose->setVisible(false);
        filters->dateCheck->setVisible(false);
        filters->dateFilter->setVisible(true);
        filters->dateUnit->setVisible(true);
        filters->dateLayout->setContentsMargins(0, 0, 0,1);

        proxyModel->setFilterOlder(filters->dateUnit->currentIndex(),
                                   filters->dateFilter->text(),
                                   QDate::fromString("-1"));
    } else {
        filters->dateChoose->setVisible(true);
        filters->dateCheck->setVisible(true);
        filters->dateFilter->setVisible(false);
        filters->dateUnit->setVisible(false);
        filters->dateLayout->setContentsMargins(1, 0, 0, 0);

        if (filters->dateCheck->isChecked())
            proxyModel->setFilterOlder(-1, "", filters->dateChoose->date());
        else
            proxyModel->setFilterOlder(0, "", QDate::fromString("-1"));
    }
}

/** The user wants to open Help Menu.
 *
 * @brief MainWindow::on_action_About_triggered
 */
void MainWindow::on_action_About_triggered() {
    helpWindow *help = new helpWindow(this);
    help->show();
}

/** The user wants to open the Preferences Dialogue.
 *
 * @brief MainWindow::on_action_Preferences_triggered
 */
void MainWindow::on_action_Preferences_triggered() {
    PreferencesDialog *pref = new PreferencesDialog(this);
    pref->exec();
}

/** Enable globally all the widgets.
 *
 * @brief MainWindow::setAllEnabled
 * @param enabled
 */
void MainWindow::setAllEnabled(bool enabled) {
    ui->centralWidget->setEnabled(enabled);
    ui->menuBar->setEnabled(enabled);
    ui->dataBaseTB->setEnabled(enabled);
    ui->tableTB->setEnabled(enabled);
    ui->entriesTB->setEnabled(enabled);
}
