#include "dictlist.h"
#include "ui_dictlist.h"

/*
 * Constructor
 */
dictList::dictList(QWidget *parent, QStringList genDict, QString path) : QDialog(parent), ui(new Ui::dictList) {
    ui->setupUi(this);
    setWindowTitle("Choose which dictionaries you want to use");
    setModal(true);

    if(path.isEmpty()) //if no entered path => zxcvbn path
        path = QCoreApplication::applicationDirPath().append("/../../SEcubeWallet/zxcvbn/");

    QDir myDir(path);
    QStringList filters;
    filters<<"words*.txt";
    QStringList strList = myDir.entryList(filters);

    ui->lw_dict->addItems(strList);

    QListWidgetItem* item = 0;
    for(int i = 0; i < ui->lw_dict->count(); ++i){
        item = ui->lw_dict->item(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        if (genDict.contains(item->text()))
            item->setCheckState(Qt::Checked);
        else
            item->setCheckState(Qt::Unchecked);
    }
}

/*
 * Destructor
 */
dictList::~dictList() {
    delete ui;
}

/*
 *
 */
QStringList dictList::getChecked() {

    QStringList checked;
    QListWidgetItem* item = 0;
    for(int i = 0; i < ui->lw_dict->count(); ++i){
        item = ui->lw_dict->item(i);
        if (item->checkState())
            checked << item->text();
    }

    return checked;
}
