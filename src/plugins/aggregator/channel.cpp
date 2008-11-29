#include <QtDebug>
#include <QDataStream>
#include <QVariant>
#include <QStringList>
#include "channel.h"
#include "item.h"

Channel::Channel ()
{
}

Channel::Channel (const Channel& channel)
: Items_ (channel.Items_)
{
	Equalify (channel);
}

Channel::~Channel ()
{
}

Channel& Channel::operator= (const Channel& channel)
{
	Equalify (channel);
    Items_ = channel.Items_;
	return *this;
}

int Channel::CountUnreadItems () const
{
    int result = 0;
    for (size_t i = 0; i < Items_.size (); ++i)
        result += (Items_ [i]->Unread_);
    return result;
}

void Channel::Equalify (const Channel& channel)
{
    Title_ = channel.Title_;
    Link_ = channel.Link_;
    Description_ = channel.Description_;
    LastBuild_ = channel.LastBuild_;
    Tags_ = channel.Tags_;
	Language_ = channel.Language_;
	Author_ = channel.Author_;
	PixmapURL_ = channel.PixmapURL_;
	Pixmap_ = channel.Pixmap_;
	Favicon_ = channel.Favicon_;
	ParentURL_ = channel.ParentURL_;
}

ChannelShort Channel::ToShort () const
{
	ChannelShort cs =
	{
		Title_,
		Link_,
		Tags_,
		LastBuild_,
		Favicon_,
		CountUnreadItems (),
		ParentURL_
	};
	return cs;
}

bool operator< (const ChannelShort& cs1, const ChannelShort& cs2)
{
	return (cs1.Title_ + cs1.Link_) < (cs2.Title_ + cs2.Link_);
}

bool operator== (const ChannelShort& cs1, const ChannelShort& cs2)
{
	return cs1.Title_ == cs2.Title_ &&
		cs1.Link_ == cs2.Link_ &&
		cs1.ParentURL_ == cs2.ParentURL_;
}

bool operator== (const Channel_ptr& ch, const ChannelShort& cs)
{
	return ch->Title_ == cs.Title_ &&
		ch->Link_ == cs.Link_ &&
		ch->ParentURL_ == cs.ParentURL_;
}

bool operator== (const ChannelShort& cs, const Channel_ptr& ch)
{
	return ch == cs;
}

bool operator== (const Channel& c1, const Channel& c2)
{
    return c1.Title_ == c2.Title_ &&
		c1.Link_ == c2.Link_ &&
		c1.ParentURL_ == c2.ParentURL_;
}

QDataStream& operator<< (QDataStream& out, const Channel& chan)
{
	int version = 1;
    out << version
		<< chan.Title_
        << chan.Link_
        << chan.Description_
        << chan.LastBuild_
        << chan.Tags_
        << chan.Language_
        << chan.Author_
		<< chan.PixmapURL_
        << chan.Pixmap_
		<< chan.Favicon_
		<< chan.ParentURL_
        << static_cast<quint32> (chan.Items_.size ());
    for (size_t i = 0; i < chan.Items_.size (); ++i)
        out << *chan.Items_ [i];
    return out;
}

QDataStream& operator>> (QDataStream& in, Channel& chan)
{
	int version = 0;
	in >> version;
	if (!version)
		return in;
	else if (version == 1)
	{
		quint32 size;
		in >> chan.Title_
			>> chan.Link_
			>> chan.Description_
			>> chan.LastBuild_
			>> chan.Tags_
			>> chan.Language_
			>> chan.Author_
			>> chan.PixmapURL_
			>> chan.Pixmap_
			>> chan.Favicon_
			>> chan.ParentURL_;
		in >> size;
		for (size_t i = 0; i < size; ++i)
		{
			Item_ptr it (new Item);
			in >> *it;
			chan.Items_.push_back (it);
		}
		return in;
	}
	else
		return in;
}

