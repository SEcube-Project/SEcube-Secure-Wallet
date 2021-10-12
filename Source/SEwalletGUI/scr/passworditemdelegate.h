#ifndef PASSWORDITEMDELEGATE_H
#define PASSWORDITEMDELEGATE_H

#include <QStyledItemDelegate>

#define UNUSED(expr) (void)(expr)

class PasswordItemDelegate: public QStyledItemDelegate {

    Q_OBJECT

public:

    PasswordItemDelegate();
    explicit PasswordItemDelegate(QObject* parent = 0);
    virtual QString displayText(const QVariant & value, const QLocale & locale) const;

};

#endif // PASSWORDITEMDELEGATE_H
