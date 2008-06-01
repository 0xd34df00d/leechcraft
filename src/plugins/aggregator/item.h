#ifndef ITEM_H
#define ITEM_H
#include <QString>
#include <QDateTime>
#include <QMetaType>

struct Item
{
    QString Title_;
    QString Link_;
    QString Description_;
    QString Author_;
    QString Category_;
    QString Comments_;
    QString Guid_;
    QDateTime PubDate_;
    bool Unread_;
};

Q_DECLARE_METATYPE (Item);

bool operator== (const Item&, const Item&);
bool operator< (const Item&, const Item&);
QDataStream& operator<< (QDataStream&, const Item&);
QDataStream& operator>> (QDataStream&, Item&);

#endif

