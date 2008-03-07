#ifndef ITEM_H
#define ITEM_H
#include <QString>
#include <QDateTime>
#include "channel.h"

struct Item
{
    Channel Parent_;
    QString Title_;
    QString Link_;
    QString Description_;
    QString Author_;
    QString Category_;
    QString Comments_;
    QString Guid_;
    QDateTime PubDate_;
};

bool operator== (const Item&, const Item&);
bool operator< (const Item&, const Item&);

#endif

