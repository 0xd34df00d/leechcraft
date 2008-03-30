#include <QtDebug>
#include <QDataStream>
#include "item.h"

bool operator== (const Item& i1, const Item& i2)
{
    return i1.Title_ == i2.Title_ &&
        i1.Link_ == i2.Link_ &&
        i1.Guid_ == i2.Guid_;
}

bool operator< (const Item& i1, const Item& i2)
{
    return i1.Guid_ < i2.Guid_;
}

QDataStream& operator<< (QDataStream& out, const Item& item)
{
    out << item.Title_
        << item.Link_
        << item.Description_
        << item.Author_
        << item.Category_
        << item.Comments_
        << item.Guid_
        << item.PubDate_
        << item.Unread_;
    return out;
}

QDataStream& operator>> (QDataStream& in, Item& item)
{
    in >> item.Title_
        >> item.Link_
        >> item.Description_
        >> item.Author_
        >> item.Category_
        >> item.Comments_
        >> item.Guid_
        >> item.PubDate_
        >> item.Unread_;
    return in;
}

