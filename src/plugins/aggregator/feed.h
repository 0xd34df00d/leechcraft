#ifndef FEED_H
#define FEED_H
#include <QString>
#include <QDateTime>
#include <QList>
#include <QMetaType>
#include <boost/shared_ptr.hpp>
#include <deque>

struct Channel;

struct Feed
{
    QString URL_;
    QDateTime LastUpdate_;
	typedef std::deque<boost::shared_ptr<Channel> > channels_container_t;
	channels_container_t Channels_;

    Feed ();
    Feed (const Feed&);
    Feed& operator= (const Feed&);
};

Q_DECLARE_METATYPE (Feed);

bool operator< (const Feed& f1, const Feed& f2);
QDataStream& operator<< (QDataStream&, const Feed&);
QDataStream& operator>> (QDataStream&, Feed&);

#endif

