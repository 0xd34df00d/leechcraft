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

using namespace LeechCraft::Plugins::Aggregator;

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
		LeechCraft::Util::DBLock::DumpError (DB_.lastError ());
		throw std::runtime_error ("Could not initialize database");
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
	DB_.close ();
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
			"item_age "
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
			"item_age"
			") VALUES ("
			":feed_url, "
			":update_timeout, "
			":num_items, "
			":item_age"
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
			"comments_page_url "
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
			"comments_page_url "
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
			"comments_page_url"
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
			":comments_page_url"
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
			"comments_page_url = :comments_page_url "
			"WHERE parents_hash = :parents_hash "
			"AND title = :title "
			"AND url = :url");

	ToggleChannelUnread_ = QSqlQuery (DB_);
	ToggleChannelUnread_.prepare ("UPDATE items SET "
			"unread = :unread "
			"WHERE parents_hash = :parents_hash");


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

	FeedSettingsGetter_.finish ();

	return result;
}

void SQLStorageBackend::SetFeedSettings (const QString& feedURL,
		const Feed::FeedSettings& settings)
{
	FeedSettingsSetter_.bindValue (":feed_url", feedURL);
	FeedSettingsSetter_.bindValue (":update_timeout", settings.UpdateTimeout_);
	FeedSettingsSetter_.bindValue (":num_items", settings.NumItems_);
	FeedSettingsSetter_.bindValue (":item_age", settings.ItemAge_);

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

	while (ItemsFullSelector_.next  ())
	{
		Item_ptr item (new Item);
		FillItem (ItemsFullSelector_, item);
		GetEnclosures (hash, item->Title_, item->Link_, item->Enclosures_);

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
		LeechCraft::Util::DBLock::DumpError (ChannelFinder_);
		throw std::runtime_error ("Unable to execute channel finder query");
	}
	ChannelFinder_.next ();
	if (!ChannelFinder_.isValid ())
	{
		AddChannel (channel, parent);
		return;
	}
	ChannelFinder_.finish ();

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
		LeechCraft::Util::DBLock::DumpError (UpdateChannel_);
		throw std::runtime_error ("failed to save channel");
	}

	UpdateChannel_.finish ();
	lock.Good ();

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
		LeechCraft::Util::DBLock::DumpError (ChannelFinder_);
		throw std::runtime_error ("Unable to execute channel finder query");
	}
	ChannelFinder_.next ();
	if (!ChannelFinder_.isValid ())
		throw std::runtime_error ("Selected channel for updating "
				"doesn't exist and we don't have enough info to "
				"insert it.");
	ChannelFinder_.finish ();

	UpdateShortChannel_.bindValue (":parent_feed_url", parent);
	UpdateShortChannel_.bindValue (":url", channel.Link_);
	UpdateShortChannel_.bindValue (":title", channel.Title_);
	UpdateShortChannel_.bindValue (":last_build", channel.LastBuild_);
	UpdateShortChannel_.bindValue (":tags",
			Core::Instance ().GetProxy ()->GetTagsManager ()->Join (channel.Tags_));

	if (!UpdateShortChannel_.exec ())
	{
		LeechCraft::Util::DBLock::DumpError (UpdateShortChannel_);
		throw std::runtime_error ("failed to save channel");
	}

	UpdateShortChannel_.finish ();

	emit channelDataUpdated (GetChannel (channel.Title_, parent));
}

void SQLStorageBackend::UpdateItem (Item_ptr item,
		const QString& parentUrl, const QString& parentTitle)
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

	if (!UpdateItem_.exec ())
	{
		LeechCraft::Util::DBLock::DumpError (UpdateItem_);
		throw std::runtime_error ("failed to save item");
	}

	lock.Good ();

	UpdateItem_.finish ();

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
		LeechCraft::Util::DBLock::DumpError (ItemFinder_);
		throw std::runtime_error ("Unable to execute item finder query");
	}
	ItemFinder_.next ();
	if (!ItemFinder_.isValid ())
		throw std::runtime_error ("Specified item doesn't exist and we "
				"couldn't add it because there isn't enough info");
	ItemFinder_.finish ();

	UpdateShortItem_.bindValue (":parents_hash", parentUrl + parentTitle);
	UpdateShortItem_.bindValue (":unread", item.Unread_);
	UpdateShortItem_.bindValue (":title", item.Title_);
	UpdateShortItem_.bindValue (":url", item.URL_);

	if (!UpdateShortItem_.exec ())
	{
		LeechCraft::Util::DBLock::DumpError (UpdateShortItem_);
		throw std::runtime_error ("failed to save item");
	}

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
		LeechCraft::Util::DBLock::DumpError (InsertChannel_);
		throw std::runtime_error ("failed to save channel");
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

	if (!InsertItem_.exec ())
	{
		LeechCraft::Util::DBLock::DumpError (InsertItem_);
		throw std::runtime_error ("failed to save item");
	}

	for (QList<Enclosure>::const_iterator i = item->Enclosures_.begin (),
			end = item->Enclosures_.end (); i != end; ++i)
	{
		WriteEnclosure_.bindValue (":url", i->URL_);
		WriteEnclosure_.bindValue (":type", i->Type_);
		WriteEnclosure_.bindValue (":length", i->Length_);
		WriteEnclosure_.bindValue (":lang", i->Lang_);
		WriteEnclosure_.bindValue (":item_parents_hash", parentUrl + parentTitle);
		WriteEnclosure_.bindValue (":item_title", item->Title_);
		WriteEnclosure_.bindValue (":item_url", item->Link_);

		if (!WriteEnclosure_.exec ())
			LeechCraft::Util::DBLock::DumpError (WriteEnclosure_);
	}

	WriteEnclosure_.finish ();

	InsertItem_.finish ();

	Channel_ptr channel = GetChannel (parentTitle, parentUrl);
	emit itemDataUpdated (item, channel);
	emit channelDataUpdated (channel);
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
	RemoveEnclosures_.bindValue (":item_parents_hash", hash);
	RemoveEnclosures_.bindValue (":item_title", item->Title_);
	RemoveEnclosures_.bindValue (":item_url", item->Link_);
	if (!RemoveEnclosures_.exec ())
	{
		LeechCraft::Util::DBLock::DumpError (RemoveItem_);
		return;
	}

	RemoveEnclosures_.finish ();

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
	ToggleChannelUnread_.bindValue (":parents_hash", purl + title);
	ToggleChannelUnread_.bindValue (":unread", state);

	if (!ToggleChannelUnread_.exec ())
	{
		LeechCraft::Util::DBLock::DumpError (ToggleChannelUnread_);
		throw std::runtime_error ("failed to toggle item");
	}

	ToggleChannelUnread_.finish ();

	emit channelDataUpdated (GetChannel (title, purl));
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
		if (!query.exec ("CREATE TABLE feeds_settings ("
					"feed_url TEXT PRIMARY KEY, "
					"update_timeout INTEGER NOT NULL, "
					"num_items INTEGER NOT NULL, "
					"item_age INTEGER NOT NULL"
					");"))
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
										"item_age = NEW.item_age "
										"WHERE feed_url = NEW.feed_url)"))
			{
				LeechCraft::Util::DBLock::DumpError (query);
				return false;
			}
		}
	}

	if (!DB_.tables ().contains ("channels"))
	{
		QString blob;
		switch (Type_)
		{
			case SBSQLite:
				blob = "BLOB";
				break;
			case SBPostgres:
				blob = "BYTEA";
				break;
		}
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
				");").arg (blob)))
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
		QString unreadType;
		switch (Type_)
		{
			case SBSQLite:
				unreadType = "TINYINT";
				break;
			case SBPostgres:
				unreadType = "BOOLEAN";
				break;
		}
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
				"comments_page_url TEXT"
				");").arg (unreadType)))
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


