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

#include "sqlstoragebackend.h"
#include <stdexcept>
#include <boost/bind.hpp>
#include <QDir>
#include <QDebug>
#include <QBuffer>
#include <QSqlError>
#include <QVariant>
#include "plugininterface/dblock.h"
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			SQLStorageBackend::SQLStorageBackend (StorageBackend::Type t)
			: Type_ (t)
			{
				QString strType;
				switch (Type_)
				{
					case SBSQLite:
						strType = "QSQLITE";
						break;
					case SBPostgres:
						strType = "QPSQL";
				}
			
				DB_ = QSqlDatabase::addDatabase (strType, "AggregatorConnection");
			
				switch (Type_)
				{
					case SBSQLite:
						{
							QDir dir = QDir::home ();
							dir.cd (".leechcraft");
							dir.cd ("aggregator");
							DB_.setDatabaseName (dir.filePath ("aggregator.db"));
						}
						break;
					case SBPostgres:
						{
							DB_.setDatabaseName (XmlSettingsManager::Instance ()->
									property ("PostgresDBName").toString ());
							DB_.setHostName (XmlSettingsManager::Instance ()->
									property ("PostgresHostname").toString ());
							DB_.setPort (XmlSettingsManager::Instance ()->
									property ("PostgresPort").toInt ());
							DB_.setUserName (XmlSettingsManager::Instance ()->
									property ("PostgresUsername").toString ());
							DB_.setPassword (XmlSettingsManager::Instance ()->
									property ("PostgresPassword").toString ());
						}
						break;
				}
			
				if (!DB_.open ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (DB_.lastError ());
					throw std::runtime_error (qPrintable (QString ("Could not initialize database: %1")
								.arg (DB_.lastError ().text ())));
				}
			
				InitializeTables ();
			}
			
			SQLStorageBackend::~SQLStorageBackend ()
			{
				if (Type_ == SBSQLite &&
						XmlSettingsManager::Instance ()->property ("SQLiteVacuum").toBool ())
				{
					QSqlQuery vacuum (DB_);
					vacuum.exec ("VACUUM;");
				}
			}
			
			void SQLStorageBackend::Prepare ()
			{
				if (Type_ == SBSQLite)
				{
					QSqlQuery pragma (DB_);
					if (!pragma.exec (QString ("PRAGMA journal_mode = %1;")
								.arg (XmlSettingsManager::Instance ()->
									property ("SQLiteJournalMode").toString ())))
						LeechCraft::Util::DBLock::DumpError (pragma);
					if (!pragma.exec (QString ("PRAGMA synchronous = %1;")
								.arg (XmlSettingsManager::Instance ()->
									property ("SQLiteSynchronous").toString ())))
						LeechCraft::Util::DBLock::DumpError (pragma);
					if (!pragma.exec (QString ("PRAGMA temp_store = %1;")
								.arg (XmlSettingsManager::Instance ()->
									property ("SQLiteTempStore").toString ())))
						LeechCraft::Util::DBLock::DumpError (pragma);
				}
			
				FeedFinderByURL_ = QSqlQuery (DB_);
				FeedFinderByURL_.prepare ("SELECT last_update "
						"FROM feeds "
						"WHERE url = :url");
			
				FeedSettingsGetter_ = QSqlQuery (DB_);
				FeedSettingsGetter_.prepare ("SELECT "
						"update_timeout, "
						"num_items, "
						"item_age, "
						"auto_download_enclosures "
						"FROM feeds_settings "
						"WHERE feed_url = :feed_url");
			
				FeedSettingsSetter_ = QSqlQuery (DB_);
				QString orReplace;
				if (Type_ == SBSQLite)
					orReplace = "OR REPLACE";
			
				FeedSettingsSetter_.prepare (QString ("INSERT %1 INTO feeds_settings ("
						"feed_url, "
						"update_timeout, "
						"num_items, "
						"item_age, "
						"auto_download_enclosures"
						") VALUES ("
						":feed_url, "
						":update_timeout, "
						":num_items, "
						":item_age, "
						":auto_download_enclosures"
						")").arg (orReplace));
			
				ChannelsShortSelector_ = QSqlQuery (DB_);
				ChannelsShortSelector_.prepare ("SELECT "
						"title, "
						"url, "
						"tags, "
						"last_build,"
						"favicon "
						"FROM channels "
						"WHERE parent_feed_url = :parent_feed_url "
						"ORDER BY title");
			
				ChannelsFullSelector_ = QSqlQuery (DB_);
				ChannelsFullSelector_.prepare ("SELECT "
						"url, "
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
						"AND title = :title "
						"ORDER BY title");
			
				UnreadItemsCounter_ = QSqlQuery (DB_);
				switch (Type_)
				{
					case SBSQLite:
						UnreadItemsCounter_.prepare ("SELECT COUNT (unread) "
								"FROM items "
								"WHERE parents_hash = :parents_hash "
								"AND unread = \"true\"");
						break;
					case SBPostgres:
						UnreadItemsCounter_.prepare ("SELECT COUNT (1) "
								"FROM items "
								"WHERE parents_hash = :parents_hash "
								"AND unread");
						break;
				}
			
				ItemsShortSelector_ = QSqlQuery (DB_);
				ItemsShortSelector_.prepare ("SELECT "
						"title, "
						"url, "
						"category, "
						"pub_date, "
						"unread "
						"FROM items "
						"WHERE parents_hash = :parents_hash "
						"ORDER BY pub_date DESC, "
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
						"longitude "
						"FROM items "
						"WHERE parents_hash = :parents_hash "
						"AND title = :title "
						"AND url = :url "
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
						"longitude "
						"FROM items "
						"WHERE parents_hash = :parents_hash "
						"ORDER BY pub_date DESC");
			
				ChannelFinder_ = QSqlQuery (DB_);
				ChannelFinder_.prepare ("SELECT description "
						"FROM channels "
						"WHERE parent_feed_url = :parent_feed_url "
						"AND title = :title "
						"AND url = :url");
			
				ItemFinder_ = QSqlQuery (DB_);
				ItemFinder_.prepare ("SELECT title "
						"FROM items "
						"WHERE parents_hash = :parents_hash "
						"AND title = :title "
						"AND url = :url");
			
				InsertFeed_ = QSqlQuery (DB_);
				InsertFeed_.prepare ("INSERT INTO feeds ("
						"url, "
						"last_update"
						") VALUES ("
						":url, "
						":last_update"
						")");
			
				InsertChannel_ = QSqlQuery (DB_);
				InsertChannel_.prepare ("INSERT INTO channels ("
						"parent_feed_url, "
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
						":parent_feed_url, "
						":url, "
						":title, "
						":description, "
						":last_build, "
						":tags, "
						":language, "
						":author, "
						":pixmap_url, "
						":pixmap, "
						":favicon"
						")");
			
				InsertItem_ = QSqlQuery (DB_);
				InsertItem_.prepare ("INSERT INTO items ("
						"parents_hash, "
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
						":parents_hash, "
						":title, "
						":url, "
						":description, "
						":author, "
						":category, "
						":guid, "
						":pub_date, "
						":unread, "
						":num_comments, "
						":comments_url, "
						":comments_page_url, "
						":latitude, "
						":longitude"
						")");
			
				UpdateShortChannel_ = QSqlQuery (DB_);
				UpdateShortChannel_.prepare ("UPDATE channels SET "
						"tags = :tags, "
						"last_build = :last_build "
						"WHERE parent_feed_url = :parent_feed_url "
						"AND url = :url "
						"AND title = :title");
			
				UpdateChannel_ = QSqlQuery (DB_);
				UpdateChannel_.prepare ("UPDATE channels SET "
						"description = :description, "
						"last_build = :last_build, "
						"tags = :tags, "
						"language = :language, "
						"author = :author, "
						"pixmap_url = :pixmap_url, "
						"pixmap = :pixmap, "
						"favicon = :favicon "
						"WHERE parent_feed_url = :parent_feed_url "
						"AND url = :url "
						"AND title = :title");
			
				UpdateShortItem_ = QSqlQuery (DB_);
				UpdateShortItem_.prepare ("UPDATE items SET "
						"unread = :unread "
						"WHERE parents_hash = :parents_hash "
						"AND title = :title "
						"AND url = :url");
			
				UpdateItem_ = QSqlQuery (DB_);
				UpdateItem_.prepare ("UPDATE items SET "
						"description = :description, "
						"author = :author, "
						"category = :category, "
						"pub_date = :pub_date, "
						"unread = :unread, "
						"num_comments = :num_comments, "
						"comments_url = :comments_url, "
						"comments_page_url = :comments_page_url, "
						"latitude = :latitude, "
						"longitude = :longitude "
						"WHERE parents_hash = :parents_hash "
						"AND title = :title "
						"AND url = :url");
			
				ToggleChannelUnread_ = QSqlQuery (DB_);
				ToggleChannelUnread_.prepare ("UPDATE items SET "
						"unread = :unread "
						"WHERE parents_hash = :parents_hash "
						"AND unread <> :unread");
			
				RemoveFeed_ = QSqlQuery (DB_);
				RemoveFeed_.prepare ("DELETE FROM feeds "
						"WHERE url = :url");
			
				RemoveChannel_ = QSqlQuery (DB_);
				RemoveChannel_.prepare ("DELETE FROM channels "
						"WHERE parent_feed_url = :parent_feed_url");
			
				RemoveItem_ = QSqlQuery (DB_);
				RemoveItem_.prepare ("DELETE FROM items "
						"WHERE parents_hash = :parents_hash "
						"AND title = :title "
						"AND url = :url");
			
				WriteEnclosure_ = QSqlQuery (DB_);
				WriteEnclosure_.prepare (QString ("INSERT %1 INTO enclosures ("
							"url, "
							"type, "
							"length, "
							"lang, "
							"item_parents_hash, "
							"item_title, "
							"item_url"
							") VALUES ("
							":url, "
							":type, "
							":length, "
							":lang, "
							":item_parents_hash, "
							":item_title, "
							":item_url"
							")").arg (orReplace));

				WriteMediaRSS_ = QSqlQuery (DB_);
				WriteMediaRSS_.prepare (QString ("INSERT %1 INTO mrss ("
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
							"tags, "
							"item_parents_hash, "
							"item_title, "
							"item_url"
							") VALUES ("
							":url, "
							":size, "
							":type, "
							":medium, "
							":is_default, "
							":expression, "
							":bitrate, "
							":framerate, "
							":samplingrate, "
							":channels, "
							":duration, "
							":width, "
							":height, "
							":lang, "
							":mediagroup, "
							":rating, "
							":rating_scheme, "
							":title, "
							":description, "
							":keywords, "
							":copyright_url, "
							":copyright_text, "
							":star_rating_average, "
							":star_rating_count, "
							":star_rating_min, "
							":star_rating_max, "
							":stat_views, "
							":stat_favs, "
							":tags, "
							":item_parents_hash, "
							":item_title, "
							":item_url"
							")").arg (orReplace));

				GetMediaRSSs_ = QSqlQuery (DB_);
				GetMediaRSSs_.prepare ("SELECT "
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
						"WHERE item_parents_hash = :item_parents_hash "
						"AND item_title = :item_title "
						"AND item_url = :item_url "
						"ORDER BY title");

				WriteMediaRSSThumbnail_ = QSqlQuery (DB_);
				WriteMediaRSSThumbnail_.prepare (QString ("INSERT %1 INTO mrss_thumbnails ("
							"parent_url, "
							"item_parents_hash, "
							"item_title, "
							"item_url, "
							"url, "
							"width, "
							"height, "
							"time"
							") VALUES ("
							":parent_url, "
							":item_parents_hash, "
							":item_title, "
							":item_url, "
							":url, "
							":width, "
							":height, "
							":time"
							")").arg (orReplace));

				GetMediaRSSThumbnails_ = QSqlQuery (DB_);
				GetMediaRSSThumbnails_.prepare ("SELECT "
						"url, "
						"width, "
						"height, "
						"time "
						"FROM mrss_thumbnails "
						"WHERE parent_url = :parent_url "
						"AND item_parents_hash = :item_parents_hash "
						"AND item_title = :item_title "
						"AND item_url = :item_url "
						"ORDER BY time");

				WriteMediaRSSCredit_ = QSqlQuery (DB_);
				WriteMediaRSSCredit_.prepare (QString ("INSERT %1 INTO mrss_credits ("
							"parent_url, "
							"item_parents_hash, "
							"item_title, "
							"item_url, "
							"role, "
							"who"
							") VALUES ("
							":parent_url, "
							":item_parents_hash, "
							":item_title, "
							":item_url, "
							":role, "
							":who"
							")").arg (orReplace));

				GetMediaRSSCredits_ = QSqlQuery (DB_);
				GetMediaRSSCredits_.prepare ("SELECT "
						"role, "
						"who "
						"FROM mrss_credits "
						"WHERE parent_url = :parent_url "
						"AND item_parents_hash = :item_parents_hash "
						"AND item_title = :item_title "
						"AND item_url = :item_url "
						"ORDER BY role");

				WriteMediaRSSComment_ = QSqlQuery (DB_);
				WriteMediaRSSComment_.prepare ("INSERT INTO mrss_comments ("
						"parent_url, "
						"item_parents_hash, "
						"item_title, "
						"item_url, "
						"type, "
						"comment"
						") VALUES ("
						":parent_url, "
						":item_parents_hash, "
						":item_title, "
						":item_url, "
						":type, "
						":comment"
						")");

				GetMediaRSSComments_ = QSqlQuery (DB_);
				GetMediaRSSComments_.prepare ("SELECT "
						"type, "
						"comment "
						"FROM mrss_comments "
						"WHERE parent_url = :parent_url "
						"AND item_parents_hash = :item_parents_hash "
						"AND item_title = :item_title "
						"AND item_url = :item_url "
						"ORDER BY comment");
				
				WriteMediaRSSPeerLink_ = QSqlQuery (DB_);
				WriteMediaRSSPeerLink_.prepare ("INSERT INTO mrss_peerlinks ("
						"parent_url, "
						"item_parents_hash, "
						"item_title, "
						"item_url, "
						"type, "
						"link"
						") VALUES ("
						":parent_url, "
						":item_parents_hash, "
						":item_title, "
						":item_url, "
						":type, "
						":link"
						")");

				GetMediaRSSPeerLinks_ = QSqlQuery (DB_);
				GetMediaRSSPeerLinks_.prepare ("SELECT "
						"type, "
						"link "
						"FROM mrss_peerlinks "
						"WHERE parent_url = :parent_url "
						"AND item_parents_hash = :item_parents_hash "
						"AND item_title = :item_title "
						"AND item_url = :item_url "
						"ORDER BY link");

				WriteMediaRSSScene_ = QSqlQuery (DB_);
				WriteMediaRSSScene_.prepare ("INSERT INTO mrss_scenes ("
						"parent_url, "
						"item_parents_hash, "
						"item_title, "
						"item_url, "
						"title, "
						"description, "
						"start_time, "
						"end_time"
						") VALUES ("
						":parent_url, "
						":item_parents_hash, "
						":item_title, "
						":item_url, "
						":title, "
						":description, "
						":start_time, "
						":end_time"
						")");

				GetMediaRSSScenes_ = QSqlQuery (DB_);
				GetMediaRSSScenes_.prepare ("SELECT "
						"title, "
						"description, "
						"start_time, "
						"end_time "
						"FROM mrss_scenes "
						"WHERE parent_url = :parent_url "
						"AND item_parents_hash = :item_parents_hash "
						"AND item_title = :item_title "
						"AND item_url = :item_url "
						"ORDER BY start_time");
			
				RemoveEnclosures_ = QSqlQuery (DB_);
				RemoveEnclosures_.prepare ("DELETE FROM enclosures "
						"WHERE item_parents_hash = :item_parents_hash "
						"AND item_title = :item_title "
						"AND item_url = :item_url");
			
				GetEnclosures_ = QSqlQuery (DB_);
				GetEnclosures_.prepare ("SELECT "
						"url, "
						"type, "
						"length, "
						"lang "
						"FROM enclosures "
						"WHERE item_parents_hash = :item_parents_hash "
						"AND item_title = :item_title "
						"AND item_url = :item_url "
						"ORDER BY url");

				RemoveMediaRSS_ = QSqlQuery (DB_);
				RemoveMediaRSS_.prepare ("DELETE FROM mrss "
						"WHERE item_parents_hash = :item_parents_hash "
						"AND item_title = :item_title "
						"AND item_url = :item_url");

				RemoveMediaRSSThumbnails_ = QSqlQuery (DB_);
				RemoveMediaRSSThumbnails_.prepare ("DELETE FROM mrss_thumbnails "
						"WHERE item_parents_hash = :item_parents_hash "
						"AND item_title = :item_title "
						"AND item_url = :item_url");

				RemoveMediaRSSCredits_ = QSqlQuery (DB_);
				RemoveMediaRSSCredits_.prepare ("DELETE FROM mrss_credits "
						"WHERE item_parents_hash = :item_parents_hash "
						"AND item_title = :item_title "
						"AND item_url = :item_url");

				RemoveMediaRSSComments_ = QSqlQuery (DB_);
				RemoveMediaRSSComments_.prepare ("DELETE FROM mrss_comments "
						"WHERE item_parents_hash = :item_parents_hash "
						"AND item_title = :item_title "
						"AND item_url = :item_url");

				RemoveMediaRSSPeerLinks_ = QSqlQuery (DB_);
				RemoveMediaRSSPeerLinks_.prepare ("DELETE FROM mrss_peerlinks "
						"WHERE item_parents_hash = :item_parents_hash "
						"AND item_title = :item_title "
						"AND item_url = :item_url");

				RemoveMediaRSSScenes_ = QSqlQuery (DB_);
				RemoveMediaRSSScenes_.prepare ("DELETE FROM mrss_scenes "
						"WHERE item_parents_hash = :item_parents_hash "
						"AND item_title = :item_title "
						"AND item_url = :item_url");
			}
			
			void SQLStorageBackend::GetFeedsURLs (feeds_urls_t& result) const
			{
				QSqlQuery feedSelector (DB_);
				QString idType;
				switch (Type_)
				{
					case SBSQLite:
						idType = "ROWID";
						break;
					case SBPostgres:
						idType = "CTID";
						break;
				}
				if (!feedSelector.exec (QString ("SELECT url "
							"FROM feeds "
							"ORDER BY %1").arg (idType)))
				{
					LeechCraft::Util::DBLock::DumpError (feedSelector);
					return;
				}
			
				while (feedSelector.next ())
					result.push_back (feedSelector.value (0).toString ());
			}
			
			Feed::FeedSettings SQLStorageBackend::GetFeedSettings (const QString& feedURL) const
			{
				Feed::FeedSettings result;
				FeedSettingsGetter_.bindValue (":feed_url", feedURL);
				if (!FeedSettingsGetter_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (FeedSettingsGetter_);
					return result;
				}
			
				if (!FeedSettingsGetter_.next ())
					return result;
			
				result.UpdateTimeout_ = FeedSettingsGetter_.value (0).toInt ();
				result.NumItems_ = FeedSettingsGetter_.value (1).toInt ();
				result.ItemAge_ = FeedSettingsGetter_.value (2).toInt ();
				result.AutoDownloadEnclosures_ = FeedSettingsGetter_.value (3).toBool ();
			
				FeedSettingsGetter_.finish ();
			
				return result;
			}
			
			void SQLStorageBackend::SetFeedSettings (const QString& feedURL,
					const Feed::FeedSettings& settings)
			{
				FeedSettingsSetter_.bindValue (":feed_url",
						feedURL);
				FeedSettingsSetter_.bindValue (":update_timeout",
						settings.UpdateTimeout_);
				FeedSettingsSetter_.bindValue (":num_items",
						settings.NumItems_);
				FeedSettingsSetter_.bindValue (":item_age",
						settings.ItemAge_);
				FeedSettingsSetter_.bindValue (":auto_download_enclosures",
						settings.AutoDownloadEnclosures_);
			
				if (!FeedSettingsSetter_.exec ())
					LeechCraft::Util::DBLock::DumpError (FeedSettingsSetter_);
			}
			
			void SQLStorageBackend::GetChannels (channels_shorts_t& shorts,
					const QString& feedURL) const
			{
				ChannelsShortSelector_.bindValue (":parent_feed_url", feedURL);
				if (!ChannelsShortSelector_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (ChannelsShortSelector_);
					return;
				}
			
				while (ChannelsShortSelector_.next ())
				{
					int unread = 0;
					QString title = ChannelsShortSelector_.value (0).toString ();
			
					UnreadItemsCounter_.bindValue (":parents_hash", feedURL + title);
					if (!UnreadItemsCounter_.exec () || !UnreadItemsCounter_.next ())
						LeechCraft::Util::DBLock::DumpError (UnreadItemsCounter_);
					else
						unread = UnreadItemsCounter_.value (0).toInt ();
			
					UnreadItemsCounter_.finish ();
			
					QStringList tags = Core::Instance ().GetProxy ()->
						GetTagsManager ()->Split (ChannelsShortSelector_.value (2).toString ());
					ChannelShort sh =
					{
						title,
						ChannelsShortSelector_.value (1).toString (),
						tags,
						ChannelsShortSelector_.value (3).toDateTime (),
						UnserializePixmap (ChannelsShortSelector_
								.value (4).toByteArray ()),
						unread,
						feedURL
					};
					shorts.push_back (sh);
				}
			
				ChannelsShortSelector_.finish ();
			}
			
			Channel_ptr SQLStorageBackend::GetChannel (const QString& title,
					const QString& feedParent) const
			{
				ChannelsFullSelector_.bindValue (":title", title);
				ChannelsFullSelector_.bindValue (":parent_feed_url", feedParent);
				if (!ChannelsFullSelector_.exec () || !ChannelsFullSelector_.next ())
				{
					LeechCraft::Util::DBLock::DumpError (ChannelsFullSelector_);
					return Channel_ptr (new Channel);
				}
			
				Channel_ptr channel (new Channel);
			
				channel->Link_ = ChannelsFullSelector_.value (0).toString ();
				channel->Title_ = title;
				channel->Description_ = ChannelsFullSelector_.value (1).toString ();
				channel->LastBuild_ = ChannelsFullSelector_.value (2).toDateTime ();
				QString tags = ChannelsFullSelector_.value (3).toString ();
				channel->Tags_ = Core::Instance ().GetProxy ()->GetTagsManager ()->Split (tags);
				channel->Language_ = ChannelsFullSelector_.value (4).toString ();
				channel->Author_ = ChannelsFullSelector_.value (5).toString ();
				channel->PixmapURL_ = ChannelsFullSelector_.value (6).toString ();
				channel->Pixmap_ = UnserializePixmap (ChannelsFullSelector_
						.value (7).toByteArray ());
				channel->Favicon_ = UnserializePixmap (ChannelsFullSelector_
						.value (8).toByteArray ());
				channel->ParentURL_ = feedParent;
			
				ChannelsFullSelector_.finish ();
			
				return channel;
			}
			
			void SQLStorageBackend::GetItems (items_shorts_t& shorts, const QString& parentsHash) const
			{
				ItemsShortSelector_.bindValue (":parents_hash", parentsHash);
			
				if (!ItemsShortSelector_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (ItemsShortSelector_);
					return;
				}
			
				while (ItemsShortSelector_.next ())
				{
					ItemShort sh =
					{
						ItemsShortSelector_.value (0).toString (),
						ItemsShortSelector_.value (1).toString (),
						ItemsShortSelector_.value (2).toString ()
							.split ("<<<", QString::SkipEmptyParts),
						ItemsShortSelector_.value (3).toDateTime (),
						ItemsShortSelector_.value (4).toBool ()
					};
			
					shorts.push_back (sh);
				}
			
				ItemsShortSelector_.finish ();
			}
			
			int SQLStorageBackend::GetUnreadItems (const QString& purl, const QString& title) const
			{
				int unread = 0;
				UnreadItemsCounter_.bindValue (":parents_hash", purl + title);
				if (!UnreadItemsCounter_.exec () || !UnreadItemsCounter_.next ())
					LeechCraft::Util::DBLock::DumpError (UnreadItemsCounter_);
				else
					unread = UnreadItemsCounter_.value (0).toInt ();
			
				UnreadItemsCounter_.finish ();
				return unread;
			}
			
			Item_ptr SQLStorageBackend::GetItem (const QString& title,
					const QString& link, const QString& hash) const
			{
				ItemFullSelector_.bindValue (":parents_hash", hash);
				ItemFullSelector_.bindValue (":title", title);
				ItemFullSelector_.bindValue (":link", link);
				if (!ItemFullSelector_.exec () || !ItemFullSelector_.next ())
				{
					LeechCraft::Util::DBLock::DumpError (ItemFullSelector_);
					return Item_ptr ();
				}
			
				Item_ptr item (new Item);
				FillItem (ItemFullSelector_, item);
				ItemFullSelector_.finish ();
			
				GetEnclosures (hash, title, link, item->Enclosures_);
				GetMRSSEntries (hash, title, link, item->MRSSEntries_);
			
				return item;
			}
			
			void SQLStorageBackend::GetItems (items_container_t& items,
					const QString& hash) const
			{
				ItemsFullSelector_.bindValue (":parents_hash", hash);
				if (!ItemsFullSelector_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (ItemsFullSelector_);
					return;
				}
			
				while (ItemsFullSelector_.next ())
				{
					Item_ptr item (new Item);
					FillItem (ItemsFullSelector_, item);
					GetEnclosures (hash, item->Title_, item->Link_, item->Enclosures_);
					GetMRSSEntries (hash, item->Title_, item->Link_, item->MRSSEntries_);
			
					items.push_back (item);
				}
			
				ItemsFullSelector_.finish ();
				GetEnclosures_.finish ();
			}
			
			void SQLStorageBackend::AddFeed (Feed_ptr feed)
			{
				LeechCraft::Util::DBLock lock (DB_);
				try
				{
					lock.Init ();
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO << e.what ();
					return;
				}
			
				InsertFeed_.bindValue (":url", feed->URL_);
				InsertFeed_.bindValue (":last_update", feed->LastUpdate_);
				if (!InsertFeed_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (InsertFeed_);
					return;
				}
			
				try
				{
					std::for_each (feed->Channels_.begin (), feed->Channels_.end (),
						   boost::bind (&SQLStorageBackend::AddChannel,
							   this,
							   _1, feed->URL_));
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO << e.what ();
					return;
				}
			
				InsertFeed_.finish ();
			
				lock.Good ();
			}
			
			void SQLStorageBackend::UpdateChannel (Channel_ptr channel, const QString& parent)
			{
				ChannelFinder_.bindValue (":parent_feed_url", parent);
				ChannelFinder_.bindValue (":title", channel->Title_);
				ChannelFinder_.bindValue (":url", channel->Link_);
				if (!ChannelFinder_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (ChannelFinder_);
					throw std::runtime_error (qPrintable (QString (
									"Unable to execute channel finder query for t %1, url %2, p %3")
								.arg (channel->Title_)
								.arg (channel->Link_)
								.arg (parent)));
				}
				ChannelFinder_.next ();
				if (!ChannelFinder_.isValid ())
				{
					qWarning () << Q_FUNC_INFO
						<< "not found such channel"
						<< channel->Title_
						<< channel->Link_
						<< parent
						<< ", inserting it";
					AddChannel (channel, parent);
					return;
				}
				ChannelFinder_.finish ();
			
				UpdateChannel_.bindValue (":parent_feed_url", parent);
				UpdateChannel_.bindValue (":url", channel->Link_);
				UpdateChannel_.bindValue (":title", channel->Title_);
				UpdateChannel_.bindValue (":description", channel->Description_);
				UpdateChannel_.bindValue (":last_build", channel->LastBuild_);
				UpdateChannel_.bindValue (":tags",
						Core::Instance ().GetProxy ()->GetTagsManager ()->Join (channel->Tags_));
				UpdateChannel_.bindValue (":language", channel->Language_);
				UpdateChannel_.bindValue (":author", channel->Author_);
				UpdateChannel_.bindValue (":pixmap_url", channel->PixmapURL_);
				UpdateChannel_.bindValue (":pixmap", SerializePixmap (channel->Pixmap_));
				UpdateChannel_.bindValue (":favicon", SerializePixmap (channel->Favicon_));
			
				if (!UpdateChannel_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (UpdateChannel_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to save channel t %1, u %2, p %3")
								.arg (channel->Title_)
								.arg (channel->Link_)
								.arg (parent)));
				}

				if (!UpdateChannel_.numRowsAffected ())
					qWarning () << Q_FUNC_INFO
						<< "no rows affected by UpdateChannel_";
			
				UpdateChannel_.finish ();
			
				emit channelDataUpdated (channel);
			}
			
			void SQLStorageBackend::UpdateChannel (const ChannelShort& channel,
					const QString& parent)
			{
				ChannelFinder_.bindValue (":parent_feed_url", parent);
				ChannelFinder_.bindValue (":title", channel.Title_);
				ChannelFinder_.bindValue (":url", channel.Link_);
				if (!ChannelFinder_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (ChannelFinder_);
					throw std::runtime_error (qPrintable (QString (
									"Unable to execute channel finder query t %1, u %2, p %3")
								.arg (channel.Title_)
								.arg (channel.Link_)
								.arg (parent)));
				}
				ChannelFinder_.next ();
				if (!ChannelFinder_.isValid ())
				{
					qWarning () << Q_FUNC_INFO;
					throw std::runtime_error (qPrintable (QString (
									"Selected channel for updating doesn't exist and we don't "
									"have enough info to insert it t %1, u %2, p %3.")
								.arg (channel.Title_)
								.arg (channel.Link_)
								.arg (parent)));
				}
				ChannelFinder_.finish ();
			
				UpdateShortChannel_.bindValue (":parent_feed_url", parent);
				UpdateShortChannel_.bindValue (":url", channel.Link_);
				UpdateShortChannel_.bindValue (":title", channel.Title_);
				UpdateShortChannel_.bindValue (":last_build", channel.LastBuild_);
				UpdateShortChannel_.bindValue (":tags",
						Core::Instance ().GetProxy ()->GetTagsManager ()->Join (channel.Tags_));
			
				if (!UpdateShortChannel_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (UpdateShortChannel_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to save channel t %1, u %2, p %3")
								.arg (channel.Title_)
								.arg (channel.Link_)
								.arg (parent)));
				}

				if (!UpdateShortChannel_.numRowsAffected ())
					qWarning () << Q_FUNC_INFO
						<< "no rows affected by UpdateShortChannel_";
			
				UpdateShortChannel_.finish ();
			
				emit channelDataUpdated (GetChannel (channel.Title_, parent));
			}
			
			void SQLStorageBackend::UpdateItem (Item_ptr item,
					const QString& parentUrl, const QString& parentTitle)
			{
				UpdateItem_.bindValue (":parents_hash", parentUrl + parentTitle);
				UpdateItem_.bindValue (":title", item->Title_);
				UpdateItem_.bindValue (":url", item->Link_);
				UpdateItem_.bindValue (":description", item->Description_);
				UpdateItem_.bindValue (":author", item->Author_);
				UpdateItem_.bindValue (":category", item->Categories_.join ("<<<"));
				UpdateItem_.bindValue (":pub_date", item->PubDate_);
				UpdateItem_.bindValue (":unread", item->Unread_);
				UpdateItem_.bindValue (":num_comments", item->NumComments_);
				UpdateItem_.bindValue (":comments_url", item->CommentsLink_);
				UpdateItem_.bindValue (":comments_page_url", item->CommentsPageLink_);
				UpdateItem_.bindValue (":latitude", QString::number (item->Latitude_));
				UpdateItem_.bindValue (":longitude", QString::number (item->Longitude_));
			
				if (!UpdateItem_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (UpdateItem_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to save item pu %1, pt %2, t %3, u %4")
								.arg (parentUrl)
								.arg (parentTitle)
								.arg (item->Title_)
								.arg (item->Link_)));
				}

				if (!UpdateItem_.numRowsAffected ())
					qWarning () << Q_FUNC_INFO
						<< "no rows affected by UpdateItem_";
			
				UpdateItem_.finish ();

				WriteEnclosures (parentUrl + parentTitle,
						item->Title_, item->Link_, item->Enclosures_);
				WriteMRSSEntries (parentUrl + parentTitle,
						item->Title_, item->Link_, item->MRSSEntries_);
			
				Channel_ptr channel = GetChannel (parentTitle, parentUrl);
				emit itemDataUpdated (item, channel);
				emit channelDataUpdated (channel);
			}
			
			void SQLStorageBackend::UpdateItem (const ItemShort& item,
					const QString& parentUrl, const QString& parentTitle)
			{
				ItemFinder_.bindValue (":parents_hash", parentUrl + parentTitle);
				ItemFinder_.bindValue (":title", item.Title_);
				ItemFinder_.bindValue (":url", item.URL_);
				if (!ItemFinder_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (ItemFinder_);
					throw std::runtime_error (qPrintable (QString (
									"Unable to execute item finder query pu %1, pt %2, t %3, u %4")
								.arg (parentUrl)
								.arg (parentTitle)
								.arg (item.Title_)
								.arg (item.URL_)));
				}
				ItemFinder_.next ();
				if (!ItemFinder_.isValid ())
				{
					qWarning () << Q_FUNC_INFO;
					throw std::runtime_error (qPrintable (QString (
									"Specified item doesn't exist and we couldn't add it because "
									"there isn't enough info pu %1, pt %2, t %3, u %4")
								.arg (parentUrl)
								.arg (parentTitle)
								.arg (item.Title_)
								.arg (item.URL_)));
				}
				ItemFinder_.finish ();
			
				UpdateShortItem_.bindValue (":parents_hash", parentUrl + parentTitle);
				UpdateShortItem_.bindValue (":unread", item.Unread_);
				UpdateShortItem_.bindValue (":title", item.Title_);
				UpdateShortItem_.bindValue (":url", item.URL_);
			
				if (!UpdateShortItem_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (UpdateShortItem_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to save item pu %1, pt %2, t %3, u %4")
								.arg (parentUrl)
								.arg (parentTitle)
								.arg (item.Title_)
								.arg (item.URL_)));
				}

				if (!UpdateShortItem_.numRowsAffected ())
					qWarning () << Q_FUNC_INFO
						<< "no rows affected by UpdateShortItem_";
			
				UpdateShortItem_.finish ();
			
				Channel_ptr channel = GetChannel (parentTitle, parentUrl);
				emit itemDataUpdated (GetItem (item.Title_,
							item.URL_, parentUrl + parentTitle), channel);
				emit channelDataUpdated (channel);
			}
			
			void SQLStorageBackend::AddChannel (Channel_ptr channel, const QString& url)
			{
				InsertChannel_.bindValue (":parent_feed_url", url);
				InsertChannel_.bindValue (":url", channel->Link_);
				InsertChannel_.bindValue (":title", channel->Title_);
				InsertChannel_.bindValue (":description", channel->Description_);
				InsertChannel_.bindValue (":last_build", channel->LastBuild_);
				InsertChannel_.bindValue (":tags",
						Core::Instance ().GetProxy ()->GetTagsManager ()->Join (channel->Tags_));
				InsertChannel_.bindValue (":language", channel->Language_);
				InsertChannel_.bindValue (":author", channel->Author_);
				InsertChannel_.bindValue (":pixmap_url", channel->PixmapURL_);
				InsertChannel_.bindValue (":pixmap", SerializePixmap (channel->Pixmap_));
				InsertChannel_.bindValue (":favicon", SerializePixmap (channel->Favicon_));
			
				if (!InsertChannel_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (InsertChannel_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to save channel t %1, u %2, p %3")
								.arg (channel->Title_)
								.arg (channel->Link_)
								.arg (url)));
				}
			
				InsertChannel_.finish ();
			
				std::for_each (channel->Items_.begin (), channel->Items_.end (),
					   boost::bind (&SQLStorageBackend::AddItem,
						   this,
						   _1,
						   url,
						   channel->Title_));
			}
			
			void SQLStorageBackend::AddItem (Item_ptr item,
					const QString& parentUrl, const QString& parentTitle)
			{
				InsertItem_.bindValue (":parents_hash", parentUrl + parentTitle);
				InsertItem_.bindValue (":title", item->Title_);
				InsertItem_.bindValue (":url", item->Link_);
				InsertItem_.bindValue (":description", item->Description_);
				InsertItem_.bindValue (":author", item->Author_);
				InsertItem_.bindValue (":category", item->Categories_.join ("<<<"));
				InsertItem_.bindValue (":guid", item->Guid_);
				InsertItem_.bindValue (":pub_date", item->PubDate_);
				InsertItem_.bindValue (":unread", item->Unread_);
				InsertItem_.bindValue (":num_comments", item->NumComments_);
				InsertItem_.bindValue (":comments_url", item->CommentsLink_);
				InsertItem_.bindValue (":comments_page_url", item->CommentsPageLink_);
				InsertItem_.bindValue (":latitude", QString::number (item->Latitude_));
				InsertItem_.bindValue (":longitude", QString::number (item->Longitude_));
			
				if (!InsertItem_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (InsertItem_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to save item pu %1, pt %2, t %3, u %4")
								.arg (parentUrl)
								.arg (parentTitle)
								.arg (item->Title_)
								.arg (item->Link_)));
				}
			
				InsertItem_.finish ();

				WriteEnclosures (parentUrl + parentTitle,
						item->Title_, item->Link_, item->Enclosures_);
				WriteMRSSEntries (parentUrl + parentTitle,
						item->Title_, item->Link_, item->MRSSEntries_);

			
				Channel_ptr channel = GetChannel (parentTitle, parentUrl);
				emit itemDataUpdated (item, channel);
				emit channelDataUpdated (channel);
			}

			namespace
			{
				bool PerformRemove (QSqlQuery& query,
						const QString& hash,
						const QString& title,
						const QString& link)
				{
					query.bindValue (":item_parents_hash", hash);
					query.bindValue (":item_title", title);
					query.bindValue (":item_url", link);
					if (!query.exec ())
					{
						LeechCraft::Util::DBLock::DumpError (query);
						return false;
					}
				
					query.finish ();

					return true;
				}
			}
			
			void SQLStorageBackend::RemoveItem (Item_ptr item,
					const QString& hash,
					const QString& parentTitle,
					const QString& parentUrl)
			{
				LeechCraft::Util::DBLock lock (DB_);
				try
				{
					lock.Init ();
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO << e.what ();
					return;
				}

				QString t = item->Title_;
				QString l = item->Link_;
				if (!PerformRemove (RemoveEnclosures_, hash, t, l) ||
						!PerformRemove (RemoveMediaRSS_, hash, t, l) ||
						!PerformRemove (RemoveMediaRSSThumbnails_, hash, t, l) ||
						!PerformRemove (RemoveMediaRSSCredits_, hash, t, l) ||
						!PerformRemove (RemoveMediaRSSComments_, hash, t, l) ||
						!PerformRemove (RemoveMediaRSSPeerLinks_, hash, t, l) ||
						!PerformRemove (RemoveMediaRSSScenes_, hash, t, l))
				{
					qWarning () << Q_FUNC_INFO
						<< "a Remove* query failed";
					return;
				}

				RemoveItem_.bindValue (":parents_hash", hash);
				RemoveItem_.bindValue (":title", item->Title_);
				RemoveItem_.bindValue (":url", item->Link_);
			
				if (!RemoveItem_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (RemoveItem_);
					return;
				}
			
				RemoveItem_.finish ();
			
				lock.Good ();
			
				Channel_ptr channel = GetChannel (parentTitle, parentUrl);
				emit itemDataUpdated (item, channel);
				emit channelDataUpdated (channel);
			}
			
			void SQLStorageBackend::RemoveFeed (const QString& url)
			{
				channels_shorts_t shorts;
				GetChannels (shorts, url);
			
				LeechCraft::Util::DBLock lock (DB_);
				try
				{
					lock.Init ();
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO << e.what ();
					return;
				}
			
				for (channels_shorts_t::iterator i = shorts.begin (),
						end = shorts.end (); i != end; ++i)
				{
					QSqlQuery query (DB_);
					query.prepare ("DELETE FROM items "
							"WHERE parents_hash = :parents_hash");
					query.bindValue (":parents_hash", url + i->Title_);
			
					if (!query.exec ())
					{
						LeechCraft::Util::DBLock::DumpError (query);
						return;
					}
			
					query.prepare ("DELETE FROM enclosures "
							"WHERE item_parents_hash = :item_parents_hash");
					query.bindValue (":item_parents_hash", url + i->Title_);
					if (!query.exec ())
					{
						LeechCraft::Util::DBLock::DumpError (query);
						return;
					}
			
					RemoveChannel_.bindValue (":parent_feed_url", url);
					if (!RemoveChannel_.exec ())
					{
						LeechCraft::Util::DBLock::DumpError (RemoveChannel_);
						return;
					}
			
					RemoveChannel_.finish ();
				}
			
				RemoveFeed_.bindValue (":url", url);
				if (!RemoveFeed_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (RemoveFeed_);
					return;
				}
			
				RemoveFeed_.finish ();
			
				lock.Good ();
			}
			
			void SQLStorageBackend::ToggleChannelUnread (const QString& purl,
					const QString& title,
					bool state)
			{
				items_container_t oldItems;
				GetItems (oldItems, purl + title);

				ToggleChannelUnread_.bindValue (0, state);
				ToggleChannelUnread_.bindValue (1, purl + title);
				ToggleChannelUnread_.bindValue (2, state);
			
				if (!ToggleChannelUnread_.exec ())
				{
					qWarning () << Q_FUNC_INFO;
					LeechCraft::Util::DBLock::DumpError (ToggleChannelUnread_);
					throw std::runtime_error (qPrintable (QString (
									"Failed to toggle item t %1, p %2")
								.arg (title)
								.arg (purl)));
				}
			
				ToggleChannelUnread_.finish ();
			
				Channel_ptr channel = GetChannel (title, purl);
				emit channelDataUpdated (channel);
				for (size_t i = 0; i < oldItems.size (); ++i)
					if (oldItems.at (i)->Unread_ != state)
					{
						oldItems.at (i)->Unread_ = state;
						emit itemDataUpdated (oldItems.at (i), channel);
					}
			}
			
			bool SQLStorageBackend::UpdateFeedsStorage (int, int)
			{
				return true;
			}
			
			bool SQLStorageBackend::UpdateChannelsStorage (int, int)
			{
				return true;
			}
			
			bool SQLStorageBackend::UpdateItemsStorage (int oldV, int newV)
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

			QString SQLStorageBackend::GetBoolType () const
			{
				switch (Type_)
				{
					case SBSQLite:
						return "TINYINT";
					case SBPostgres:
						return "BOOLEAN";
				}
			}

			QString SQLStorageBackend::GetBlobType () const
			{
				switch (Type_)
				{
					case SBSQLite:
						return "BLOB";
					case SBPostgres:
						return "BYTEA";
				}
			}
			
			bool SQLStorageBackend::InitializeTables ()
			{
				QSqlQuery query (DB_);
				if (!DB_.tables ().contains ("feeds"))
				{
					if (!query.exec ("CREATE TABLE feeds ("
							"url TEXT PRIMARY KEY, "
							"last_update TIMESTAMP "
							");"))
					{
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
						return false;
					}
				}
			
				if (!DB_.tables ().contains ("feeds_settings"))
				{
					if (!query.exec (QString ("CREATE TABLE feeds_settings ("
									"feed_url TEXT PRIMARY KEY, "
									"update_timeout INTEGER NOT NULL, "
									"num_items INTEGER NOT NULL, "
									"item_age INTEGER NOT NULL, "
									"auto_download_enclosures %1 NOT NULL"
									");").arg (GetBoolType ())))
					{
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
						return false;
					}
			
					if (Type_ == SBPostgres)
					{
						if (!query.exec ("CREATE RULE \"replace_feeds_settings\" AS "
											"ON INSERT TO \"feeds_settings\" "
											"WHERE "
												"EXISTS (SELECT 1 FROM feeds_settings "
													"WHERE feed_url = NEW.feed_url) "
											"DO INSTEAD "
												"(UPDATE feeds_settings "
													"SET update_timeout = NEW.update_timeout, "
													"num_items = NEW.num_items, "
													"item_age = NEW.item_age, "
													"auto_download_enclosures = NEW.auto_download_enclosures "
													"WHERE feed_url = NEW.feed_url)"))
						{
							LeechCraft::Util::DBLock::DumpError (query);
							return false;
						}
					}
				}
			
				if (!DB_.tables ().contains ("channels"))
				{
					if (!query.exec (QString ("CREATE TABLE channels ("
							"parent_feed_url TEXT, "
							"url TEXT, "
							"title TEXT, "
							"description TEXT, "
							"last_build TIMESTAMP, "
							"tags TEXT, "
							"language TEXT, "
							"author TEXT, "
							"pixmap_url TEXT, "
							"pixmap %1, "
							"favicon %1 "
							");").arg (GetBlobType ())))
					{
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
						return false;
					}
			
					if (!query.exec ("CREATE INDEX idx_channels_parent_feed_url "
								"ON channels (parent_feed_url);"))
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
			
					if (!query.exec ("CREATE UNIQUE INDEX idx_channels_parent_feed_url_title "
								"ON channels (parent_feed_url, title);"))
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
			
					if (!query.exec ("CREATE UNIQUE INDEX "
								"idx_channels_parent_feed_url_title_url "
								"ON channels (parent_feed_url, title, url);"))
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
				}
			
				if (!DB_.tables ().contains ("items"))
				{
					if (!query.exec (QString ("CREATE TABLE items ("
							"parents_hash TEXT, "
							"title TEXT, "
							"url TEXT, "
							"description TEXT, "
							"author TEXT, "
							"category TEXT, "
							"guid TEXT, "
							"pub_date TIMESTAMP, "
							"unread %1, "
							"num_comments SMALLINT, "
							"comments_url TEXT, "
							"comments_page_url TEXT, "
							"latitude TEXT, "
							"longitude TEXT"
							");").arg (GetBoolType ())))
					{
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
						return false;
					}
			
					if (!query.exec ("CREATE INDEX idx_items_parents_hash "
								"ON items (parents_hash);"))
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
			
					if (!query.exec ("CREATE INDEX idx_items_parents_hash_unread "
								"ON items (parents_hash, unread);"))
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
			
					if (!query.exec ("CREATE INDEX "
								"idx_items_parents_hash_title_url "
								"ON items (parents_hash, title, url);"))
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
				}
			
				if (!DB_.tables ().contains ("enclosures"))
				{
					if (!query.exec ("CREATE TABLE enclosures ("
								"url TEXT NOT NULL, "
								"type TEXT NOT NULL, "
								"length BIGINT NOT NULL, "
								"lang TEXT, "
								"item_parents_hash TEXT, "
								"item_title TEXT, "
								"item_url TEXT, "
								"PRIMARY KEY (item_parents_hash, item_title, item_url, url)"
								");"))
					{
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
						return false;
					}
			
					if (Type_ == SBPostgres)
					{
						if (!query.exec ("CREATE RULE \"replace_enclosures\" AS "
											"ON INSERT TO \"enclosures\" "
											"WHERE "
												"EXISTS (SELECT 1 FROM enclosures "
													"WHERE item_parents_hash = NEW.item_parents_hash "
													"AND item_title = NEW.item_title "
													"AND item_url = NEW.item_url "
													"AND url = NEW.url) "
											"DO INSTEAD "
												"(UPDATE enclosures "
													"SET type = NEW.type, "
													"length = NEW.length, "
													"lang = NEW.lang "
													"WHERE item_parents_hash = NEW.item_parents_hash "
													"AND item_title = NEW.item_title "
													"AND item_url = NEW.item_url "
													"AND url = NEW.url)"))
						{
							LeechCraft::Util::DBLock::DumpError (query);
							return false;
						}
					}
			
					if (!query.exec ("CREATE INDEX "
								"idx_enclosures_item_parents_hash_item_title_item_url "
								"ON enclosures (item_parents_hash, item_title, item_url);"))
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
				}

				if (!DB_.tables ().contains ("mrss"))
				{
					if (!query.exec (QString ("CREATE TABLE mrss ("
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
									"item_url TEXT, "
									"PRIMARY KEY (item_parents_hash, item_title, item_url, url)"
									");").arg (GetBoolType ())))
					{
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
						return false;
					}

					if (Type_ == SBPostgres)
					{
						if (!query.exec ("CREATE RULE \"replace_mrss\" AS "
											"ON INSERT TO \"mrss\" "
											"WHERE "
												"EXISTS (SELECT 1 FROM mrss "
													"WHERE item_parents_hash = NEW.item_parents_hash "
													"AND item_title = NEW.item_title "
													"AND item_url = NEW.item_url "
													"AND url = NEW.url) "
											"DO INSTEAD "
												"(UPDATE mrss "
													"SET size = NEW.size, "
													"type = NEW.type, "
													"medium = NEW.medium, "
													"is_default = NEW.is_default, "
													"expression = NEW.expression, "
													"bitrate = NEW.bitrate, "
													"framerate = NEW.framerate, "
													"samplingrate = NEW.samplingrate, "
													"channels = NEW.channels, "
													"duration = NEW.duration, "
													"width = NEW.width, "
													"height = NEW.height, "
													"lang = NEW.lang, "
													"mediagroup = NEW.mediagroup, "
													"rating = NEW.rating, "
													"rating_scheme = NEW.rating_scheme, "
													"title = NEW.title, "
													"description = NEW.description, "
													"keywords = NEW.keywords, "
													"copyright_url = NEW.copyright_url, "
													"copyright_text = NEW.copyright_text, "
													"star_rating_average = NEW.star_rating_average, "
													"star_rating_count = NEW.star_rating_count, "
													"star_rating_min = NEW.star_rating_min, "
													"star_rating_max = NEW.star_rating_max, "
													"stat_views = NEW.stat_views, "
													"stat_favs = NEW.stat_favs, "
													"tags = NEW.tags "
													"WHERE item_parents_hash = NEW.item_parents_hash "
													"AND item_title = NEW.item_title "
													"AND item_url = NEW.item_url "
													"AND url = NEW.url)"))
						{
							LeechCraft::Util::DBLock::DumpError (query);
							return false;
						}
					}

					if (!query.exec ("CREATE INDEX "
								"idx_mrss_item_parents_hash_item_title_item_url "
								"ON mrss (item_parents_hash, item_title, item_url);"))
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
				}
				
				if (!DB_.tables ().contains ("mrss_thumbnails"))
				{
					if (!query.exec ("CREATE TABLE mrss_thumbnails ("
								"parent_url TEXT, "
								"item_parents_hash TEXT, "
								"item_title TEXT, "
								"item_url TEXT, "
								"url TEXT, "
								"width INTEGER, "
								"height INTEGER, "
								"time TEXT, "
								"PRIMARY KEY (item_parents_hash, item_title, item_url, parent_url, url)"
								");"))
					{
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
						return false;
					}

					if (Type_ == SBPostgres)
					{
						if (!query.exec ("CREATE RULE \"replace_mrss_thumbnails\" AS "
											"ON INSERT TO \"mrss_thumbnails\" "
											"WHERE "
												"EXISTS (SELECT 1 FROM mrss_thumbnails "
													"WHERE item_parents_hash = NEW.item_parents_hash "
													"AND item_title = NEW.item_title "
													"AND item_url = NEW.item_url "
													"AND parent_url = NEW.parent_url "
													"AND url = NEW.url) "
											"DO INSTEAD "
												"(UPDATE mrss_thumbnails "
													"SET width = NEW.width, "
													"height = NEW.height, "
													"time = NEW.time "
													"WHERE item_parents_hash = NEW.item_parents_hash "
													"AND item_title = NEW.item_title "
													"AND item_url = NEW.item_url "
													"AND parent_url = NEW.parent_url "
													"AND url = NEW.url)"))
						{
							LeechCraft::Util::DBLock::DumpError (query);
							return false;
						}
					}

					if (!query.exec ("CREATE INDEX "
								"idx_mrss_thumbnails_parent_url_item_parents_hash_item_title_item_url "
								"ON mrss_thumbnails (parent_url, item_parents_hash, item_title, item_url);"))
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
				}

				if (!DB_.tables ().contains ("mrss_credits"))
				{

					if (!query.exec ("CREATE TABLE mrss_credits ("
								"parent_url TEXT, "
								"item_parents_hash TEXT, "
								"item_title TEXT, "
								"item_url TEXT, "
								"role TEXT, "
								"who TEXT, "
								"PRIMARY KEY (item_parents_hash, item_title, item_url, parent_url, role)"
								");"))
					{
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
						return false;
					}

					if (!query.exec ("CREATE INDEX "
								"idx_mrss_credits_parent_url_item_parents_hash_item_title_item_url "
								"ON mrss_credits (parent_url, item_parents_hash, item_title, item_url);"))
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
				}

				if (!DB_.tables ().contains ("mrss_comments"))
				{
					if (!query.exec ("CREATE TABLE mrss_comments ("
								"parent_url TEXT, "
								"item_parents_hash TEXT, "
								"item_title TEXT, "
								"item_url TEXT, "
								"type TEXT, "
								"comment TEXT"
								");"))
					{
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
						return false;
					}

					if (!query.exec ("CREATE INDEX "
								"idx_mrss_comments_parent_url_item_parents_hash_item_title_item_url "
								"ON mrss_comments (parent_url, item_parents_hash, item_title, item_url);"))
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
				}

				if (!DB_.tables ().contains ("mrss_peerlinks"))
				{
					if (!query.exec ("CREATE TABLE mrss_peerlinks ("
								"parent_url TEXT, "
								"item_parents_hash TEXT, "
								"item_title TEXT, "
								"item_url TEXT, "
								"type TEXT, "
								"link TEXT"
								");"))
					{
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
						return false;
					}

					if (!query.exec ("CREATE INDEX "
								"idx_mrss_peerlinks_parent_url_item_parents_hash_item_title_item_url "
								"ON mrss_peerlinks (parent_url, item_parents_hash, item_title, item_url);"))
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
				}

				if (!DB_.tables ().contains ("mrss_scenes"))
				{
					if (!query.exec ("CREATE TABLE mrss_scenes ("
								"parent_url TEXT, "
								"item_parents_hash TEXT, "
								"item_title TEXT, "
								"item_url TEXT, "
								"title TEXT, "
								"description TEXT, "
								"start_time TEXT, "
								"end_time TEXT"
								");"))
					{
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
						return false;
					}

					if (!query.exec ("CREATE INDEX "
								"idx_mrss_scenes_parent_url_item_parents_hash_item_title_item_url "
								"ON mrss_scenes (parent_url, item_parents_hash, item_title, item_url);"))
						LeechCraft::Util::DBLock::DumpError (query.lastError ());
				}
			
				return true;
			}
			
			QByteArray SQLStorageBackend::SerializePixmap (const QPixmap& pixmap) const
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
			
			QPixmap SQLStorageBackend::UnserializePixmap (const QByteArray& bytes) const
			{
				QPixmap result;
				if (bytes.size ())
					result.loadFromData (bytes, "PNG");
				return result;
			}
			
			bool SQLStorageBackend::RollItemsStorage (int version)
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
					QString adeType;
					switch (Type_)
					{
						case SBSQLite:
							adeType = "TINYINT";
							break;
						case SBPostgres:
							adeType = "BOOLEAN";
							break;
					}

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
			
				lock.Good ();
				return true;
			}
			
			void SQLStorageBackend::FillItem (const QSqlQuery& query, Item_ptr& item) const
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

			void SQLStorageBackend::WriteEnclosures (const QString& hash,
					const QString& title, const QString& link,
					const QList<Enclosure>& enclosures)
			{
				for (QList<Enclosure>::const_iterator i = enclosures.begin (),
						end = enclosures.end (); i != end; ++i)
				{
					WriteEnclosure_.bindValue (":url", i->URL_);
					WriteEnclosure_.bindValue (":type", i->Type_);
					WriteEnclosure_.bindValue (":length", i->Length_);
					WriteEnclosure_.bindValue (":lang", i->Lang_);
					WriteEnclosure_.bindValue (":item_parents_hash", hash);
					WriteEnclosure_.bindValue (":item_title", title);
					WriteEnclosure_.bindValue (":item_url", link);
			
					if (!WriteEnclosure_.exec ())
						LeechCraft::Util::DBLock::DumpError (WriteEnclosure_);
				}
			
				WriteEnclosure_.finish ();
			}
			
			void SQLStorageBackend::GetEnclosures (const QString& hash, const QString& title,
					const QString& link, QList<Enclosure>& enclosures) const
			{
				GetEnclosures_.bindValue (":item_parents_hash", hash);
				GetEnclosures_.bindValue (":item_title", title);
				GetEnclosures_.bindValue (":item_url", link);
			
				if (!GetEnclosures_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (GetEnclosures_);
					return;
				}
			
				while (GetEnclosures_.next ())
				{
					Enclosure e =
					{
						GetEnclosures_.value (0).toString (),
						GetEnclosures_.value (1).toString (),
						GetEnclosures_.value (2).toLongLong (),
						GetEnclosures_.value (3).toString ()
					};
			
					enclosures << e;
				}
			
				GetEnclosures_.finish ();
			}

			void SQLStorageBackend::WriteMRSSEntries (const QString& hash,
					const QString& title, const QString& link,
					const QList<MRSSEntry>& entries)
			{
				Q_FOREACH (MRSSEntry e, entries)
				{
					WriteMediaRSS_.bindValue (":url", e.URL_);
					WriteMediaRSS_.bindValue (":size", e.Size_);
					WriteMediaRSS_.bindValue (":type", e.Type_);
					WriteMediaRSS_.bindValue (":medium", e.Medium_);
					WriteMediaRSS_.bindValue (":is_default", e.IsDefault_);
					WriteMediaRSS_.bindValue (":expression", e.Expression_);
					WriteMediaRSS_.bindValue (":bitrate", e.Bitrate_);
					WriteMediaRSS_.bindValue (":framerate", e.Framerate_);
					WriteMediaRSS_.bindValue (":samplingrate", e.SamplingRate_);
					WriteMediaRSS_.bindValue (":channels", e.Channels_);
					WriteMediaRSS_.bindValue (":duration", e.Duration_);
					WriteMediaRSS_.bindValue (":width", e.Width_);
					WriteMediaRSS_.bindValue (":height", e.Height_);
					WriteMediaRSS_.bindValue (":lang", e.Lang_);
					WriteMediaRSS_.bindValue (":mediagroup", e.Group_);
					WriteMediaRSS_.bindValue (":rating", e.Rating_);
					WriteMediaRSS_.bindValue (":rating_scheme", e.RatingScheme_);
					WriteMediaRSS_.bindValue (":title", e.Title_);
					WriteMediaRSS_.bindValue (":description", e.Description_);
					WriteMediaRSS_.bindValue (":keywords", e.Keywords_);
					WriteMediaRSS_.bindValue (":copyright_url", e.CopyrightURL_);
					WriteMediaRSS_.bindValue (":copyright_text", e.CopyrightText_);
					WriteMediaRSS_.bindValue (":star_rating_average", e.RatingAverage_);
					WriteMediaRSS_.bindValue (":star_rating_count", e.RatingCount_);
					WriteMediaRSS_.bindValue (":star_rating_min", e.RatingMin_);
					WriteMediaRSS_.bindValue (":star_rating_max", e.RatingMax_);
					WriteMediaRSS_.bindValue (":stat_views", e.Views_);
					WriteMediaRSS_.bindValue (":stat_favs", e.Favs_);
					WriteMediaRSS_.bindValue (":tags", e.Tags_);
					WriteMediaRSS_.bindValue (":item_parents_hash", hash);
					WriteMediaRSS_.bindValue (":item_title", title);
					WriteMediaRSS_.bindValue (":item_url", link);

					if (!WriteMediaRSS_.exec ())
					{
						LeechCraft::Util::DBLock::DumpError (WriteMediaRSS_);
						continue;
					}
					
					WriteMediaRSS_.finish ();

					Q_FOREACH (MRSSThumbnail t, e.Thumbnails_)
					{
						WriteMediaRSSThumbnail_.bindValue (":parent_url", e.URL_);
						WriteMediaRSSThumbnail_.bindValue (":item_parents_hash", hash);
						WriteMediaRSSThumbnail_.bindValue (":item_title", title);
						WriteMediaRSSThumbnail_.bindValue (":item_url", link);
						WriteMediaRSSThumbnail_.bindValue (":url", t.URL_);
						WriteMediaRSSThumbnail_.bindValue (":width", t.Width_);
						WriteMediaRSSThumbnail_.bindValue (":height", t.Height_);
						WriteMediaRSSThumbnail_.bindValue (":time", t.Time_);

						if (!WriteMediaRSSThumbnail_.exec ())
							LeechCraft::Util::DBLock::DumpError (WriteMediaRSSThumbnail_);
						
						WriteMediaRSSThumbnail_.finish ();
					}

					Q_FOREACH (MRSSCredit c, e.Credits_)
					{
						WriteMediaRSSCredit_.bindValue (":parent_url", e.URL_);
						WriteMediaRSSCredit_.bindValue (":item_parents_hash", hash);
						WriteMediaRSSCredit_.bindValue (":item_title", title);
						WriteMediaRSSCredit_.bindValue (":item_url", link);
						WriteMediaRSSCredit_.bindValue (":role", c.Role_);
						WriteMediaRSSCredit_.bindValue (":who", c.Who_);

						if (!WriteMediaRSSCredit_.exec ())
							LeechCraft::Util::DBLock::DumpError (WriteMediaRSSCredit_);
						
						WriteMediaRSSCredit_.finish ();
					}

					Q_FOREACH (MRSSComment c, e.Comments_)
					{
						WriteMediaRSSComment_.bindValue (":parent_url", e.URL_);
						WriteMediaRSSComment_.bindValue (":item_parents_hash", hash);
						WriteMediaRSSComment_.bindValue (":item_title", title);
						WriteMediaRSSComment_.bindValue (":item_url", link);
						WriteMediaRSSComment_.bindValue (":type", c.Type_);
						WriteMediaRSSComment_.bindValue (":comment", c.Comment_);

						if (!WriteMediaRSSComment_.exec ())
							LeechCraft::Util::DBLock::DumpError (WriteMediaRSSComment_);
						
						WriteMediaRSSComment_.finish ();
					}

					Q_FOREACH (MRSSPeerLink p, e.PeerLinks_)
					{
						WriteMediaRSSPeerLink_.bindValue (":parent_url", e.URL_);
						WriteMediaRSSPeerLink_.bindValue (":item_parents_hash", hash);
						WriteMediaRSSPeerLink_.bindValue (":item_title", title);
						WriteMediaRSSPeerLink_.bindValue (":item_url", link);
						WriteMediaRSSPeerLink_.bindValue (":type", p.Type_);
						WriteMediaRSSPeerLink_.bindValue (":link", p.Link_);

						if (!WriteMediaRSSPeerLink_.exec ())
							LeechCraft::Util::DBLock::DumpError (WriteMediaRSSPeerLink_);
						
						WriteMediaRSSPeerLink_.finish ();
					}

					Q_FOREACH (MRSSScene s, e.Scenes_)
					{
						WriteMediaRSSScene_.bindValue (":parent_url", e.URL_);
						WriteMediaRSSScene_.bindValue (":item_parents_hash", hash);
						WriteMediaRSSScene_.bindValue (":item_title", title);
						WriteMediaRSSScene_.bindValue (":item_url", link);
						WriteMediaRSSScene_.bindValue (":title", s.Title_);
						WriteMediaRSSScene_.bindValue (":description", s.Description_);
						WriteMediaRSSScene_.bindValue (":start_time", s.StartTime_);
						WriteMediaRSSScene_.bindValue (":end_time", s.EndTime_);

						if (!WriteMediaRSSScene_.exec ())
							LeechCraft::Util::DBLock::DumpError (WriteMediaRSSScene_);
						
						WriteMediaRSSScene_.finish ();
					}
				}
			}

			void SQLStorageBackend::GetMRSSEntries (const QString& hash, const QString& title,
					const QString& link, QList<MRSSEntry>& entries) const
			{
				GetMediaRSSs_.bindValue (":item_parents_hash", hash);
				GetMediaRSSs_.bindValue (":item_title", title);
				GetMediaRSSs_.bindValue (":item_url", link);

				if (!GetMediaRSSs_.exec ())
				{
					LeechCraft::Util::DBLock::DumpError (GetMediaRSSs_);
					return;
				}

				while (GetMediaRSSs_.next ())
				{
					QString eUrl = GetMediaRSSs_.value (0).toString ();
					MRSSEntry e =
					{
						eUrl,
						GetMediaRSSs_.value (1).toLongLong (),
						GetMediaRSSs_.value (2).toString (),
						GetMediaRSSs_.value (3).toString (),
						GetMediaRSSs_.value (4).toBool (),
						GetMediaRSSs_.value (5).toString (),
						GetMediaRSSs_.value (6).toInt (),
						GetMediaRSSs_.value (7).toDouble (),
						GetMediaRSSs_.value (8).toDouble (),
						GetMediaRSSs_.value (9).toInt (),
						GetMediaRSSs_.value (10).toInt (),
						GetMediaRSSs_.value (11).toInt (),
						GetMediaRSSs_.value (12).toInt (),
						GetMediaRSSs_.value (13).toString (),
						GetMediaRSSs_.value (14).toInt (),
						GetMediaRSSs_.value (15).toString (),
						GetMediaRSSs_.value (16).toString (),
						GetMediaRSSs_.value (17).toString (),
						GetMediaRSSs_.value (18).toString (),
						GetMediaRSSs_.value (19).toString (),
						GetMediaRSSs_.value (20).toString (),
						GetMediaRSSs_.value (21).toString (),
						GetMediaRSSs_.value (22).toInt (),
						GetMediaRSSs_.value (23).toInt (),
						GetMediaRSSs_.value (24).toInt (),
						GetMediaRSSs_.value (25).toInt (),
						GetMediaRSSs_.value (26).toInt (),
						GetMediaRSSs_.value (27).toInt (),
						GetMediaRSSs_.value (28).toString (),
						QList<MRSSThumbnail> (),
						QList<MRSSCredit> (),
						QList<MRSSComment> (),
						QList<MRSSPeerLink> (),
						QList<MRSSScene> ()
					};

					GetMediaRSSThumbnails_.bindValue (":parent_url", eUrl);
					GetMediaRSSThumbnails_.bindValue (":item_parents_hash", hash);
					GetMediaRSSThumbnails_.bindValue (":item_title", title);
					GetMediaRSSThumbnails_.bindValue (":item_url", link);
					if (!GetMediaRSSThumbnails_.exec ())
						LeechCraft::Util::DBLock::DumpError (GetMediaRSSThumbnails_);
					else
					{
						while (GetMediaRSSThumbnails_.next ())
						{
							MRSSThumbnail th =
							{
								GetMediaRSSThumbnails_.value (0).toString (),
								GetMediaRSSThumbnails_.value (1).toInt (),
								GetMediaRSSThumbnails_.value (2).toInt (),
								GetMediaRSSThumbnails_.value (3).toString ()
							};
							e.Thumbnails_ << th;
						}
						GetMediaRSSThumbnails_.finish ();
					}

					GetMediaRSSCredits_.bindValue (":parent_url", eUrl);
					GetMediaRSSCredits_.bindValue (":item_parents_hash", hash);
					GetMediaRSSCredits_.bindValue (":item_title", title);
					GetMediaRSSCredits_.bindValue (":item_url", link);
					if (!GetMediaRSSCredits_.exec ())
						LeechCraft::Util::DBLock::DumpError (GetMediaRSSCredits_);
					else
					{
						while (GetMediaRSSCredits_.next ())
						{
							MRSSCredit cr =
							{
								GetMediaRSSCredits_.value (0).toString (),
								GetMediaRSSCredits_.value (1).toString ()
							};
							e.Credits_ << cr;
						}
						GetMediaRSSCredits_.finish ();
					}

					GetMediaRSSComments_.bindValue (":parent_url", eUrl);
					GetMediaRSSComments_.bindValue (":item_parents_hash", hash);
					GetMediaRSSComments_.bindValue (":item_title", title);
					GetMediaRSSComments_.bindValue (":item_url", link);
					if (!GetMediaRSSComments_.exec ())
						LeechCraft::Util::DBLock::DumpError (GetMediaRSSComments_);
					else
					{
						while (GetMediaRSSComments_.next ())
						{
							MRSSComment cm =
							{
								GetMediaRSSComments_.value (0).toString (),
								GetMediaRSSComments_.value (1).toString ()
							};
							e.Comments_ << cm;
						}
						GetMediaRSSComments_.finish ();
					}

					GetMediaRSSPeerLinks_.bindValue (":parent_url", eUrl);
					GetMediaRSSPeerLinks_.bindValue (":item_parents_hash", hash);
					GetMediaRSSPeerLinks_.bindValue (":item_title", title);
					GetMediaRSSPeerLinks_.bindValue (":item_url", link);
					if (!GetMediaRSSPeerLinks_.exec ())
						LeechCraft::Util::DBLock::DumpError (GetMediaRSSPeerLinks_);
					else
					{
						while (GetMediaRSSPeerLinks_.next ())
						{
							MRSSPeerLink pl =
							{
								GetMediaRSSPeerLinks_.value (0).toString (),
								GetMediaRSSPeerLinks_.value (1).toString ()
							};
							e.PeerLinks_ << pl;
						}
						GetMediaRSSPeerLinks_.finish ();
					}

					GetMediaRSSScenes_.bindValue (":parent_url", eUrl);
					GetMediaRSSScenes_.bindValue (":item_parents_hash", hash);
					GetMediaRSSScenes_.bindValue (":item_title", title);
					GetMediaRSSScenes_.bindValue (":item_url", link);
					if (!GetMediaRSSScenes_.exec ())
						LeechCraft::Util::DBLock::DumpError (GetMediaRSSScenes_);
					else
					{
						while (GetMediaRSSScenes_.next ())
						{
							MRSSScene th =
							{
								GetMediaRSSScenes_.value (0).toString (),
								GetMediaRSSScenes_.value (1).toString (),
								GetMediaRSSScenes_.value (2).toString (),
								GetMediaRSSScenes_.value (3).toString ()
							};
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

