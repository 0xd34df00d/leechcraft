/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <QtDebug>
#include "item.h"
#include "poolsmanager.h"

namespace LC
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
		if (t1.size () != t2.size ())
			return false;

		return std::all_of (t1.begin (), t1.end (), [t2] (const T& val) { return t2.contains (val); });
	}

	bool operator== (const Enclosure& e1, const Enclosure& e2)
	{
		return e1.URL_ == e2.URL_ &&
			e1.Type_ == e2.Type_ &&
			e1.Length_ == e2.Length_ &&
			e1.Lang_ == e2.Lang_;
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

	Enclosure Enclosure::CreateForItem (IDType_t itemId)
	{
		Enclosure enc;
		enc.ItemID_ = itemId;
		enc.EnclosureID_ = PoolsManager::Instance ().GetPool (PTEnclosure).GetID ();
		return enc;
	}

	MRSSThumbnail MRSSThumbnail::CreateForEntry (IDType_t entryId)
	{
		MRSSThumbnail thumbnail;
		thumbnail.MRSSThumbnailID_ = PoolsManager::Instance ().GetPool (PTMRSSThumbnail).GetID ();
		thumbnail.MRSSEntryID_ = entryId;
		return thumbnail;
	}

	MRSSCredit MRSSCredit::CreateForEntry (IDType_t entryId)
	{
		MRSSCredit credit;
		credit.MRSSCreditID_ = PoolsManager::Instance ().GetPool (PTMRSSCredit).GetID ();
		credit.MRSSEntryID_ = entryId;
		return credit;
	}

	MRSSComment MRSSComment::CreateForEntry (IDType_t entryId)
	{
		MRSSComment comment;
		comment.MRSSCommentID_ = PoolsManager::Instance ().GetPool (PTMRSSComment).GetID ();
		comment.MRSSEntryID_ = entryId;
		return comment;
	}

	MRSSPeerLink MRSSPeerLink::CreateForEntry (IDType_t entryId)
	{
		MRSSPeerLink link;
		link.MRSSPeerLinkID_ = PoolsManager::Instance ().GetPool (PTMRSSPeerLink).GetID ();
		link.MRSSEntryID_ = entryId;
		return link;
	}

	MRSSScene MRSSScene::CreateForEntry (IDType_t entryId)
	{
		MRSSScene scene;
		scene.MRSSSceneID_ = PoolsManager::Instance ().GetPool (PTMRSSScene).GetID ();
		scene.MRSSEntryID_ = entryId;
		return scene;
	}

	MRSSEntry MRSSEntry::CreateForItem (IDType_t itemId)
	{
		MRSSEntry entry;
		entry.MRSSEntryID_ = PoolsManager::Instance ().GetPool (PTMRSSEntry).GetID ();
		entry.ItemID_ = itemId;
		return entry;
	}

	Item Item::CreateForChannel (IDType_t channelId)
	{
		Item item;
		item.ChannelID_ = channelId;
		item.ItemID_ = PoolsManager::Instance ().GetPool (PTItem).GetID ();
		return item;
	}

	ItemShort Item::ToShort () const
	{
		return
		{
			ItemID_,
			ChannelID_,
			Title_,
			Link_,
			Categories_,
			PubDate_,
			Unread_
		};
	}

	void Item::FixDate ()
	{
		if (!PubDate_.isValid ())
			PubDate_ = QDateTime::currentDateTime ();
	}

	bool operator== (const Item& i1, const Item& i2)
	{
		return i1.ItemID_ == i2.ItemID_;
	}

	void Print (const Item& item)
	{
		auto trimmedDescr = item.Description_;
		const auto showLen = 50;
		if (trimmedDescr.size () > 2 * showLen)
			trimmedDescr = trimmedDescr.left (showLen) + " [...] " + trimmedDescr.right (showLen);

		qDebug () << "item:" << item.ItemID_
			<< "\ncid:" << item.ChannelID_
			<< "\ntitle:" << item.Title_
			<< "\nlink:" << item.Link_
			<< "\ndescr:" << trimmedDescr
			<< "\nauthor:" << item.Author_
			<< "\ncategories:" << item.Categories_
			<< "\nguid:" << item.Guid_
			<< "\npubdate:" << item.PubDate_
			<< "\nnum comments:" << item.NumComments_
			<< "\ncomments link:" << item.CommentsLink_
			<< "\ncomments page link:" << item.CommentsPageLink_
			<< "\nenclosures #:" << item.Enclosures_.size ()
			<< "\nlat/lon:" << item.Latitude_ << item.Longitude_
			<< "\nmrss #:" << item.MRSSEntries_.size ();
	}

	void Diff (const Item& i1, const Item& i2)
	{
		qDebug () << Q_FUNC_INFO << "for" << i1.Title_;
		const auto check = [&] (const char *name, auto getter)
		{
			if (i1.*getter != i2.*getter)
			{
				qDebug () << name;
				qDebug () << i1.*getter;
				qDebug () << i2.*getter;
			}
		};
		check ("title", &Item::Title_);
		check ("link", &Item::Link_);
		check ("description", &Item::Description_);
		check ("author", &Item::Author_);
		check ("categories", &Item::Categories_);
		check ("pubdate", &Item::PubDate_);
		check ("numComments", &Item::NumComments_);
		check ("commentsLink", &Item::CommentsLink_);
		check ("commentsPageLink", &Item::CommentsPageLink_);
		check ("latitude", &Item::Latitude_);
		check ("longitude", &Item::Longitude_);
	}

	bool IsModified (const Item& i1, const Item& i2)
	{
		return !(i1.Title_ == i2.Title_ &&
				i1.Link_ == i2.Link_ &&
				i1.Description_ == i2.Description_ &&
				i1.Author_ == i2.Author_ &&
				i1.Categories_ == i2.Categories_ &&
				(!i1.PubDate_.isValid () ||
					!i2.PubDate_.isValid () ||
					i1.PubDate_ == i2.PubDate_) &&
				i1.NumComments_ == i2.NumComments_ &&
				i1.CommentsLink_ == i2.CommentsLink_ &&
				i1.CommentsPageLink_ == i2.CommentsPageLink_ &&
				i1.Latitude_ == i2.Latitude_ &&
				i1.Longitude_ == i2.Longitude_ &&
				SameSets (i1.Enclosures_, i2.Enclosures_) &&
				SameSets (i1.MRSSEntries_, i2.MRSSEntries_));
	}
}
}
