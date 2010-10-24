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

#ifndef PLUGINS_AGGREGATOR_SQLSTORAGEBACKEND_MYSQL_H
#define PLUGINS_AGGREGATOR_SQLSTORAGEBACKEND_MYSQL_H
#include <QSqlDatabase>
#include <QSqlQuery>
#include "storagebackend.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			class SQLStorageBackendMysql : public StorageBackend
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
								   * - url
								   * - last_update
								   *
								   * Binds:
								   * - feed_id
								   */
								  FeedGetter_,
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
								   * - author
								   *
								   * Binds:
								   * - feed_id
								   */
								  ChannelsShortSelector_,
								  /** Returns:
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
								   *
								   * Binds:
								   * - channel_id
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
								   * - item_id
								   * - title
								   * - url
								   * - category
								   * - pub_date
								   * - unread
								   *
								   * Binds:
								   * - channel_id
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
								   * - latitude
								   * - longitue
								   * - channel_id
								   *
								   * Binds:
								   * - item_id
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
								   * - channel_id
								   * - item_id
								   *
								   * Binds:
								   * - channel_id
								   */
								  ItemsFullSelector_,
								  /** Returns:
								   * - 1
								   *
								   * Binds:
								   * - channel_id
								   */
								  ChannelFinder_,
								  /** Returns:
								   * - channel_id
								   *
								   * Binds:
								   * - title
								   * - url
								   * - feed_id
								   */
								  ChannelIDFromTitleURL_,
								  /** Returns:
								   * - item_id
								   *
								   * Binds:
								   * - title
								   * - url
								   * - channel_id
								   */
								  ItemIDFromTitleURL_,
								  /** Binds:
								   * - url
								   * - last_update
								   */
								  InsertFeed_,
								  /** Binds:
								   * - channel_id
								   * - feed_id
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
								   * - channel_id
								   * - tags
								   * - last_build
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
								   * - channel_id
								   */
								  UpdateChannel_,
								  /** Binds:
								   * - channel_id
								   * - age
								   */
								  ChannelDateTrimmer_,
								  /** Binds:
								   * - channel_id
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
								   * - guid
								   * - latitude
								   * - longitude
								   * - item_id
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
								   * - channel_id
								   */
								  RemoveChannel_,
								  /** Binds:
								   * - item_id
								   */
								  RemoveItem_,
								  /** Binds:
								   * - url
								   * - type
								   * - length
								   * - lang
								   * - item_id
								   * - enclosure_id
								   */
								  WriteEnclosure_,
								  /** Binds:
								   * - item_id
								   */
								  RemoveEnclosures_,
								  /** Returns:
								   * - enclosure_id
								   * - url
								   * - type
								   * - length
								   * - lang
								   *
								   * Binds:
								   * - item_id
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
								   * - item_id
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
								   * - item_id
								   */
								  RemoveMediaRSSThumbnails_,
								  /** Binds:
								   * - item_id
								   */
								  RemoveMediaRSSCredits_,
								  /** Binds:
								   * - item_id
								   */
								  RemoveMediaRSSComments_,
								  /** Binds:
								   * - item_id
								   */
								  RemoveMediaRSSPeerLinks_,
								  /** Binds:
								   * - item_id
								   */
								  RemoveMediaRSSScenes_;
			public:
				SQLStorageBackendMysql (Type);
				virtual ~SQLStorageBackendMysql ();

				virtual void Prepare ();

				virtual void GetFeedsIDs (ids_t&) const;
				virtual Feed_ptr GetFeed (const IDType_t&) const;
				virtual IDType_t FindFeed (const QString&) const;
				virtual Feed::FeedSettings GetFeedSettings (const IDType_t&) const;
				virtual void SetFeedSettings (const Feed::FeedSettings&);
				virtual void GetChannels (channels_shorts_t&, const IDType_t&) const;
				virtual Channel_ptr GetChannel (const IDType_t&,
						const IDType_t&) const;
				virtual IDType_t FindChannel (const QString& ,
						const QString&, const IDType_t&) const;
				virtual void TrimChannel (const IDType_t&, int, int);
				virtual void GetItems (items_shorts_t&, const IDType_t&) const;
				virtual int GetUnreadItems (const IDType_t&) const;
				virtual Item_ptr GetItem (const IDType_t&) const;
				virtual IDType_t FindItem (const QString&,
						const QString&, const IDType_t&) const;
				virtual void GetItems (items_container_t&,
						const IDType_t&) const;

				virtual void AddFeed (Feed_ptr);
				virtual void UpdateChannel (Channel_ptr);
				virtual void UpdateChannel (const ChannelShort&);
				virtual void UpdateItem (Item_ptr);
				virtual void UpdateItem (const ItemShort&);
				virtual void AddChannel (Channel_ptr);
				virtual void AddItem (Item_ptr);
				virtual void RemoveItem (const IDType_t&);
				virtual void RemoveFeed (const IDType_t&);
				virtual bool UpdateFeedsStorage (int, int);
				virtual bool UpdateChannelsStorage (int, int);
				virtual bool UpdateItemsStorage (int, int);
				virtual void ToggleChannelUnread (const IDType_t&, bool);
			private:
				QString GetBoolType () const;
				QString GetBlobType () const;
				bool InitializeTables ();
				QByteArray SerializePixmap (const QPixmap&) const;
				QPixmap UnserializePixmap (const QByteArray&) const;

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

				IDType_t FindParentFeedForChannel (const IDType_t&) const;
				void FillItem (const QSqlQuery&, Item_ptr&) const;
				void WriteEnclosures (const QList<Enclosure>&);
				void GetEnclosures (const IDType_t&, QList<Enclosure>&) const;
				void WriteMRSSEntries (const QList<MRSSEntry>&);
				void GetMRSSEntries (const IDType_t&, QList<MRSSEntry>&) const;
			};
		};
	};
};

#endif

