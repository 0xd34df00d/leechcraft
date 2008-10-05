#ifndef FEED_H
#define FEED_H
#include <QString>
#include <QDateTime>
#include <QList>
#include <QMetaType>
#include <boost/shared_ptr.hpp>
#include <vector>
#include "channel.h"

struct Feed
{
    QString URL_;
    QDateTime LastUpdate_;
	channels_container_t Channels_;

    Feed ();
    Feed (const Feed&);
    Feed& operator= (const Feed&);
};

typedef boost::shared_ptr<Feed> Feed_ptr;
typedef std::vector<Feed_ptr> feeds_container_t;

Q_DECLARE_METATYPE (Feed);

bool operator< (const Feed& f1, const Feed& f2);
QDataStream& operator<< (QDataStream&, const Feed&);
QDataStream& operator>> (QDataStream&, Feed&);

#endif

