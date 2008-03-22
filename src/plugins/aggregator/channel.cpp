#include <QtDebug>
#include <QDataStream>
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
{
    for (int i = 0; i < channel.Items_.size (); ++i)
        Items_.append (channel.Items_.at (i));
}

Channel& Channel::operator= (const Channel& channel)
{
    Title_ = channel.Title_;
    Link_ = channel.Link_;
    Description_ = channel.Description_;
    LastBuild_ = channel.LastBuild_;
    for (int i = 0; i < channel.Items_.size (); ++i)
        Items_.append (channel.Items_.at (i));
}

bool operator== (const Channel& c1, const Channel& c2)
{
    return c1.Title_ == c2.Title_ &&
        c1.Link_ == c2.Link_;
}

QDataStream& operator<< (QDataStream& out, const Channel& chan)
{
    out << chan.Title_
        << chan.Link_
        << chan.Description_
        << chan.LastBuild_
        << chan.Items_.size ();
    for (int i = 0; i < chan.Items_.size (); ++i)
        out << *chan.Items_.at (i);
    return out;
}

QDataStream& operator>> (QDataStream& in, Channel& chan)
{
    int size;
    in >> chan.Title_
        >> chan.Link_
        >> chan.Description_
        >> chan.LastBuild_;
    in >> size;
    for (int i = 0; i < size; ++i)
    {
        Item *it = new Item;
        in >> *it;
        chan.Items_.append (it);
    }
    return in;
}

