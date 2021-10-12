#include "helpwindow.h"
#include "ui_helpwindow.h"

/*
 * Constructor
 */
helpWindow::helpWindow(QWidget *parent) : QDialog(parent), ui(new Ui::helpWindow) {
    ui->setupUi(this);
    setWindowTitle( tr("Help") );
}

/*
 * Destructor
 */
helpWindow::~helpWindow()
{
    delete ui;
}
