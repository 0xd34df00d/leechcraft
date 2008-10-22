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
	FeedFinderByURL_ = QSqlQuery ();
	FeedFinderByURL_.prepare ("SELECT last_update FROM feeds WHERE url = :url");

	ChannelFinder_ = QSqlQuery ();
	ChannelFinder_.prepare ("SELECT description FROM channels "
			"WHERE parent_feed_url = :parent_feed_url "
			"AND title = :title "
			"AND url = :url");

	ItemFinder_ = QSqlQuery ();
	ItemFinder_.prepare ("SELECT title FROM items "
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

void SQLStorageBackend::GetFeeds (feeds_container_t& result) const
{
	QSqlQuery feedSelector;
	if (!feedSelector.exec ("SELECT url, last_update FROM feeds"))
	{
		DumpError (feedSelector);
		return;
	}

	while (feedSelector.next ())
	{
		Feed_ptr feed (new Feed);
		feed->URL_ = feedSelector.value (0).toString ();
		feed->LastUpdate_ = feedSelector.value (1).toDateTime ();
		GetChannels (feed);
		result.push_back (feed);
	}
}

void SQLStorageBackend::GetChannels (Feed_ptr feed) const
{
	QSqlQuery channelsSelector;
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
	channelsSelector.bindValue (":parent_feed_url", feed->URL_);
	if (!channelsSelector.exec ())
	{
		DumpError (channelsSelector);
		return;
	}

	while (channelsSelector.next ())
	{
		Channel_ptr channel (new Channel);

		channel->Link_ = channelsSelector.value (0).toString ();
		channel->Title_ = channelsSelector.value (1).toString ();
		channel->Description_ = channelsSelector.value (2).toString ();
		channel->LastBuild_ = channelsSelector.value (3).toDateTime ();
		channel->Tags_ = channelsSelector.value (4).toString ().split (' ',
				QString::SkipEmptyParts);
		channel->Language_ = channelsSelector.value (5).toString ();
		channel->Author_ = channelsSelector.value (6).toString ();
		channel->PixmapURL_ = channelsSelector.value (7).toString ();
		channel->Pixmap_ = UnserializePixmap (channelsSelector
				.value (8).toByteArray ());
		channel->Favicon_ = UnserializePixmap (channelsSelector
				.value (9).toByteArray ());

		GetItems (channel);

		feed->Channels_.push_back (channel);
	}
}

void SQLStorageBackend::GetItems (Channel_ptr channel) const
{
	QSqlQuery itemsSelector;
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
			"comments_url "
			"FROM items "
			"WHERE parents_hash = :parents_hash "
			"ORDER BY pub_date DESC");
	itemsSelector.bindValue (":parents_hash", channel->Link_ + channel->Title_);
	if (!itemsSelector.exec ())
	{
		DumpError (itemsSelector);
		return;
	}

	while (itemsSelector.next ())
	{
		Item_ptr item (new Item);

		item->Title_ = itemsSelector.value (0).toString ();
		item->Link_ = itemsSelector.value (1).toString ();
		item->Description_ = itemsSelector.value (2).toString ();
		item->Author_ = itemsSelector.value (3).toString ();
		item->Categories_ = itemsSelector.value (4).toString ().split ("<<<");
		item->Guid_ = itemsSelector.value (5).toString ();
		item->PubDate_ = itemsSelector.value (6).toDateTime ();
		item->Unread_ = itemsSelector.value (7).toBool ();
		item->NumComments_ = itemsSelector.value (8).toInt ();
		item->CommentsLink_ = itemsSelector.value (9).toString ();

		channel->Items_.push_back (item);
	}
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
			   _1, channel->Link_ + channel->Title_));
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

bool SQLStorageBackend::UpdateFeedsStorage (int oldV, int newV)
{
    return true;
}

bool SQLStorageBackend::UpdateChannelsStorage (int oldV, int newV)
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

