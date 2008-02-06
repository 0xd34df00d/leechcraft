#include <QTime>
#include "logshower.h"

LogShower::LogShower (QWidget *parent)
: QListWidget (parent)
{
}

void LogShower::AddDownloadMessage (const QString& msg)
{
    if (msg.isEmpty ())
        return;
    QListWidgetItem *it = new QListWidgetItem (QString ("[") + QTime::currentTime ().toString ("hh:mm:ss") + QString ("]  ") + msg);
    it->setBackground (QBrush (QColor (255, 0, 0, 50)));
    addItem (it);
    scrollToItem (it);
}

void LogShower::AddUploadMessage (const QString& msg)
{
    if (msg.isEmpty ())
        return;
    QListWidgetItem *it = new QListWidgetItem (QString ("[") + QTime::currentTime ().toString ("hh:mm:ss") + QString ("]  ") + msg);
    it->setBackground (QBrush (QColor (0, 255, 0, 50)));
    addItem (it);
    scrollToItem (it);
}

