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

	FeedFinderByURL_ = QSqlQuery ();
	FeedFinderByURL_.prepare ("SELECT last_update FROM feeds WHERE url = :url");

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
			"unread"
			") VALUES ("
			":parents_hash, "
			":title, "
			":url, "
			":description, "
			":author, "
			":category, "
			":guid, "
			":pub_date, "
			":unread"
			")");
}

SQLStorageBackend::~SQLStorageBackend ()
{
	DB_.close ();
}

void SQLStorageBackend::GetFeeds (feeds_container_t& result) const
{
}

void SQLStorageBackend::GetChannels (Feed_ptr feed,
		channels_container_t& result) const
{
}

void SQLStorageBackend::GetItems (Channel_ptr channel,
		items_container_t& result) const
{
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

void SQLStorageBackend::UpdateFeed (Feed_ptr feed)
{
	QString url = feed->URL_;

	FeedFinderByURL_.bindValue (":url", url);
	if (!FeedFinderByURL_.exec ())
	{
		DumpError (FeedFinderByURL_);
		return;
	}
	FeedFinderByURL_.next ();
	if (!FeedFinderByURL_.isValid ())
		AddFeed (feed);
}

void SQLStorageBackend::UpdateItem (Item_ptr item)
{
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
	InsertItem_.bindValue (":category", item->Category_);
	InsertItem_.bindValue (":guid", item->Guid_);
	InsertItem_.bindValue (":pub_date", item->PubDate_);
	InsertItem_.bindValue (":unread", item->Unread_);

	if (!InsertItem_.exec ())
	{
		DumpError (InsertItem_);
		throw std::runtime_error ("failed to save item");
	}
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
			"unread TINYINT "
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
	qDebug () << lastError.text () << "|"
		<< lastError.databaseText () << "|"
		<< lastError.driverText () << "|"
		<< lastError.type () << "|"
		<< lastError.number ();
}

void SQLStorageBackend::DumpError (const QSqlQuery& lastQuery) const
{
	qDebug () << lastQuery.lastQuery ();
	DumpError (lastQuery.lastError ());
}

QByteArray SQLStorageBackend::SerializePixmap (const QPixmap& pixmap) const
{
	QByteArray bytes;
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

