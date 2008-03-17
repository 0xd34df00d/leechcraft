#include <QDataStream>
#include <QtDebug>
#include "feed.h"
#include "channel.h"

bool operator< (const Feed& f1, const Feed& f2)
{
    return f1.URL_ < f2.URL_;
}

QDataStream& operator<< (QDataStream& out, const Feed& feed)
{
    out << feed.URL_
        << feed.LastUpdate_
        << feed.Channels_.size ();
    for (int i = 0; i < feed.Channels_.size (); ++i)
        out << (*feed.Channels_.at (i));
    return out;
}

QDataStream& operator>> (QDataStream& in, Feed& feed)
{
    int size;
    in >> feed.URL_
        >> feed.LastUpdate_
        >> size;
    for (int i = 0; i < size; ++i)
    {
        Channel *channel = new Channel;
        in >> *channel;
        feed.Channels_.append (channel);
    }
    return in;
}

