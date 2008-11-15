#ifndef CHANNEL_H
#define CHANNEL_H
#include <QString>
#include <QList>
#include <QDateTime>
#include <QStringList>
#include <QPixmap>
#include <boost/shared_ptr.hpp>
#include <vector>
#include "item.h"

struct ChannelShort
{
	QString Title_;
	QString Link_;
	QStringList Tags_;
	QDateTime LastBuild_;
	QPixmap Favicon_;
	int Unread_;
	QString ParentURL_;
};

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
	QString ParentURL_;
	items_container_t Items_;

    Channel ();
    Channel (const Channel&);
	~Channel ();
    Channel& operator= (const Channel&);

    int CountUnreadItems () const;
	void Equalify (const Channel&);
	ChannelShort ToShort () const;
};

typedef boost::shared_ptr<Channel> Channel_ptr;
typedef std::vector<Channel_ptr> channels_container_t;
typedef std::vector<ChannelShort> channels_shorts_t;

bool operator< (const ChannelShort&, const ChannelShort&);
bool operator== (const ChannelShort&, const ChannelShort&);
bool operator== (const Channel_ptr&, const ChannelShort&);
bool operator== (const ChannelShort&, const Channel_ptr&);
bool operator== (const Channel&, const Channel&);
QDataStream& operator<< (QDataStream&, const Channel&);
QDataStream& operator>> (QDataStream&, Channel&);

#endif

