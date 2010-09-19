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

#include "sqlstoragebackend_mysql.h"
#include <stdexcept>
#include <boost/bind.hpp>
#include <boost/optional.hpp>
#include <QDir>
#include <QDebug>
#include <QBuffer>
#include <QSqlError>
#include <QVariant>
#include <QSqlRecord>
#include "plugininterface/dblock.h"
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{

			SQLStorageBackendMysql::SQLStorageBackendMysql (StorageBackend::Type t)
			: Type_ (t)
			{
		
				DB_ = QSqlDatabase::addDatabase ("QMYSQL", "AggregatorConnection");
                DB_.setDatabaseName (XmlSettingsManager::Instance ()->
                        property ("MysqlDBName").toString ());
                DB_.setHostName (XmlSettingsManager::Instance ()->
                        property ("MysqlHostname").toString ());
                DB_.setPort (XmlSettingsManager::Instance ()->
                        property ("MysqlPort").toInt ());
                DB_.setUserName (XmlSettingsManager::Instance ()->
                        property ("MysqlUsername").toString ());
                DB_.setPassword (XmlSettingsManager::Instance ()->
                        property ("MysqlPassword").toString ());
			
				if (!DB_.open ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (DB_.lastError ());
					throw std::runtime_error (qPrintable (QString ("Could not initialize database: %1")
								.arg (DB_.lastError ().text ())));
				}
			
				InitializeTables ();
			}
			
			SQLStorageBackendMysql::~SQLStorageBackendMysql ()
			{

			}
			
			void SQLStorageBackendMysql::Prepare ()
			{
                //TODO set collation
				FeedFinderByURL_ = QSqlQuery (DB_);
        		FeedFinderByURL_.prepare ("SELECT feed_id "
    						"FROM feeds "
    						"WHERE url = ?");

				FeedGetter_ = QSqlQuery (DB_);
				FeedGetter_.prepare ("SELECT "
						"url, "
						"last_update "
						"FROM feeds "
						"WHERE feed_id = ?");

				FeedSettingsGetter_ = QSqlQuery (DB_);
				FeedSettingsGetter_.prepare ("SELECT "
						"settings_id, "
						"update_timeout, "
						"num_items, "
						"item_age, "
						"auto_download_enclosures "
						"FROM feeds_settings "
						"WHERE feed_id = ?");

				FeedSettingsSetter_ = QSqlQuery (DB_);
				QString queryStart;
			
				FeedSettingsSetter_.prepare (QString ("REPLACE INTO feeds_settings ("
						"feed_id, "
						"settings_id, "
						"update_timeout, "
						"num_items, "
						"item_age, "
						"auto_download_enclosures"
						") VALUES ("
						"?, "
						"?, "
						"?, "
						"?, "
						"?, "
						"?"
						")").arg (queryStart));
			
				ChannelsShortSelector_ = QSqlQuery (DB_);
				ChannelsShortSelector_.prepare ("SELECT "
						"channel_id, "
						"title, "
						"url, "
						"tags, "
						"last_build,"
						"favicon, "
						"author "
						"FROM channels "
						"WHERE feed_id = ? "
						"ORDER BY title");
			
				ChannelsFullSelector_ = QSqlQuery (DB_);
				ChannelsFullSelector_.prepare ("SELECT "
						"url, "
						"title, "
						"description, "
						"last_build, "
						"tags, "
						"language, "
						"author, "
						"pixmap_url, "
						"pixmap, "
						"favicon "
						"FROM channels "
						"WHERE channel_id = ? "
						"ORDER BY title");

				UnreadItemsCounter_ = QSqlQuery (DB_);
                UnreadItemsCounter_.prepare ("SELECT COUNT(*) "
                                "FROM items "
                                "WHERE channel_id = ? "
                                "AND unread = 1");
			
				ItemsShortSelector_ = QSqlQuery (DB_);
				ItemsShortSelector_.prepare ("SELECT "
						"item_id, "
						"title, "
						"url, "
						"category, "
						"pub_date, "
						"unread "
						"FROM items "
						"WHERE channel_id = ? "
						"ORDER BY unread ASC, pub_date DESC, "
						"title DESC");
			
				ItemFullSelector_ = QSqlQuery (DB_);
				ItemFullSelector_.prepare ("SELECT "
						"title, "
						"url, "
						"description, "
						"author, "
						"category, "
						"guid, "
						"pub_date, "
						"unread, "
						"num_comments, "
						"comments_url, "
						"comments_page_url, "
						"latitude, "
						"longitude, "
						"channel_id "
						"FROM items "
						"WHERE item_id = ? "
						"ORDER BY pub_date DESC");
			
				ItemsFullSelector_ = QSqlQuery (DB_);
				ItemsFullSelector_.prepare ("SELECT "
						"title, "
						"url, "
						"description, "
						"author, "
						"category, "
						"guid, "
						"pub_date, "
						"unread, "
						"num_comments, "
						"comments_url, "
						"comments_page_url, "
						"latitude, "
						"longitude, "
						"channel_id,"
						"item_id "
						"FROM items "
						"WHERE channel_id = ? "
						"ORDER BY pub_date DESC");

				ChannelFinder_ = QSqlQuery (DB_);
				ChannelFinder_.prepare ("SELECT 1 "
						"FROM channels "
						"WHERE channel_id = ? ");

				ChannelIDFromTitleURL_ = QSqlQuery (DB_);
				ChannelIDFromTitleURL_.prepare ("SELECT channel_id "
						"FROM channels "
						"WHERE feed_id = ? "
						"AND title = ? "
						"AND url = ? ");

				ItemIDFromTitleURL_ = QSqlQuery (DB_);
				ItemIDFromTitleURL_.prepare ("SELECT item_id "
						"FROM items "
						"WHERE channel_id = ? "
						"AND title = ? "
						"AND url = ? ");

				InsertFeed_ = QSqlQuery (DB_);
				InsertFeed_.prepare ("INSERT INTO feeds "
                                     "(feed_id, url, last_update) "
                                     " VALUES ( ? , ? ,  ? );");
			
				InsertChannel_ = QSqlQuery (DB_);
				InsertChannel_.prepare ("INSERT INTO channels ("
						"channel_id, "
						"feed_id, "
						"url, "
						"title, "
						"description, "
						"last_build, "
						"tags, "
						"language, "
						"author, "
						"pixmap_url, "
						"pixmap, "
						"favicon"
						") VALUES ("
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? "
						");");
			
				InsertItem_ = QSqlQuery (DB_);
				InsertItem_.prepare ("INSERT INTO items ("
						"item_id, "
						"channel_id, "
						"title, "
						"url, "
						"description, "
						"author, "
						"category, "
						"guid, "
						"pub_date, "
						"unread, "
						"num_comments, "
						"comments_url, "
						"comments_page_url, "
						"latitude, "
						"longitude"
						") VALUES ("
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ?"
						");");

				UpdateShortChannel_ = QSqlQuery (DB_);
				UpdateShortChannel_.prepare ("UPDATE channels SET "
						"tags = ? , "
						"last_build =  ? "
						"WHERE channel_id = ? ");
			
				UpdateChannel_ = QSqlQuery (DB_);
				UpdateChannel_.prepare ("UPDATE channels SET "
						"description =  ? , "
						"last_build = ? , "
						"tags =  ? , "
						"language = ? , "
						"author = ? , "
						"pixmap_url = ? , "
						"pixmap = ? , "
						"favicon =  ? "
						"WHERE channel_id = ? ");

				QString common = "DELETE FROM items "
					"WHERE channel_id = ? ";
				QString cdt = "AND DATE_ADD( pub_date, INTERVAL ?  DAY ) < now () ";
				QString cnt = "AND pub_date IN "
                    "(SELECT pub_date FROM items WHERE channel_id = ? "
                    "ORDER BY pub_date DESC LIMIT ?, 1000000000 )";

				ChannelDateTrimmer_ = QSqlQuery (DB_);
				ChannelDateTrimmer_.prepare (common + cdt);

				ChannelNumberTrimmer_ = QSqlQuery (DB_);
				ChannelNumberTrimmer_.prepare (common + cnt);
			
				UpdateShortItem_ = QSqlQuery (DB_);
				UpdateShortItem_.prepare ("UPDATE items SET "
						"unread = ? "
						"WHERE item_id = ? ");

				UpdateItem_ = QSqlQuery (DB_);
				UpdateItem_.prepare ("UPDATE items SET "
						"description = ? , "
						"author = ? , "
						"category = ? , "
						"pub_date = ? , "
						"unread = ? , "
						"num_comments = ? , "
						"comments_url = ? , "
						"comments_page_url = ? , "
						"latitude = ? , "
						"longitude = ? "
						"WHERE item_id = ? ");
			
				ToggleChannelUnread_ = QSqlQuery (DB_);
				ToggleChannelUnread_.prepare ("UPDATE items SET "
						"unread = ? "
						"WHERE channel_id = ? "
						"AND unread <> ? ");
			
				RemoveFeed_ = QSqlQuery (DB_);
				RemoveFeed_.prepare ("DELETE FROM feeds "
						"WHERE feed_id = ? ");
			
				RemoveChannel_ = QSqlQuery (DB_);
				RemoveChannel_.prepare ("DELETE FROM channels "
						"WHERE channel_id = ? ");
			
				RemoveItem_ = QSqlQuery (DB_);
				RemoveItem_.prepare ("DELETE FROM items "
						"WHERE item_id = ? ");
			
				WriteEnclosure_ = QSqlQuery (DB_);
				WriteEnclosure_.prepare ("REPLACE INTO enclosures ("
						"url, "
						"type, "
						"length, "
						"lang, "
						"item_id, "
						"enclosure_id"
						") VALUES ("
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? "
						")");

				WriteMediaRSS_ = QSqlQuery (DB_);
				WriteMediaRSS_.prepare ("REPLACE INTO mrss ("
						"mrss_id, "
						"item_id, "
						"url, "
						"size, "
						"type, "
						"medium, "
						"is_default, "
						"expression, "
						"bitrate, "
						"framerate, "
						"samplingrate, "
						"channels, "
						"duration, "
						"width, "
						"height, "
						"lang, "
						"mediagroup, "
						"rating, "
						"rating_scheme, "
						"title, "
						"description, "
						"keywords, "
						"copyright_url, "
						"copyright_text, "
						"star_rating_average, "
						"star_rating_count, "
						"star_rating_min, "
						"star_rating_max, "
						"stat_views, "
						"stat_favs, "
						"tags"
						") VALUES ("
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? "
						")");

				GetMediaRSSs_ = QSqlQuery (DB_);
				GetMediaRSSs_.prepare ("SELECT "
						"mrss_id, "
						"url, "
						"size, "
						"type, "
						"medium, "
						"is_default, "
						"expression, "
						"bitrate, "
						"framerate, "
						"samplingrate, "
						"channels, "
						"duration, "
						"width, "
						"height, "
						"lang, "
						"mediagroup, "
						"rating, "
						"rating_scheme, "
						"title, "
						"description, "
						"keywords, "
						"copyright_url, "
						"copyright_text, "
						"star_rating_average, "
						"star_rating_count, "
						"star_rating_min, "
						"star_rating_max, "
						"stat_views, "
						"stat_favs, "
						"tags "
						"FROM mrss "
						"WHERE item_id = ? "
						"ORDER BY title");

				WriteMediaRSSThumbnail_ = QSqlQuery (DB_);
				WriteMediaRSSThumbnail_.prepare ("REPLACE INTO mrss_thumbnails ("
						"mrss_thumb_id, "
						"mrss_id, "
						"url, "
						"width, "
						"height, "
						"time"
						") VALUES ("
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? "
						")");

				GetMediaRSSThumbnails_ = QSqlQuery (DB_);
				GetMediaRSSThumbnails_.prepare ("SELECT "
						"mrss_thumb_id, "
						"url, "
						"width, "
						"height, "
						"time "
						"FROM mrss_thumbnails "
						"WHERE mrss_id = ? "
						"ORDER BY time");

				WriteMediaRSSCredit_ = QSqlQuery (DB_);
				WriteMediaRSSCredit_.prepare ("REPLACE INTO mrss_credits ("
						"mrss_credits_id, "
						"mrss_id, "
						"role, "
						"who"
						") VALUES ("
						" ? , "
						" ? , "
						" ? , "
						" ? "
						")");

				GetMediaRSSCredits_ = QSqlQuery (DB_);
				GetMediaRSSCredits_.prepare ("SELECT "
						"mrss_credits_id, "
						"role, "
						"who "
						"FROM mrss_credits "
						"WHERE mrss_id = ? "
						"ORDER BY role");

				WriteMediaRSSComment_ = QSqlQuery (DB_);
				WriteMediaRSSComment_.prepare ("REPLACE INTO mrss_comments ("
						"mrss_comment_id, "
						"mrss_id, "
						"type, "
						"comment"
						") VALUES ("
						" ? , "
						" ? , "
						" ? , "
						" ? "
						")");

				GetMediaRSSComments_ = QSqlQuery (DB_);
				GetMediaRSSComments_.prepare ("SELECT "
						"mrss_comment_id, "
						"type, "
						"comment "
						"FROM mrss_comments "
						"WHERE mrss_id = ? "
						"ORDER BY comment");
				
				WriteMediaRSSPeerLink_ = QSqlQuery (DB_);
				WriteMediaRSSPeerLink_.prepare ("REPLACE INTO mrss_peerlinks ("
						"mrss_peerlink_id, "
						"mrss_id, "
						"type, "
						"link"
						") VALUES ("
						" ? , "
						" ? , "
						" ? , "
						" ? "
						")");

				GetMediaRSSPeerLinks_ = QSqlQuery (DB_);
				GetMediaRSSPeerLinks_.prepare ("SELECT "
						"mrss_peerlink_id, "
						"type, "
						"link "
						"FROM mrss_peerlinks "
						"WHERE mrss_id = :mrss_id "
						"ORDER BY link");

				WriteMediaRSSScene_ = QSqlQuery (DB_);
				WriteMediaRSSScene_.prepare ("REPLACE INTO mrss_scenes ("
						"mrss_scene_id, "
						"mrss_id, "
						"title, "
						"description, "
						"start_time, "
						"end_time"
						") VALUES ("
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? , "
						" ? "
						")");

				GetMediaRSSScenes_ = QSqlQuery (DB_);
				GetMediaRSSScenes_.prepare ("SELECT "
						"mrss_scene_id, "
						"title, "
						"description, "
						"start_time, "
						"end_time "
						"FROM mrss_scenes "
						"WHERE mrss_id = ? "
						"ORDER BY start_time");
			
				RemoveEnclosures_ = QSqlQuery (DB_);
				RemoveEnclosures_.prepare ("DELETE FROM enclosures "
						"WHERE item_id = ? ");
			
				GetEnclosures_ = QSqlQuery (DB_);
				GetEnclosures_.prepare ("SELECT "
						"enclosure_id, "
						"url, "
						"type, "
						"length, "
						"lang "
						"FROM enclosures "
						"WHERE item_id = ? "
						"ORDER BY url");

				RemoveMediaRSS_ = QSqlQuery (DB_);
				RemoveMediaRSS_.prepare ("DELETE FROM mrss "
						"WHERE mrss_id = ? ");

				RemoveMediaRSSThumbnails_ = QSqlQuery (DB_);
				RemoveMediaRSSThumbnails_.prepare ("DELETE FROM mrss_thumbnails "
						"WHERE mrss_thumb_id = ? ");

				RemoveMediaRSSCredits_ = QSqlQuery (DB_);
				RemoveMediaRSSCredits_.prepare ("DELETE FROM mrss_credits "
						"WHERE mrss_credits_id = ? ");

				RemoveMediaRSSComments_ = QSqlQuery (DB_);
				RemoveMediaRSSComments_.prepare ("DELETE FROM mrss_comments "
						"WHERE mrss_comment_id = ? ");

				RemoveMediaRSSPeerLinks_ = QSqlQuery (DB_);
				RemoveMediaRSSPeerLinks_.prepare ("DELETE FROM mrss_peerlinks "
						"WHERE mrss_peerlink_id = ? ");

				RemoveMediaRSSScenes_ = QSqlQuery (DB_);
				RemoveMediaRSSScenes_.prepare ("DELETE FROM mrss_scenes "
						"WHERE mrss_scene_id = ? ");
			}
			
			void SQLStorageBackendMysql::GetFeedsIDs (ids_t& result) const
			{
				QSqlQuery feedSelector (DB_);
				if (!feedSelector.exec ("SELECT feed_id "
							"FROM feeds "
							"ORDER BY feed_id"))
				{
					Util::DBLock::DumpError (feedSelector);
					return;
				}
			
				while (feedSelector.next ())
					result.push_back (feedSelector.value (0).toInt ());
			}
			
			Feed_ptr SQLStorageBackendMysql::GetFeed (const IDType_t& feedId) const
			{
				FeedGetter_.bindValue (0, feedId);
				if (!FeedGetter_.exec ())
				{
					Util::DBLock::DumpError (FeedGetter_);
					throw FeedGettingError ();
				}

				if (!FeedGetter_.next ())
				{
					qWarning () << Q_FUNC_INFO
							<< "no feed found with"
							<< feedId;
					throw FeedNotFoundError ();
				}

				Feed_ptr feed (new Feed (feedId));
				feed->URL_ = FeedGetter_.value (0).toString ();
				feed->LastUpdate_ = FeedGetter_.value (1).toDateTime ();
				FeedGetter_.finish ();
				return feed;
			}

			IDType_t SQLStorageBackendMysql::FindFeed (const QString& url) const
			{
                FeedFinderByURL_.bindValue(0,url);
				if (!FeedFinderByURL_.exec ())
				{
					Util::DBLock::DumpError (FeedFinderByURL_);
					throw FeedGettingError ();
				}

				if (!FeedFinderByURL_.next ())
				{
                    /*
					qWarning () << Q_FUNC_INFO
							<< "no feed for"
							<< url;*/ //TODO check this
					return -1;
				}

				IDType_t id = FeedFinderByURL_.value (0).value<IDType_t> ();
				FeedFinderByURL_.finish ();
				return id;
			}

			Feed::FeedSettings SQLStorageBackendMysql::GetFeedSettings (const IDType_t& feedId) const
			{
				FeedSettingsGetter_.bindValue (0, feedId);
				if (!FeedSettingsGetter_.exec ())
				{
					Util::DBLock::DumpError (FeedSettingsGetter_);
					throw std::runtime_error (FeedSettingsGetter_
							.lastError ().text ().toStdString ());
				}
			
				if (!FeedSettingsGetter_.next ())
					throw FeedSettingsNotFoundError ();
			
				Feed::FeedSettings result (feedId,
						FeedSettingsGetter_.value (0).value<IDType_t> (),
						FeedSettingsGetter_.value (1).toInt (),
						FeedSettingsGetter_.value (2).toInt (),
						FeedSettingsGetter_.value (3).toInt (),
						FeedSettingsGetter_.value (4).toBool ());
				FeedSettingsGetter_.finish ();
			
				return result;
			}
			
			void SQLStorageBackendMysql::SetFeedSettings (const Feed::FeedSettings& settings)
			{
				FeedSettingsSetter_.bindValue (0, settings.FeedID_);//feed_id
				FeedSettingsSetter_.bindValue (1, settings.SettingsID_);
				FeedSettingsSetter_.bindValue (2, settings.UpdateTimeout_);
				FeedSettingsSetter_.bindValue (3, settings.NumItems_);
				FeedSettingsSetter_.bindValue (4, settings.ItemAge_);
				FeedSettingsSetter_.bindValue (5, settings.AutoDownloadEnclosures_);
			
				if (!FeedSettingsSetter_.exec ())
					LeechCraft::Util::DBLock::DumpError (FeedSettingsSetter_);
			}
			
			void SQLStorageBackendMysql::GetChannels (channels_shorts_t& shorts, const IDType_t& feedId) const
			{
				ChannelsShortSelector_.bindValue (0, feedId);//feed_id
				if (!ChannelsShortSelector_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (ChannelsShortSelector_);
					return;
				}
			
				while (ChannelsShortSelector_.next ())
				{
					int unread = 0;

					IDType_t id = ChannelsShortSelector_.value (0).value<IDType_t> ();

					UnreadItemsCounter_.bindValue (0, id);//channel_id
					if (!UnreadItemsCounter_.exec () ||
							!UnreadItemsCounter_.next ())
						Util::DBLock::DumpError (UnreadItemsCounter_);
					else
						unread = UnreadItemsCounter_.value (0).toInt ();
			
					UnreadItemsCounter_.finish ();
			
					QStringList tags = Core::Instance ().GetProxy ()->
						GetTagsManager ()->Split (ChannelsShortSelector_.value (3).toString ());
					ChannelShort sh =
					{
						id,
						feedId,
						ChannelsShortSelector_.value (6).toString (),
						ChannelsShortSelector_.value (1).toString (),
						ChannelsShortSelector_.value (2).toString (),
						tags,
						ChannelsShortSelector_.value (4).toDateTime (),
						UnserializePixmap (ChannelsShortSelector_
								.value (5).toByteArray ()),
						unread
					};
					shorts.push_back (sh);
				}
			
				ChannelsShortSelector_.finish ();
			}
			
			Channel_ptr SQLStorageBackendMysql::GetChannel (const IDType_t& channelId,
					const IDType_t& parentFeed) const
			{
				ChannelsFullSelector_.bindValue (0, channelId);//channelID
				if (!ChannelsFullSelector_.exec ())
					Util::DBLock::DumpError (ChannelsFullSelector_);
					
				if (!ChannelsFullSelector_.next ())
					throw ChannelNotFoundError ();
			
				Channel_ptr channel (new Channel (parentFeed, channelId));
			
				channel->Link_ = ChannelsFullSelector_.value (0).toString ();
				channel->Title_ = ChannelsFullSelector_.value (1).toString ();
				channel->Description_ = ChannelsFullSelector_.value (2).toString ();
				channel->LastBuild_ = ChannelsFullSelector_.value (3).toDateTime ();
				QString tags = ChannelsFullSelector_.value (4).toString ();
				channel->Tags_ = Core::Instance ().GetProxy ()->GetTagsManager ()->Split (tags);
				channel->Language_ = ChannelsFullSelector_.value (5).toString ();
				channel->Author_ = ChannelsFullSelector_.value (6).toString ();
				channel->PixmapURL_ = ChannelsFullSelector_.value (7).toString ();
				channel->Pixmap_ = UnserializePixmap (ChannelsFullSelector_
						.value (8).toByteArray ());
				channel->Favicon_ = UnserializePixmap (ChannelsFullSelector_
						.value (9).toByteArray ());
			
				ChannelsFullSelector_.finish ();
			
				return channel;
			}

			IDType_t SQLStorageBackendMysql::FindChannel (const QString& title,
					const QString& link, const IDType_t& feedId) const
			{
				ChannelIDFromTitleURL_.bindValue (0, feedId);//feed_id
				ChannelIDFromTitleURL_.bindValue (1, title);//title
				ChannelIDFromTitleURL_.bindValue (2, link);//url
				if (!ChannelIDFromTitleURL_.exec ())
				{
					Util::DBLock::DumpError (ChannelIDFromTitleURL_);
					throw ChannelGettingError ();
				}

				if (!ChannelIDFromTitleURL_.next ())
					throw ChannelNotFoundError ();

				IDType_t result = ChannelIDFromTitleURL_.value (0).value<IDType_t> ();
				ChannelIDFromTitleURL_.finish ();
				return result;
			}

			IDType_t SQLStorageBackendMysql::FindItem (const QString& title,
					const QString& link, const IDType_t& channelId) const
			{
				ItemIDFromTitleURL_.bindValue (0, channelId);//channel_id
				ItemIDFromTitleURL_.bindValue (1, title);//title
				ItemIDFromTitleURL_.bindValue (2, link);//url
				if (!ItemIDFromTitleURL_.exec ())
				{
					Util::DBLock::DumpError (ItemIDFromTitleURL_);
					throw ItemGettingError ();
				}

				if (!ItemIDFromTitleURL_.next ())
					throw ItemNotFoundError ();

				IDType_t result = ItemIDFromTitleURL_.value (0).value<IDType_t> ();
				ItemIDFromTitleURL_.finish ();
				return result;
			}

			void SQLStorageBackendMysql::TrimChannel (const IDType_t& channelId,
					int days, int number)
			{
				ChannelDateTrimmer_.bindValue (0 , channelId);//channel_id
				ChannelDateTrimmer_.bindValue (1 , days);//age
				if (!ChannelDateTrimmer_.exec ())
					LeechCraft::Util::DBLock::DumpError (ChannelDateTrimmer_);

				ChannelNumberTrimmer_.bindValue (2, channelId);//channel_id
				ChannelNumberTrimmer_.bindValue (3, number);//number
				if (!ChannelNumberTrimmer_.exec ())
					LeechCraft::Util::DBLock::DumpError (ChannelNumberTrimmer_);

				try
				{
					emit channelDataUpdated (GetChannel (channelId,
							FindParentFeedForChannel (channelId)));
				}
				catch (const ChannelNotFoundError&)
				{
					qWarning () << Q_FUNC_INFO
							<< "channel not found"
							<< channelId;
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "error updating channel data"
							<< channelId
							<< e.what ();
				}
			}
			
			void SQLStorageBackendMysql::GetItems (items_shorts_t& shorts,
					const IDType_t& channelId) const
			{
				ItemsShortSelector_.bindValue (0, channelId);//channel_id
			
				if (!ItemsShortSelector_.exec ())
				{
					Util::DBLock::DumpError (ItemsShortSelector_);
					return;
				}
			
				while (ItemsShortSelector_.next ())
				{
					ItemShort sh =
					{
						ItemsShortSelector_.value (0).value<IDType_t> (),
						channelId,
						ItemsShortSelector_.value (1).toString (),
						ItemsShortSelector_.value (2).toString (),
						ItemsShortSelector_.value (3).toString ()
							.split ("<<<", QString::SkipEmptyParts),
						ItemsShortSelector_.value (4).toDateTime (),
						ItemsShortSelector_.value (5).toBool ()
					};
			
					shorts.push_back (sh);
				}
			
				ItemsShortSelector_.finish ();
			}
			
			int SQLStorageBackendMysql::GetUnreadItems (const IDType_t& channelId) const
			{
				int unread = 0;
				UnreadItemsCounter_.bindValue (0, channelId);//channel_id
				if (!UnreadItemsCounter_.exec () ||
						!UnreadItemsCounter_.next ())
					Util::DBLock::DumpError (UnreadItemsCounter_);
				else
					unread = UnreadItemsCounter_.value (0).toInt ();
			
				UnreadItemsCounter_.finish ();
				return unread;
			}
			
			Item_ptr SQLStorageBackendMysql::GetItem (const IDType_t& itemId) const
			{
				ItemFullSelector_.bindValue (0, itemId);
				if (!ItemFullSelector_.exec ())
					Util::DBLock::DumpError (ItemFullSelector_);

				if (!ItemFullSelector_.next ())
					throw ItemNotFoundError ();
			
				Item_ptr item (new Item (ItemFullSelector_.value (13).toInt (),
						itemId));
				FillItem (ItemFullSelector_, item);
				ItemFullSelector_.finish ();
			
				GetEnclosures (itemId, item->Enclosures_);
				GetMRSSEntries (itemId, item->MRSSEntries_);
			
				return item;
			}

			void SQLStorageBackendMysql::GetItems (items_container_t& items,
					const IDType_t& channelId) const
			{
				ItemsFullSelector_.bindValue (":channel_id", channelId);
				if (!ItemsFullSelector_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (ItemsFullSelector_);
					return;
				}
			
				while (ItemsFullSelector_.next ())
				{
					IDType_t itemId = ItemsFullSelector_.value (14 ).value<IDType_t> ();
					Item_ptr item (new Item (channelId,
							itemId));
					FillItem (ItemsFullSelector_, item);
					GetEnclosures (itemId, item->Enclosures_);
					GetMRSSEntries (itemId, item->MRSSEntries_);
			
					items.push_back (item);
				}
			
				ItemsFullSelector_.finish ();
				GetEnclosures_.finish ();
			}
			
			void SQLStorageBackendMysql::AddFeed (Feed_ptr feed)
			{
				InsertFeed_.bindValue (0, feed->FeedID_);
				InsertFeed_.bindValue (1, feed->URL_);
				InsertFeed_.bindValue (2, feed->LastUpdate_);
				if (!InsertFeed_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (InsertFeed_);
					return;
				}
			
				try
				{
					std::for_each (feed->Channels_.begin (), feed->Channels_.end (),
						   boost::bind (&SQLStorageBackendMysql::AddChannel,
							   this,
							   _1));
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO << e.what ();
					return;
				}

				InsertFeed_.finish ();
			}
			
			void SQLStorageBackendMysql::UpdateChannel (Channel_ptr channel)
			{
				ChannelFinder_.bindValue (0, channel->ChannelID_);
				if (!ChannelFinder_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					Util::DBLock::DumpError (ChannelFinder_);
					throw std::runtime_error (qPrintable (QString (
									"Unable to execute channel finder query for channel {title: %1; url: %2}")
								.arg (channel->Title_)
								.arg (channel->Link_)));
				}
				if (!ChannelFinder_.next ())
				{
					qWarning () << Q_FUNC_INFO
						<< "not found channel"
						<< channel->Title_
						<< channel->Link_
						<< ", inserting it";
					AddChannel (channel);
					return;
				}
				ChannelFinder_.finish ();
			
				UpdateChannel_.bindValue (0, channel->ChannelID_);
				UpdateChannel_.bindValue (1, channel->Description_);
				UpdateChannel_.bindValue (2, channel->LastBuild_);
				UpdateChannel_.bindValue (3,
						Core::Instance ().GetProxy ()->GetTagsManager ()->Join (channel->Tags_));
				UpdateChannel_.bindValue (4, channel->Language_);
				UpdateChannel_.bindValue (5, channel->Author_);
				UpdateChannel_.bindValue (6, channel->PixmapURL_);
				UpdateChannel_.bindValue (7, SerializePixmap (channel->Pixmap_));
				UpdateChannel_.bindValue (8, SerializePixmap (channel->Favicon_));
			
				if (!UpdateChannel_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					Util::DBLock::DumpError (UpdateChannel_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to save channel t %1, u %2")
								.arg (channel->Title_)
								.arg (channel->Link_)));
				}

				if (!UpdateChannel_.numRowsAffected ())
					qWarning () << Q_FUNC_INFO
						<< "no rows affected by UpdateChannel_";
			
				UpdateChannel_.finish ();
			
				emit channelDataUpdated (channel);
			}
			
			void SQLStorageBackendMysql::UpdateChannel (const ChannelShort& channel)
			{
				ChannelFinder_.bindValue (0, channel.ChannelID_);
				if (!ChannelFinder_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (ChannelFinder_);
					throw std::runtime_error (qPrintable (QString (
									"Unable to execute channel finder query {title: %1, url: %2}")
								.arg (channel.Title_)
								.arg (channel.Link_)));
				}
				if (!ChannelFinder_.next ())
				{
					qWarning () << Q_FUNC_INFO;
					throw std::runtime_error (qPrintable (QString (
									"Selected channel for updating doesn't exist and we don't "
									"have enough info to insert it {title: %1, url: %2}")
								.arg (channel.Title_)
								.arg (channel.Link_)));
				}
				ChannelFinder_.finish ();
			
				UpdateShortChannel_.bindValue (0, channel.ChannelID_);
				UpdateShortChannel_.bindValue (1, channel.LastBuild_);
				UpdateShortChannel_.bindValue (2, Core::Instance ().GetProxy ()->GetTagsManager ()->Join (channel.Tags_));
			
				if (!UpdateShortChannel_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (UpdateShortChannel_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to save channel {title: %1, url: %2}")
								.arg (channel.Title_)
								.arg (channel.Link_)));
				}

				if (!UpdateShortChannel_.numRowsAffected ())
					qWarning () << Q_FUNC_INFO
						<< "no rows affected by UpdateShortChannel_";
			
				UpdateShortChannel_.finish ();
			
				try
				{
					emit channelDataUpdated (GetChannel (channel.ChannelID_,
							channel.FeedID_));
				}
				catch (const ChannelNotFoundError&)
				{
					qWarning () << Q_FUNC_INFO
						<< "channel not found"
						<< channel.Title_
						<< channel.Link_
						<< channel.ChannelID_;
				}
			}
			
			void SQLStorageBackendMysql::UpdateItem (Item_ptr item)
			{
				UpdateItem_.bindValue (0, item->ItemID_);
				UpdateItem_.bindValue (1, item->Description_);
				UpdateItem_.bindValue (2, item->Author_);
				UpdateItem_.bindValue (3, item->Categories_.join ("<<<"));
				UpdateItem_.bindValue (4, item->PubDate_);
				UpdateItem_.bindValue (5, item->Unread_);
				UpdateItem_.bindValue (6, item->NumComments_);
				UpdateItem_.bindValue (7, item->CommentsLink_);
				UpdateItem_.bindValue (8, item->CommentsPageLink_);
				UpdateItem_.bindValue (9, QString::number (item->Latitude_));
				UpdateItem_.bindValue (10, QString::number (item->Longitude_));
			
				if (!UpdateItem_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					Util::DBLock::DumpError (UpdateItem_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to save item {id: %1, title: %2, url: %3}")
								.arg (item->ItemID_)
								.arg (item->Title_)
								.arg (item->Link_)));
				}

				if (!UpdateItem_.numRowsAffected ())
					qWarning () << Q_FUNC_INFO
						<< "no rows affected by UpdateItem_";
			
				UpdateItem_.finish ();

				WriteEnclosures (item->Enclosures_);
				WriteMRSSEntries (item->MRSSEntries_);
			
				try
				{
					IDType_t cid = item->ChannelID_;
					Channel_ptr channel = GetChannel (cid,
							FindParentFeedForChannel (cid));
					emit itemDataUpdated (item, channel);
					emit channelDataUpdated (channel);
				}
				catch (const ChannelNotFoundError&)
				{
					qWarning () << Q_FUNC_INFO
						<< "channel not found"
						<< item->ChannelID_;
				}
			}
			
			void SQLStorageBackendMysql::UpdateItem (const ItemShort& item)
			{
				UpdateShortItem_.bindValue (1, item.ItemID_);
				UpdateShortItem_.bindValue (0, item.Unread_);
			
				if (!UpdateShortItem_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (UpdateShortItem_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to save item {id: %1, title: %2, url: %3}")
								.arg (item.ItemID_)
								.arg (item.Title_)
								.arg (item.URL_)));
				}

				/**
                 * @TODO check if we really need this warning 
                 *
                 * if (!UpdateShortItem_.numRowsAffected ())
				 *	qWarning () << Q_FUNC_INFO
				 *		<< "no rows affected by UpdateShortItem_";
                 */
			
				UpdateShortItem_.finish ();
			
				try
				{
					IDType_t cid = item.ChannelID_;
					Channel_ptr channel = GetChannel (cid,
							FindParentFeedForChannel (cid));
					emit itemDataUpdated (GetItem (item.ItemID_), channel);
					emit channelDataUpdated (channel);
				}
				catch (const ChannelNotFoundError&)
				{
					qWarning () << Q_FUNC_INFO
						<< "channel not found"
						<< item.ChannelID_;
				}
			}
			
			void SQLStorageBackendMysql::AddChannel (Channel_ptr channel)
			{
				InsertChannel_.bindValue (0, channel->ChannelID_);
				InsertChannel_.bindValue (1, channel->FeedID_);
				InsertChannel_.bindValue (2, channel->Link_);
				InsertChannel_.bindValue (3, channel->Title_);
				InsertChannel_.bindValue (4, channel->Description_);
				InsertChannel_.bindValue (5, channel->LastBuild_);
				InsertChannel_.bindValue (6,
						Core::Instance ().GetProxy ()->GetTagsManager ()->Join (channel->Tags_));
				InsertChannel_.bindValue (7, channel->Language_);
				InsertChannel_.bindValue (8, channel->Author_);
				InsertChannel_.bindValue (9, channel->PixmapURL_);
				InsertChannel_.bindValue (10, SerializePixmap (channel->Pixmap_));
				InsertChannel_.bindValue (11, SerializePixmap (channel->Favicon_));
			
				if (!InsertChannel_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					Util::DBLock::DumpError (InsertChannel_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to save channel {id: %1, title: %2, url: %3, parent: %4}")
								.arg (channel->ChannelID_)
								.arg (channel->Title_)
								.arg (channel->Link_)
								.arg (channel->FeedID_)));
				}
			
				InsertChannel_.finish ();
			
				std::for_each (channel->Items_.begin (), channel->Items_.end (),
					   boost::bind (&SQLStorageBackendMysql::AddItem,
						   this,
						   _1));
			}
			
			void SQLStorageBackendMysql::AddItem (Item_ptr item)
			{
				InsertItem_.bindValue (0, item->ItemID_);
				InsertItem_.bindValue (1, item->ChannelID_);
				InsertItem_.bindValue (2, item->Title_);
				InsertItem_.bindValue (3, item->Link_);
				InsertItem_.bindValue (4, item->Description_);
				InsertItem_.bindValue (5, item->Author_);
				InsertItem_.bindValue (6, item->Categories_.join ("<<<"));
				InsertItem_.bindValue (7, item->Guid_);
				InsertItem_.bindValue (8, item->PubDate_);
				InsertItem_.bindValue (9, item->Unread_);
				InsertItem_.bindValue (10, item->NumComments_);
				InsertItem_.bindValue (11, item->CommentsLink_);
				InsertItem_.bindValue (12, item->CommentsPageLink_);
				InsertItem_.bindValue (13, QString::number (item->Latitude_));
				InsertItem_.bindValue (14, QString::number (item->Longitude_));
			
				if (!InsertItem_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (InsertItem_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to save item {id: %1, channel: %2, title: %3, url: %4")
								.arg (item->ItemID_)
								.arg (item->ChannelID_)
								.arg (item->Title_)
								.arg (item->Link_)));
				}
			
				InsertItem_.finish ();

				WriteEnclosures (item->Enclosures_);
				WriteMRSSEntries (item->MRSSEntries_);

				try
				{
					IDType_t cid = item->ChannelID_;
					Channel_ptr channel = GetChannel (cid,
							FindParentFeedForChannel (cid));
					emit itemDataUpdated (item, channel);
					emit channelDataUpdated (channel);
				}
				catch (const ChannelNotFoundError&)
				{
					qWarning () << Q_FUNC_INFO
						<< "channel not found"
						<< item->ChannelID_;
				}
			}

			namespace
			{
				bool PerformRemove (QSqlQuery& query,
						const IDType_t& itemId)
				{
					query.bindValue (0, itemId);
					if (!query.exec ())
					{
						LeechCraft::Util::DBLock::DumpError (query);
						return false;
					}
				
					query.finish ();

					return true;
				}
			}
			
			void SQLStorageBackendMysql::RemoveItem (const IDType_t& itemId)
			{
				boost::optional<IDType_t> cid;
				try
				{
					Item_ptr item = GetItem (itemId);
					*cid = item->ChannelID_;
				}
				catch (const ItemNotFoundError&)
				{
					qWarning () << Q_FUNC_INFO
							<< "tried to delete item"
							<< itemId
							<< ", but it doesn't exist already";
					return;
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to obtain more info on"
							<< itemId
							<< "so won't update channel data";
				}

				Util::DBLock lock (DB_);
				try
				{
					lock.Init ();
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO << e.what ();
					return;
				}

				if (!PerformRemove (RemoveEnclosures_, itemId) ||
						!PerformRemove (RemoveMediaRSS_, itemId) ||
						!PerformRemove (RemoveMediaRSSThumbnails_, itemId) ||
						!PerformRemove (RemoveMediaRSSCredits_, itemId) ||
						!PerformRemove (RemoveMediaRSSComments_, itemId) ||
						!PerformRemove (RemoveMediaRSSPeerLinks_, itemId) ||
						!PerformRemove (RemoveMediaRSSScenes_, itemId))
				{
					qWarning () << Q_FUNC_INFO
						<< "a Remove* query failed";
					return;
				}

				RemoveItem_.bindValue (0, itemId);
			
				if (!RemoveItem_.exec ())
				{
					Util::DBLock::DumpError (RemoveItem_);
					return;
				}
			
				RemoveItem_.finish ();
			
				lock.Good ();
			
				if (cid)
				{
					try
					{
						Channel_ptr channel = GetChannel (*cid,
								FindParentFeedForChannel (*cid));
						emit channelDataUpdated (channel);
					}
					catch (const ChannelNotFoundError&)
					{
						qWarning () << Q_FUNC_INFO
							<< "channel not found"
							<< *cid;
					}
				}
			}
			
			void SQLStorageBackendMysql::RemoveFeed (const IDType_t& feedId)
			{
				Util::DBLock lock (DB_);
				try
				{
					lock.Init ();
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO
							<< e.what ();
					return;
				}
			
				RemoveFeed_.bindValue (0, feedId);
				if (!RemoveFeed_.exec ())
				{
					Util::DBLock::DumpError (RemoveFeed_);
					return;
				}
			
				RemoveFeed_.finish ();
			
				lock.Good ();
			}
			
			void SQLStorageBackendMysql::ToggleChannelUnread (const IDType_t& channelId,
					bool state)
			{
				items_container_t oldItems;
				GetItems (oldItems, channelId);

				ToggleChannelUnread_.bindValue (0, state);
				ToggleChannelUnread_.bindValue (1, channelId);
				ToggleChannelUnread_.bindValue (2, state);
			
				if (!ToggleChannelUnread_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (ToggleChannelUnread_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to toggle items {cid: %1, state %2}")
								.arg (channelId)
								.arg (state)));
				}
			
				ToggleChannelUnread_.finish ();
			
				try
				{
					Channel_ptr channel = GetChannel (channelId,
							FindParentFeedForChannel (channelId));
					emit channelDataUpdated (channel);
					for (size_t i = 0; i < oldItems.size (); ++i)
						if (oldItems.at (i)->Unread_ != state)
						{
							oldItems.at (i)->Unread_ = state;
							emit itemDataUpdated (oldItems.at (i), channel);
						}
				}
				catch (const ChannelNotFoundError&)
				{
					qWarning () << Q_FUNC_INFO
						<< "channel not found"
						<< channelId;
				}
			}
			
			bool SQLStorageBackendMysql::UpdateFeedsStorage (int, int)
			{
				return true;
			}
			
			bool SQLStorageBackendMysql::UpdateChannelsStorage (int, int)
			{
				return true;
			}
			
			bool SQLStorageBackendMysql::UpdateItemsStorage (int oldV, int newV)
			{
				bool success = true;
				while (oldV < newV)
				{
					success = RollItemsStorage (++oldV);
					if (!success)
						break;
				}
			
				return success;
			}

			QString SQLStorageBackendMysql::GetBoolType () const
			{
                return "TINYINT";
			}

			QString SQLStorageBackendMysql::GetBlobType () const
			{
                return "BLOB";
			}
			
			bool SQLStorageBackendMysql::InitializeTables ()
			{
				QSqlQuery query (DB_);
				if (!DB_.tables ().contains ("feeds"))
				{
                    if (!query.exec ( "CREATE TABLE feeds ("
                        "feed_id BIGINT PRIMARY KEY, "
                        "url VARCHAR(333) UNIQUE NOT NULL, "
                        "last_update TIMESTAMP );") )
                    {
						Util::DBLock::DumpError (query);
						return false;
					}
				}
			
				if (!DB_.tables ().contains ("feeds_settings"))
				{
					if (!query.exec ("CREATE TABLE feeds_settings ("
									"settings_id BIGINT PRIMARY KEY, "
									"feed_id BIGINT UNIQUE REFERENCES feeds ON DELETE CASCADE, "
									"update_timeout INTEGER NOT NULL, "
									"num_items INTEGER NOT NULL, "
									"item_age INTEGER NOT NULL, "
									"auto_download_enclosures TINYINT NOT NULL"
									") Engine=InnoDB;") )
					{
						Util::DBLock::DumpError (query);
						return false;
					}
			
				}
			
				if (!DB_.tables ().contains ("channels"))
				{
					if (!query.exec ("CREATE TABLE channels ("
							"channel_id BIGINT PRIMARY KEY, "
							"feed_id BIGINT NOT NULL REFERENCES feeds ON DELETE CASCADE, "
							"url TEXT, "
							"title TEXT, "
							"description TEXT, "
							"last_build TIMESTAMP, "
							"tags TEXT, "
							"language TEXT, "
							"author TEXT, "
							"pixmap_url TEXT, "
							"pixmap BLOB, "
							"favicon BLOB "
							");" ))
					{
						LeechCraft::Util::DBLock::DumpError (query);
						return false;
					}
				}
			
				if (!DB_.tables ().contains ("items"))
				{
					if (!query.exec ("CREATE TABLE items ("
							"item_id BIGINT PRIMARY KEY, "
							"channel_id BIGINT NOT NULL REFERENCES channels ON DELETE CASCADE, "
							"title TEXT, "
							"url TEXT, "
							"description TEXT, "
							"author TEXT, "
							"category TEXT, "
							"guid TEXT, "
							"pub_date TIMESTAMP, "
							"unread TINYINT, "
							"num_comments SMALLINT, "
							"comments_url TEXT, "
							"comments_page_url TEXT, "
							"latitude TEXT, "
							"longitude TEXT"
							");"))
					{
						LeechCraft::Util::DBLock::DumpError (query);
						return false;
					}

					if (!query.exec ("CREATE INDEX idx_items_channel_id ON items (channel_id);"))
					{
						Util::DBLock::DumpError (query);
						qWarning () << Q_FUNC_INFO
								<< "could not create index, performance would suffer";
					}
				}
			
				if (!DB_.tables ().contains ("enclosures"))
				{
					if (!query.exec ("CREATE TABLE enclosures ("
								"enclosure_id BIGINT PRIMARY KEY, "
								"item_id BIGINT NOT NULL REFERENCES items ON DELETE CASCADE, "
								"url TEXT NOT NULL, "
								"type TEXT NOT NULL, "
								"length BIGINT NOT NULL, "
								"lang TEXT "
								");"))
					{
						Util::DBLock::DumpError (query);
						return false;
					}
			
				}

				if (!DB_.tables ().contains ("mrss"))
				{
					if (!query.exec (QString ("CREATE TABLE mrss ("
									"mrss_id BIGINT PRIMARY KEY, "
									"item_id BIGINT NOT NULL REFERENCES items ON DELETE CASCADE, "
									"url TEXT, "
									"size BIGINT, "
									"type TEXT, "
									"medium TEXT, "
									"is_default %1, "
									"expression TEXT, "
									"bitrate INTEGER, "
									"framerate REAL, "
									"samplingrate REAL, "
									"channels SMALLINT, "
									"duration INTEGER, "
									"width INTEGER, "
									"height INTEGER, "
									"lang TEXT, "
									"mediagroup INTEGER, "
									"rating TEXT, "
									"rating_scheme TEXT, "
									"title TEXT, "
									"description TEXT, "
									"keywords TEXT, "
									"copyright_url TEXT, "
									"copyright_text TEXT, "
									"star_rating_average SMALLINT, "
									"star_rating_count INTEGER, "
									"star_rating_min SMALLINT, "
									"star_rating_max SMALLINT, "
									"stat_views INTEGER, "
									"stat_favs INTEGER, "
									"tags TEXT, "
									"item_parents_hash TEXT, "
									"item_title TEXT, "
									"item_url TEXT"
									");").arg (GetBoolType ())))
					{
						Util::DBLock::DumpError (query);
						return false;
					}

				}
				
				if (!DB_.tables ().contains ("mrss_thumbnails"))
				{
					if (!query.exec ("CREATE TABLE mrss_thumbnails ("
								"mrss_thumb_id BIGINT PRIMARY KEY, "
								"mrss_id BIGINT NOT NULL REFERENCES mrss ON DELETE CASCADE, "
								"url TEXT, "
								"width INTEGER, "
								"height INTEGER, "
								"time TEXT"
								");"))
					{
						Util::DBLock::DumpError (query);
						return false;
					}

				}

				if (!DB_.tables ().contains ("mrss_credits"))
				{
					if (!query.exec ("CREATE TABLE mrss_credits ("
								"mrss_credits_id BIGINT PRIMARY KEY, "
								"mrss_id BIGINT NOT NULL REFERENCES mrss ON DELETE CASCADE, "
								"role TEXT, "
								"who TEXT"
								");"))
					{
						Util::DBLock::DumpError (query);
						return false;
					}

				}

				if (!DB_.tables ().contains ("mrss_comments"))
				{
					if (!query.exec ("CREATE TABLE mrss_comments ("
								"mrss_comment_id BIGINT PRIMARY KEY, "
								"mrss_id BIGINT NOT NULL REFERENCES mrss ON DELETE CASCADE, "
								"type TEXT, "
								"comment TEXT"
								");"))
					{
						Util::DBLock::DumpError (query);
						return false;
					}

				}

				if (!DB_.tables ().contains ("mrss_peerlinks"))
				{
					if (!query.exec ("CREATE TABLE mrss_peerlinks ("
								"mrss_peerlink_id BIGINT PRIMARY KEY, "
								"mrss_id BIGINT NOT NULL REFERENCES mrss ON DELETE CASCADE, "
								"type TEXT, "
								"link TEXT"
								");"))
					{
						Util::DBLock::DumpError (query.lastError ());
						return false;
					}

				}

				if (!DB_.tables ().contains ("mrss_scenes"))
				{
					if (!query.exec ("CREATE TABLE mrss_scenes ("
								"mrss_scene_id BIGINT PRIMARY KEY, "
								"mrss_id BIGINT NOT NULL REFERENCES mrss ON DELETE CASCADE, "
								"title TEXT, "
								"description TEXT, "
								"start_time TEXT, "
								"end_time TEXT"
								");"))
					{
						Util::DBLock::DumpError (query.lastError ());
						return false;
					}

				}

				return true;
			}
			
			QByteArray SQLStorageBackendMysql::SerializePixmap (const QPixmap& pixmap) const
			{
				QByteArray bytes;
				if (!pixmap.isNull ())
				{
					QBuffer buffer (&bytes);
					buffer.open (QIODevice::WriteOnly);
					pixmap.save (&buffer, "PNG");
				}
				return bytes;
			}
			
			QPixmap SQLStorageBackendMysql::UnserializePixmap (const QByteArray& bytes) const
			{
				QPixmap result;
				if (bytes.size ())
					result.loadFromData (bytes, "PNG");
				return result;
			}
			
			bool SQLStorageBackendMysql::RollItemsStorage (int version)
			{
				LeechCraft::Util::DBLock lock (DB_);
				try
				{
					lock.Init ();
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO << e.what ();
					return false;
				}
			
				if (version == 2)
				{
					QSqlQuery updateQuery = QSqlQuery (DB_);
					if (!updateQuery.exec ("ALTER TABLE items "
							"ADD num_comments SMALLINT"))
					{
						LeechCraft::Util::DBLock::DumpError (updateQuery);
						return false;
					}
			
					if (!updateQuery.exec ("ALTER TABLE items "
								"ADD comments_url TEXT"))
					{
						LeechCraft::Util::DBLock::DumpError (updateQuery);
						return false;
					}
			
					if (!updateQuery.exec ("UPDATE items "
								"SET num_comments = -1"))
					{
						LeechCraft::Util::DBLock::DumpError (updateQuery);
						return false;
					}
				}
				else if (version == 3)
				{
					QSqlQuery updateQuery = QSqlQuery (DB_);
					if (!updateQuery.exec ("ALTER TABLE items "
								"ADD comments_page_url TEXT"))
					{
						LeechCraft::Util::DBLock::DumpError (updateQuery);
						return false;
					}
				}
				else if (version == 4)
				{
					QString adeType = "TINYINT";

					QSqlQuery updateQuery = QSqlQuery (DB_);
					if (!updateQuery.exec (QString ("ALTER TABLE feeds_settings "
									"ADD auto_download_enclosures %1").arg (adeType)))
					{
						LeechCraft::Util::DBLock::DumpError (updateQuery);
						return false;
					}
				}
				else if (version == 5)
				{
					QSqlQuery updateQuery = QSqlQuery (DB_);
					if (!(updateQuery.exec (QString ("ALTER TABLE items "
										"ADD latitude TEXT")) &&
								updateQuery.exec (QString ("ALTER TABLE items "
										"ADD longitude TEXT"))))
					{
						LeechCraft::Util::DBLock::DumpError (updateQuery);
						return false;
					}
				}
				else if (version == 6)
				{
					qDebug () << Q_FUNC_INFO
							<< "preparing to migrate to version 6 of SQL storage format";

					qDebug () << Q_FUNC_INFO << "loading feeds...";
					QList<Feed_ptr> feeds;
					try
					{
						feeds = LoadFeedsFromVersion5 ();
					}
					catch (const std::exception& e)
					{
						qWarning () << Q_FUNC_INFO
								<< "unable to temporarily load feeds, aborting"
								<< e.what ();
						return false;
					}

					qDebug () << Q_FUNC_INFO << "loading settings...";

					QList<Feed::FeedSettings> settings;
					try
					{
						Q_FOREACH (Feed_ptr feed, feeds)
						{
							try
							{
								settings << GetFeedSettingsFromVersion5 (feed);
							}
							catch (const FeedSettingsNotFoundError&)
							{
								continue;
							}
						}
					}
					catch (const std::exception& e)
					{
						qWarning () << Q_FUNC_INFO
								<< "unable to temporarily load settings, aborting"
								<< e.what ();
						return false;
					}

					qDebug () << Q_FUNC_INFO << "loaded, dropping data...";

					try
					{
						RemoveTables ();
					}
					catch (const std::exception& e)
					{
						qWarning () << Q_FUNC_INFO
								<< "unable to drop tables, aborting"
								<< e.what ();
						return false;
					}

					qDebug () << Q_FUNC_INFO << "DB has now" << DB_.tables ();
					qDebug () << Q_FUNC_INFO << "initializing fresh tables...";

					if (!InitializeTables ())
						return false;

					qDebug () << Q_FUNC_INFO << "(re)initializing queries...";

					Prepare ();

					qDebug () << Q_FUNC_INFO << "re-adding feeds...";

					Q_FOREACH (Feed_ptr feed, feeds)
					{
						qDebug () << "adding feed" << feed->URL_;
						AddFeed (feed);
					}

					qDebug () << Q_FUNC_INFO << "re-adding settings...";

					Q_FOREACH (Feed::FeedSettings s, settings)
						SetFeedSettings (s);

					qDebug () << Q_FUNC_INFO << "syncing pools and exiting";

					Core::Instance ().SyncPools ();
				}
			
				lock.Good ();
				return true;
			}
			
			void SQLStorageBackendMysql::RemoveTables ()
			{

				struct
				{
					const QSqlDatabase& ThisDB_;

					void operator() (const QString& drstr)
					{
						QSqlQuery dropper = QSqlQuery (ThisDB_);
						if (!dropper.exec (drstr))
						{
							Util::DBLock::DumpError (dropper);
							throw std::runtime_error (qPrintable (dropper
									.lastError ().text ()));
						}
					}
				} rd = { DB_ };

				rd ("ALTER TABLE feeds DROP CONSTRAINT feeds_pkey;");
				rd ("ALTER TABLE enclosures DROP CONSTRAINT enclosures_pkey;");
				rd ("ALTER TABLE feeds_settings DROP CONSTRAINT feeds_settings_pkey;");
				rd ("ALTER TABLE mrss DROP CONSTRAINT mrss_pkey;");
				rd ("ALTER TABLE mrss_credits DROP CONSTRAINT mrss_credits_pkey;");
				rd ("ALTER TABLE mrss_thumbnails DROP CONSTRAINT mrss_thumbnails_pkey;");
				rd ("DROP INDEX idx_enclosures_item_parents_hash_item_title_item_url;");
				rd ("DROP INDEX idx_channels_parent_feed_url;");
				rd ("DROP INDEX idx_channels_parent_feed_url_title;");
				rd ("DROP INDEX idx_channels_parent_feed_url_title_url;");
				rd ("DROP INDEX idx_items_parents_hash;");
				rd ("DROP INDEX idx_items_parents_hash_title_url;");
				rd ("DROP INDEX idx_items_parents_hash_unread;");
				rd ("DROP INDEX idx_items_title;");
				rd ("DROP INDEX idx_items_url;");
				rd ("DROP INDEX idx_mrss_item_parents_hash_item_title_item_url;");
				rd ("DROP INDEX idx_mrss_item_title;");
				rd ("DROP INDEX idx_mrss_item_url;");
				rd ("DROP INDEX idx_mrss_comments_parent_url_item_parents_hash_item_title_item_;");
				rd ("DROP INDEX idx_mrss_credits_parent_url_item_parents_hash_item_title_item_u;");
				rd ("DROP INDEX idx_mrss_peerlinks_parent_url_item_parents_hash_item_title_item;");
				rd ("DROP INDEX idx_mrss_scenes_parent_url_item_parents_hash_item_title_item_ur;");
				rd ("DROP INDEX idx_mrss_thumbnails_parent_url_item_parents_hash_item_title_ite;");

				rd ("DROP TABLE "
					"channels, enclosures, feeds, "
					"feeds_settings, items, mrss, "
					"mrss_comments, mrss_credits, "
					"mrss_peerlinks, mrss_scenes, "
					"mrss_thumbnails");
			}

			Feed::FeedSettings SQLStorageBackendMysql::GetFeedSettingsFromVersion5 (Feed_ptr feed) const
			{
				Feed::FeedSettings settings = Feed::FeedSettings (feed->FeedID_);

				QSqlQuery getter = QSqlQuery (DB_);
				getter.prepare ("SELECT "
						"update_timeout, "
						"num_items, "
						"item_age, "
						"auto_download_enclosures "
						"FROM feeds_settings "
						"WHERE feed_url = :feed_url");
				getter.bindValue (":feed_url", feed->URL_);

				if (!getter.exec ())
				{
					Util::DBLock::DumpError (getter);
					throw std::runtime_error (qPrintable (getter.lastError ().text ()));
				}

				if (!getter.next ())
					throw FeedSettingsNotFoundError ();

				settings.UpdateTimeout_ = getter.value (0).toInt ();
				settings.NumItems_ = getter.value (1).toInt ();
				settings.ItemAge_ = getter.value (2).toInt ();
				settings.AutoDownloadEnclosures_ = getter.value (3).toBool ();

				return settings;
			}

			QList<Feed_ptr> SQLStorageBackendMysql::LoadFeedsFromVersion5 () const
			{
				QList<Feed_ptr> result = GetFeedsFromVersion5 ();

				Q_FOREACH (Feed_ptr feed, result)
				{
					QList<Channel_ptr> channels =
							GetChannelsFromVersion5 (feed->URL_, feed->FeedID_);
					Q_FOREACH (Channel_ptr channel, channels)
					{
						QString hash = feed->URL_ + channel->Title_;
						channel->Items_ =
								GetItemsFromVersion5 (hash, channel->ChannelID_)
									.toVector ().toStdVector ();
					}

					feed->Channels_ = channels.toVector ().toStdVector ();
				}

				return result;
			}

			QList<Feed_ptr> SQLStorageBackendMysql::GetFeedsFromVersion5 () const
			{
				QSqlQuery feedSelector = QSqlQuery (DB_);
				if (!feedSelector.exec ("SELECT url,"
						"last_update "
						"FROM feeds "
						"ORDER BY url"))
				{
					Util::DBLock::DumpError (feedSelector);
					throw std::runtime_error (qPrintable (feedSelector.lastError ().text ()));
				}

				QList<Feed_ptr> result;
				while (feedSelector.next ())
				{
					Feed_ptr feed (new Feed ());
					feed->URL_ = feedSelector.value (0).toString ();
					feed->LastUpdate_ = feedSelector.value (0).toDateTime ();
					result << feed;
				}
				return result;
			}

			QList<Channel_ptr> SQLStorageBackendMysql::GetChannelsFromVersion5 (const QString& feedUrl,
					const IDType_t& feedId) const
			{
				QSqlQuery channelsSelector = QSqlQuery (DB_);
				channelsSelector.prepare ("SELECT "
						"url, "
						"title, "
						"description, "
						"last_build, "
						"tags, "
						"language, "
						"author, "
						"pixmap_url, "
						"pixmap, "
						"favicon "
						"FROM channels "
						"WHERE parent_feed_url = :parent_feed_url "
						"ORDER BY title");
				channelsSelector.bindValue (":parent_feed_url", feedUrl);
				if (!channelsSelector.exec ())
				{
					Util::DBLock::DumpError (channelsSelector);
					throw std::runtime_error (qPrintable (channelsSelector.lastError ().text ()));
				}

				QList<Channel_ptr> result;
				while (channelsSelector.next ())
				{
					Channel_ptr channel (new Channel (feedId));

					channel->Link_ = channelsSelector.value (0).toString ();
					channel->Title_ = channelsSelector.value (1).toString ();
					channel->Description_ = channelsSelector.value (2).toString ();
					channel->LastBuild_ = channelsSelector.value (3).toDateTime ();
					QString tags = channelsSelector.value (4).toString ();
					channel->Tags_ = Core::Instance ().GetProxy ()->GetTagsManager ()->Split (tags);
					channel->Language_ = channelsSelector.value (5).toString ();
					channel->Author_ = channelsSelector.value (6).toString ();
					channel->PixmapURL_ = channelsSelector.value (7).toString ();
					channel->Pixmap_ = UnserializePixmap (channelsSelector
							.value (8).toByteArray ());
					channel->Favicon_ = UnserializePixmap (channelsSelector
							.value (9).toByteArray ());

					result << channel;
				}
				return result;
			}

			QList<Item_ptr> SQLStorageBackendMysql::GetItemsFromVersion5 (const QString& hash,
					const IDType_t& channelId) const
			{
				QSqlQuery itemsSelector = QSqlQuery (DB_);
				itemsSelector.prepare ("SELECT "
						"title, "
						"url, "
						"description, "
						"author, "
						"category, "
						"guid, "
						"pub_date, "
						"unread, "
						"num_comments, "
						"comments_url, "
						"comments_page_url, "
						"latitude, "
						"longitude "
						"FROM items "
						"WHERE parents_hash = ? "
						"ORDER BY unread ASC, pub_date DESC");
				itemsSelector.bindValue (0, hash);

				if (!itemsSelector.exec ())
				{
					Util::DBLock::DumpError (itemsSelector);
					throw std::runtime_error (qPrintable (itemsSelector.lastError ().text ()));
				}

				QList<Item_ptr> result;
				while (itemsSelector.next ())
				{
					Item_ptr item (new Item (channelId));
					FillItemVersion5 (itemsSelector, item);
					GetEnclosuresVersion5 (hash, item->Title_,
							item->Link_, item->Enclosures_, item->ItemID_);
					GetMRSSEntriesVersion5 (hash, item->Title_,
							item->Link_, item->MRSSEntries_, item->ItemID_);
					result << item;
				}
				return result;
			}

			void SQLStorageBackendMysql::FillItemVersion5 (const QSqlQuery& query, Item_ptr& item) const
			{
				item->Title_ = query.value (0).toString ();
				item->Link_ = query.value (1).toString ();
				item->Description_ = query.value (2).toString ();
				item->Author_ = query.value (3).toString ();
				item->Categories_ = query.value (4).toString ().split ("<<<", QString::SkipEmptyParts);
				item->Guid_ = query.value (5).toString ();
				item->PubDate_ = query.value (6).toDateTime ();
				item->Unread_ = query.value (7).toBool ();
				item->NumComments_ = query.value (8).toInt ();
				item->CommentsLink_ = query.value (9).toString ();
				item->CommentsPageLink_ = query.value (10).toString ();
				item->Latitude_ = query.value (11).toString ().toDouble ();
				item->Longitude_ = query.value (12).toString ().toDouble ();
			}

			void SQLStorageBackendMysql::GetEnclosuresVersion5 (const QString& hash, const QString& title,
					const QString& link, QList<Enclosure>& enclosures, const IDType_t& itemId) const
			{
				QSqlQuery getter = QSqlQuery (DB_);
				getter.prepare ("SELECT "
						"url, "
						"type, "
						"length, "
						"lang "
						"FROM enclosures "
						"WHERE item_parents_hash = ? "
						"AND item_title = ? "
						"AND item_url = ? "
						"ORDER BY url");
				getter.bindValue (0, hash);
				getter.bindValue (1, title);
				getter.bindValue (2, link);

				if (!getter.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (GetEnclosures_);
					return;
				}

				while (getter.next ())
				{
					Enclosure e (itemId);
					e.URL_ = getter.value (0).toString ();
					e.Type_ = getter.value (1).toString ();
					e.Length_ = getter.value (2).toLongLong ();
					e.Lang_ = getter.value (3).toString ();
					enclosures << e;
				}
			}

			void SQLStorageBackendMysql::GetMRSSEntriesVersion5 (const QString& hash, const QString& title,
					const QString& link, QList<MRSSEntry>& entries, const IDType_t& itemId) const
			{
				QSqlQuery getMediaRSSs = QSqlQuery (DB_);
				getMediaRSSs.prepare ("SELECT "
						"url, "
						"size, "
						"type, "
						"medium, "
						"is_default, "
						"expression, "
						"bitrate, "
						"framerate, "
						"samplingrate, "
						"channels, "
						"duration, "
						"width, "
						"height, "
						"lang, "
						"mediagroup, "
						"rating, "
						"rating_scheme, "
						"title, "
						"description, "
						"keywords, "
						"copyright_url, "
						"copyright_text, "
						"star_rating_average, "
						"star_rating_count, "
						"star_rating_min, "
						"star_rating_max, "
						"stat_views, "
						"stat_favs, "
						"tags "
						"FROM mrss "
						"WHERE item_parents_hash = ? "
						"AND item_title = ? "
						"AND item_url = ? "
						"ORDER BY title");
				getMediaRSSs.bindValue (0, hash);
				getMediaRSSs.bindValue (1, title);
				getMediaRSSs.bindValue (2, link);

				if (!getMediaRSSs.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (getMediaRSSs);
					return;
				}

				while (getMediaRSSs.next ())
				{
					QString eUrl = getMediaRSSs.value (0).toString ();
					MRSSEntry e (itemId);
					e.URL_ = eUrl;
					e.Size_ = getMediaRSSs.value (1).toLongLong ();
					e.Type_ = getMediaRSSs.value (2).toString ();
					e.Medium_ = getMediaRSSs.value (3).toString ();
					e.IsDefault_ = getMediaRSSs.value (4).toBool ();
					e.Expression_ = getMediaRSSs.value (5).toString ();
					e.Bitrate_ = getMediaRSSs.value (6).toInt ();
					e.Framerate_ = getMediaRSSs.value (7).toDouble ();
					e.SamplingRate_ = getMediaRSSs.value (8).toDouble ();
					e.Channels_ = getMediaRSSs.value (9).toInt ();
					e.Duration_ = getMediaRSSs.value (10).toInt ();
					e.Width_ = getMediaRSSs.value (11).toInt ();
					e.Height_ = getMediaRSSs.value (12).toInt ();
					e.Lang_ = getMediaRSSs.value (13).toString ();
					e.Group_ = getMediaRSSs.value (14).toInt ();
					e.Rating_ = getMediaRSSs.value (15).toString ();
					e.RatingScheme_ = getMediaRSSs.value (16).toString ();
					e.Title_ = getMediaRSSs.value (17).toString ();
					e.Description_ = getMediaRSSs.value (18).toString ();
					e.Keywords_ = getMediaRSSs.value (19).toString ();
					e.CopyrightURL_ = getMediaRSSs.value (20).toString ();
					e.CopyrightText_ = getMediaRSSs.value (21).toString ();
					e.RatingAverage_ = getMediaRSSs.value (22).toInt ();
					e.RatingCount_ = getMediaRSSs.value (23).toInt ();
					e.RatingMin_ = getMediaRSSs.value (24).toInt ();
					e.RatingMax_ = getMediaRSSs.value (25).toInt ();
					e.Views_ = getMediaRSSs.value (26).toInt ();
					e.Favs_ = getMediaRSSs.value (27).toInt ();
					e.Tags_ = getMediaRSSs.value (28).toString ();

					QSqlQuery getMediaRSSThumbnails = QSqlQuery (DB_);
					getMediaRSSThumbnails.prepare ("SELECT "
							"url, "
							"width, "
							"height, "
							"time "
							"FROM mrss_thumbnails "
							"WHERE parent_url = ? "
							"AND item_parents_hash = ? "
							"AND item_title = ? "
							"AND item_url = ? "
							"ORDER BY time");
					getMediaRSSThumbnails.bindValue (0, eUrl);
					getMediaRSSThumbnails.bindValue (1, hash);
					getMediaRSSThumbnails.bindValue (2, title);
					getMediaRSSThumbnails.bindValue (3, link);
					if (!getMediaRSSThumbnails.exec ())
						LeechCraft::Util::DBLock::DumpError (getMediaRSSThumbnails);
					else
					{
						while (getMediaRSSThumbnails.next ())
						{
							MRSSThumbnail th (e.MRSSEntryID_);
							th.URL_ = GetMediaRSSThumbnails_.value (0).toString ();
							th.Width_ = GetMediaRSSThumbnails_.value (1).toInt ();
							th.Height_ = GetMediaRSSThumbnails_.value (2).toInt ();
							th.Time_ = GetMediaRSSThumbnails_.value (3).toString ();
							e.Thumbnails_ << th;
						}
						getMediaRSSThumbnails.finish ();
					}

					QSqlQuery getMediaRSSCredits = QSqlQuery (DB_);
					getMediaRSSCredits.prepare ("SELECT "
							"role, "
							"who "
							"FROM mrss_credits "
							"WHERE parent_url = ? "
							"AND item_parents_hash = ? "
							"AND item_title = ? "
							"AND item_url = ? "
							"ORDER BY role");
					getMediaRSSCredits.bindValue (0, eUrl);
					getMediaRSSCredits.bindValue (1, hash);
					getMediaRSSCredits.bindValue (2, title);
					getMediaRSSCredits.bindValue (3, link);
					if (!getMediaRSSCredits.exec ())
						LeechCraft::Util::DBLock::DumpError (getMediaRSSCredits);
					else
					{
						while (getMediaRSSCredits.next ())
						{
							MRSSCredit cr (e.MRSSEntryID_);
							cr.Role_ = getMediaRSSCredits.value (0).toString ();
							cr.Who_ = getMediaRSSCredits.value (1).toString ();
							e.Credits_ << cr;
						}
						getMediaRSSCredits.finish ();
					}

					QSqlQuery getMediaRSSComments = QSqlQuery (DB_);
					getMediaRSSComments.prepare ("SELECT "
							"type, "
							"comment "
							"FROM mrss_comments "
							"WHERE parent_url = ? "
							"AND item_parents_hash = ? "
							"AND item_title = ? "
							"AND item_url = ? "
							"ORDER BY comment");
					getMediaRSSComments.bindValue (0, eUrl);
					getMediaRSSComments.bindValue (1, hash);
					getMediaRSSComments.bindValue (2, title);
					getMediaRSSComments.bindValue (3, link);
					if (!getMediaRSSComments.exec ())
						LeechCraft::Util::DBLock::DumpError (getMediaRSSComments);
					else
					{
						while (getMediaRSSComments.next ())
						{
							MRSSComment cm (e.MRSSEntryID_);
							cm.Type_ = getMediaRSSComments.value (0).toString ();
							cm.Comment_ = getMediaRSSComments.value (1).toString ();
							e.Comments_ << cm;
						}
						getMediaRSSComments.finish ();
					}

					QSqlQuery getMediaRSSPeerLinks = QSqlQuery (DB_);
					getMediaRSSPeerLinks.prepare ("SELECT "
							"type, "
							"link "
							"FROM mrss_peerlinks "
							"WHERE parent_url = ? "
							"AND item_parents_hash = ? "
							"AND item_title = ? "
							"AND item_url = ? "
							"ORDER BY link");
					getMediaRSSPeerLinks.bindValue (0, eUrl);
					getMediaRSSPeerLinks.bindValue (1, hash);
					getMediaRSSPeerLinks.bindValue (2, title);
					getMediaRSSPeerLinks.bindValue (3, link);
					if (!getMediaRSSPeerLinks.exec ())
						LeechCraft::Util::DBLock::DumpError (getMediaRSSPeerLinks);
					else
					{
						while (getMediaRSSPeerLinks.next ())
						{
							MRSSPeerLink pl (e.MRSSEntryID_);
							pl.Type_ = getMediaRSSPeerLinks.value (0).toString ();
							pl.Link_ = getMediaRSSPeerLinks.value (1).toString ();
							e.PeerLinks_ << pl;
						}
						getMediaRSSPeerLinks.finish ();
					}

					QSqlQuery getMediaRSSScenes = QSqlQuery (DB_);
					getMediaRSSScenes.prepare ("SELECT "
							"title, "
							"description, "
							"start_time, "
							"end_time "
							"FROM mrss_scenes "
							"WHERE parent_url = ? "
							"AND item_parents_hash = ? "
							"AND item_title = ? "
							"AND item_url = ? "
							"ORDER BY start_time");
					getMediaRSSScenes.bindValue (0, eUrl);
					getMediaRSSScenes.bindValue (1, hash);
					getMediaRSSScenes.bindValue (2, title);
					getMediaRSSScenes.bindValue (3, link);
					if (!getMediaRSSScenes.exec ())
						LeechCraft::Util::DBLock::DumpError (getMediaRSSScenes);
					else
					{
						while (getMediaRSSScenes.next ())
						{
							MRSSScene th (e.MRSSEntryID_);
							th.Title_ = getMediaRSSScenes.value (0).toString ();
							th.Description_ = getMediaRSSScenes.value (1).toString ();
							th.StartTime_ = getMediaRSSScenes.value (2).toString ();
							th.EndTime_ = getMediaRSSScenes.value (3).toString ();
							e.Scenes_ << th;
						}
						getMediaRSSScenes.finish ();
					}

					entries << e;
				}
			}

			IDType_t SQLStorageBackendMysql::FindParentFeedForChannel (const IDType_t& channel) const
			{
				QSqlQuery query (DB_);
				query.prepare ("SELECT feed_id FROM channels WHERE channel_id = ? ");
				query.bindValue (0, channel);
				if (!query.exec ())
				{
					Util::DBLock::DumpError (query);
					throw std::runtime_error ("Unable to find parent feed for channel");
				}

				if (!query.next ())
					throw ChannelNotFoundError ();

				return query.value (0).value<IDType_t> ();
			}

			void SQLStorageBackendMysql::FillItem (const QSqlQuery& query, Item_ptr& item) const
			{
				item->Title_ = query.value (0).toString ();
				item->Link_ = query.value (1).toString ();
				item->Description_ = query.value (2).toString ();
				item->Author_ = query.value (3).toString ();
				item->Categories_ = query.value (4).toString ().split ("<<<", QString::SkipEmptyParts);
				item->Guid_ = query.value (5).toString ();
				item->PubDate_ = query.value (6).toDateTime ();
				item->Unread_ = query.value (7).toBool ();
				item->NumComments_ = query.value (8).toInt ();
				item->CommentsLink_ = query.value (9).toString ();
				item->CommentsPageLink_ = query.value (10).toString ();
				item->Latitude_ = query.value (11).toString ().toDouble ();
				item->Longitude_ = query.value (12).toString ().toDouble ();
				item->ChannelID_ = query.value (13).toString ().toDouble ();
			}

			void SQLStorageBackendMysql::WriteEnclosures (const QList<Enclosure>& enclosures)
			{
				for (QList<Enclosure>::const_iterator i = enclosures.begin (),
						end = enclosures.end (); i != end; ++i)
				{
					WriteEnclosure_.bindValue (4, i->ItemID_);
					WriteEnclosure_.bindValue (5, i->EnclosureID_);
					WriteEnclosure_.bindValue (0, i->URL_);
					WriteEnclosure_.bindValue (1, i->Type_);
					WriteEnclosure_.bindValue (2, i->Length_);
					WriteEnclosure_.bindValue (3, i->Lang_);
			
					if (!WriteEnclosure_.exec ())
						LeechCraft::Util::DBLock::DumpError (WriteEnclosure_);
				}
			
				WriteEnclosure_.finish ();
			}
			
			void SQLStorageBackendMysql::GetEnclosures (const IDType_t& itemId,
					QList<Enclosure>& enclosures) const
			{
				GetEnclosures_.bindValue (0, itemId);
			
				if (!GetEnclosures_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (GetEnclosures_);
					return;
				}
			
				while (GetEnclosures_.next ())
				{
					Enclosure e (itemId, GetEnclosures_.value (0).value<IDType_t> ());
					e.URL_ = GetEnclosures_.value (1).toString ();
					e.Type_ = GetEnclosures_.value (2).toString ();
					e.Length_ = GetEnclosures_.value (3).toLongLong ();
					e.Lang_ = GetEnclosures_.value (4).toString ();
			
					enclosures << e;
				}
			
				GetEnclosures_.finish ();
			}

			void SQLStorageBackendMysql::WriteMRSSEntries (const QList<MRSSEntry>& entries)
			{
				Q_FOREACH (MRSSEntry e, entries)
				{
					WriteMediaRSS_.bindValue (0, e.MRSSEntryID_);
					WriteMediaRSS_.bindValue (1, e.ItemID_);
					WriteMediaRSS_.bindValue (2, e.URL_);
					WriteMediaRSS_.bindValue (3, e.Size_);
					WriteMediaRSS_.bindValue (4, e.Type_);
					WriteMediaRSS_.bindValue (5, e.Medium_);
					WriteMediaRSS_.bindValue (6, e.IsDefault_);
					WriteMediaRSS_.bindValue (7, e.Expression_);
					WriteMediaRSS_.bindValue (8, e.Bitrate_);
					WriteMediaRSS_.bindValue (9, e.Framerate_);
					WriteMediaRSS_.bindValue (10, e.SamplingRate_);
					WriteMediaRSS_.bindValue (11, e.Channels_);
					WriteMediaRSS_.bindValue (12, e.Duration_);
					WriteMediaRSS_.bindValue (13, e.Width_);
					WriteMediaRSS_.bindValue (14, e.Height_);
					WriteMediaRSS_.bindValue (15, e.Lang_);
					WriteMediaRSS_.bindValue (16, e.Group_);
					WriteMediaRSS_.bindValue (17, e.Rating_);
					WriteMediaRSS_.bindValue (18, e.RatingScheme_);
					WriteMediaRSS_.bindValue (19, e.Title_);
					WriteMediaRSS_.bindValue (20, e.Description_);
					WriteMediaRSS_.bindValue (21, e.Keywords_);
					WriteMediaRSS_.bindValue (22, e.CopyrightURL_);
					WriteMediaRSS_.bindValue (23, e.CopyrightText_);
					WriteMediaRSS_.bindValue (24, e.RatingAverage_);
					WriteMediaRSS_.bindValue (25, e.RatingCount_);
					WriteMediaRSS_.bindValue (26, e.RatingMin_);
					WriteMediaRSS_.bindValue (27, e.RatingMax_);
					WriteMediaRSS_.bindValue (28, e.Views_);
					WriteMediaRSS_.bindValue (29, e.Favs_);
					WriteMediaRSS_.bindValue (30, e.Tags_);

					if (!WriteMediaRSS_.exec ())
					{
						Util::DBLock::DumpError (WriteMediaRSS_);
						continue;
					}
					
					WriteMediaRSS_.finish ();

					Q_FOREACH (MRSSThumbnail t, e.Thumbnails_)
					{
						WriteMediaRSSThumbnail_.bindValue (0, t.MRSSThumbnailID_);
						WriteMediaRSSThumbnail_.bindValue (1, t.MRSSEntryID_);
						WriteMediaRSSThumbnail_.bindValue (2, t.URL_);
						WriteMediaRSSThumbnail_.bindValue (3, t.Width_);
						WriteMediaRSSThumbnail_.bindValue (4, t.Height_);
						WriteMediaRSSThumbnail_.bindValue (5, t.Time_);

						if (!WriteMediaRSSThumbnail_.exec ())
							Util::DBLock::DumpError (WriteMediaRSSThumbnail_);
						
						WriteMediaRSSThumbnail_.finish ();
					}

					Q_FOREACH (MRSSCredit c, e.Credits_)
					{
						WriteMediaRSSCredit_.bindValue (0, c.MRSSCreditID_);
						WriteMediaRSSCredit_.bindValue (1, c.MRSSEntryID_);
						WriteMediaRSSCredit_.bindValue (2, c.Role_);
						WriteMediaRSSCredit_.bindValue (3, c.Who_);

						if (!WriteMediaRSSCredit_.exec ())
							Util::DBLock::DumpError (WriteMediaRSSCredit_);
						
						WriteMediaRSSCredit_.finish ();
					}

					Q_FOREACH (MRSSComment c, e.Comments_)
					{
						WriteMediaRSSComment_.bindValue (0, c.MRSSCommentID_);
						WriteMediaRSSComment_.bindValue (1, c.MRSSEntryID_);
						WriteMediaRSSComment_.bindValue (2, c.Type_);
						WriteMediaRSSComment_.bindValue (3, c.Comment_);

						if (!WriteMediaRSSComment_.exec ())
							Util::DBLock::DumpError (WriteMediaRSSComment_);
						
						WriteMediaRSSComment_.finish ();
					}

					Q_FOREACH (MRSSPeerLink p, e.PeerLinks_)
					{
						WriteMediaRSSPeerLink_.bindValue (0, p.MRSSPeerLinkID_);
						WriteMediaRSSPeerLink_.bindValue (1, p.MRSSEntryID_);
						WriteMediaRSSPeerLink_.bindValue (2, p.Type_);
						WriteMediaRSSPeerLink_.bindValue (3, p.Link_);

						if (!WriteMediaRSSPeerLink_.exec ())
							Util::DBLock::DumpError (WriteMediaRSSPeerLink_);
						
						WriteMediaRSSPeerLink_.finish ();
					}

					Q_FOREACH (MRSSScene s, e.Scenes_)
					{
						WriteMediaRSSScene_.bindValue (0, s.MRSSSceneID_);
						WriteMediaRSSScene_.bindValue (1, s.MRSSEntryID_);
						WriteMediaRSSScene_.bindValue (2, s.Title_);
						WriteMediaRSSScene_.bindValue (3, s.Description_);
						WriteMediaRSSScene_.bindValue (4, s.StartTime_);
						WriteMediaRSSScene_.bindValue (5, s.EndTime_);

						if (!WriteMediaRSSScene_.exec ())
							Util::DBLock::DumpError (WriteMediaRSSScene_);
						
						WriteMediaRSSScene_.finish ();
					}
				}
			}

			void SQLStorageBackendMysql::GetMRSSEntries (const IDType_t& itemId, QList<MRSSEntry>& entries) const
			{
				GetMediaRSSs_.bindValue (0, itemId);

				if (!GetMediaRSSs_.exec ())
				{
					Util::DBLock::DumpError (GetMediaRSSs_);
					return;
				}

				while (GetMediaRSSs_.next ())
				{
					IDType_t mrssId = GetMediaRSSs_.value (0).value<IDType_t> ();
					MRSSEntry e (itemId, mrssId);
					e.URL_ = GetMediaRSSs_.value (1).toString ();
					e.Size_ = GetMediaRSSs_.value (2).toLongLong ();
					e.Type_ = GetMediaRSSs_.value (3).toString ();
					e.Medium_ = GetMediaRSSs_.value (4).toString ();
					e.IsDefault_ = GetMediaRSSs_.value (5).toBool ();
					e.Expression_ = GetMediaRSSs_.value (6).toString ();
					e.Bitrate_ = GetMediaRSSs_.value (7).toInt ();
					e.Framerate_ = GetMediaRSSs_.value (8).toDouble ();
					e.SamplingRate_ = GetMediaRSSs_.value (9).toDouble ();
					e.Channels_ = GetMediaRSSs_.value (10).toInt ();
					e.Duration_ = GetMediaRSSs_.value (11).toInt ();
					e.Width_ = GetMediaRSSs_.value (12).toInt ();
					e.Height_ = GetMediaRSSs_.value (13).toInt ();
					e.Lang_ = GetMediaRSSs_.value (14).toString ();
					e.Group_ = GetMediaRSSs_.value (15).toInt ();
					e.Rating_ = GetMediaRSSs_.value (16).toString ();
					e.RatingScheme_ = GetMediaRSSs_.value (17).toString ();
					e.Title_ = GetMediaRSSs_.value (18).toString ();
					e.Description_ = GetMediaRSSs_.value (19).toString ();
					e.Keywords_ = GetMediaRSSs_.value (20).toString ();
					e.CopyrightURL_ = GetMediaRSSs_.value (21).toString ();
					e.CopyrightText_ = GetMediaRSSs_.value (22).toString ();
					e.RatingAverage_ = GetMediaRSSs_.value (23).toInt ();
					e.RatingCount_ = GetMediaRSSs_.value (24).toInt ();
					e.RatingMin_ = GetMediaRSSs_.value (25).toInt ();
					e.RatingMax_ = GetMediaRSSs_.value (26).toInt ();
					e.Views_ = GetMediaRSSs_.value (27).toInt ();
					e.Favs_ = GetMediaRSSs_.value (28).toInt ();
					e.Tags_ = GetMediaRSSs_.value (29).toString ();

					GetMediaRSSThumbnails_.bindValue (0, mrssId);
					if (!GetMediaRSSThumbnails_.exec ())
						Util::DBLock::DumpError (GetMediaRSSThumbnails_);
					else
					{
						while (GetMediaRSSThumbnails_.next ())
						{
							MRSSThumbnail th (mrssId,
									GetMediaRSSThumbnails_.value (0).value<IDType_t> ());
							th.URL_ = GetMediaRSSThumbnails_.value (1).toString ();
							th.Width_ = GetMediaRSSThumbnails_.value (2).toInt ();
							th.Height_ = GetMediaRSSThumbnails_.value (3).toInt ();
							th.Time_ = GetMediaRSSThumbnails_.value (4).toString ();
							e.Thumbnails_ << th;
						}
						GetMediaRSSThumbnails_.finish ();
					}

					GetMediaRSSCredits_.bindValue (0, mrssId);
					if (!GetMediaRSSCredits_.exec ())
						Util::DBLock::DumpError (GetMediaRSSCredits_);
					else
					{
						while (GetMediaRSSCredits_.next ())
						{
							MRSSCredit cr (mrssId,
									GetMediaRSSCredits_.value (0).value<IDType_t> ());
							cr.Role_ = GetMediaRSSCredits_.value (1).toString ();
							cr.Who_ = GetMediaRSSCredits_.value (2).toString ();
							e.Credits_ << cr;
						}
						GetMediaRSSCredits_.finish ();
					}

					GetMediaRSSComments_.bindValue (0, mrssId);
					if (!GetMediaRSSComments_.exec ())
						Util::DBLock::DumpError (GetMediaRSSComments_);
					else
					{
						while (GetMediaRSSComments_.next ())
						{
							MRSSComment cm (mrssId,
									GetMediaRSSComments_.value (0).value<IDType_t> ());
							cm.Type_ = GetMediaRSSComments_.value (1).toString ();
							cm.Comment_ = GetMediaRSSComments_.value (2).toString ();
							e.Comments_ << cm;
						}
						GetMediaRSSComments_.finish ();
					}

					GetMediaRSSPeerLinks_.bindValue (0, mrssId);
					if (!GetMediaRSSPeerLinks_.exec ())
						Util::DBLock::DumpError (GetMediaRSSPeerLinks_);
					else
					{
						while (GetMediaRSSPeerLinks_.next ())
						{
							MRSSPeerLink pl (mrssId,
									GetMediaRSSPeerLinks_.value (0).value<IDType_t> ());
							pl.Type_ = GetMediaRSSPeerLinks_.value (1).toString ();
							pl.Link_ = GetMediaRSSPeerLinks_.value (2).toString ();
							e.PeerLinks_ << pl;
						}
						GetMediaRSSPeerLinks_.finish ();
					}

					GetMediaRSSScenes_.bindValue (0, mrssId);
					if (!GetMediaRSSScenes_.exec ())
						Util::DBLock::DumpError (GetMediaRSSScenes_);
					else
					{
						while (GetMediaRSSScenes_.next ())
						{
							MRSSScene th (mrssId,
									GetMediaRSSScenes_.value (0).value<IDType_t> ());
							th.Title_ = GetMediaRSSScenes_.value (1).toString ();
							th.Description_ = GetMediaRSSScenes_.value (2).toString ();
							th.StartTime_ = GetMediaRSSScenes_.value (3).toString ();
							th.EndTime_ = GetMediaRSSScenes_.value (4).toString ();
							e.Scenes_ << th;
						}
						GetMediaRSSScenes_.finish ();
					}

					entries << e;
				}

				GetMediaRSSs_.finish ();
			}
		};
	};
};

