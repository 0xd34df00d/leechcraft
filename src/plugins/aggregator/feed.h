#ifndef FEED_H
#define FEED_H
#include <QString>
#include <QDateTime>
#include <QList>
#include <QMetaType>
#include <boost/shared_ptr.hpp>
#include <vector>

class Channel;

struct Feed
{
    QString URL_;
    QDateTime LastUpdate_;
    std::vector<boost::shared_ptr<Channel> > Channels_;

    Feed ();
    Feed (const Feed&);
    Feed& operator= (const Feed&);
};

Q_DECLARE_METATYPE (Feed);

bool operator< (const Feed& f1, const Feed& f2);
QDataStream& operator<< (QDataStream&, const Feed&);
QDataStream& operator>> (QDataStream&, Feed&);

#endif

