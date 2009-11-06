/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include <QtDebug>
#include <QDataStream>
#include "item.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			bool operator== (const MRSSThumbnail& t1, const MRSSThumbnail& t2)
			{
				return t1.URL_ == t2.URL_ &&
					t1.Width_ == t2.Width_ &&
					t1.Height_ == t2.Height_ &&
					t1.Time_ == t2.Time_;
			}

			bool operator== (const MRSSCredit& c1, const MRSSCredit& c2)
			{
				return c1.Role_ == c2.Role_ &&
					c1.Who_ == c2.Who_;
			}

			bool operator== (const MRSSComment& c1, const MRSSComment& c2)
			{
				return c1.Type_ == c2.Type_ &&
					c1.Comment_ == c2.Comment_;
			}

			bool operator== (const MRSSPeerLink& pl1, const MRSSPeerLink& pl2)
			{
				return pl1.Type_ == pl2.Type_ &&
					pl1.Link_ == pl2.Link_;
			}

			bool operator== (const MRSSScene& s1, const MRSSScene& s2)
			{
				return s1.Title_ == s2.Title_ &&
					s1.Description_ == s2.Description_ &&
					s1.StartTime_ == s2.StartTime_ &&
					s1.EndTime_ == s2.EndTime_;
			}

			template<typename T>
			bool SameSets (const QList<T>& t1, const QList<T>& t2)
			{
				bool ok = true;
				Q_FOREACH (T t, t1)
					if (!t2.contains (t))
					{
						ok = false;
						break;
					}
				return ok;
			}

			bool operator== (const MRSSEntry& e1, const MRSSEntry& e2)
			{
				return e1.URL_ == e2.URL_ &&
					e1.Size_ == e2.Size_ &&
					e1.Type_ == e2.Type_ &&
					e1.Medium_ == e2.Medium_ &&
					e1.IsDefault_ == e2.IsDefault_ &&
					e1.Expression_ == e2.Expression_ &&
					e1.Bitrate_ == e2.Bitrate_ &&
					e1.Framerate_ == e2.Framerate_ &&
					e1.SamplingRate_ == e2.SamplingRate_ &&
					e1.Channels_ == e2.Channels_ &&
					e1.Duration_ == e2.Duration_ &&
					e1.Width_ == e2.Width_ &&
					e1.Height_ == e2.Height_ &&
					e1.Lang_ == e2.Lang_ &&
					e1.Group_ == e2.Group_ &&
					e1.Rating_ == e2.Rating_ &&
					e1.RatingScheme_ == e2.RatingScheme_ &&
					e1.Title_ == e2.Title_ &&
					e1.Description_ == e2.Description_ &&
					e1.Keywords_ == e2.Keywords_ &&
					e1.CopyrightURL_ == e2.CopyrightURL_ &&
					e1.CopyrightText_ == e2.CopyrightText_ &&
					e1.RatingAverage_ == e2.RatingAverage_ &&
					e1.RatingCount_ == e2.RatingCount_ &&
					e1.RatingMin_ == e2.RatingMin_ &&
					e1.RatingMax_ == e2.RatingMax_ &&
					e1.Views_ == e2.Views_ &&
					e1.Favs_ == e2.Favs_ &&
					e1.Tags_ == e2.Tags_ &&
					SameSets (e1.Thumbnails_, e2.Thumbnails_) &&
					SameSets (e1.Credits_, e2.Credits_) &&
					SameSets (e1.Comments_, e2.Comments_) &&
					SameSets (e1.PeerLinks_, e2.PeerLinks_) &&
					SameSets (e1.Scenes_, e2.Scenes_);
			}

			ItemShort Item::ToShort () const
			{
				ItemShort is =
				{
					Title_,
					Link_,
					Categories_,
					PubDate_,
					Unread_
				};
				return is;
			}
			
			ItemComparator::ItemComparator (const Item_ptr& item)
			: Item_ (item)
			{
			}
			
			bool ItemComparator::operator() (const Item_ptr& item)
			{
				return *Item_ == *item;
			}
			
			bool operator== (const Item& i1, const Item& i2)
			{
				return i1.Title_ == i2.Title_ &&
					i1.Link_ == i2.Link_;
			}

			QDataStream& operator<< (QDataStream& out, const Enclosure& enc)
			{
				int version = 1;
				out << version
					<< enc.URL_
					<< enc.Type_
					<< enc.Length_
					<< enc.Lang_;
				return out;
			}

			QDataStream& operator>> (QDataStream& in, Enclosure& enc)
			{
				int version = 0;
				in >> version;
				if (version == 1)
					in >> enc.URL_
						>> enc.Type_
						>> enc.Length_
						>> enc.Lang_;
				else
					qWarning () << Q_FUNC_INFO
						<< "unknown version"
						<< version;
				return in;
			}

			QDataStream& operator<< (QDataStream& out, const MRSSThumbnail& thumb)
			{
				int version = 1;
				out << version
					<< thumb.URL_
					<< thumb.Width_
					<< thumb.Height_
					<< thumb.Time_;
				return out;
			}

			QDataStream& operator>> (QDataStream& in, MRSSThumbnail& thumb)
			{
				int version = 0;
				in >> version;
				if (version == 1)
					in >> thumb.URL_
						>> thumb.Width_
						>> thumb.Height_
						>> thumb.Time_;
				else
					qWarning () << Q_FUNC_INFO
						<< "unkown version"
						<< version;
				return in;
			}

			QDataStream& operator<< (QDataStream& out, const MRSSCredit& credit)
			{
				int version = 1;
				out << version
					<< credit.Role_
					<< credit.Who_;
				return out;
			}

			QDataStream& operator>> (QDataStream& in, MRSSCredit& credit)
			{
				int version = 0;
				in >> version;
				if (version == 1)
					in >> credit.Role_
						>> credit.Who_;
				else
					qWarning () << Q_FUNC_INFO
						<< "unkown version"
						<< version;
				return in;
			}

			QDataStream& operator<< (QDataStream& out, const MRSSComment& c)
			{
				int version = 1;
				out << version
					<< c.Type_
					<< c.Comment_;
				return out;
			}

			QDataStream& operator>> (QDataStream& in, MRSSComment& c)
			{
				int version = 0;
				in >> version;
				if (version == 1)
					in >> c.Type_
						>> c.Comment_;
				else
					qWarning () << Q_FUNC_INFO
						<< "unkown version"
						<< version;
				return in;
			}

			QDataStream& operator<< (QDataStream& out, const MRSSPeerLink& p)
			{
				int version = 1;
				out << version
					<< p.Type_
					<< p.Link_;
				return out;
			}

			QDataStream& operator>> (QDataStream& in, MRSSPeerLink& p)
			{
				int version = 0;
				in >> version;
				if (version == 1)
					in >> p.Type_
						>> p.Link_;
				else
					qWarning () << Q_FUNC_INFO
						<< "unkown version"
						<< version;
				return in;
			}

			QDataStream& operator<< (QDataStream& out, const MRSSScene& s)
			{
				int version = 1;
				out << version
					<< s.Title_
					<< s.Description_
					<< s.StartTime_
					<< s.EndTime_;
				return out;
			}

			QDataStream& operator>> (QDataStream& in, MRSSScene& s)
			{
				int version = 0;
				in >> version;
				if (version == 1)
					in >> s.Title_
						>> s.Description_
						>> s.StartTime_
						>> s.EndTime_;
				else
					qWarning () << Q_FUNC_INFO
						<< "unkown version"
						<< version;
				return in;
			}

			QDataStream& operator<< (QDataStream& out, const MRSSEntry& e)
			{
				int version = 1;
				out << version
					<< e.URL_
					<< e.Size_
					<< e.Type_
					<< e.Medium_
					<< e.IsDefault_
					<< e.Expression_
					<< e.Bitrate_
					<< e.Framerate_
					<< e.SamplingRate_
					<< e.Channels_
					<< e.Duration_
					<< e.Width_
					<< e.Height_
					<< e.Lang_
					<< e.Group_
					<< e.Rating_
					<< e.RatingScheme_
					<< e.Title_
					<< e.Description_
					<< e.Keywords_
					<< e.CopyrightURL_
					<< e.CopyrightText_
					<< e.RatingAverage_
					<< e.RatingCount_
					<< e.RatingMin_
					<< e.RatingMax_
					<< e.Views_
					<< e.Favs_
					<< e.Tags_
					<< e.Thumbnails_
					<< e.Credits_
					<< e.Comments_
					<< e.PeerLinks_
					<< e.Scenes_;
				return out;
			}

			QDataStream& operator>> (QDataStream& in, MRSSEntry& e)
			{
				int version = 0;
				in >> version;
				if (version == 1)
				in >> e.URL_
					>> e.Size_
					>> e.Type_
					>> e.Medium_
					>> e.IsDefault_
					>> e.Expression_
					>> e.Bitrate_
					>> e.Framerate_
					>> e.SamplingRate_
					>> e.Channels_
					>> e.Duration_
					>> e.Width_
					>> e.Height_
					>> e.Lang_
					>> e.Group_
					>> e.Rating_
					>> e.RatingScheme_
					>> e.Title_
					>> e.Description_
					>> e.Keywords_
					>> e.CopyrightURL_
					>> e.CopyrightText_
					>> e.RatingAverage_
					>> e.RatingCount_
					>> e.RatingMin_
					>> e.RatingMax_
					>> e.Views_
					>> e.Favs_
					>> e.Tags_
					>> e.Thumbnails_
					>> e.Credits_
					>> e.Comments_
					>> e.PeerLinks_
					>> e.Scenes_;
				else
					qWarning () << Q_FUNC_INFO
						<< "unkown version"
						<< version;
				return in;
			}

			QDataStream& operator<< (QDataStream& out, const Item& item)
			{
				int version = 4;
				out << version
					<< item.Title_
					<< item.Link_
					<< item.Description_
					<< item.Author_
					<< item.Categories_
					<< item.Guid_
					<< item.PubDate_
					<< item.Unread_
					<< item.NumComments_
					<< item.CommentsLink_
					<< item.CommentsPageLink_
					<< item.Enclosures_
					<< item.Latitude_
					<< item.Longitude_
					<< item.MRSSEntries_;
				return out;
			}

			QDataStream& operator>> (QDataStream& in, Item& item)
			{
				int version = 0;
				in >> version;
				if (version >= 1)
					in >> item.Title_
						>> item.Link_
						>> item.Description_
						>> item.Author_
						>> item.Categories_
						>> item.Guid_
						>> item.PubDate_
						>> item.Unread_
						>> item.NumComments_
						>> item.CommentsLink_
						>> item.CommentsPageLink_;
				if (version >= 2)
					in >> item.Enclosures_;
				if (version >= 3)
					in >> item.Latitude_
						>> item.Longitude_;
				if (version == 4)
					in >> item.MRSSEntries_;

				if (version < 1 || version > 4)
					qWarning () << Q_FUNC_INFO << "unknown version" << version;

				return in;
			}

			void Print (const Item& item)
			{
				qDebug () << item.Title_
					<< item.Link_
					<< item.Description_
					<< item.Author_
					<< item.Categories_
					<< item.Guid_
					<< item.PubDate_
					<< item.NumComments_
					<< item.CommentsLink_
					<< item.CommentsPageLink_;
			}

			void Diff (const Item& i1, const Item& i2)
			{
				qDebug () << Q_FUNC_INFO << "for" << i1.Title_;
				if (i1.Title_ != i2.Title_)
				{
					qDebug () << i1.Title_;
					qDebug () << i2.Title_;
				}
				if (i1.Link_ != i2.Link_)
				{
					qDebug () << i1.Link_;
					qDebug () << i2.Link_;
				}
				if (i1.Description_ != i2.Description_)
				{
					qDebug () << i1.Description_;
					qDebug () << i2.Description_;
				}
				if (i1.Author_ != i2.Author_)
				{
					qDebug () << i1.Author_;
					qDebug () << i2.Author_;
				}
				if (i1.Categories_ != i2.Categories_)
				{
					qDebug () << i1.Categories_;
					qDebug () << i2.Categories_;
				}
				if (i1.PubDate_ != i2.PubDate_)
				{
					qDebug () << i1.PubDate_;
					qDebug () << i2.PubDate_;
				}
				if (i1.NumComments_ != i2.NumComments_)
				{
					qDebug () << i1.NumComments_;
					qDebug () << i2.NumComments_;
				}
				if (i1.CommentsLink_ != i2.CommentsLink_)
				{
					qDebug () << i1.CommentsLink_;
					qDebug () << i2.CommentsLink_;
				}
				if (i1.CommentsPageLink_ != i2.CommentsPageLink_)
				{
					qDebug () << i1.CommentsPageLink_;
					qDebug () << i2.CommentsPageLink_;
				}
			}

			bool IsModified (Item_ptr i1, Item_ptr i2)
			{
				return !(i1->Title_ == i2->Title_ &&
						i1->Link_ == i2->Link_ &&
						i1->Description_ == i2->Description_ &&
						i1->Author_ == i2->Author_ &&
						i1->Categories_ == i2->Categories_ &&
						i1->PubDate_ == i2->PubDate_ &&
						i1->NumComments_ == i2->NumComments_ &&
						i1->CommentsLink_ == i2->CommentsLink_ &&
						i1->CommentsPageLink_ == i2->CommentsPageLink_ &&
						i1->Latitude_ == i2->Latitude_ &&
						i1->Longitude_ == i2->Longitude_ &&
						i1->MRSSEntries_ == i2->MRSSEntries_);
			}
		};
	};
};

