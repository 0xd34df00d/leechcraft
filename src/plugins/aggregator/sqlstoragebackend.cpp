#include "sqlstoragebackend.h"
#include <stdexcept>
#include <boost/bind.hpp>
#include <QDir>
#include <QDebug>
#include <QBuffer>
#include <QSqlError>
#include <QVariant>

SQLStorageBackend::DBLock::DBLock (QSqlDatabase& database)
: Database_ (database)
, Good_ (false)
, Initialized_ (false)
{
}

SQLStorageBackend::DBLock::~DBLock ()
{
	if (!Initialized_)
		return;

	if (Good_ ? !Database_.commit () : !Database_.rollback ())
		DumpError (Database_.lastError ());
}

void SQLStorageBackend::DBLock::Init ()
{
	if (!Database_.transaction ())
	{
		DumpError (Database_.lastError ());
		throw std::runtime_error ("Could not start transaction");
	}
	Initialized_ = true;
}

void SQLStorageBackend::DBLock::Good ()
{
	Good_ = true;
}

void SQLStorageBackend::DBLock::DumpError (const QSqlError& lastError)
{
	qWarning () << lastError.text () << "|"
		<< lastError.databaseText () << "|"
		<< lastError.driverText () << "|"
		<< lastError.type () << "|"
		<< lastError.number ();
}

void SQLStorageBackend::DBLock::DumpError (const QSqlQuery& lastQuery)
{
	qWarning () << lastQuery.lastQuery ();
	DumpError (lastQuery.lastError ());
}

SQLStorageBackend::SQLStorageBackend ()
: DB_ (QSqlDatabase::addDatabase ("QSQLITE"))
{
	QDir dir = QDir::home ();
	dir.cd (".leechcraft");

	DB_.setDatabaseName (dir.filePath ("aggregator.db"));
	if (!DB_.open ())
		DBLock::DumpError (DB_.lastError ());

	if (!DB_.tables ().contains ("feeds"))
		InitializeTables ();
}

void SQLStorageBackend::Prepare ()
{
	QSqlQuery pragma;
	if (!pragma.exec ("PRAGMA cache_size = 6000;"))
		DBLock::DumpError (pragma);
	if (!pragma.exec ("PRAGMA journal_mode = TRUNCATE;"))
		DBLock::DumpError (pragma);
	if (!pragma.exec ("PRAGMA synchronous = OFF;"))
		DBLock::DumpError (pragma);
	if (!pragma.exec ("PRAGMA temp_store = MEMORY;"))
		DBLock::DumpError (pragma);

	FeedFinderByURL_ = QSqlQuery ();
	FeedFinderByURL_.prepare ("SELECT last_update "
			"FROM feeds "
			"WHERE url = :url");

	ChannelsShortSelector_ = QSqlQuery ();
	ChannelsShortSelector_.prepare ("SELECT "
			"title, "
			"url, "
			"tags, "
			"last_build,"
			"favicon "
			"FROM channels "
			"WHERE parent_feed_url = :parent_feed_url "
			"ORDER BY title");

	ChannelsFullSelector_ = QSqlQuery ();
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

	UnreadItemsCounter_ = QSqlQuery ();
	UnreadItemsCounter_.prepare ("SELECT COUNT (unread) "
			"FROM items "
			"WHERE parents_hash = :parents_hash "
			"AND unread = \"true\"");

	ItemsShortSelector_ = QSqlQuery ();
	ItemsShortSelector_.prepare ("SELECT "
			"title, "
			"url, "
			"pub_date, "
			"unread "
			"FROM items "
			"WHERE parents_hash = :parents_hash "
			"ORDER BY pub_date DESC");

	ItemsFullSelector_ = QSqlQuery ();
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
			"comments_url "
			"FROM items "
			"WHERE parents_hash = :parents_hash "
			"AND title = :title "
			"AND url = :url "
			"ORDER BY pub_date DESC");

	ChannelFinder_ = QSqlQuery ();
	ChannelFinder_.prepare ("SELECT description "
			"FROM channels "
			"WHERE parent_feed_url = :parent_feed_url "
			"AND title = :title "
			"AND url = :url");

	ItemFinder_ = QSqlQuery ();
	ItemFinder_.prepare ("SELECT title "
			"FROM items "
			"WHERE parents_hash = :parents_hash "
			"AND title = :title "
			"AND url = :url");

	InsertFeed_ = QSqlQuery ();
	InsertFeed_.prepare ("INSERT INTO feeds ("
			"url, "
			"last_update"
			") VALUES ("
			":url, "
			":last_update"
			")");

	InsertChannel_ = QSqlQuery ();
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

	InsertItem_ = QSqlQuery ();
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
			"comments_url"
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
			":comments_url"
			")");

	UpdateShortChannel_ = QSqlQuery ();
	UpdateShortChannel_.prepare ("UPDATE channels SET "
			"tags = :tags, "
			"last_build = :last_build "
			"WHERE parent_feed_url = :parent_feed_url "
			"AND url = :url "
			"AND title = :title");

	UpdateChannel_ = QSqlQuery ();
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

	UpdateShortItem_ = QSqlQuery ();
	UpdateShortItem_.prepare ("UPDATE items SET "
			"unread = :unread "
			"WHERE parents_hash = :parents_hash "
			"AND title = :title "
			"AND url = :url");

	UpdateItem_ = QSqlQuery ();
	UpdateItem_.prepare ("UPDATE items SET "
			"description = :description, "
			"author = :author, "
			"category = :category, "
			"pub_date = :pub_date, "
			"unread = :unread, "
			"num_comments = :num_comments, "
			"comments_url = :comments_url "
			"WHERE parents_hash = :parents_hash "
			"AND title = :title "
			"AND url = :url "
			"AND guid = :guid");

	ToggleChannelUnread_ = QSqlQuery ();
	ToggleChannelUnread_.prepare ("UPDATE items SET "
			"unread = :unread "
			"WHERE parents_hash = :parents_hash");


	RemoveFeed_ = QSqlQuery ();
	RemoveFeed_.prepare ("DELETE FROM feeds "
			"WHERE url = :url");

	RemoveChannel_ = QSqlQuery ();
	RemoveChannel_.prepare ("DELETE FROM channels "
			"WHERE parent_feed_url = :parent_feed_url");

	RemoveItem_ = QSqlQuery ();
	RemoveItem_.prepare ("DELETE FROM items "
			"WHERE parents_hash = :parents_hash "
			"AND title = :title "
			"AND url = :url "
			"AND guid = :guid");
}

SQLStorageBackend::~SQLStorageBackend ()
{
	DB_.close ();
}

void SQLStorageBackend::GetFeedsURLs (feeds_urls_t& result) const
{
	QSqlQuery feedSelector;
	if (!feedSelector.exec ("SELECT url "
				"FROM feeds "
				"ORDER BY url"))
	{
		DBLock::DumpError (feedSelector);
		return;
	}

	while (feedSelector.next ())
		result.push_back (feedSelector.value (0).toString ());
}

void SQLStorageBackend::GetChannels (channels_shorts_t& shorts,
		const QString& feedURL) const
{
	ChannelsShortSelector_.bindValue (":parent_feed_url", feedURL);
	if (!ChannelsShortSelector_.exec ())
	{
		DBLock::DumpError (ChannelsShortSelector_);
		return;
	}

	while (ChannelsShortSelector_.next ())
	{
		int unread = 0;
		QString title = ChannelsShortSelector_.value (0).toString ();

		UnreadItemsCounter_.bindValue (":parents_hash", feedURL + title);
		if (!UnreadItemsCounter_.exec () || !UnreadItemsCounter_.next ())
			DBLock::DumpError (UnreadItemsCounter_);
		else
			unread = UnreadItemsCounter_.value (0).toInt ();

		UnreadItemsCounter_.finish ();

		ChannelShort sh =
		{
			title,
			ChannelsShortSelector_.value (1).toString (),
			ChannelsShortSelector_.value (2).toString ().split (' ',
					QString::SkipEmptyParts),
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
		DBLock::DumpError (ChannelsFullSelector_);
		return Channel_ptr (new Channel);
	}

	Channel_ptr channel (new Channel);

	channel->Link_ = ChannelsFullSelector_.value (0).toString ();
	channel->Title_ = title;
	channel->Description_ = ChannelsFullSelector_.value (1).toString ();
	channel->LastBuild_ = ChannelsFullSelector_.value (2).toDateTime ();
	channel->Tags_ = ChannelsFullSelector_.value (3).toString ().split (' ',
			QString::SkipEmptyParts);
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
		DBLock::DumpError (ItemsShortSelector_);
		return;
	}

	while (ItemsShortSelector_.next ())
	{
		ItemShort sh =
		{
			ItemsShortSelector_.value (0).toString (),
			ItemsShortSelector_.value (1).toString (),
			ItemsShortSelector_.value (2).toDateTime (),
			ItemsShortSelector_.value (3).toBool ()
		};

		shorts.push_back (sh);
	}

	ItemsShortSelector_.finish ();
}

Item_ptr SQLStorageBackend::GetItem (const QString& title,
		const QString& link, const QString& hash) const
{
	ItemsFullSelector_.bindValue (":parents_hash", hash);
	ItemsFullSelector_.bindValue (":title", title);
	ItemsFullSelector_.bindValue (":link", link);
	if (!ItemsFullSelector_.exec () || !ItemsFullSelector_.next ())
	{
		DBLock::DumpError (ItemsFullSelector_);
		return Item_ptr ();
	}

	Item_ptr item (new Item);

	item->Title_ = ItemsFullSelector_.value (0).toString ();
	item->Link_ = ItemsFullSelector_.value (1).toString ();
	item->Description_ = ItemsFullSelector_.value (2).toString ();
	item->Author_ = ItemsFullSelector_.value (3).toString ();
	item->Categories_ = ItemsFullSelector_.value (4).toString ().split ("<<<");
	item->Guid_ = ItemsFullSelector_.value (5).toString ();
	item->PubDate_ = ItemsFullSelector_.value (6).toDateTime ();
	item->Unread_ = ItemsFullSelector_.value (7).toBool ();
	item->NumComments_ = ItemsFullSelector_.value (8).toInt ();
	item->CommentsLink_ = ItemsFullSelector_.value (9).toString ();

	ItemsFullSelector_.finish ();

	return item;
}

void SQLStorageBackend::AddFeed (Feed_ptr feed)
{
	DBLock lock (DB_);
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
		DBLock::DumpError (InsertFeed_);
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
		DBLock::DumpError (ChannelFinder_);
		throw std::runtime_error ("Unable to execute channel finder query");
	}
	ChannelFinder_.next ();
	if (!ChannelFinder_.isValid ())
	{
		AddChannel (channel, parent);
		return;
	}
	ChannelFinder_.finish ();

	UpdateChannel_.bindValue (":parent_feed_url", parent);
	UpdateChannel_.bindValue (":url", channel->Link_);
	UpdateChannel_.bindValue (":title", channel->Title_);
	UpdateChannel_.bindValue (":description", channel->Description_);
	UpdateChannel_.bindValue (":last_build", channel->LastBuild_);
	UpdateChannel_.bindValue (":tags", channel->Tags_.join (" "));
	UpdateChannel_.bindValue (":language", channel->Language_);
	UpdateChannel_.bindValue (":author", channel->Author_);
	UpdateChannel_.bindValue (":pixmap_url", channel->PixmapURL_);
	UpdateChannel_.bindValue (":pixmap", SerializePixmap (channel->Pixmap_));
	UpdateChannel_.bindValue (":favicon", SerializePixmap (channel->Favicon_));

	if (!UpdateChannel_.exec ())
	{
		DBLock::DumpError (UpdateChannel_);
		throw std::runtime_error ("failed to save channel");
	}

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
		DBLock::DumpError (ChannelFinder_);
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
	UpdateShortChannel_.bindValue (":tags", channel.Tags_.join (" "));

	if (!UpdateShortChannel_.exec ())
	{
		DBLock::DumpError (UpdateShortChannel_);
		throw std::runtime_error ("failed to save channel");
	}

	UpdateShortChannel_.finish ();

	emit channelDataUpdated (GetChannel (channel.Title_, parent));
}

void SQLStorageBackend::UpdateItem (Item_ptr item,
		const QString& parentUrl, const QString& parentTitle)
{
	ItemFinder_.bindValue (":parents_hash", parentUrl + parentTitle);
	ItemFinder_.bindValue (":title", item->Title_);
	ItemFinder_.bindValue (":url", item->Link_);
	if (!ItemFinder_.exec ())
	{
		DBLock::DumpError (ItemFinder_);
		throw std::runtime_error ("Unable to execute item finder query");
	}
	ItemFinder_.next ();
	if (!ItemFinder_.isValid ())
	{
		AddItem (item, parentUrl, parentTitle);
		return;
	}
	ItemFinder_.finish ();

	UpdateItem_.bindValue (":parents_hash", parentUrl + parentTitle);
	UpdateItem_.bindValue (":title", item->Title_);
	UpdateItem_.bindValue (":url", item->Link_);
	UpdateItem_.bindValue (":description", item->Description_);
	UpdateItem_.bindValue (":author", item->Author_);
	UpdateItem_.bindValue (":category", item->Categories_.join ("<<<"));
	UpdateItem_.bindValue (":guid", item->Guid_);
	UpdateItem_.bindValue (":pub_date", item->PubDate_);
	UpdateItem_.bindValue (":unread", item->Unread_);
	UpdateItem_.bindValue (":num_comments", item->NumComments_);
	UpdateItem_.bindValue (":comments_url", item->CommentsLink_);

	if (!UpdateItem_.exec ())
	{
		DBLock::DumpError (UpdateItem_);
		throw std::runtime_error ("failed to save item");
	}

	UpdateItem_.finish ();

	emit itemDataUpdated (item);
	emit channelDataUpdated (GetChannel (parentTitle, parentUrl));
}

void SQLStorageBackend::UpdateItem (const ItemShort& item,
		const QString& parentUrl, const QString& parentTitle)
{
	ItemFinder_.bindValue (":parents_hash", parentUrl + parentTitle);
	ItemFinder_.bindValue (":title", item.Title_);
	ItemFinder_.bindValue (":url", item.URL_);
	if (!ItemFinder_.exec ())
	{
		DBLock::DumpError (ItemFinder_);
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
		DBLock::DumpError (UpdateShortItem_);
		throw std::runtime_error ("failed to save item");
	}

	UpdateShortItem_.finish ();

	emit itemDataUpdated (GetItem (item.Title_, item.URL_, parentUrl + parentTitle));
	emit channelDataUpdated (GetChannel (parentTitle, parentUrl));
}

void SQLStorageBackend::AddChannel (Channel_ptr channel, const QString& url)
{
	InsertChannel_.bindValue (":parent_feed_url", url);
	InsertChannel_.bindValue (":url", channel->Link_);
	InsertChannel_.bindValue (":title", channel->Title_);
	InsertChannel_.bindValue (":description", channel->Description_);
	InsertChannel_.bindValue (":last_build", channel->LastBuild_);
	InsertChannel_.bindValue (":tags", channel->Tags_.join (" "));
	InsertChannel_.bindValue (":language", channel->Language_);
	InsertChannel_.bindValue (":author", channel->Author_);
	InsertChannel_.bindValue (":pixmap_url", channel->PixmapURL_);
	InsertChannel_.bindValue (":pixmap", SerializePixmap (channel->Pixmap_));
	InsertChannel_.bindValue (":favicon", SerializePixmap (channel->Favicon_));

	if (!InsertChannel_.exec ())
	{
		DBLock::DumpError (InsertChannel_);
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

	if (!InsertItem_.exec ())
	{
		DBLock::DumpError (InsertItem_);
		throw std::runtime_error ("failed to save item");
	}

	InsertItem_.finish ();

	emit itemDataUpdated (item);
	emit channelDataUpdated (GetChannel (parentTitle, parentUrl));
}

void SQLStorageBackend::RemoveItem (Item_ptr item,
		const QString& hash,
		const QString& parentHash,
		const QString& feedURL)
{
	RemoveItem_.bindValue (":parents_hash", hash);
	RemoveItem_.bindValue (":title", item->Title_);
	RemoveItem_.bindValue (":url", item->Link_);
	RemoveItem_.bindValue (":guid", item->Guid_);

	if (!RemoveItem_.exec ())
	{
		DBLock::DumpError (RemoveItem_);
		return;
	}

	RemoveItem_.finish ();

	emit channelDataUpdated (GetChannel (parentHash, feedURL));
	emit itemDataUpdated (item);
}

void SQLStorageBackend::RemoveFeed (const QString& url)
{
	channels_shorts_t shorts;
	GetChannels (shorts, url);

	DBLock lock (DB_);
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
		QSqlQuery removeItem;
		removeItem.prepare ("DELETE FROM items "
				"WHERE parents_hash = :parents_hash");
		removeItem.bindValue (":parents_hash", url + i->Title_);

		if (!removeItem.exec ())
		{
			DBLock::DumpError (removeItem);
			return;
		}

		RemoveChannel_.bindValue (":parent_feed_url", url);
		if (!RemoveChannel_.exec ())
		{
			DBLock::DumpError (RemoveChannel_);
			return;
		}

		RemoveChannel_.finish ();
	}

	RemoveFeed_.bindValue (":url", url);
	if (!RemoveFeed_.exec ())
	{
		DBLock::DumpError (RemoveFeed_);
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
		DBLock::DumpError (ToggleChannelUnread_);
		throw std::runtime_error ("failed to toggle item");
	}

	ToggleChannelUnread_.finish ();

	emit channelDataUpdated (GetChannel (title, purl));
}

int SQLStorageBackend::GetUnreadItemsNumber () const
{
	QSqlQuery query ("SELECT COUNT (unread) "
			"FROM items "
			"WHERE unread = \"true\"");
	if (!query.exec () || !query.next ())
	{
		DBLock::DumpError (query);
		return 0;
	}

	return query.value (0).toInt ();
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
	QSqlQuery query;
	if (!query.exec ("CREATE TABLE feeds ("
			"url TEXT PRIMARY KEY, "
			"last_update TIMESTAMP "
			");"))
	{
		DBLock::DumpError (query.lastError ());
		return false;
	}

	if (!query.exec ("CREATE TABLE channels ("
			"parent_feed_url TEXT, "
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
			");"))
	{
		DBLock::DumpError (query.lastError ());
		return false;
	}

	if (!query.exec ("CREATE TABLE items ("
			"parents_hash TEXT, "
			"title TEXT, "
			"url TEXT, "
			"description TEXT, "
			"author TEXT, "
			"category TEXT, "
			"guid TEXT, "
			"pub_date TIMESTAMP, "
			"unread TINYINT, "
			"num_comments SMALLINT, "
			"comments_url TEXT "
			");"))
	{
		DBLock::DumpError (query.lastError ());
		return false;
	}

	if (!query.exec ("CREATE UNIQUE INDEX feeds_url ON feeds (url);"))
		DBLock::DumpError (query.lastError ());
	if (!query.exec ("CREATE INDEX channels_parent_url ON channels (parent_feed_url);"))
		DBLock::DumpError (query.lastError ());
	if (!query.exec ("CREATE INDEX items_parents_hash ON items (parents_hash);"))
		DBLock::DumpError (query.lastError ());

	if (!query.exec ("CREATE TABLE item_bucket ("
			"title TEXT, "
			"url TEXT, "
			"description TEXT, "
			"author TEXT, "
			"category TEXT, "
			"guid TEXT, "
			"pub_date TIMESTAMP, "
			"unread TINYINT "
			");"))
	{
		DBLock::DumpError (query.lastError ());
		return false;
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
	DBLock lock (DB_);
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
		QSqlQuery updateQuery = QSqlQuery ();
		if (!updateQuery.exec ("ALTER TABLE items "
				"ADD num_comments SMALLINT"))
		{
			DBLock::DumpError (updateQuery);
			return false;
		}

		if (!updateQuery.exec ("ALTER TABLE items "
					"ADD comments_url TEXT"))
		{
			DBLock::DumpError (updateQuery);
			return false;
		}

		if (!updateQuery.exec ("UPDATE items "
					"SET num_comments = -1"))
		{
			DBLock::DumpError (updateQuery);
			return false;
		}
	}

	lock.Good ();
	return true;
}

