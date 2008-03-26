#ifndef CHANNEL_H
#define CHANNEL_H
#include <QString>
#include <QList>
#include <QDateTime>
#include <QStringList>
#include <QPixmap>
#include <boost/shared_ptr.hpp>
#include <vector>

class Item;

struct Channel
{
    QString Title_;
    QString Link_;
    QString Description_;
    QDateTime LastBuild_;
    QStringList Tags_;
    QString Language_;
    QString Author_;
    QPixmap Pixmap_;
    std::vector<boost::shared_ptr<Item> > Items_;

    Channel ();
    Channel (const Channel&);
    Channel& operator= (const Channel&);

    int CountUnreadItems () const;
};

bool operator== (const Channel&, const Channel&);
QDataStream& operator<< (QDataStream&, const Channel&);
QDataStream& operator>> (QDataStream&, Channel&);

#endif

