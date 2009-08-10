#include <QDataStream>
#include <QtDebug>
#include "feed.h"
#include "channel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			Feed::FeedSettings::FeedSettings (int ut, int ni, int ia, bool ade)
			: UpdateTimeout_ (ut)
			, NumItems_ (ni)
			, ItemAge_ (ia)
			, AutoDownloadEnclosures_ (ade)
			{
			}
			
			Feed::Feed ()
			{
			}
			
			Feed::Feed (const Feed& feed)
			: URL_ (feed.URL_)
			, LastUpdate_ (feed.LastUpdate_)
			, Channels_ (feed.Channels_)
			{
			}
			
			Feed& Feed::operator= (const Feed& feed)
			{
				URL_ = feed.URL_;
				LastUpdate_ = feed.LastUpdate_;
				Channels_ = feed.Channels_;
				return *this;
			}
			
			bool operator< (const Feed& f1, const Feed& f2)
			{
				return f1.URL_ < f2.URL_;
			}
			
			QDataStream& operator<< (QDataStream& out, const Feed& feed)
			{
				out << feed.URL_
					<< feed.LastUpdate_
					<< static_cast<quint32> (feed.Channels_.size ());
				for (quint32 i = 0; i < feed.Channels_.size (); ++i)
					out << *feed.Channels_.at (i);
				return out;
			}
			
			QDataStream& operator>> (QDataStream& in, Feed& feed)
			{
				quint32 size = 0;
				in >> feed.URL_
					>> feed.LastUpdate_
					>> size;
				for (quint32 i = 0; i < size; ++i)
				{
					Channel_ptr chan (new Channel);
					in >> *chan;
					feed.Channels_.push_back (chan);
				}
				return in;
			}
		};
	};
};

