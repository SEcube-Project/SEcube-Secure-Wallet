#include "securefiledialog.h"

/** Constructor
 *
 * @brief SecureFileDialog::SecureFileDialog
 * @param parent
 * @param newFile
 * @param client
 */
SecureFileDialog::SecureFileDialog(QWidget *parent, int newFile, SEwalletClient *client) : QDialog(parent) {
    this->client = client;
    oldPath = QDir::currentPath();
    currentPath = QDir::currentPath();
    this->newFile = newFile;
    setUpGUI();
    if (!newFile) {
        setWindowTitle(tr("Choose Wallet"));
    } else {
        setWindowTitle(tr("Create New Wallet"));
    }
    setModal(true);
}

/** Destructor
 *
 * @brief SecureFileDialog::~SecureFileDialog
 */
SecureFileDialog::~SecureFileDialog() {
    QDir::setCurrent(oldPath.absolutePath());
}

/** Setup GUI
 *
 * @brief SecureFileDialog::setUpGUI
 */
void SecureFileDialog::setUpGUI() {

    // Set up the layout
    //QGridLayout* formGridLayout = new QGridLayout( this );
    formGridLayout = new QGridLayout(this);

    // Initialize buttons
    buttons = new QDialogButtonBox(this);
    buttons->addButton(QDialogButtonBox::Ok);
    buttons->addButton(QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Ok"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("Abort"));
    buttons->setCenterButtons(true);

    choosePath = new QLineEdit(this);
    labelCurrentPath = new QLabel(this);
    labelFileView = new QLabel(this);
    browsePath = new QFileDialog(this);
    openBrowse = new QDialogButtonBox(this);
    openBrowse->addButton(QString("Browse"), QDialogButtonBox::AcceptRole);

    labelCurrentPath->setText(tr("Path:"));
    labelCurrentPath->setBuddy(choosePath);

    browsePath->setFileMode(QFileDialog::DirectoryOnly);
    if(newFile) {
        labelFileView->setText(tr("Wallet name:"));
        chooseNewFile = new QLineEdit (this);
        labelFileView->setBuddy(chooseNewFile );
        formGridLayout->addWidget(chooseNewFile, 1, 1);

        chooseFile = new QListView (this);
        chooseFile->setViewMode(QListView::ListMode);
        chooseFile->setEditTriggers(QListView::NoEditTriggers);
        connect (chooseFile, SIGNAL(clicked(QModelIndex)), this, SLOT(updateNewFile()));
        labelFileView2=new QLabel(this);
        labelFileView2->setText(tr("Wallets:"));
        labelFileView2->setBuddy(chooseFile);
        formGridLayout->addWidget(labelFileView2,2,0);
        formGridLayout->addWidget(chooseFile,2,1);
        formGridLayout->addWidget( buttons, 3, 0, 1, 3);
    } else {
        labelFileView->setText(tr("Wallets:"));
        chooseFile = new QListView(this);
        chooseFile->setViewMode(QListView::ListMode);
        labelFileView->setBuddy(chooseFile);
        connect(chooseFile, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotAcceptFile()));
        formGridLayout->addWidget( chooseFile, 1, 1);
        formGridLayout->addWidget( buttons, 2, 0, 2, 2);
    }

    // todo - choose which and how many column we want
    refreshFileView();

    // Connects slots
    connect(buttons->button(QDialogButtonBox::Cancel),SIGNAL (clicked()), this, SLOT(slotAbortFile()));
    connect(buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(slotAcceptFile()));
    connect(choosePath, SIGNAL(editingFinished()), this, SLOT(refreshFileView()));
    connect(openBrowse->buttons().at(0), SIGNAL(clicked(bool)), browsePath, SLOT(exec()));
    connect(browsePath, SIGNAL(accepted()), this, SLOT(refreshFileView()));

    // Place components into the dialog
    formGridLayout->addWidget(labelCurrentPath, 0, 0);
    formGridLayout->addWidget(choosePath, 0, 1);
    formGridLayout->addWidget(openBrowse, 0, 2);
    formGridLayout->addWidget(labelFileView, 1, 0);

    setLayout(formGridLayout);
}

/**
 *
 * @brief SecureFileDialog::slotAcceptFile
 */
void SecureFileDialog::slotAcceptFile() {

    // Set the filename
    if (!newFile) {
        if (chooseFile->currentIndex().row() == -1) {
            reject();
            return;
        }
        chosenFile = listFiles->at(chooseFile->currentIndex().row());
//        if (choosePath->text().indexOf("/") != -1) {
//            chosenFile = choosePath->text() + "/" + listFiles->at(chooseFile->currentIndex().row());
//        } else if (choosePath->text().indexOf("\\") != -1) {
//            chosenFile = choosePath->text() + "\\" + listFiles->at(chooseFile->currentIndex().row());
//        } else {
//            chosenFile = listFiles->at(chooseFile->currentIndex().row());
//        }
        accept();
    } else {
        if (!chooseNewFile->text().isEmpty()) {
            if (chooseNewFile->text().endsWith(QString(".sqlite")))
                chosenFile = chooseNewFile->text();
            else
                chosenFile = chooseNewFile->text().append(".sqlite");
            accept();
        }
    }

    // Set the path
    chosenPath = choosePath->text().append("/");
}

/** Get the chosen path.
 *
 * @brief SecureFileDialog::getChosenPath
 * @return
 */
QString SecureFileDialog::getChosenPath() {
    return chosenPath;
}

/** Get the chosen file.
 *
 * @brief SecureFileDialog::getChosenFile
 * @return
 */
QString SecureFileDialog::getChosenFile() {
    return chosenFile;
}

/** Refresh the dialog view
 *
 * @brief SecureFileDialog::refreshFileView
 */
void SecureFileDialog::refreshFileView() {

    // Set the chosen path
    if (browsePath->result() == QDialog::Accepted) {
        currentPath.setCurrent(browsePath->selectedFiles().at(0));
        choosePath->setText(QDir::currentPath().append("/"));
    } else {
        currentPath.setCurrent(choosePath->text());
        choosePath->setText(QDir::currentPath().append("/"));
    }

    // Get the list of encrypted files in the chosen directory
    SEwallet_File_GetList_Params param;
    param.path = choosePath->text().toStdString();

    if (client->txrx(SEwalletCommand::GET_FILE_LIST, &param) == 0) {
        if (param.retvalue == 0) {
            listFiles = new QStringList();
            for (auto i = param.list.begin(); i != param.list.end(); ++i) {
                listFiles->append(QString::fromStdString(*i));
            }
            QStringListModel* model = new QStringListModel(*listFiles);
            chooseFile->setModel((QAbstractItemModel*)model);
            updateGeometry();
        } else {
            logger(param.errmessage);
        }
    } else {
        logger("Protocol transmission error. Encrypted file list not available.");
    }
}

/** Update new file

 * @brief SecureFileDialog::updateNewFile
 */
void SecureFileDialog::updateNewFile() {
    QString aux;
    if (chooseFile->currentIndex().row() == -1) {
        reject();
        return;
    }
    //  chosenFile = listFiles->at(chooseFile->currentIndex().row());
    if(choosePath->text().indexOf("/") != -1)
        aux = choosePath->text() +"/" + listFiles->at(chooseFile->currentIndex().row());
    else if (choosePath->text().indexOf("\\") != -1)
        aux = choosePath->text() +"\\" + listFiles->at(chooseFile->currentIndex().row());
    else
        aux = listFiles->at(chooseFile->currentIndex().row());
    chooseNewFile->setText(QFileInfo(QFile(aux)).fileName().remove(".sqlite"));
}

/** The user do not want to perfom the operation

 * @brief SecureFileDialog::slotAbortFile
 */
void SecureFileDialog::slotAbortFile() {
    reject();
}
