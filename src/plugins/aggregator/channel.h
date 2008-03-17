#ifndef CHANNEL_H
#define CHANNEL_H
#include <QString>
#include <QList>
#include <QDateTime>

class Item;

struct Channel
{
    QString Title_;
    QString Link_;
    QString Description_;
    QDateTime LastBuild_;
    QList<Item*> Items_;
};

bool operator== (const Channel&, const Channel&);
QDataStream& operator<< (QDataStream&, const Channel&);
QDataStream& operator>> (QDataStream&, Channel&);

#endif

