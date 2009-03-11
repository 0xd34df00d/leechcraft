#ifndef ADDFEED_H
#define ADDFEED_H
#include <QDialog>
#include "ui_addfeed.h"

class AddFeed : public QDialog, private Ui::AddFeed
{
    Q_OBJECT
public:
    AddFeed (const QString& = QString (), QWidget *parent = 0);
    QString GetURL () const;
    QStringList GetTags () const;
};

#endif

