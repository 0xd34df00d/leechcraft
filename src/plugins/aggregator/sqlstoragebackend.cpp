/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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
#include <boost/optional.hpp>
#include <QDir>
#include <QDebug>
#include <QBuffer>
#include <QSqlError>
#include <QVariant>
#include <QSqlRecord>
#include <util/dblock.h>
#include <util/defaulthookproxy.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LeechCraft
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
				break;
			case SBMysql:
				break;
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
			case SBMysql:
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
				Util::DBLock::DumpError (pragma);
			if (!pragma.exec (QString ("PRAGMA synchronous = %1;")
						.arg (XmlSettingsManager::Instance ()->
							property ("SQLiteSynchronous").toString ())))
				Util::DBLock::DumpError (pragma);
			if (!pragma.exec (QString ("PRAGMA temp_store = %1;")
						.arg (XmlSettingsManager::Instance ()->
							property ("SQLiteTempStore").toString ())))
				Util::DBLock::DumpError (pragma);
			if (!pragma.exec ("PRAGMA foreign_keys = ON;"))
			{
				Util::DBLock::DumpError (pragma);
				qWarning () << Q_FUNC_INFO
						<< "unable to enable foreign keys, DB work may be incorrect";
			}
		}

		FeedFinderByURL_ = QSqlQuery (DB_);
		FeedFinderByURL_.prepare ("SELECT feed_id "
				"FROM feeds "
				"WHERE url = :url");

		FeedGetter_ = QSqlQuery (DB_);
		FeedGetter_.prepare ("SELECT "
				"url, "
				"last_update "
				"FROM feeds "
				"WHERE feed_id = :feed_id");

		FeedSettingsGetter_ = QSqlQuery (DB_);
		FeedSettingsGetter_.prepare ("SELECT "
				"settings_id, "
				"update_timeout, "
				"num_items, "
				"item_age, "
				"auto_download_enclosures "
				"FROM feeds_settings "
				"WHERE feed_id = :feed_id");

		FeedSettingsSetter_ = QSqlQuery (DB_);
		QString orReplace;
		if (Type_ == SBSQLite)
			orReplace = "OR REPLACE";

		FeedSettingsSetter_.prepare (QString ("INSERT %1 INTO feeds_settings ("
				"feed_id, "
				"settings_id, "
				"update_timeout, "
				"num_items, "
				"item_age, "
				"auto_download_enclosures"
				") VALUES ("
				":feed_id, "
				":settings_id, "
				":update_timeout, "
				":num_items, "
				":item_age, "
				":auto_download_enclosures"
				")").arg (orReplace));

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
				"WHERE feed_id = :feed_id "
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
				"WHERE channel_id = :channel_id "
				"ORDER BY title");

		UnreadItemsCounter_ = QSqlQuery (DB_);
		switch (Type_)
		{
			case SBSQLite:
				UnreadItemsCounter_.prepare ("SELECT COUNT (unread) "
						"FROM items "
						"WHERE channel_id = :channel_id "
						"AND unread = \"true\"");
				break;
			case SBPostgres:
				UnreadItemsCounter_.prepare ("SELECT COUNT (1) "
						"FROM items "
						"WHERE channel_id = :channel_id "
						"AND unread");
				break;
			case SBMysql:
				break;
		}

		ItemsShortSelector_ = QSqlQuery (DB_);
		ItemsShortSelector_.prepare ("SELECT "
				"item_id, "
				"title, "
				"url, "
				"category, "
				"pub_date, "
				"unread "
				"FROM items "
				"WHERE channel_id = :channel_id "
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
				"longitude, "
				"channel_id "
				"FROM items "
				"WHERE item_id = :item_id "
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
				"WHERE channel_id = :channel_id "
				"ORDER BY pub_date DESC");

		ChannelFinder_ = QSqlQuery (DB_);
		ChannelFinder_.prepare ("SELECT 1 "
				"FROM channels "
				"WHERE channel_id = :channel_id");

		ChannelIDFromTitleURL_ = QSqlQuery (DB_);
		ChannelIDFromTitleURL_.prepare ("SELECT channel_id "
				"FROM channels "
				"WHERE feed_id = :feed_id "
				"AND title = :title "
				"AND url = :url");

		ItemIDFromTitleURL_ = QSqlQuery (DB_);
		ItemIDFromTitleURL_.prepare ("SELECT item_id "
				"FROM items "
				"WHERE channel_id = :channel_id "
				"AND COALESCE (title,'') = COALESCE (:title,'') "
				"AND COALESCE (url,'') = COALESCE (:url,'')");

		InsertFeed_ = QSqlQuery (DB_);
		InsertFeed_.prepare ("INSERT INTO feeds (feed_id, url, last_update) VALUES (:feed_id, :url, :last_update);");

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
				":channel_id, "
				":feed_id, "
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
				":item_id, "
				":channel_id, "
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
				");");

		UpdateShortChannel_ = QSqlQuery (DB_);
		UpdateShortChannel_.prepare ("UPDATE channels SET "
				"tags = :tags, "
				"last_build = :last_build "
				"WHERE channel_id = :channel_id");

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
				"WHERE channel_id = :channel_id");

		QString common = "DELETE FROM items "
			"WHERE channel_id = :channel_id ";
		QString cdt;
		QString cnt;
		switch (Type_)
		{
			case SBSQLite:
				cdt = "AND (julianday ('now') - julianday (pub_date) > :age)";
				cnt = "AND item_id IN (SELECT item_id FROM items "
						"WHERE channel_id = :channel_id ORDER BY pub_date "
						"DESC LIMIT 10000 OFFSET :number)";
				break;
			case SBPostgres:
				cdt = "AND (pub_date - now () > :age * interval '1 day')";
				cnt = "AND pub_date IN "
					"(SELECT pub_date FROM items WHERE channel_id = :channel_id ORDER BY pub_date DESC OFFSET :number)";
				break;
			case SBMysql:
				break;
		}

		ChannelDateTrimmer_ = QSqlQuery (DB_);
		ChannelDateTrimmer_.prepare (common + cdt);

		ChannelNumberTrimmer_ = QSqlQuery (DB_);
		ChannelNumberTrimmer_.prepare (common + cnt);

		UpdateShortItem_ = QSqlQuery (DB_);
		UpdateShortItem_.prepare ("UPDATE items SET "
				"unread = :unread "
				"WHERE item_id = :item_id");

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
				"WHERE item_id = :item_id");

		ToggleChannelUnread_ = QSqlQuery (DB_);
		ToggleChannelUnread_.prepare ("UPDATE items SET "
				"unread = :unread "
				"WHERE channel_id = :channel_id "
				"AND unread <> :unread");

		RemoveFeed_ = QSqlQuery (DB_);
		RemoveFeed_.prepare ("DELETE FROM feeds "
				"WHERE feed_id = :feed_id");

		RemoveChannel_ = QSqlQuery (DB_);
		RemoveChannel_.prepare ("DELETE FROM channels "
				"WHERE channel_id = :channel_id");

		RemoveItem_ = QSqlQuery (DB_);
		RemoveItem_.prepare ("DELETE FROM items "
				"WHERE item_id = :item_id");

		WriteEnclosure_ = QSqlQuery (DB_);
		WriteEnclosure_.prepare (QString ("INSERT %1 INTO enclosures ("
				"url, "
				"type, "
				"length, "
				"lang, "
				"item_id, "
				"enclosure_id"
				") VALUES ("
				":url, "
				":type, "
				":length, "
				":lang, "
				":item_id, "
				":enclosure_id"
				")").arg (orReplace));

		WriteMediaRSS_ = QSqlQuery (DB_);
		WriteMediaRSS_.prepare (QString ("INSERT %1 INTO mrss ("
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
				":mrss_id, "
				":item_id, "
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
				":tags"
				")").arg (orReplace));

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
				"WHERE item_id = :item_id "
				"ORDER BY title");

		WriteMediaRSSThumbnail_ = QSqlQuery (DB_);
		WriteMediaRSSThumbnail_.prepare (QString ("INSERT %1 INTO mrss_thumbnails ("
				"mrss_thumb_id, "
				"mrss_id, "
				"url, "
				"width, "
				"height, "
				"time"
				") VALUES ("
				":mrss_thumb_id, "
				":mrss_id, "
				":url, "
				":width, "
				":height, "
				":time"
				")").arg (orReplace));

		GetMediaRSSThumbnails_ = QSqlQuery (DB_);
		GetMediaRSSThumbnails_.prepare ("SELECT "
				"mrss_thumb_id, "
				"url, "
				"width, "
				"height, "
				"time "
				"FROM mrss_thumbnails "
				"WHERE mrss_id = :mrss_id "
				"ORDER BY time");

		WriteMediaRSSCredit_ = QSqlQuery (DB_);
		WriteMediaRSSCredit_.prepare (QString ("INSERT %1 INTO mrss_credits ("
				"mrss_credits_id, "
				"mrss_id, "
				"role, "
				"who"
				") VALUES ("
				":mrss_credits_id, "
				":mrss_id, "
				":role, "
				":who"
				")").arg (orReplace));

		GetMediaRSSCredits_ = QSqlQuery (DB_);
		GetMediaRSSCredits_.prepare ("SELECT "
				"mrss_credits_id, "
				"role, "
				"who "
				"FROM mrss_credits "
				"WHERE mrss_id = :mrss_id "
				"ORDER BY role");

		WriteMediaRSSComment_ = QSqlQuery (DB_);
		WriteMediaRSSComment_.prepare (QString ("INSERT %1 INTO mrss_comments ("
				"mrss_comment_id, "
				"mrss_id, "
				"type, "
				"comment"
				") VALUES ("
				":mrss_comment_id, "
				":mrss_id, "
				":type, "
				":comment"
				")").arg (orReplace));

		GetMediaRSSComments_ = QSqlQuery (DB_);
		GetMediaRSSComments_.prepare ("SELECT "
				"mrss_comment_id, "
				"type, "
				"comment "
				"FROM mrss_comments "
				"WHERE mrss_id = :mrss_id "
				"ORDER BY comment");

		WriteMediaRSSPeerLink_ = QSqlQuery (DB_);
		WriteMediaRSSPeerLink_.prepare (QString ("INSERT %1 INTO mrss_peerlinks ("
				"mrss_peerlink_id, "
				"mrss_id, "
				"type, "
				"link"
				") VALUES ("
				":mrss_peerlink_id, "
				":mrss_id, "
				":type, "
				":link"
				")").arg (orReplace));

		GetMediaRSSPeerLinks_ = QSqlQuery (DB_);
		GetMediaRSSPeerLinks_.prepare ("SELECT "
				"mrss_peerlink_id, "
				"type, "
				"link "
				"FROM mrss_peerlinks "
				"WHERE mrss_id = :mrss_id "
				"ORDER BY link");

		WriteMediaRSSScene_ = QSqlQuery (DB_);
		WriteMediaRSSScene_.prepare (QString ("INSERT %1 INTO mrss_scenes ("
				"mrss_scene_id, "
				"mrss_id, "
				"title, "
				"description, "
				"start_time, "
				"end_time"
				") VALUES ("
				":mrss_scene_id, "
				":mrss_id, "
				":title, "
				":description, "
				":start_time, "
				":end_time"
				")").arg (orReplace));

		GetMediaRSSScenes_ = QSqlQuery (DB_);
		GetMediaRSSScenes_.prepare ("SELECT "
				"mrss_scene_id, "
				"title, "
				"description, "
				"start_time, "
				"end_time "
				"FROM mrss_scenes "
				"WHERE mrss_id = :mrss_id "
				"ORDER BY start_time");

		RemoveEnclosures_ = QSqlQuery (DB_);
		RemoveEnclosures_.prepare ("DELETE FROM enclosures "
				"WHERE item_id = :item_id");

		GetEnclosures_ = QSqlQuery (DB_);
		GetEnclosures_.prepare ("SELECT "
				"enclosure_id, "
				"url, "
				"type, "
				"length, "
				"lang "
				"FROM enclosures "
				"WHERE item_id = :item_id "
				"ORDER BY url");

		RemoveMediaRSS_ = QSqlQuery (DB_);
		RemoveMediaRSS_.prepare ("DELETE FROM mrss "
				"WHERE mrss_id = :mrss_id");

		RemoveMediaRSSThumbnails_ = QSqlQuery (DB_);
		RemoveMediaRSSThumbnails_.prepare ("DELETE FROM mrss_thumbnails "
				"WHERE mrss_thumb_id = :mrss_thumb_id");

		RemoveMediaRSSCredits_ = QSqlQuery (DB_);
		RemoveMediaRSSCredits_.prepare ("DELETE FROM mrss_credits "
				"WHERE mrss_credits_id = :mrss_credits_id");

		RemoveMediaRSSComments_ = QSqlQuery (DB_);
		RemoveMediaRSSComments_.prepare ("DELETE FROM mrss_comments "
				"WHERE mrss_comment_id = :mrss_comment_id");

		RemoveMediaRSSPeerLinks_ = QSqlQuery (DB_);
		RemoveMediaRSSPeerLinks_.prepare ("DELETE FROM mrss_peerlinks "
				"WHERE mrss_peerlink_id = :mrss_peerlink_id");

		RemoveMediaRSSScenes_ = QSqlQuery (DB_);
		RemoveMediaRSSScenes_.prepare ("DELETE FROM mrss_scenes "
				"WHERE mrss_scene_id = :mrss_scene_id");

		GetItemTags_ = QSqlQuery (DB_);
		GetItemTags_.prepare ("SELECT tag FROM items2tags "
				"WHERE item_id = :item_id");

		AddItemTag_ = QSqlQuery (DB_);
		AddItemTag_.prepare ("INSERT INTO items2tags "
				"(item_id, tag) VALUES (:item_id, :tag)");

		ClearItemTags_ = QSqlQuery (DB_);
		ClearItemTags_.prepare ("DELETE FROM items2tags "
				"WHERE item_id = :item_id");

		GetItemsForTag_ = QSqlQuery (DB_);
		GetItemsForTag_.prepare ("SELECT item_id FROM items2tags "
				"WHERE tag = :tag");
	}

	void SQLStorageBackend::GetFeedsIDs (ids_t& result) const
	{
		QSqlQuery feedSelector (DB_);
		if (!feedSelector.exec (QString ("SELECT feed_id "
					"FROM feeds "
					"ORDER BY feed_id")))
		{
			Util::DBLock::DumpError (feedSelector);
			return;
		}

		while (feedSelector.next ())
			result.push_back (feedSelector.value (0).toInt ());
	}

	QList<ITagsManager::tag_id> SQLStorageBackend::GetItemTags (const IDType_t& id)
	{
		QList<ITagsManager::tag_id> result;

		GetItemTags_.bindValue (":item_id", id);
		if (!GetItemTags_.exec ())
		{
			Util::DBLock::DumpError (GetItemTags_);
			return result;
		}

		while (GetItemTags_.next ())
			result << GetItemTags_.value (0).toString ();

		GetItemTags_.finish ();

		return result;
	}

	void SQLStorageBackend::SetItemTags (const IDType_t& id, const QList<ITagsManager::tag_id>& tags)
	{
		Util::DBLock lock (DB_);
		try
		{
			lock.Init ();
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to begin transaction:"
					<< e.what ();
			return;
		}

		ClearItemTags_.bindValue (":item_id", id);
		if (!ClearItemTags_.exec ())
		{
			Util::DBLock::DumpError (ClearItemTags_);
			return;
		}

		ClearItemTags_.finish ();

		Q_FOREACH (const ITagsManager::tag_id& tag, tags)
		{
			AddItemTag_.bindValue (":tag", tag);
			AddItemTag_.bindValue (":item_id", id);
			if (!AddItemTag_.exec ())
			{
				Util::DBLock::DumpError (AddItemTag_);
				return;
			}
		}

		lock.Good ();
	}

	QList<IDType_t> SQLStorageBackend::GetItemsForTag (const ITagsManager::tag_id& tag)
	{
		QList<IDType_t> result;

		GetItemsForTag_.bindValue (":tag", tag);
		if (!GetItemsForTag_.exec ())
		{
			Util::DBLock::DumpError (GetItemsForTag_);
			return result;
		}

		while (GetItemsForTag_.next ())
			result << GetItemsForTag_.value (0).toInt ();

		return result;
	}

	IDType_t SQLStorageBackend::GetHighestID (const PoolType& type) const
	{
		QString field, table;
		switch (type)
		{
			case PTFeed:
				field = "feed_id";
				table = "feeds";
				break;

			case PTChannel:
				field = "channel_id";
				table = "channels";
				break;

			case PTItem:
				field = "item_id";
				table = "items";
				break;

			case PTFeedSettings:
				field = "settings_id";
				table = "feeds_settings";
				break;

			case PTEnclosure:
				field = "enclosure_id";
				table = "enclosures";
				break;

			case PTMRSSEntry:
				field = "mrss_id";
				table = "mrss";
				break;

			case PTMRSSThumbnail:
				field = "mrss_thumb_id";
				table = "mrss_thumbnails";
				break;

			case PTMRSSCredit:
				field = "mrss_credits_id";
				table = "mrss_credits";
				break;

			case PTMRSSComment:
				field = "mrss_comment_id";
				table = "mrss_comments";
				break;

			case PTMRSSPeerLink:
				field = "mrss_peerlink_id";
				table = "mrss_peerlinks";
				break;

			case PTMRSSScene:
				field = "mrss_scene_id";
				table = "mrss_scenes";
				break;

			default:
				qWarning () << Q_FUNC_INFO
						<< "supplied unknown type"
						<< type;
				return 0;
		}

		return GetHighestID (field, table);
	}

	IDType_t SQLStorageBackend::GetHighestID (const QString& idName, const QString& tableName) const
	{
		QSqlQuery findHighestID (DB_);
		//due to some strange troubles with QSqlQuery::bindValue ()
		//we'll bind values by ourselves. It should be safe as this is our
		//internal function.
		if (!findHighestID.exec (QString ("SELECT MAX (%1) FROM %2")
						.arg (idName).arg (tableName)))
		{
			Util::DBLock::DumpError (findHighestID);
			return 0;
		}

		if (findHighestID.first ())
			return findHighestID.value (0).toInt ();
		else
			return 0;
	}

	Feed_ptr SQLStorageBackend::GetFeed (const IDType_t& feedId) const
	{
		FeedGetter_.bindValue (":feed_id", feedId);
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

	IDType_t SQLStorageBackend::FindFeed (const QString& url) const
	{
		FeedFinderByURL_.bindValue (":url", url);
		if (!FeedFinderByURL_.exec ())
		{
			Util::DBLock::DumpError (FeedFinderByURL_);
			throw FeedGettingError ();
		}

		if (!FeedFinderByURL_.next ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no feed for"
					<< url;
			return -1;
		}

		IDType_t id = FeedFinderByURL_.value (0).value<IDType_t> ();
		FeedFinderByURL_.finish ();
		return id;
	}

	Feed::FeedSettings SQLStorageBackend::GetFeedSettings (const IDType_t& feedId) const
	{
		FeedSettingsGetter_.bindValue (":feed_id", feedId);
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

	void SQLStorageBackend::SetFeedSettings (const Feed::FeedSettings& settings)
	{
		FeedSettingsSetter_.bindValue (":settings_id",
				settings.SettingsID_);
		FeedSettingsSetter_.bindValue (":feed_id",
				settings.FeedID_);
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

	void SQLStorageBackend::GetChannels (channels_shorts_t& shorts, const IDType_t& feedId) const
	{
		ChannelsShortSelector_.bindValue (":feed_id", feedId);
		if (!ChannelsShortSelector_.exec ())
		{
			LeechCraft::Util::DBLock::DumpError (ChannelsShortSelector_);
			return;
		}

		while (ChannelsShortSelector_.next ())
		{
			int unread = 0;

			IDType_t id = ChannelsShortSelector_.value (0).value<IDType_t> ();

			UnreadItemsCounter_.bindValue (":channel_id", id);
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

	Channel_ptr SQLStorageBackend::GetChannel (const IDType_t& channelId,
			const IDType_t& parentFeed) const
	{
		ChannelsFullSelector_.bindValue (":channelId", channelId);
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

	IDType_t SQLStorageBackend::FindChannel (const QString& title,
			const QString& link, const IDType_t& feedId) const
	{
		ChannelIDFromTitleURL_.bindValue (":feed_id", feedId);
		ChannelIDFromTitleURL_.bindValue (":title", title);
		ChannelIDFromTitleURL_.bindValue (":url", link);
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

	IDType_t SQLStorageBackend::FindItem (const QString& title,
			const QString& link, const IDType_t& channelId) const
	{
		ItemIDFromTitleURL_.bindValue (":channel_id", channelId);
		ItemIDFromTitleURL_.bindValue (":title", title);
		ItemIDFromTitleURL_.bindValue (":url", link);
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
	void SQLStorageBackend::TrimChannel (const IDType_t& channelId,
			int days, int number)
	{
		ChannelDateTrimmer_.bindValue (":channel_id", channelId);
		ChannelDateTrimmer_.bindValue (":age", days);
		if (!ChannelDateTrimmer_.exec ())
			LeechCraft::Util::DBLock::DumpError (ChannelDateTrimmer_);

		ChannelNumberTrimmer_.bindValue (":channel_id", channelId);
		ChannelNumberTrimmer_.bindValue (":number", number);

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

	void SQLStorageBackend::GetItems (items_shorts_t& shorts,
			const IDType_t& channelId) const
	{
		ItemsShortSelector_.bindValue (":channel_id", channelId);

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

	int SQLStorageBackend::GetUnreadItems (const IDType_t& channelId) const
	{
		int unread = 0;
		UnreadItemsCounter_.bindValue (":channel_id", channelId);
		if (!UnreadItemsCounter_.exec () ||
				!UnreadItemsCounter_.next ())
			Util::DBLock::DumpError (UnreadItemsCounter_);
		else
			unread = UnreadItemsCounter_.value (0).toInt ();

		UnreadItemsCounter_.finish ();
		return unread;
	}

	Item_ptr SQLStorageBackend::GetItem (const IDType_t& itemId) const
	{
		ItemFullSelector_.bindValue (":item_id", itemId);
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

		emit hookItemLoad (Util::DefaultHookProxy_ptr (new Util::DefaultHookProxy), item.get ());

		return item;
	}

	void SQLStorageBackend::GetItems (items_container_t& items,
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

	void SQLStorageBackend::AddFeed (Feed_ptr feed)
	{
		InsertFeed_.bindValue (":feed_id", feed->FeedID_);
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
						_1));
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO << e.what ();
			return;
		}

		InsertFeed_.finish ();
	}

	void SQLStorageBackend::UpdateChannel (Channel_ptr channel)
	{
		ChannelFinder_.bindValue (":channel_id", channel->ChannelID_);
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

		UpdateChannel_.bindValue (":channel_id", channel->ChannelID_);
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

	void SQLStorageBackend::UpdateChannel (const ChannelShort& channel)
	{
		ChannelFinder_.bindValue (":channel_id", channel.ChannelID_);
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

		UpdateShortChannel_.bindValue (":channel_id", channel.ChannelID_);
		UpdateShortChannel_.bindValue (":last_build", channel.LastBuild_);
		UpdateShortChannel_.bindValue (":tags", Core::Instance ().GetProxy ()->GetTagsManager ()->Join (channel.Tags_));

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

	void SQLStorageBackend::UpdateItem (Item_ptr item)
	{
		UpdateItem_.bindValue (":item_id", item->ItemID_);
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

	void SQLStorageBackend::UpdateItem (const ItemShort& item)
	{
		UpdateShortItem_.bindValue (":item_id", item.ItemID_);
		UpdateShortItem_.bindValue (":unread", item.Unread_);

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

		if (!UpdateShortItem_.numRowsAffected ())
			qWarning () << Q_FUNC_INFO
				<< "no rows affected by UpdateShortItem_";

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

	void SQLStorageBackend::AddChannel (Channel_ptr channel)
	{
		InsertChannel_.bindValue (":channel_id", channel->ChannelID_);
		InsertChannel_.bindValue (":feed_id", channel->FeedID_);
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
				boost::bind (&SQLStorageBackend::AddItem,
					this,
					_1));
	}

	void SQLStorageBackend::AddItem (Item_ptr item)
	{
		InsertItem_.bindValue (":item_id", item->ItemID_);
		InsertItem_.bindValue (":channel_id", item->ChannelID_);
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
			query.bindValue (":item_id", itemId);
			if (!query.exec ())
			{
				LeechCraft::Util::DBLock::DumpError (query);
				return false;
			}

			query.finish ();

			return true;
		}
	}

	void SQLStorageBackend::RemoveItem (const IDType_t& itemId)
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

		RemoveItem_.bindValue (":item_id", itemId);

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

	void SQLStorageBackend::RemoveChannel (const IDType_t& channelId)
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

		RemoveChannel_.bindValue (":channel_id", channelId);
		if (!RemoveChannel_.exec ())
		{
			Util::DBLock::DumpError (RemoveChannel_);
			return;
		}

		RemoveChannel_.finish ();

		lock.Good ();
	}

	void SQLStorageBackend::RemoveFeed (const IDType_t& feedId)
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

		RemoveFeed_.bindValue (":feed_id", feedId);
		if (!RemoveFeed_.exec ())
		{
			Util::DBLock::DumpError (RemoveFeed_);
			return;
		}

		RemoveFeed_.finish ();

		lock.Good ();
	}

	void SQLStorageBackend::ToggleChannelUnread (const IDType_t& channelId,
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
			default:
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
			default:
				return "BLOB";
		}
	}

	bool SQLStorageBackend::InitializeTables ()
	{
		QSqlQuery query (DB_);
		if (!DB_.tables ().contains ("feeds"))
		{
			if (!query.exec ("CREATE TABLE feeds ("
					"feed_id BIGINT PRIMARY KEY, "
					"url TEXT UNIQUE NOT NULL, "
					"last_update TIMESTAMP "
					");"))
			{
				Util::DBLock::DumpError (query);
				return false;
			}
		}

		if (!DB_.tables ().contains ("feeds_settings"))
		{
			if (!query.exec (QString ("CREATE TABLE feeds_settings ("
							"settings_id BIGINT PRIMARY KEY, "
							"feed_id BIGINT UNIQUE REFERENCES feeds ON DELETE CASCADE, "
							"update_timeout INTEGER NOT NULL, "
							"num_items INTEGER NOT NULL, "
							"item_age INTEGER NOT NULL, "
							"auto_download_enclosures %1 NOT NULL"
							");").arg (GetBoolType ())))
			{
				Util::DBLock::DumpError (query);
				return false;
			}

			if (Type_ == SBPostgres)
			{
				if (!query.exec ("CREATE RULE \"replace_feeds_settings\" AS "
									"ON INSERT TO \"feeds_settings\" "
									"WHERE "
										"EXISTS (SELECT 1 FROM feeds_settings "
											"WHERE feed_id = NEW.feed_id) "
									"DO INSTEAD "
										"(UPDATE feeds_settings "
											"SET settings_id = NEW.settings_id, "
											"update_timeout = NEW.update_timeout, "
											"num_items = NEW.num_items, "
											"item_age = NEW.item_age, "
											"auto_download_enclosures = NEW.auto_download_enclosures "
											"WHERE feed_id = NEW.feed_id)"))
				{
					Util::DBLock::DumpError (query);
					return false;
				}
			}
		}

		if (!DB_.tables ().contains ("channels"))
		{
			if (!query.exec (QString ("CREATE TABLE channels ("
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
					"pixmap %1, "
					"favicon %1 "
					");").arg (GetBlobType ())))
			{
				LeechCraft::Util::DBLock::DumpError (query);
				return false;
			}
		}

		if (!DB_.tables ().contains ("items"))
		{
			if (!query.exec (QString ("CREATE TABLE items ("
					"item_id BIGINT PRIMARY KEY, "
					"channel_id BIGINT NOT NULL REFERENCES channels ON DELETE CASCADE, "
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

			if (Type_ == SBPostgres)
			{
				if (!query.exec ("CREATE RULE \"replace_enclosures\" AS "
									"ON INSERT TO \"enclosures\" "
									"WHERE "
										"EXISTS (SELECT 1 FROM enclosures "
											"WHERE item_id = NEW.item_id) "
									"DO INSTEAD "
										"(UPDATE enclosures "
											"SET type = NEW.type, "
											"length = NEW.length, "
											"lang = NEW.lang "
											"WHERE item_id = NEW.item_id)"))
				{
					Util::DBLock::DumpError (query);
					return false;
				}
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

			if (Type_ == SBPostgres)
			{
				if (!query.exec ("CREATE RULE \"replace_mrss\" AS "
									"ON INSERT TO \"mrss\" "
									"WHERE "
										"EXISTS (SELECT 1 FROM mrss "
											"WHERE mrss_id = NEW.mrss_id) "
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
											"WHERE mrss_id = NEW.mrss_id)"))
				{
					Util::DBLock::DumpError (query);
					return false;
				}
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

			if (Type_ == SBPostgres)
			{
				if (!query.exec ("CREATE RULE \"replace_mrss_thumbnails\" AS "
									"ON INSERT TO \"mrss_thumbnails\" "
									"WHERE "
										"EXISTS (SELECT 1 FROM mrss_thumbnails "
											"WHERE mrss_thumb_id = NEW.mrss_thumb_id) "
									"DO INSTEAD "
										"(UPDATE mrss_thumbnails "
											"SET width = NEW.width, "
											"height = NEW.height, "
											"time = NEW.time "
											"WHERE mrss_thumb_id = NEW.mrss_thumb_id)"))
				{
					Util::DBLock::DumpError (query);
					return false;
				}
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

			if (Type_ == SBPostgres)
			{
				if (!query.exec ("CREATE RULE \"replace_mrss_credits\" AS "
									"ON INSERT TO \"mrss_credits\" "
									"WHERE "
										"EXISTS (SELECT 1 FROM mrss_credits "
											"WHERE mrss_credits_id = NEW.mrss_credits_id) "
									"DO INSTEAD "
										"(UPDATE mrss_credits "
											"SET role = NEW.role, "
											"who = NEW.who "
											"WHERE mrss_credits_id = NEW.mrss_credits_id)"))
				{
					Util::DBLock::DumpError (query);
					return false;
				}
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

			if (Type_ == SBPostgres)
			{
				if (!query.exec ("CREATE RULE \"replace_mrss_comments\" AS "
									"ON INSERT TO \"mrss_comments\" "
									"WHERE "
										"EXISTS (SELECT 1 FROM mrss_comments "
											"WHERE mrss_comment_id = NEW.mrss_comment_id) "
									"DO INSTEAD "
										"(UPDATE mrss_comments "
											"SET type = NEW.type, "
											"comment = NEW.comment "
											"WHERE mrss_comment_id = NEW.mrss_comment_id)"))
				{
					Util::DBLock::DumpError (query);
					return false;
				}
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

			if (Type_ == SBPostgres)
			{
				if (!query.exec ("CREATE RULE \"replace_mrss_peerlinks\" AS "
									"ON INSERT TO \"mrss_peerlinks\" "
									"WHERE "
										"EXISTS (SELECT 1 FROM mrss_peerlinks "
											"WHERE mrss_peerlink_id = NEW.mrss_peerlink_id) "
									"DO INSTEAD "
										"(UPDATE mrss_peerlinks "
											"SET type = NEW.type, "
											"link = NEW.link "
											"WHERE mrss_peerlink_id = NEW.mrss_peerlink_id)"))
				{
					Util::DBLock::DumpError (query);
					return false;
				}
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

			if (Type_ == SBPostgres)
			{
				if (!query.exec ("CREATE RULE \"replace_mrss_scenes\" AS "
									"ON INSERT TO \"mrss_scenes\" "
									"WHERE "
										"EXISTS (SELECT 1 FROM mrss_scenes "
											"WHERE mrss_scene_id = NEW.mrss_scene_id) "
									"DO INSTEAD "
										"(UPDATE mrss_scenes "
											"SET title = NEW.title, "
											"description = NEW.description, "
											"start_time = NEW.start_time, "
											"end_time = NEW.end_time "
											"WHERE mrss_scene_id = NEW.mrss_scene_id)"))
				{
					Util::DBLock::DumpError (query);
					return false;
				}
			}
		}

		if (!DB_.tables ().contains ("items2tags"))
		{
			if (!query.exec ("CREATE TABLE items2tags ("
						"item_id BIGINT NOT NULL, "
						"tag TEXT NOT NULL"
						");"))
			{
				Util::DBLock::DumpError (query.lastError ());
				return false;
			}
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
		Util::DBLock lock (DB_);
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
				case SBMysql:
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
		}

		lock.Good ();
		return true;
	}

	void SQLStorageBackend::RemoveTables ()
	{
		if (Type_ == SBSQLite)
		{
			QDir dir = QDir::home ();
			dir.cd (".leechcraft");
			dir.cd ("aggregator");
			if (!dir.rename ("aggregator.db", "aggregator.db.version5backup"))
			{
				qWarning () << Q_FUNC_INFO
						<< "could not rename old file";
				throw std::runtime_error ("Could not rename old file");
			}
			DB_.setDatabaseName (dir.filePath ("aggregator.db"));
			if (!DB_.open ())
			{
				qWarning () << Q_FUNC_INFO;
				Util::DBLock::DumpError (DB_.lastError ());
				throw std::runtime_error (qPrintable (QString ("Could not re-initialize database: %1")
							.arg (DB_.lastError ().text ())));
			}
			return;
		}

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

		if (Type_ == SBPostgres)
		{
			rd ("DROP RULE replace_mrss_thumbnails ON mrss_thumbnails;");
			rd ("DROP RULE replace_mrss_credits ON mrss_credits;");
			rd ("DROP RULE replace_mrss ON mrss;");
			rd ("DROP RULE replace_feeds_settings ON feeds_settings;");
			rd ("DROP RULE replace_enclosures ON enclosures;");
		}

		rd ("DROP TABLE "
			"channels, enclosures, feeds, "
			"feeds_settings, items, mrss, "
			"mrss_comments, mrss_credits, "
			"mrss_peerlinks, mrss_scenes, "
			"mrss_thumbnails");
	}

	Feed::FeedSettings SQLStorageBackend::GetFeedSettingsFromVersion5 (Feed_ptr feed) const
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

	QList<Feed_ptr> SQLStorageBackend::LoadFeedsFromVersion5 () const
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

	QList<Feed_ptr> SQLStorageBackend::GetFeedsFromVersion5 () const
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

	QList<Channel_ptr> SQLStorageBackend::GetChannelsFromVersion5 (const QString& feedUrl,
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

	QList<Item_ptr> SQLStorageBackend::GetItemsFromVersion5 (const QString& hash,
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
				"WHERE parents_hash = :parents_hash "
				"ORDER BY pub_date DESC");
		itemsSelector.bindValue (":parents_hash", hash);

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

	void SQLStorageBackend::FillItemVersion5 (const QSqlQuery& query, Item_ptr& item) const
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

	void SQLStorageBackend::GetEnclosuresVersion5 (const QString& hash, const QString& title,
			const QString& link, QList<Enclosure>& enclosures, const IDType_t& itemId) const
	{
		QSqlQuery getter = QSqlQuery (DB_);
		getter.prepare ("SELECT "
				"url, "
				"type, "
				"length, "
				"lang "
				"FROM enclosures "
				"WHERE item_parents_hash = :item_parents_hash "
				"AND item_title = :item_title "
				"AND item_url = :item_url "
				"ORDER BY url");
		getter.bindValue (":item_parents_hash", hash);
		getter.bindValue (":item_title", title);
		getter.bindValue (":item_url", link);

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

	void SQLStorageBackend::GetMRSSEntriesVersion5 (const QString& hash, const QString& title,
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
				"WHERE item_parents_hash = :item_parents_hash "
				"AND item_title = :item_title "
				"AND item_url = :item_url "
				"ORDER BY title");
		getMediaRSSs.bindValue (":item_parents_hash", hash);
		getMediaRSSs.bindValue (":item_title", title);
		getMediaRSSs.bindValue (":item_url", link);

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
					"WHERE parent_url = :parent_url "
					"AND item_parents_hash = :item_parents_hash "
					"AND item_title = :item_title "
					"AND item_url = :item_url "
					"ORDER BY time");
			getMediaRSSThumbnails.bindValue (":parent_url", eUrl);
			getMediaRSSThumbnails.bindValue (":item_parents_hash", hash);
			getMediaRSSThumbnails.bindValue (":item_title", title);
			getMediaRSSThumbnails.bindValue (":item_url", link);
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
					"WHERE parent_url = :parent_url "
					"AND item_parents_hash = :item_parents_hash "
					"AND item_title = :item_title "
					"AND item_url = :item_url "
					"ORDER BY role");
			getMediaRSSCredits.bindValue (":parent_url", eUrl);
			getMediaRSSCredits.bindValue (":item_parents_hash", hash);
			getMediaRSSCredits.bindValue (":item_title", title);
			getMediaRSSCredits.bindValue (":item_url", link);
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
					"WHERE parent_url = :parent_url "
					"AND item_parents_hash = :item_parents_hash "
					"AND item_title = :item_title "
					"AND item_url = :item_url "
					"ORDER BY comment");
			getMediaRSSComments.bindValue (":parent_url", eUrl);
			getMediaRSSComments.bindValue (":item_parents_hash", hash);
			getMediaRSSComments.bindValue (":item_title", title);
			getMediaRSSComments.bindValue (":item_url", link);
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
					"WHERE parent_url = :parent_url "
					"AND item_parents_hash = :item_parents_hash "
					"AND item_title = :item_title "
					"AND item_url = :item_url "
					"ORDER BY link");
			getMediaRSSPeerLinks.bindValue (":parent_url", eUrl);
			getMediaRSSPeerLinks.bindValue (":item_parents_hash", hash);
			getMediaRSSPeerLinks.bindValue (":item_title", title);
			getMediaRSSPeerLinks.bindValue (":item_url", link);
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
					"WHERE parent_url = :parent_url "
					"AND item_parents_hash = :item_parents_hash "
					"AND item_title = :item_title "
					"AND item_url = :item_url "
					"ORDER BY start_time");
			getMediaRSSScenes.bindValue (":parent_url", eUrl);
			getMediaRSSScenes.bindValue (":item_parents_hash", hash);
			getMediaRSSScenes.bindValue (":item_title", title);
			getMediaRSSScenes.bindValue (":item_url", link);
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

	IDType_t SQLStorageBackend::FindParentFeedForChannel (const IDType_t& channel) const
	{
		QSqlQuery query (DB_);
		query.prepare ("SELECT feed_id FROM channels WHERE channel_id = :channel");
		query.bindValue (":channel", channel);
		if (!query.exec ())
		{
			Util::DBLock::DumpError (query);
			throw std::runtime_error ("Unable to find parent feed for channel");
		}

		if (!query.next ())
			throw ChannelNotFoundError ();

		return query.value (0).value<IDType_t> ();
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
		item->ChannelID_ = query.value (13).toString ().toDouble ();
	}

	void SQLStorageBackend::WriteEnclosures (const QList<Enclosure>& enclosures)
	{
		for (QList<Enclosure>::const_iterator i = enclosures.begin (),
				end = enclosures.end (); i != end; ++i)
		{
			WriteEnclosure_.bindValue (":item_id", i->ItemID_);
			WriteEnclosure_.bindValue (":enclosure_id", i->EnclosureID_);
			WriteEnclosure_.bindValue (":url", i->URL_);
			WriteEnclosure_.bindValue (":type", i->Type_);
			WriteEnclosure_.bindValue (":length", i->Length_);
			WriteEnclosure_.bindValue (":lang", i->Lang_);

			if (!WriteEnclosure_.exec ())
				LeechCraft::Util::DBLock::DumpError (WriteEnclosure_);
		}

		WriteEnclosure_.finish ();
	}

	void SQLStorageBackend::GetEnclosures (const IDType_t& itemId,
			QList<Enclosure>& enclosures) const
	{
		GetEnclosures_.bindValue (":item_id", itemId);

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

	void SQLStorageBackend::WriteMRSSEntries (const QList<MRSSEntry>& entries)
	{
		Q_FOREACH (MRSSEntry e, entries)
		{
			WriteMediaRSS_.bindValue (":mrss_id", e.MRSSEntryID_);
			WriteMediaRSS_.bindValue (":item_id", e.ItemID_);
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

			if (!WriteMediaRSS_.exec ())
			{
				Util::DBLock::DumpError (WriteMediaRSS_);
				continue;
			}

			WriteMediaRSS_.finish ();

			Q_FOREACH (MRSSThumbnail t, e.Thumbnails_)
			{
				WriteMediaRSSThumbnail_.bindValue (":mrss_thumb_id", t.MRSSThumbnailID_);
				WriteMediaRSSThumbnail_.bindValue (":mrss_id", t.MRSSEntryID_);
				WriteMediaRSSThumbnail_.bindValue (":url", t.URL_);
				WriteMediaRSSThumbnail_.bindValue (":width", t.Width_);
				WriteMediaRSSThumbnail_.bindValue (":height", t.Height_);
				WriteMediaRSSThumbnail_.bindValue (":time", t.Time_);

				if (!WriteMediaRSSThumbnail_.exec ())
					Util::DBLock::DumpError (WriteMediaRSSThumbnail_);

				WriteMediaRSSThumbnail_.finish ();
			}

			Q_FOREACH (MRSSCredit c, e.Credits_)
			{
				WriteMediaRSSCredit_.bindValue (":mrss_credits_id", c.MRSSCreditID_);
				WriteMediaRSSCredit_.bindValue (":mrss_id", c.MRSSEntryID_);
				WriteMediaRSSCredit_.bindValue (":role", c.Role_);
				WriteMediaRSSCredit_.bindValue (":who", c.Who_);

				if (!WriteMediaRSSCredit_.exec ())
					Util::DBLock::DumpError (WriteMediaRSSCredit_);

				WriteMediaRSSCredit_.finish ();
			}

			Q_FOREACH (MRSSComment c, e.Comments_)
			{
				WriteMediaRSSComment_.bindValue (":mrss_comment_id", c.MRSSCommentID_);
				WriteMediaRSSComment_.bindValue (":mrss_id", c.MRSSEntryID_);
				WriteMediaRSSComment_.bindValue (":type", c.Type_);
				WriteMediaRSSComment_.bindValue (":comment", c.Comment_);

				if (!WriteMediaRSSComment_.exec ())
					Util::DBLock::DumpError (WriteMediaRSSComment_);

				WriteMediaRSSComment_.finish ();
			}

			Q_FOREACH (MRSSPeerLink p, e.PeerLinks_)
			{
				WriteMediaRSSPeerLink_.bindValue (":mrss_peerlink_id", p.MRSSPeerLinkID_);
				WriteMediaRSSPeerLink_.bindValue (":mrss_id", p.MRSSEntryID_);
				WriteMediaRSSPeerLink_.bindValue (":type", p.Type_);
				WriteMediaRSSPeerLink_.bindValue (":link", p.Link_);

				if (!WriteMediaRSSPeerLink_.exec ())
					Util::DBLock::DumpError (WriteMediaRSSPeerLink_);

				WriteMediaRSSPeerLink_.finish ();
			}

			Q_FOREACH (MRSSScene s, e.Scenes_)
			{
				WriteMediaRSSScene_.bindValue (":mrss_scene_id", s.MRSSSceneID_);
				WriteMediaRSSScene_.bindValue (":mrss_id", s.MRSSEntryID_);
				WriteMediaRSSScene_.bindValue (":title", s.Title_);
				WriteMediaRSSScene_.bindValue (":description", s.Description_);
				WriteMediaRSSScene_.bindValue (":start_time", s.StartTime_);
				WriteMediaRSSScene_.bindValue (":end_time", s.EndTime_);

				if (!WriteMediaRSSScene_.exec ())
					Util::DBLock::DumpError (WriteMediaRSSScene_);

				WriteMediaRSSScene_.finish ();
			}
		}
	}

	void SQLStorageBackend::GetMRSSEntries (const IDType_t& itemId, QList<MRSSEntry>& entries) const
	{
		GetMediaRSSs_.bindValue (":item_id", itemId);

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

			GetMediaRSSThumbnails_.bindValue (":mrss_id", mrssId);
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

			GetMediaRSSCredits_.bindValue (":mrss_id", mrssId);
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

			GetMediaRSSComments_.bindValue (":mrss_id", mrssId);
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

			GetMediaRSSPeerLinks_.bindValue (":mrss_id", mrssId);
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

			GetMediaRSSScenes_.bindValue (":mrss_id", mrssId);
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
}
}
