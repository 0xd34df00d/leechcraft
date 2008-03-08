#ifndef FEED_H
#define FEED_H
#include <QString>
#include <QDateTime>
#include <QList>

struct Feed
{
    QString URL_;
    QDateTime LastUpdate_;
    QList<Channel*> Channels_;
};

Q_DECLARE_METATYPE (Feed);

bool operator< (const Feed& f1, const Feed& f2);
QDataStream& operator<< (QDataStream&, const Feed&);
QDataStream& operator>> (QDataStream&, Feed&);

#endif

