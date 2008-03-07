#ifndef FEED_H
#define FEED_H
#include <QString>
#include <QDateTime>
#include <QList>

struct Feed
{
    QString URL_;
    QByteArray Previous_;
    QDateTime LastUpdate_;
    QList<Item> Items_;
};

bool operator< (const Feed& f1, const Feed& f2);

#endif

