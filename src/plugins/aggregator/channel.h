#ifndef CHANNEL_H
#define CHANNEL_H
#include <QString>
#include <QList>
#include <QDateTime>
#include <QStringList>
#include <QPixmap>
#include <boost/shared_ptr.hpp>
#include <deque>

struct Item;

struct Channel
{
    QString Title_;
    QString Link_;
    QString Description_;
    QDateTime LastBuild_;
    QStringList Tags_;
    QString Language_;
    QString Author_;
    QString PixmapURL_;
    QPixmap Pixmap_;
	QPixmap Favicon_;
	typedef std::deque<boost::shared_ptr<Item> > items_container_t;
    items_container_t Items_;

    Channel ();
    Channel (const Channel&);
	~Channel ();
    Channel& operator= (const Channel&);

    int CountUnreadItems () const;
	void Equalify (const Channel&);
};

bool operator== (const Channel&, const Channel&);
QDataStream& operator<< (QDataStream&, const Channel&);
QDataStream& operator>> (QDataStream&, Channel&);

#endif

