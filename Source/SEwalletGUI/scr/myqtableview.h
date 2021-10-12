#ifndef MYQTABLEVIEW_H
#define MYQTABLEVIEW_H

#include <QTableView>
#include <QDebug>
#include <QHeaderView>
#define QD qDebug()

#define IDENT_COL   0
#define USER_COL    1
#define DOM_COL     2
#define PASS_COL    3
#define DATE_COL    4
#define DESC_COL    5

class MyQTableView : public QTableView {

    Q_OBJECT

public:

    MyQTableView(QWidget *parent = 0);

protected:

    void resizeEvent(QResizeEvent* event) override;

};

#endif // MYQTABLEVIEW_H
