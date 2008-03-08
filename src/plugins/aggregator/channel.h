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

    Channel& operator= (const Channel&);
};

bool operator== (const Channel&, const Channel&);

#endif

