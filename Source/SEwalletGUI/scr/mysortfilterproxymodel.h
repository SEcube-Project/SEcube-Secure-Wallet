#ifndef MYSORTFILTERPROXYMODEL_H
#define MYSORTFILTERPROXYMODEL_H

#include <QDate>
#include <QSortFilterProxyModel>
#include <QtWidgets>

#define USER_COL    1
#define DOM_COL     2
#define PASS_COL    3
#define DATE_COL    4
#define DESC_COL    5

class MySortFilterProxyModel : public QSortFilterProxyModel {

    Q_OBJECT

public:

    MySortFilterProxyModel(QObject *parent = 0);

    void setFilterUser(const QString &regExp);
    void setFilterDomain(const QString &regExp);
    void setFilterPass(const QString &regExp);
    void setFilterDesc(const QString &regExp);
    void setFilterOlder(int index, const QString &QuantityS, QDate date);

private:

    QRegExp userRegExp;
    QRegExp domainRegExp;
    QRegExp passRegExp;
    QRegExp descRegExp;
    QDate   filterDate_older;
    QDate   filterDate_exact;
    QString format;

protected:

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

};

#endif // MYSORTFILTERPROXYMODEL_H
