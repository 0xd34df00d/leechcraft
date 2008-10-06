#ifndef ITEM_H
#define ITEM_H
#include <vector>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QMetaType>
#include <boost/shared_ptr.hpp>

struct Item
{
    QString Title_;
    QString Link_;
    QString Description_;
    QString Author_;
    QStringList Categories_;
    QString Comments_;
    QString Guid_;
    QDateTime PubDate_;
    bool Unread_;
};

typedef boost::shared_ptr<Item> Item_ptr;
typedef std::vector<Item_ptr> items_container_t;

Q_DECLARE_METATYPE (Item);

bool operator== (const Item&, const Item&);
bool operator< (const Item&, const Item&);
QDataStream& operator<< (QDataStream&, const Item&);
QDataStream& operator>> (QDataStream&, Item&);

#endif

