#include <QtDebug>
#include <QDataStream>
#include "item.h"

ItemShort Item::ToShort () const
{
	ItemShort is =
	{
		Title_,
		Link_,
		Categories_,
		PubDate_,
		Unread_
	};
	return is;
}

ItemComparator::ItemComparator (const Item_ptr& item)
: Item_ (item)
{
}

bool ItemComparator::operator() (const Item_ptr& item)
{
	return *Item_ == *item;
}

bool operator== (const Item& i1, const Item& i2)
{
    return i1.Title_ == i2.Title_ &&
        i1.Link_ == i2.Link_;
}

QDataStream& operator<< (QDataStream& out, const Item& item)
{
	int version = 1;
    out << version
		<< item.Title_
        << item.Link_
        << item.Description_
        << item.Author_
        << item.Categories_
        << item.Guid_
        << item.PubDate_
        << item.Unread_
		<< item.NumComments_
		<< item.CommentsLink_
		<< item.CommentsPageLink_;
    return out;
}

QDataStream& operator>> (QDataStream& in, Item& item)
{
	int version = 0;
	in >> version;
	if (version == 1)
	{
		in >> item.Title_
			>> item.Link_
			>> item.Description_
			>> item.Author_
			>> item.Categories_
			>> item.Guid_
			>> item.PubDate_
			>> item.Unread_
			>> item.NumComments_
			>> item.CommentsLink_
			>> item.CommentsPageLink_;
		return in;
	}
	else
		return in;
}

bool IsModified (Item_ptr i1, Item_ptr i2)
{
	return !(i1->Title_ == i2->Title_ &&
			i1->Link_ == i2->Link_ &&
			i1->Description_ == i2->Description_ &&
			i1->Author_ == i2->Author_ &&
			i1->Categories_ == i2->Categories_ &&
			i1->Guid_ == i2->Guid_ &&
			i1->PubDate_ == i2->PubDate_ &&
			i1->NumComments_ == i2->NumComments_ &&
			i1->CommentsLink_ == i2->CommentsLink_ &&
			i1->CommentsPageLink_ == i2->CommentsPageLink_);
}

