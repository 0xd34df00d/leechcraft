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

#ifndef PLUGINS_AGGREGATOR_SQLSTORAGEBACKEND_H
#define PLUGINS_AGGREGATOR_SQLSTORAGEBACKEND_H
#include <QSqlDatabase>
#include <QSqlQuery>
#include "storagebackend.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class SQLStorageBackend : public StorageBackend
			{
				Q_OBJECT

				QSqlDatabase DB_;

				Type Type_;
								  /** Returns:
								   * - last_update
								   *
								   * Binds:
								   * - url
								   */
				mutable QSqlQuery FeedFinderByURL_,
								  /** Returns:
								   * - update_timeout
								   * - num_items
								   * - item_age
								   *
								   * Binds:
								   * - feed_url
								   */
								  FeedSettingsGetter_,
								  /** Binds:
								   * - feed_url
								   * - update_timeout
								   * - num_items
								   * - item_age
								   */
								  FeedSettingsSetter_,
								  /** Returns:
								   * - channel_id
								   * - title
								   * - url
								   * - tags
								   * - last_build
								   * - favicon
								   *
								   * Binds:
								   * - feed_id
								   */
								  ChannelsShortSelector_,
								  /** Returns:
								   * - url
								   * - description
								   * - last_build
								   * - tags
								   * - language
								   * - author
								   * - pixmap_url
								   * - pixmap
								   * - favicon
								   *
								   * Binds:
								   * - parent_feed_url
								   * - title
								   */
								  ChannelsFullSelector_,
								  /** Returns:
								   * - number of unread items
								   *
								   * Binds:
								   * - channel_id
								   */
								  UnreadItemsCounter_,
								  /** Returns:
								   * - title
								   * - url
								   * - category
								   * - pub_date
								   * - unread
								   *
								   * Binds:
								   * - parents_hash
								   */
								  ItemsShortSelector_,
								  /** Returns:
								   * - title
								   * - url
								   * - description
								   * - author
								   * - category
								   * - guid
								   * - pub_date
								   * - unread
								   * - num_comments
								   * - comments_url
								   * - comments_page_url
								   *
								   * Binds:
								   * - parents_hash
								   * - title
								   * - url
								   */
								  ItemFullSelector_,
								  /** Returns:
								   * - title
								   * - url
								   * - description
								   * - author
								   * - category
								   * - guid
								   * - pub_date
								   * - unread
								   * - num_comments
								   * - comments_url
								   * - comments_page_url
								   * - latitude
								   * - longitude
								   *
								   * Binds:
								   * - parents_hash
								   */
								  ItemsFullSelector_,
								  /** Returns:
								   * - description
								   *
								   * Binds:
								   * - parent_feed_url
								   * - title
								   * - url
								   */
								  ChannelFinder_,
								  /** Returns:
								   * - title
								   *
								   * Binds:
								   * - parents_hash
								   * - title
								   * - url
								   */
								  ItemFinder_,
								  /** Binds:
								   * - url
								   * - last_update
								   */
								  InsertFeed_,
								  /** Binds:
								   * - parent_feed_url
								   * - url
								   * - title
								   * - description
								   * - last_build
								   * - tags
								   * - language
								   * - author
								   * - pixmap_url
								   * - pixmap
								   * - favicon
								   */
								  InsertChannel_,
								  /** Binds:
								   * - parents_hash
								   * - title
								   * - url
								   * - description
								   * - author
								   * - category
								   * - guid
								   * - pub_date
								   * - unread
								   * - num_comments
								   * - comments_url
								   * - comments_page_url
								   * - latitude
								   * - longitude
								   */
								  InsertItem_,
								  /** Binds:
								   * - title
								   * - url
								   * - tags
								   * - last_build
								   * - parent_feed_url
								   */
								  UpdateShortChannel_,
								  /** Binds:
								   * - description
								   * - last_build
								   * - tags
								   * - language
								   * - author
								   * - pixmap_url
								   * - pixmap
								   * - favicon
								   * - parent_feed_url
								   * - url
								   * - title
								   */
								  UpdateChannel_,
								  /** Binds:
								   * - parents_hash
								   * - age
								   */
								  ChannelDateTrimmer_,
								  /** Binds:
								   * - parents_hash
								   * - number
								   */
								  ChannelNumberTrimmer_,
								  /** Binds:
								   * - unread
								   * - parents_hash
								   * - title
								   * - url
								   */
								  UpdateShortItem_,
								  /** Binds:
								   * - description
								   * - author
								   * - category
								   * - pub_date
								   * - unread
								   * - num_comments
								   * - comments_url
								   * - comments_page_url
								   * - parents_hash
								   * - title
								   * - url
								   * - guid
								   * - latitude
								   * - longitude
								   */
								  UpdateItem_,
								  /** Binds:
								   * - unread
								   * - parents_hash
								   */
								  ToggleChannelUnread_,
								  /** Binds:
								   * - feed_id
								   */
								  RemoveFeed_,
								  /** Binds:
								   * - parent_feed_url
								   */
								  RemoveChannel_,
								  /** Binds:
								   * - parents_hash
								   * - title
								   * - url
								   * - guid
								   */
								  RemoveItem_,
								  /** Binds:
								   * - url
								   * - type
								   * - length
								   * - lang
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   */
								  WriteEnclosure_,
								  /** Binds:
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   */
								  RemoveEnclosures_,
								  /** Returns:
								   * - url
								   * - type
								   * - length
								   * - lang
								   *
								   * Binds:
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   */
								  GetEnclosures_,
								  /** Binds:
								   * - url
								   * - size
								   * - type
								   * - medium
								   * - is_default
								   * - expression
								   * - bitrate
								   * - framerate
								   * - samplingrate
								   * - channels
								   * - duration
								   * - width
								   * - height
								   * - lang
								   * - mediagroup
								   * - rating
								   * - rating_scheme
								   * - title
								   * - description
								   * - keywords
								   * - copyright_url
								   * - copyright_text
								   * - star_rating_average
								   * - star_rating_count
								   * - star_rating_min
								   * - star_rating_max
								   * - stat_views
								   * - stat_favs
								   * - tags
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   */
								  WriteMediaRSS_,
								  /** Returns:
								   * - url
								   * - size
								   * - type
								   * - medium
								   * - is_default
								   * - expression
								   * - bitrate
								   * - framerate
								   * - samplingrate
								   * - channels
								   * - duration
								   * - width
								   * - height
								   * - lang
								   * - mediagroup
								   * - rating
								   * - rating_scheme
								   * - title
								   * - description
								   * - keywords
								   * - copyright_url
								   * - copyright_text
								   * - star_rating_average
								   * - star_rating_count
								   * - star_rating_min
								   * - star_rating_max
								   * - stat_views
								   * - stat_favs
								   * - tags
								   *
								   * Binds:
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   */
								  GetMediaRSSs_,
								  /** Binds:
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   */
								  RemoveMediaRSS_,
								  /** Binds:
								   * - parent_url
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   * - url
								   * - width
								   * - height
								   * - time
								   */
								  WriteMediaRSSThumbnail_,
								  /** Returns:
								   * - url
								   * - width
								   * - height
								   * - time
								   *
								   * Binds:
								   * - parent_url
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   */
								  GetMediaRSSThumbnails_,
								  /** Binds:
								   * - parent_url
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   * - role
								   * - who
								   */
								  WriteMediaRSSCredit_,
								  /** Returns:
								   * - role
								   * - who
								   *
								   * Binds:
								   * - parent_url
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   */
								  GetMediaRSSCredits_,
								  /** Binds:
								   * - parent_url
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   * - type
								   * - comment
								   */
								  WriteMediaRSSComment_,
								  /** Returns:
								   * - type
								   * - comment
								   *
								   * Binds:
								   * - parent_url
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   */
								  GetMediaRSSComments_,
								  /** Binds:
								   * - parent_url
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   * - type
								   * - link
								   */
								  WriteMediaRSSPeerLink_,
								  /** Returns:
								   * - type
								   * - linkz
								   *
								   * Binds:
								   * - parent_url
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   */
								  GetMediaRSSPeerLinks_,
								  /** Binds:
								   * - parent_url
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   * - title
								   * - description
								   * - start_time
								   * - end_time
								   */
								  WriteMediaRSSScene_,
								  /** Returns:
								   * - title
								   * - description
								   * - start_time
								   * - end_time
								   *
								   * Binds:
								   * - parent_url
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   */
								  GetMediaRSSScenes_,
								  /** Binds:
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   */
								  RemoveMediaRSSThumbnails_,
								  /** Binds:
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   */
								  RemoveMediaRSSCredits_,
								  /** Binds:
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   */
								  RemoveMediaRSSComments_,
								  /** Binds:
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   */
								  RemoveMediaRSSPeerLinks_,
								  /** Binds:
								   * - item_parents_hash
								   * - item_title
								   * - item_url
								   */
								  RemoveMediaRSSScenes_;
			public:
				SQLStorageBackend (Type);
				virtual ~SQLStorageBackend ();

				virtual void Prepare ();

				virtual void GetFeedsURLs (feeds_urls_t&) const;
				virtual Feed::FeedSettings GetFeedSettings (const IDType_t&) const;
				virtual void SetFeedSettings (const QString&, const Feed::FeedSettings&);
				virtual void GetChannels (channels_shorts_t&, const IDType_t&) const;
				virtual Channel_ptr GetChannel (const QString&,
						const QString&) const;
				virtual void TrimChannel (const QString&,
						const QString&, int, int);
				virtual void GetItems (items_shorts_t&, const QString&) const;
				virtual int GetUnreadItems (const QString&, const QString&) const;
				virtual Item_ptr GetItem (const IDType_t&) const;
				virtual void GetItems (items_container_t&,
						const QString&) const;

				virtual void AddFeed (Feed_ptr);
				virtual void UpdateChannel (Channel_ptr, const QString&);
				virtual void UpdateChannel (const ChannelShort&,
						const QString&);
				virtual void UpdateItem (Item_ptr,
						const QString&, const QString&);
				virtual void UpdateItem (const ItemShort&,
						const QString&, const QString&);
				virtual void AddChannel (Channel_ptr, const QString&);
				virtual void AddItem (Item_ptr, const QString&, const QString&);
				virtual void RemoveItem (Item_ptr,
						const QString&,
						const QString&,
						const QString&);
				virtual void RemoveFeed (const IDType_t&);
				virtual bool UpdateFeedsStorage (int, int);
				virtual bool UpdateChannelsStorage (int, int);
				virtual bool UpdateItemsStorage (int, int);
				virtual void ToggleChannelUnread (const QString&, const QString&, bool);
			private:
				QString GetBoolType () const;
				QString GetBlobType () const;
				bool InitializeTables ();
				QByteArray SerializePixmap (const QPixmap&) const;
				QPixmap UnserializePixmap (const QByteArray&) const;
				bool RollItemsStorage (int);

				void RemoveTables ();
				Feed::FeedSettings GetFeedSettingsFromVersion5 (Feed_ptr) const;
				QList<Feed_ptr> LoadFeedsFromVersion5 () const;
				QList<Feed_ptr> GetFeedsFromVersion5 () const;
				QList<Channel_ptr> GetChannelsFromVersion5 (const QString&,
						const IDType_t&) const;
				QList<Item_ptr> GetItemsFromVersion5 (const QString&,
						const IDType_t&) const;
				void FillItemVersion5 (const QSqlQuery&, Item_ptr&) const;
				void GetEnclosuresVersion5 (const QString&, const QString&, const QString&,
						QList<Enclosure>&, const IDType_t&) const;
				void GetMRSSEntriesVersion5 (const QString&, const QString&, const QString&,
						QList<MRSSEntry>&, const IDType_t&) const;

				void FillItem (const QSqlQuery&, Item_ptr&) const;
				void WriteEnclosures (const QString&, const QString&, const QString&,
						const QList<Enclosure>&);
				void GetEnclosures (const IDType_t&, QList<Enclosure>&) const;
				void WriteMRSSEntries (const QString&, const QString&, const QString&,
						const QList<MRSSEntry>&);
				void GetMRSSEntries (const IDType_t&, QList<MRSSEntry>&) const;
			};
		};
	};
};

#endif

