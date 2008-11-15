#ifndef ITEM_H
#define ITEM_H
#include <vector>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QMetaType>
#include <boost/shared_ptr.hpp>

struct ItemShort
{
	QString Title_;
	QString URL_;
	QDateTime PubDate_;
	bool Unread_;
};

struct Item
{
    QString Title_;
    QString Link_;
    QString Description_;
    QString Author_;
    QStringList Categories_;
    QString Guid_;
    QDateTime PubDate_;
    bool Unread_;
	int NumComments_;
	QString CommentsLink_;

	ItemShort ToShort () const;
};

typedef boost::shared_ptr<Item> Item_ptr;
typedef std::vector<Item_ptr> items_container_t;
typedef std::vector<ItemShort> items_shorts_t;

struct ItemComparator
{
	Item_ptr Item_;

	ItemComparator (const Item_ptr&);
	bool operator() (const Item_ptr&);
};

Q_DECLARE_METATYPE (Item);

bool operator== (const Item&, const Item&);
bool operator< (const Item&, const Item&);
QDataStream& operator<< (QDataStream&, const Item&);
QDataStream& operator>> (QDataStream&, Item&);

bool IsModified (Item_ptr, Item_ptr);

#endif

