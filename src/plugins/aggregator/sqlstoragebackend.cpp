#include "sqlstoragebackend.h"
#include <stdexcept>
#include <boost/bind.hpp>
#include <QDir>
#include <QDebug>
#include <QBuffer>
#include <QSqlError>
#include <QVariant>

SQLStorageBackend::SQLStorageBackend ()
: DB_ (QSqlDatabase::addDatabase ("QSQLITE"))
{
	QDir dir = QDir::home ();
	dir.cd (".leechcraft");

	DB_.setDatabaseName (dir.filePath ("aggregator.db"));
	if (!DB_.open ())
		DumpError (DB_.lastError ());

	if (!DB_.tables ().contains ("feeds"))
		InitializeTables ();
}

void SQLStorageBackend::Prepare ()
{
	QSqlQuery pragma;
	if (!pragma.exec ("PRAGMA cache_size = 6000;"))
		DumpError (pragma);
	if (!pragma.exec ("PRAGMA journal_mode = TRUNCATE;"))
		DumpError (pragma);
	if (!pragma.exec ("PRAGMA synchronous = OFF;"))
		DumpError (pragma);
	if (!pragma.exec ("PRAGMA temp_store = MEMORY;"))
		DumpError (pragma);

	FeedFinderByURL_ = QSqlQuery ();
	FeedFinderByURL_.prepare ("SELECT last_update "
			"FROM feeds "
			"WHERE url = :url");

	ChannelsShortSelector_ = QSqlQuery ();
	ChannelsShortSelector_.prepare ("SELECT "
			"title, "
			"tags, "
			"last_build "
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
			"WHERE parents_hash = :parents_hash");

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
			"AND url = :url "
			"AND guid = :guid");

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
	if (!feedSelector.exec ("SELECT url FROM feeds"))
	{
		DumpError (feedSelector);
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
		DumpError (ChannelsShortSelector_);
		return;
	}

	while (ChannelsShortSelector_.next ())
	{
		int unread = 0;
		QString title = ChannelsShortSelector_.value (0).toString ();

		UnreadItemsCounter_.bindValue (":parents_hash", feedURL + title);
		if (!UnreadItemsCounter_.exec () || !UnreadItemsCounter_.next ())
			DumpError (UnreadItemsCounter_);
		else
			unread = UnreadItemsCounter_.value (0).toInt ();

		ChannelShort sh =
		{
			title,
			ChannelsShortSelector_.value (1).toString ().split (' ',
					QString::SkipEmptyParts),
			ChannelsShortSelector_.value (2).toDateTime (),
			unread
		};
		shorts.push_back (sh);
	}
}

Channel_ptr SQLStorageBackend::GetChannel (const QString& title,
		const QString& feedParent) const
{
	ChannelsFullSelector_.bindValue (":title", title);
	ChannelsFullSelector_.bindValue (":parent_feed_url", feedParent);
	if (ChannelsFullSelector_.exec () || !ChannelsFullSelector_.next ())
	{
		DumpError (ChannelsFullSelector_);
		return Channel_ptr ();
	}

	Channel_ptr channel (new Channel);

	channel->Link_ = ChannelsFullSelector_.value (0).toString ();
	channel->Title_ = ChannelsFullSelector_.value (1).toString ();
	channel->Description_ = ChannelsFullSelector_.value (2).toString ();
	channel->LastBuild_ = ChannelsFullSelector_.value (3).toDateTime ();
	channel->Tags_ = ChannelsFullSelector_.value (4).toString ().split (' ',
			QString::SkipEmptyParts);
	channel->Language_ = ChannelsFullSelector_.value (5).toString ();
	channel->Author_ = ChannelsFullSelector_.value (6).toString ();
	channel->PixmapURL_ = ChannelsFullSelector_.value (7).toString ();
	channel->Pixmap_ = UnserializePixmap (ChannelsFullSelector_
			.value (8).toByteArray ());
	channel->Favicon_ = UnserializePixmap (ChannelsFullSelector_
			.value (9).toByteArray ());

	return channel;
}

void SQLStorageBackend::GetItems (items_shorts_t& shorts, const QString& parentsHash) const
{
	ItemsShortSelector_.bindValue (":parents_hash", parentsHash);

	if (!ItemsShortSelector_.exec ())
	{
		DumpError (ItemsShortSelector_);
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
}

Item_ptr SQLStorageBackend::GetItem (const QString& title,
		const QString& link, const QString& hash) const
{
	ItemsFullSelector_.bindValue (":parents_hash", hash);
	ItemsFullSelector_.bindValue (":title", title);
	ItemsFullSelector_.bindValue (":link", link);
	if (!ItemsFullSelector_.exec () || !ItemsFullSelector_.next ())
	{
		DumpError (ItemsFullSelector_);
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

	return item;
}

void SQLStorageBackend::AddFeed (Feed_ptr feed)
{
	if (!DB_.transaction ())
	{
		DumpError (DB_.lastError ());
		qWarning () << Q_FUNC_INFO << "failed to start transaction";
		return;
	}
	InsertFeed_.bindValue (":url", feed->URL_);
	InsertFeed_.bindValue (":last_update", feed->LastUpdate_);
	if (!InsertFeed_.exec ())
	{
		DumpError (InsertFeed_);
		if (!DB_.rollback ())
		{
			DumpError (DB_.lastError ());
			qWarning () << Q_FUNC_INFO << "failed to rollback";
		}
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
		if (!DB_.rollback ())
		{
			DumpError (DB_.lastError ());
			qWarning () << Q_FUNC_INFO << "failed to rollback";
		}
		return;
	}

	if (!DB_.commit ())
	{
		DumpError (DB_.lastError ());
		qWarning () << Q_FUNC_INFO << "failed to commit";
	}
}

void SQLStorageBackend::UpdateChannel (Channel_ptr channel, const QString& parent)
{
	ChannelFinder_.bindValue (":parent_feed_url", parent);
	ChannelFinder_.bindValue (":title", channel->Title_);
	ChannelFinder_.bindValue (":url", channel->Link_);
	if (!ChannelFinder_.exec ())
	{
		DumpError (ChannelFinder_);
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
		DumpError (UpdateChannel_);
		throw std::runtime_error ("failed to save channel");
	}
}

void SQLStorageBackend::UpdateItem (Item_ptr item, const QString& parent)
{
	ItemFinder_.bindValue (":parents_hash", parent);
	ItemFinder_.bindValue (":guid", item->Guid_);
	ItemFinder_.bindValue (":title", item->Title_);
	ItemFinder_.bindValue (":url", item->Link_);
	if (!ItemFinder_.exec ())
	{
		DumpError (ItemFinder_);
		throw std::runtime_error ("Unable to execute item finder query");
	}
	ItemFinder_.next ();
	if (!ItemFinder_.isValid ())
	{
		AddItem (item, parent);
		return;
	}
	ItemFinder_.finish ();

	UpdateItem_.bindValue (":parents_hash", parent);
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
		DumpError (UpdateItem_);
		throw std::runtime_error ("failed to save item");
	}
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
		DumpError (InsertChannel_);
		throw std::runtime_error ("failed to save channel");
	}

	std::for_each (channel->Items_.begin (), channel->Items_.end (),
		   boost::bind (&SQLStorageBackend::AddItem,
			   this,
			   _1, url + channel->Title_));
}

void SQLStorageBackend::AddItem (Item_ptr item, const QString& parent)
{
	InsertItem_.bindValue (":parents_hash", parent);
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
		DumpError (InsertItem_);
		throw std::runtime_error ("failed to save item");
	}
}

void SQLStorageBackend::RemoveItem (Item_ptr item, const QString& parent)
{
	if (!DB_.transaction ())
	{
		qWarning () << Q_FUNC_INFO << "failed to start transaction";
		DumpError (DB_.lastError ());
		return;
	}

	bool error = false;

	RemoveItem_.bindValue (":parents_hash", parent);
	RemoveItem_.bindValue (":title", item->Title_);
	RemoveItem_.bindValue (":url", item->Link_);
	RemoveItem_.bindValue (":guid", item->Guid_);

	if (!RemoveItem_.exec ())
	{
		error = true;
		DumpError (RemoveItem_);
	}

	if (!(error ? DB_.rollback () : DB_.commit ()))
	{
		qWarning () << Q_FUNC_INFO << "failed to" << (error ? "rollback" : "commit");
		DumpError (DB_.lastError ());
	}
}

void SQLStorageBackend::RemoveFeed (Feed_ptr feed)
{
	if (!DB_.transaction ())
	{
		qWarning () << Q_FUNC_INFO << "failed to start transaction";
		DumpError (DB_.lastError ());
		return;
	}

	bool error = false;

	for (channels_container_t::iterator i = feed->Channels_.begin (),
			end = feed->Channels_.end (); i != end; ++i)
	{
		QSqlQuery removeItem;
		removeItem.prepare ("DELETE FROM items "
				"WHERE parents_hash = :parents_hash");
		removeItem.bindValue (":parents_hash", (*i)->Link_ + (*i)->Title_);

		if (!removeItem.exec ())
		{
			DumpError (removeItem);
			error = true;
			break;
		}

		RemoveChannel_.bindValue (":parent_feed_url", feed->URL_);
		if (!RemoveChannel_.exec ())
		{
			DumpError (RemoveChannel_);
			error = true;
			break;
		}
	}

	RemoveFeed_.bindValue (":url", feed->URL_);
	if (!RemoveFeed_.exec ())
	{
		DumpError (RemoveFeed_);
		error = true;
	}

	if (!(error ? DB_.rollback () : DB_.commit ()))
	{
		qWarning () << Q_FUNC_INFO << "failed to" << (error ? "rollback" : "commit");
		DumpError (DB_.lastError ());
	}
}

void SQLStorageBackend::ToggleChannelUnread (const QString& hash, bool state)
{
	ToggleChannelUnread_.bindValue (":parents_hash", hash);
	ToggleChannelUnread_.bindValue (":unread", state);

	if (!ToggleChannelUnread_.exec ())
	{
		DumpError (ToggleChannelUnread_);
		throw std::runtime_error ("failed to toggle item");
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

bool SQLStorageBackend::InitializeTables ()
{
	QSqlQuery query;
	if (!query.exec ("CREATE TABLE feeds ("
			"url TEXT PRIMARY KEY, "
			"last_update TIMESTAMP "
			");"))
	{
		DumpError (query.lastError ());
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
		DumpError (query.lastError ());
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
		DumpError (query.lastError ());
		return false;
	}

	if (!query.exec ("CREATE UNIQUE INDEX feeds_url ON feeds (url);"))
		DumpError (query.lastError ());
	if (!query.exec ("CREATE INDEX channels_parent_url ON channels (parent_feed_url);"))
		DumpError (query.lastError ());
	if (!query.exec ("CREATE INDEX items_parents_hash ON items (parents_hash);"))
		DumpError (query.lastError ());

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
		DumpError (query.lastError ());
		return false;
	}

	return true;
}

void SQLStorageBackend::DumpError (const QSqlError& lastError) const
{
	qWarning () << lastError.text () << "|"
		<< lastError.databaseText () << "|"
		<< lastError.driverText () << "|"
		<< lastError.type () << "|"
		<< lastError.number ();
}

void SQLStorageBackend::DumpError (const QSqlQuery& lastQuery) const
{
	qWarning () << lastQuery.lastQuery ();
	DumpError (lastQuery.lastError ());
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
	qDebug () << Q_FUNC_INFO << version;
	if (!DB_.transaction ())
	{
		DumpError (DB_.lastError ());
		qWarning () << Q_FUNC_INFO << "failed to start transaction";
		return false;
	}

	bool success = true;

	if (version == 2)
	{
		QSqlQuery updateQuery = QSqlQuery ();
		success = updateQuery.exec ("ALTER TABLE items "
				"ADD num_comments SMALLINT");

		if (!success)
			DumpError (updateQuery);
		else
			success = updateQuery.exec ("ALTER TABLE items "
					"ADD comments_url TEXT");

		if (!success)
			DumpError (updateQuery);
		else
			success = updateQuery.exec ("UPDATE items "
					"SET num_comments = -1");

		if (!success)
			DumpError (updateQuery);
	}

	if (!(success ? DB_.commit () : DB_.rollback ()))
	{
		DumpError (DB_.lastError ());
		qWarning () << Q_FUNC_INFO << "failed to" << (success ? "commit" : "rollback");
		success = false;
	}

	qDebug () << Q_FUNC_INFO <<success ;

	return success ;
}

