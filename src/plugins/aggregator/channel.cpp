#include <QtDebug>
#include <QDataStream>
#include <QVariant>
#include <QStringList>
#include "channel.h"
#include "item.h"

Channel::Channel ()
{
}

Channel::Channel (const Channel& channel)
: Title_ (channel.Title_)
, Link_ (channel.Link_)
, Description_ (channel.Description_)
, LastBuild_ (channel.LastBuild_)
, Tags_ (channel.Tags_)
, Items_ (channel.Items_)
{
}

Channel::~Channel ()
{
}

Channel& Channel::operator= (const Channel& channel)
{
    Title_ = channel.Title_;
    Link_ = channel.Link_;
    Description_ = channel.Description_;
    LastBuild_ = channel.LastBuild_;
    Tags_ = channel.Tags_;
    Items_ = channel.Items_;
	return *this;
}

int Channel::CountUnreadItems () const
{
    int result = 0;
    for (size_t i = 0; i < Items_.size (); ++i)
        result += (Items_ [i]->Unread_);
    return result;
}

bool operator== (const Channel& c1, const Channel& c2)
{
    return c1.Title_ == c2.Title_ && c1.Link_ == c2.Link_;
}

QDataStream& operator<< (QDataStream& out, const Channel& chan)
{
    out << chan.Title_
        << chan.Link_
        << chan.Description_
        << chan.Tags_
        << chan.LastBuild_
        << chan.Language_
        << chan.Author_
        << chan.Pixmap_
        << static_cast<quint32> (chan.Items_.size ());
    for (size_t i = 0; i < chan.Items_.size (); ++i)
        out << *chan.Items_ [i];
    return out;
}

QDataStream& operator>> (QDataStream& in, Channel& chan)
{
    quint32 size;
    in >> chan.Title_
        >> chan.Link_
        >> chan.Description_
        >> chan.Tags_
        >> chan.LastBuild_
        >> chan.Language_
        >> chan.Author_
        >> chan.Pixmap_;
    in >> size;
    chan.Items_.reserve (size);
    for (size_t i = 0; i < size; ++i)
    {
        boost::shared_ptr<Item> it (new Item);
        in >> *it;
        chan.Items_.push_back (it);
    }
    return in;
}

