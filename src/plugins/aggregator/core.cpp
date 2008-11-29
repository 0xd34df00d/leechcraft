#include <stdexcept>
#include <numeric>
#include <boost/bind.hpp>
#include <QtDebug>
#include <QImage>
#include <QSettings>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QTemporaryFile>
#include <QTimer>
#include <QNetworkReply>
#include <plugininterface/proxy.h>
#include <plugininterface/tagscompletionmodel.h>
#include "core.h"
#include "regexpmatchermanager.h"
#include "xmlsettingsmanager.h"
#include "parserfactory.h"
#include "rss20parser.h"
#include "rss10parser.h"
#include "rss091parser.h"
#include "atom10parser.h"
#include "atom03parser.h"
#include "channelsmodel.h"
#include "itembucket.h"
#include "opmlparser.h"
#include "opmlwriter.h"
#include "sqlstoragebackend.h"
#include "itemmodel.h"

Core::Core ()
: SaveScheduled_ (false)
, StorageBackend_ (new SQLStorageBackend ())
{
	qRegisterMetaTypeStreamOperators<Feed> ("Feed");
	qRegisterMetaTypeStreamOperators<Item> ("Item");

	const int feedsTable = 1;
	const int channelsTable = 1;
	const int itemsTable = 3;

	bool tablesOK = true;

	if (StorageBackend_->UpdateFeedsStorage (XmlSettingsManager::Instance ()->
			Property ("FeedsTableVersion", feedsTable).toInt (),
			feedsTable))
		XmlSettingsManager::Instance ()->setProperty ("FeedsTableVersion",
				feedsTable);
	else
		tablesOK = false;

	if (StorageBackend_->UpdateChannelsStorage (XmlSettingsManager::Instance ()->
			Property ("ChannelsTableVersion", channelsTable).toInt (),
			channelsTable))
		XmlSettingsManager::Instance ()->setProperty ("ChannelsTableVersion",
				channelsTable);
	else
		tablesOK = false;

	if (StorageBackend_->UpdateItemsStorage (XmlSettingsManager::Instance ()->
			Property ("ItemsTableVersion", itemsTable).toInt (),
			itemsTable))
		XmlSettingsManager::Instance ()->setProperty ("ItemsTableVersion",
				itemsTable);
	else
		tablesOK = false;

	StorageBackend_->Prepare ();

	connect (StorageBackend_.get (),
			SIGNAL (channelDataUpdated (Channel_ptr)),
			this,
			SLOT (handleChannelDataUpdated (Channel_ptr)));
	connect (StorageBackend_.get (),
			SIGNAL (itemDataUpdated (Item_ptr, Channel_ptr)),
			this,
			SLOT (handleItemDataUpdated (Item_ptr, Channel_ptr)));

	ParserFactory::Instance ().Register (&RSS20Parser::Instance ());
	ParserFactory::Instance ().Register (&Atom10Parser::Instance ());
	ParserFactory::Instance ().Register (&RSS091Parser::Instance ());
	ParserFactory::Instance ().Register (&Atom03Parser::Instance ());
	ParserFactory::Instance ().Register (&RSS10Parser::Instance ());
	ItemHeaders_ << tr ("Name") << tr ("Date");

	ItemModel_ = new ItemModel ();

	ChannelsModel_ = new ChannelsModel (this);
	connect (ChannelsModel_,
			SIGNAL (channelDataUpdated ()),
			this,
			SIGNAL (channelDataUpdated ()));

	if (tablesOK)
	{
		feeds_urls_t feeds;
		StorageBackend_->GetFeedsURLs (feeds);
		for (feeds_urls_t::const_iterator i = feeds.begin (),
				end = feeds.end (); i != end; ++i)
		{
			channels_shorts_t channels;
			StorageBackend_->GetChannels (channels, *i);
			std::for_each (channels.begin (), channels.end (),
					boost::bind (&ChannelsModel::AddChannel,
						ChannelsModel_,
						_1));
		}
	}

	TagsCompletionModel_ = new TagsCompletionModel (this);
	TagsCompletionModel_->UpdateTags (XmlSettingsManager::Instance ()->
			Property ("GlobalTags", QStringList ("untagged")).toStringList ());
}

Core& Core::Instance ()
{
	static Core core;
	return core;
}

void Core::Release ()
{
	ItemModel_->saveSettings ();
	XmlSettingsManager::Instance ()->Release ();
	saveSettings ();
	StorageBackend_.reset (0);
}

void Core::DoDelayedInit ()
{
	UpdateTimer_ = new QTimer (this);
	UpdateTimer_->setSingleShot (true);
	QDateTime currentDateTime = QDateTime::currentDateTime ();
	QDateTime lastUpdated = XmlSettingsManager::Instance ()->Property ("LastUpdateDateTime", currentDateTime).toDateTime ();
	connect (UpdateTimer_, SIGNAL (timeout ()), this, SLOT (updateFeeds ()));

	int updateDiff = lastUpdated.secsTo (currentDateTime);
	if ((XmlSettingsManager::Instance ()->property ("UpdateOnStartup").toBool ()) ||
		(updateDiff > XmlSettingsManager::Instance ()->property ("UpdateInterval").toInt () * 60))
			QTimer::singleShot (2000, this, SLOT (updateFeeds ()));
	else
		UpdateTimer_->start (updateDiff * 1000);

	QTimer *saveTimer = new QTimer (this);
	saveTimer->start (60 * 1000);
	connect (saveTimer, SIGNAL (timeout ()), this, SLOT (scheduleSave ()));

	XmlSettingsManager::Instance ()->RegisterObject ("UpdateInterval", this, "updateIntervalChanged");
	XmlSettingsManager::Instance ()->RegisterObject ("ShowIconInTray", this, "showIconInTrayChanged");
	UpdateUnreadItemsNumber ();
}

void Core::SetProvider (QObject *provider, const QString& feature)
{
	Providers_ [feature] = provider;
	if (feature == "http")
	{
		connect (provider,
				SIGNAL (jobFinished (int)),
				this,
				SLOT (handleJobFinished (int)));
		connect (provider,
				SIGNAL (jobRemoved (int)),
				this,
				SLOT (handleJobRemoved (int)));
		connect (provider,
				SIGNAL (jobError (int, IDownload::Error)),
				this,
				SLOT (handleJobError (int, IDownload::Error)));
	}
}

void Core::AddFeed (const QString& url, const QStringList& tags)
{
	feeds_urls_t feeds;
	StorageBackend_->GetFeedsURLs (feeds);
	feeds_urls_t::const_iterator pos =
		std::find (feeds.begin (), feeds.end (), url);
	if (pos != feeds.end ())
	{
		emit error (tr ("This feed is already added"));
		return;
	}

	QObject *provider = Providers_ ["http"];
	IDownload *iid = qobject_cast<IDownload*> (provider);
	if (!provider || !iid)
	{
		emit error (tr ("Strange, but no suitable provider found"));
		return;
	}
	if (!iid->CouldDownload (url, LeechCraft::Autostart))
	{
		emit error (tr ("Could not handle URL %1").arg (url));
		return;
	}

	QString name;
	{
		QTemporaryFile file;
		file.open ();
		name = file.fileName ();
		file.close ();
		file.remove ();
	}
	LeechCraft::DownloadParams params =
	{
		url,
		name
	};
	PendingJob pj =
	{
		PendingJob::RFeedAdded,
		url,
		name,
		tags
	};
	int id = iid->AddJob (params,
			LeechCraft::Internal |
			LeechCraft::Autostart |
			LeechCraft::DoNotNotifyUser |
			LeechCraft::DoNotSaveInHistory |
			LeechCraft::NotPersistent);
	PendingJobs_ [id] = pj;
}

void Core::RemoveFeed (const QModelIndex& index)
{
	if (!index.isValid ())
		return;
	ChannelShort channel = ChannelsModel_->GetChannelForIndex (index);

	QString feedURL = channel.ParentURL_;
	if (feedURL.isEmpty ())
	{
		qWarning () << Q_FUNC_INFO << "could not find feed for channel" ;
		return;
	}

	channels_shorts_t shorts;
	StorageBackend_->GetChannels (shorts, feedURL);

	for (size_t i = 0, size = shorts.size (); i < size; ++i)
		ChannelsModel_->RemoveChannel (shorts [i]);
	StorageBackend_->RemoveFeed (feedURL);

	if (channel.Link_ == CurrentChannelHash_.first && 
			channel.Title_ == CurrentChannelHash_.second)
	{
		CurrentChannelHash_ = qMakePair (QString (), QString ());
		reset ();
	}
}

void Core::Activated (const QModelIndex& index)
{
	OpenLink (CurrentItems_ [index.row ()].URL_);
}

void Core::FeedActivated (const QModelIndex& index)
{
	OpenLink (ChannelsModel_->GetChannelForIndex (index).Link_);
}

void Core::Selected (const QModelIndex& index)
{
	ItemShort item = CurrentItems_ [index.row ()];
	item.Unread_ = false;
	StorageBackend_->UpdateItem (item,
			CurrentChannelHash_.first, CurrentChannelHash_.second);
}

Item_ptr Core::GetItem (const QModelIndex& index) const
{
	ItemShort item = CurrentItems_ [index.row ()];
	return StorageBackend_->GetItem (item.Title_, item.URL_,
			CurrentChannelHash_.first + CurrentChannelHash_.second);
}

QAbstractItemModel* Core::GetChannelsModel ()
{
	return ChannelsModel_;
}

TagsCompletionModel* Core::GetTagsCompletionModel ()
{
	return TagsCompletionModel_;
}

void Core::UpdateTags (const QStringList& tags)
{
	TagsCompletionModel_->UpdateTags (tags);
}

void Core::MarkItemAsUnread (const QModelIndex& i)
{
	ItemShort is = CurrentItems_ [i.row ()];
	is.Unread_ = true;
	StorageBackend_->UpdateItem (is,
			CurrentChannelHash_.first, CurrentChannelHash_.second);
}

bool Core::IsItemRead (int item) const
{
	return !CurrentItems_ [item].Unread_;
}

void Core::MarkChannelAsRead (const QModelIndex& i)
{
	MarkChannel (i, false);
}

void Core::MarkChannelAsUnread (const QModelIndex& i)
{
	MarkChannel (i, true);
}

QStringList Core::GetTagsForIndex (int i) const
{
	return ChannelsModel_->
		GetChannelForIndex (ChannelsModel_->index (i, 0)).Tags_;
}

Core::ChannelInfo Core::GetChannelInfo (const QModelIndex& i) const
{
	ChannelShort channel = ChannelsModel_->GetChannelForIndex (i);
	ChannelInfo ci;
	ci.Link_ = channel.Link_;

	Channel_ptr rc = StorageBackend_->
		GetChannel (channel.Title_, channel.ParentURL_);
	ci.Description_ = rc->Description_;
	ci.Author_ = rc->Author_;
	return ci;
}

QPixmap Core::GetChannelPixmap (const QModelIndex& i) const
{
	ChannelShort channel = ChannelsModel_->GetChannelForIndex (i);

	Channel_ptr rc = StorageBackend_->
		GetChannel (channel.Title_, channel.ParentURL_);
	return rc->Pixmap_;
}

void Core::SetTagsForIndex (const QString& tags, const QModelIndex& index)
{
	ChannelShort channel = ChannelsModel_->GetChannelForIndex (index);
	channel.Tags_ = tags.split (' ');
	StorageBackend_->UpdateChannel (channel, channel.ParentURL_);
}

QStringList Core::GetCategories (const QModelIndex& index) const
{
	ChannelShort cs = ChannelsModel_->GetChannelForIndex (index);
	items_shorts_t items;
	StorageBackend_->GetItems (items, cs.ParentURL_ + cs.Title_);

	QStringList result;
	for (items_shorts_t::const_iterator i = items.begin (),
			end = items.end (); i != end; ++i)
	{
		QStringList categories = i->Categories_;
		for (QStringList::const_iterator j = categories.begin (),
				endJ = categories.end (); j != endJ; ++j)
			if (!result.contains (*j))
				result << *j;
	}
	std::sort (result.begin (), result.end ());
	return result;
}

QStringList Core::GetItemCategories (int index) const
{
	return CurrentItems_ [index].Categories_;
}

void Core::UpdateFeed (const QModelIndex& index)
{
	ChannelShort channel = ChannelsModel_->GetChannelForIndex (index);
	QString url = channel.ParentURL_;
	if (url.isEmpty ())
	{
		qWarning () << Q_FUNC_INFO << "could not found feed for index" << index;
		return;
	}

	QObject *provider = Providers_ ["http"];
	IDownload *isd = qobject_cast<IDownload*> (provider);
	if (!provider || !isd)
	{
		emit error (tr ("Strange, but no suitable provider found"));
		return;
	}
	if (!isd->CouldDownload (url, LeechCraft::Autostart))
	{
		emit error (tr ("Could not handle URL %1").arg (url));
		return;
	}

	QString name;
	{
		QTemporaryFile file;
		file.open ();
		name = file.fileName ();
		file.close ();
		file.remove ();
	}
	LeechCraft::DownloadParams params = { url, name };
	PendingJob pj = {
		PendingJob::RFeedUpdated,
		url,
		name,
		QStringList ()
	};
	int id = isd->AddJob (params,
			LeechCraft::Internal |
			LeechCraft::Autostart |
			LeechCraft::DoNotNotifyUser |
			LeechCraft::DoNotSaveInHistory |
			LeechCraft::NotPersistent);
	PendingJobs_ [id] = pj;
}

QModelIndex Core::GetUnreadChannelIndex ()
{
	return ChannelsModel_->GetUnreadChannelIndex ();
}

void Core::AddToItemBucket (const QModelIndex& index) const
{
	if (!index.isValid () || index.row () >= rowCount ())
		return;

	ItemShort is = CurrentItems_ [index.row ()];
	ItemModel_->AddItem (StorageBackend_->GetItem (is.Title_, is.URL_,
				CurrentChannelHash_.first + CurrentChannelHash_.second));
}

void Core::AddFromOPML (const QString& filename,
		const QString& tags,
		const std::vector<bool>& mask)
{
	QFile file (filename);
	if (!file.open (QIODevice::ReadOnly))
	{
		emit error (tr ("Could not open file %1 for reading.")
					.arg (filename));
		return;
	}

	QByteArray data = file.readAll ();
	file.close ();

	QString errorMsg;
	int errorLine, errorColumn;
	QDomDocument document;
	if (!document.setContent (data,
				true,
				&errorMsg,
				&errorLine,
				&errorColumn))
	{
		emit error (tr ("XML error, file %1, line %2, column %3, error:<br />%4")
					.arg (filename)
					.arg (errorLine)
					.arg (errorColumn)
					.arg (errorMsg));
		return;
	}

	OPMLParser parser (document);
	if (!parser.IsValid ())
	{
		emit error (tr ("OPML from file %1 is not valid.")
					.arg (filename));
		return;
	}

	OPMLParser::items_container_t items = parser.Parse ();
	for (std::vector<bool>::const_iterator begin = mask.begin (),
			i = mask.end () - 1; i >= begin; --i)
		if (!*i)
		{
			size_t distance = std::distance (mask.begin (), i);
			OPMLParser::items_container_t::iterator eraser = items.begin ();
			std::advance (eraser, distance);
			items.erase (eraser);
		}

	QStringList tagsList = tags.split (" ", QString::SkipEmptyParts);
	for (OPMLParser::items_container_t::const_iterator i = items.begin (),
			end = items.end (); i != end; ++i)
		AddFeed (i->URL_, tagsList + i->Categories_);
}

void Core::ExportToOPML (const QString& where,
		const QString& title,
		const QString& owner,
		const QString& ownerEmail,
		const std::vector<bool>& mask) const
{
	channels_shorts_t channels;
	GetChannels (channels);

	for (std::vector<bool>::const_iterator begin = mask.begin (),
			i = mask.end () - 1; i >= begin; --i)
		if (!*i)
		{
			size_t distance = std::distance (mask.begin (), i);
			channels_shorts_t::iterator eraser = channels.begin ();
			std::advance (eraser, distance);
			channels.erase (eraser);
		}

	OPMLWriter writer;
	QString data = writer.Write (channels, title, owner, ownerEmail);

	QFile f (where);
	if (!f.open (QIODevice::WriteOnly))
	{
		emit error (QString ("Could not open file %1 for write.").arg (where));
		return;
	}

	f.write (data.toUtf8 ());
	f.close ();
}

void Core::ExportToBinary (const QString& where,
		const QString& title,
		const QString& owner,
		const QString& ownerEmail,
		const std::vector<bool>& mask) const
{
	channels_shorts_t channels;
	GetChannels (channels);

	for (std::vector<bool>::const_iterator begin = mask.begin (),
			i = mask.end () - 1; i >= begin; --i)
		if (!*i)
		{
			size_t distance = std::distance (mask.begin (), i);
			channels_shorts_t::iterator eraser = channels.begin ();
			std::advance (eraser, distance);
			channels.erase (eraser);
		}

	QFile f (where);
	if (!f.open (QIODevice::WriteOnly))
	{
		emit error (QString ("Could not open file %1 for write.").arg (where));
		return;
	}

	QDataStream data (&f);

	int version = 1;
	int magic = 0xd34df00d;
	data << magic << version;

	for (channels_shorts_t::const_iterator i = channels.begin (),
			end = channels.end (); i != end; ++i)
	{
		Channel_ptr channel = StorageBackend_->GetChannel (i->Title_,
				i->ParentURL_);
		items_shorts_t items;

		QString hash = i->ParentURL_ + i->Title_;

		StorageBackend_->GetItems (items, hash);

		for (items_shorts_t::const_iterator j = items.begin (),
				endJ = items.end (); j != endJ; ++j)
			channel->Items_.push_back (StorageBackend_->GetItem (j->Title_,
						j->URL_, hash));

		data << (*channel);
	}
}

ItemModel* Core::GetItemModel () const
{
	return ItemModel_;
}

void Core::SubscribeToComments (const QModelIndex& index)
{
	Item_ptr it = GetItem (index);
	QString commentRSS = it->CommentsLink_;
	QStringList tags = it->Categories_;

	QStringList addTags = XmlSettingsManager::Instance ()->
		property ("CommentsTags").toString ().split (' ',
				QString::SkipEmptyParts);
	AddFeed (commentRSS, tags + addTags);
}

void Core::OpenLink (const QString& url)
{
	if (!Providers_.contains ("webbrowser"))
	{
		QDesktopServices::openUrl (QUrl (url));
		return;
	}
	QObject *provider = Providers_ ["webbrowser"];
	QMetaObject::invokeMethod (provider,
			"openURL", Q_ARG (QString, url));
}

QWebView* Core::CreateWindow ()
{
	if (!Providers_.contains ("webbrowser"))
		return 0;

	QWebView *result;
	QObject *provider = Providers_ ["webbrowser"];
	QMetaObject::invokeMethod (provider,
			"createWindow", Q_RETURN_ARG (QWebView*, result));

	return result;
}

void Core::GetChannels (channels_shorts_t& channels) const
{
	feeds_urls_t urls;
	StorageBackend_->GetFeedsURLs (urls);
	for (feeds_urls_t::const_iterator i = urls.begin (),
			end = urls.end (); i != end; ++i)
		StorageBackend_->GetChannels (channels, *i);
}

void Core::AddFeeds (const feeds_container_t& feeds,
		const QString& tagsString)
{
	QStringList tags = tagsString.split (" ", QString::SkipEmptyParts);

	for (feeds_container_t::const_iterator i = feeds.begin (),
			end = feeds.end (); i != end; ++i)
	{
		for (channels_container_t::const_iterator j =
				(*i)->Channels_.begin (), jEnd = (*i)->Channels_.end ();
				j != jEnd; ++j)
		{
			for (QStringList::const_iterator tag = tags.begin (),
					tagEnd = tags.end (); tag != tagEnd; ++tag)
				if (!(*j)->Tags_.contains (*tag))
					(*j)->Tags_ << *tag;

			ChannelsModel_->AddChannel ((*j)->ToShort ());
		}

		StorageBackend_->AddFeed (*i);
	}
}

int Core::columnCount (const QModelIndex&) const
{
	return ItemHeaders_.size ();
}

QVariant Core::data (const QModelIndex& index, int role) const
{
	if (!index.isValid () || index.row () >= rowCount ())
		return QVariant ();

	if (role == Qt::DisplayRole)
	{
		switch (index.column ())
		{
			case 0:
				return CurrentItems_ [index.row ()].Title_;
			case 1:
				return CurrentItems_ [index.row ()].PubDate_;
			default:
				return QVariant ();
		}
	}
	else if (role == Qt::ForegroundRole)
		return CurrentItems_ [index.row ()].Unread_ ? Qt::red : Qt::black;
	else
		return QVariant ();
}

Qt::ItemFlags Core::flags (const QModelIndex&) const
{
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool Core::hasChildren (const QModelIndex& index) const
{
	return !index.isValid ();
}

QVariant Core::headerData (int column, Qt::Orientation orient, int role) const
{
	if (orient == Qt::Horizontal && role == Qt::DisplayRole)
		return ItemHeaders_.at (column);
	else
		return QVariant ();
}

QModelIndex Core::index (int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex (row, column, parent))
		return QModelIndex ();

	return createIndex (row, column);
}

QModelIndex Core::parent (const QModelIndex&) const
{
	return QModelIndex ();
}

int Core::rowCount (const QModelIndex& parent) const
{
	return CurrentItems_.size ();
}

void Core::currentChannelChanged (const QModelIndex& index)
{
	ChannelShort ch = ChannelsModel_->GetChannelForIndex (index);
	CurrentChannelHash_ = qMakePair<QString, QString> (ch.ParentURL_,
			ch.Title_);
	CurrentItems_.clear ();
	StorageBackend_->GetItems (CurrentItems_, ch.ParentURL_ + ch.Title_);
	reset ();
}

void Core::scheduleSave ()
{
	if (SaveScheduled_)
		return;
	QTimer::singleShot (500, this, SLOT (saveSettings ()));
	SaveScheduled_ = true;
}

namespace
{
	struct IsDateSuitable
	{
		QDateTime DateTime_;

		IsDateSuitable (const QDateTime& dt)
		: DateTime_ (dt)
		{
		}

		bool operator() (const Item_ptr& it)
		{
			return it->PubDate_ < DateTime_;
		}
	};
};

void Core::handleJobFinished (int id)
{
	class FileRemoval : public QFile
	{
		public:
			FileRemoval (const QString& name)
				: QFile (name)
			{
			}

			virtual ~FileRemoval ()
			{
				close ();
				remove ();
			}
	};

	if (!PendingJobs_.contains (id))
		return;
	PendingJob pj = PendingJobs_ [id];
	PendingJobs_.remove (id);

	FileRemoval file (pj.Filename_);
	if (!file.open (QIODevice::ReadOnly))
	{
		qWarning () << Q_FUNC_INFO << "could not open file for pj " << pj.Filename_;
		return;
	}
	if (!file.size ())
	{
		emit error (tr ("Downloaded file from url %1 has null size!").arg (pj.URL_));
		return;
	}

	feeds_urls_t feeds;
	StorageBackend_->GetFeedsURLs (feeds);
	feeds_urls_t::const_iterator pos =
		std::find (feeds.begin (), feeds.end (), pj.URL_);

	if (pj.Role_ == PendingJob::RFeedUpdated &&
			pos == feeds.end ())
	{
		emit error (tr ("Feed with url %1 not found.").arg (pj.URL_));
		return;
	}

	channels_container_t channels, modifiedItems, ourChannels;
	if (pj.Role_ != PendingJob::RFeedExternalData)
	{
		QByteArray data = file.readAll ();
		QDomDocument doc;
		QString errorMsg;
		int errorLine, errorColumn;
		if (!doc.setContent (data, true, &errorMsg, &errorLine, &errorColumn))
		{
			file.copy (QDir::tempPath () + "/failedFile.xml");
			emit error (tr ("XML file parse error: %1, line %2, column %3, filename %4, from %5")
					.arg (errorMsg)
					.arg (errorLine)
					.arg (errorColumn)
					.arg (pj.Filename_)
					.arg (pj.URL_));
			return;
		}

		Parser *parser = ParserFactory::Instance ().Return (doc);
		if (!parser)
		{
			file.copy (QDir::tempPath () + "/failedFile.xml");
			emit error (tr ("Could not find parser to parse file %1 from %2")
					.arg (pj.Filename_)
					.arg (pj.URL_));
			return;
		}
		
		// We should form the list of already existing channels
		channels_shorts_t shorts;
		StorageBackend_->GetChannels (shorts, pj.URL_);
		for (channels_shorts_t::const_iterator i = shorts.begin (),
				end = shorts.end (); i != end; ++i)
		{
			Channel_ptr channel2push = StorageBackend_->
				GetChannel (i->Title_, pj.URL_);
			// And we shouldn't forget about their items.
			items_shorts_t itemShorts;
			StorageBackend_->GetItems (itemShorts,
					pj.URL_ + channel2push->Title_);
			for (items_shorts_t::const_iterator j = itemShorts.begin (),
					endJ = itemShorts.end (); j != endJ; ++j)
				channel2push->Items_.push_back (StorageBackend_->GetItem (j->Title_,
							j->URL_, pj.URL_ + channel2push->Title_));

			ourChannels.push_back (channel2push);
		}

		channels = parser->Parse (ourChannels, modifiedItems, doc);
		for (size_t i = 0; i < channels.size (); ++i)
			channels [i]->ParentURL_ = pj.URL_;
	}
	QString emitString;
	if (pj.Role_ == PendingJob::RFeedAdded)
	{
		for (size_t i = 0; i < channels.size (); ++i)
		{
			channels [i]->Tags_ = pj.Tags_;
			FetchPixmap (channels [i]);
			FetchFavicon (channels [i]);
			ChannelsModel_->AddChannel (channels [i]->ToShort ());
		}

		Feed_ptr feed (new Feed ());
		feed->Channels_ = channels;
		feed->URL_ = pj.URL_;
		TagsCompletionModel_->UpdateTags (pj.Tags_);
		StorageBackend_->AddFeed (feed);
	}
	else if (pj.Role_ == PendingJob::RFeedUpdated)
		emitString += HandleFeedUpdated (channels, modifiedItems,
				ourChannels, pj);
	else if (pj.Role_ == PendingJob::RFeedExternalData)
		HandleExternalData (pj.URL_, file);
	UpdateUnreadItemsNumber ();
	if (!emitString.isEmpty ())
		emit showDownloadMessage (emitString);
	scheduleSave ();
}

void Core::handleJobRemoved (int id)
{
	if (PendingJobs_.contains (id))
		PendingJobs_.remove (id);
}

void Core::handleJobError (int id, IDownload::Error)
{
	if (!PendingJobs_.contains (id))
		return;
	PendingJobs_.remove (id);
}

void Core::updateFeeds ()
{
	QObject *provider = Providers_ ["http"];
	IDownload *isd = qobject_cast<IDownload*> (provider);
	if (!provider || !isd)
	{
		emit error (tr ("Strange, but no suitable provider found"));
		return;
	}
	feeds_urls_t urls;
	StorageBackend_->GetFeedsURLs (urls);
	for (feeds_urls_t::const_iterator i = urls.begin (),
			end = urls.end (); i != end; ++i)
	{
		if (!isd->CouldDownload (*i, LeechCraft::Autostart))
		{
			emit error (tr ("Could not handle URL %1").arg (*i));
			continue;
		}

		QString filename;
		{
			QTemporaryFile file;
			file.open ();
			filename = file.fileName ();
			file.close ();
			file.remove ();
		}
		LeechCraft::DownloadParams params =
		{
			*i,
			filename
		};
		PendingJob pj =
		{
			PendingJob::RFeedUpdated,
			*i,
			filename,
			QStringList ()
		};
		int id = isd->AddJob (params,
				LeechCraft::Internal |
				LeechCraft::Autostart |
				LeechCraft::DoNotNotifyUser |
				LeechCraft::DoNotSaveInHistory |
				LeechCraft::NotPersistent);
		PendingJobs_ [id] = pj;
	}
	XmlSettingsManager::Instance ()->setProperty ("LastUpdateDateTime", QDateTime::currentDateTime ());
	UpdateTimer_->start (XmlSettingsManager::Instance ()->property ("UpdateInterval").toInt () * 60 * 1000);
}

void Core::fetchExternalFile (const QString& url, const QString& where)
{
	QObject *provider = Providers_ ["http"];
	IDownload *isd = qobject_cast<IDownload*> (provider);
	if (!provider || !isd)
	{
		emit error (tr ("Strange, but no suitable provider found"));
		throw std::runtime_error ("no suitable provider");
	}
	if (!isd->CouldDownload (url, LeechCraft::Autostart))
	{
		emit error (tr ("Could not handle URL %1").arg (url));
		throw std::runtime_error ("could not handle URL");
	}

	LeechCraft::DownloadParams params =
	{
		url,
		where
	};
	PendingJob pj =
	{
		PendingJob::RFeedExternalData,
		url,
		where,
		QStringList ()
	};
	int id = isd->AddJob (params,
			LeechCraft::Internal |
			LeechCraft::Autostart |
			LeechCraft::DoNotNotifyUser |
			LeechCraft::DoNotSaveInHistory |
			LeechCraft::NotPersistent);
	PendingJobs_ [id] = pj;
}

void Core::saveSettings ()
{
	SaveScheduled_ = false;
}

namespace
{
	struct UnreadAccumulator
	{
		int operator() (int i, const ItemShort& cs)
		{
			return cs.Unread_ ? i + 1: i;
		}
	};
};

void Core::handleChannelDataUpdated (Channel_ptr channel)
{
	ChannelShort cs = channel->ToShort ();

	items_shorts_t channelItems;
	StorageBackend_->GetItems (channelItems, cs.ParentURL_ + cs.Title_);
	cs.Unread_ = std::accumulate (channelItems.begin (),
			channelItems.end (), 0, UnreadAccumulator ());
	ChannelsModel_->UpdateChannelData (cs);
	UpdateUnreadItemsNumber ();

	if (cs.ParentURL_ == CurrentChannelHash_.first &&
			cs.Title_ == CurrentChannelHash_.second)
	{
		CurrentItems_ = channelItems;
		emit dataChanged (index (0, 0), index (CurrentItems_.size (), 1));
	}
}

namespace
{
	struct FindEarlierDate
	{
		QDateTime Pattern_;

		FindEarlierDate (const QDateTime& pattern)
		: Pattern_ (pattern)
		{
		}

		bool operator() (const ItemShort& is)
		{
			return Pattern_ > is.PubDate_;
		}
	};
};

void Core::handleItemDataUpdated (Item_ptr item, Channel_ptr channel)
{
	if (channel->ParentURL_ != CurrentChannelHash_.first ||
			channel->Title_ != CurrentChannelHash_.second)
		return;

	ItemShort is = item->ToShort ();

	items_shorts_t::iterator pos = CurrentItems_.end ();

	for (items_shorts_t::iterator i = CurrentItems_.begin (),
			end = CurrentItems_.end (); i != end; ++i)
		if (is.Title_ == i->Title_ &&
				is.URL_ == i->URL_)
		{
			pos = i;
			break;
		}

	// Item is new
	if (pos == CurrentItems_.end ())
	{
		items_shorts_t::iterator insertPos =
			std::find_if (CurrentItems_.begin (), CurrentItems_.end (),
					FindEarlierDate (item->PubDate_));

		int shift = std::distance (CurrentItems_.begin (), insertPos);

		beginInsertRows (QModelIndex (), shift, shift);
		CurrentItems_.insert (insertPos, is);
		endInsertRows ();
	}
	// Item exists already
	else
	{
		*pos = is;

		int distance = std::distance (CurrentItems_.begin (), pos);
		emit dataChanged (index (distance, 0), index (distance, 1));
	}
}

void Core::updateIntervalChanged ()
{
	UpdateTimer_->setInterval (XmlSettingsManager::Instance ()->property ("UpdateInterval").toInt () * 60 * 1000);
}

void Core::showIconInTrayChanged ()
{
	UpdateUnreadItemsNumber ();
}

void Core::handleSslError (QNetworkReply *reply)
{
	reply->ignoreSslErrors ();
}

void Core::UpdateUnreadItemsNumber () const
{
	emit unreadNumberChanged (StorageBackend_->GetUnreadItemsNumber ());
}

void Core::FetchPixmap (const Channel_ptr& channel)
{
	if (QUrl (channel->PixmapURL_).isValid () &&
			!QUrl (channel->PixmapURL_).isRelative ())
	{
		ExternalData data;
		data.Type_ = ExternalData::TImage;
		data.RelatedChannel_ = channel;
		QString exFName;
		{
			QTemporaryFile file;
			file.open ();
			exFName = file.fileName ();
			file.close ();
			file.remove ();
		}
		try
		{
			fetchExternalFile (channel->PixmapURL_, exFName);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO << e.what ();
			return;
		}
		PendingJob2ExternalData_ [channel->PixmapURL_] = data;
	}
}

void Core::FetchFavicon (const Channel_ptr& channel)
{
	QUrl oldUrl (channel->Link_);
	oldUrl.setPath ("/favicon.ico");
	QString iconUrl = oldUrl.toString ();

	ExternalData iconData;
	iconData.Type_ = ExternalData::TIcon;
	iconData.RelatedChannel_ = channel;
	QString exFName;
	{
		QTemporaryFile file;
		file.open ();
		exFName = file.fileName ();
		file.close ();
		file.remove ();
	}
	try
	{
		fetchExternalFile (iconUrl, exFName);
	}
	catch (const std::runtime_error& e)
	{
		qWarning () << Q_FUNC_INFO << e.what ();
		return;
	}
	PendingJob2ExternalData_ [iconUrl] = iconData;
}

void Core::HandleExternalData (const QString& url, const QFile& file)
{
	ExternalData data = PendingJob2ExternalData_.take (url);
	if (data.RelatedChannel_.get ())
	{
		switch (data.Type_)
		{
			case ExternalData::TImage:
				data.RelatedChannel_->Pixmap_ =
					QPixmap::fromImage (QImage (file.fileName ()));
				break;
			case ExternalData::TIcon:
				data.RelatedChannel_->Favicon_ =
					QPixmap::fromImage (QImage (file.fileName ()))
					.scaled (16, 16);;
				break;
		}
		StorageBackend_->UpdateChannel (data.RelatedChannel_,
				data.RelatedChannel_->ParentURL_);
	}
	else if (data.RelatedFeed_)
	{
	}
}

namespace
{
	struct ChannelFinder
	{
		const Channel_ptr& Channel_;

		ChannelFinder (const Channel_ptr& channel)
		: Channel_ (channel)
		{
		}

		bool operator() (const Channel_ptr& obj)
		{
			return *Channel_ == *obj;
		}
	};
};

QString Core::HandleFeedUpdated (const channels_container_t& channels,
		const channels_container_t& modifiedChannels,
		const channels_container_t& ourChannels,
		const Core::PendingJob& pj)
{
	QString emitString;

	for (channels_container_t::const_iterator i = modifiedChannels.begin (),
			end = modifiedChannels.end (); i != end; ++i)
	{
		channels_container_t::const_iterator position =
			std::find_if (ourChannels.begin (), ourChannels.end (),
					ChannelFinder (*i));

		if (position == ourChannels.end ())
			continue;

		for (items_container_t::const_iterator item = (*i)->Items_.begin (),
				itemEnd = (*i)->Items_.end (); item != itemEnd; ++item)
		{
			items_container_t::iterator ourItem =
				std::find_if ((*position)->Items_.begin (),
						(*position)->Items_.end (),
						ItemComparator (*item));
			if (ourItem == (*position)->Items_.end ())
			{
				qWarning () << Q_FUNC_INFO << "not found modified item";
				continue;
			}

			if (!IsModified (*ourItem, *item))
				continue;

			(*ourItem)->Categories_ = (*item)->Categories_;
			(*ourItem)->NumComments_ = (*item)->NumComments_;
			(*ourItem)->CommentsLink_ = (*item)->CommentsLink_;
			(*ourItem)->CommentsPageLink_ = (*item)->CommentsPageLink_;

			StorageBackend_->UpdateItem ((*ourItem),
					(*position)->ParentURL_, (*position)->Title_);
		}
	}

	for (channels_container_t::const_iterator i = channels.begin (),
			end = channels.end (); i != end; ++i)
	{
		channels_container_t::const_iterator position =
			std::find_if (ourChannels.begin (), ourChannels.end (),
					ChannelFinder (*i));

		std::for_each ((*i)->Items_.begin (), (*i)->Items_.end (),
				boost::bind (&RegexpMatcherManager::HandleItem,
					&RegexpMatcherManager::Instance (),
					_1));

		if (position == ourChannels.end ())
		{
			ChannelsModel_->AddChannel ((*i)->ToShort ());
			StorageBackend_->AddChannel (*i, pj.URL_);
			emitString += tr ("Added channel \"%1\" (has %2 items)")
				.arg ((*i)->Title_)
				.arg ((*i)->Items_.size ());
		}
		else
		{
			if ((*i)->Items_.size ())
				emitString += tr ("Updated channel \"%1\" (%2 new items)")
					.arg ((*i)->Title_)
					.arg ((*i)->Items_.size ());

			std::for_each ((*i)->Items_.begin (), (*i)->Items_.end (),
					boost::bind (&StorageBackend::AddItem,
						StorageBackend_.get (),
						_1,
						pj.URL_,
						(*i)->Title_));

			if ((*i)->LastBuild_.isValid ())
				(*position)->LastBuild_ = (*i)->LastBuild_;
			else 
				(*position)->LastBuild_ = QDateTime::currentDateTime ();

			// Now cut off old and overwhelming items.
			XmlSettingsManager::Instance ()->
				property ("ItemsPerChannel").value<size_t> ();
			QDateTime current = QDateTime::currentDateTime ();
			int removeFrom = -1;
			int days = XmlSettingsManager::Instance ()->
				property ("ItemsMaxAge").toInt ();
			for (size_t j = 0; j < (*position)->Items_.size (); ++j)
				if ((*position)->Items_ [j]->PubDate_.daysTo (current) > days)
				{
					removeFrom = j;
					break;
				}
			if (removeFrom == 0)
				removeFrom = 1;

			removeFrom = std::min (removeFrom,
					XmlSettingsManager::Instance ()->
					property ("ItemsPerChannel").toInt ());

			if ((*position)->Items_.size () > removeFrom)
				std::for_each ((*position)->Items_.begin () + removeFrom,
						(*position)->Items_.end (),
						boost::bind (&StorageBackend::RemoveItem,
							StorageBackend_.get (),
							_1,
							pj.URL_ + (*position)->Title_,
							(*position)->Title_,
							pj.URL_));

			StorageBackend_->UpdateChannel ((*position), pj.URL_);
		}
	}

	return emitString;
}

void Core::MarkChannel (const QModelIndex& i, bool state)
{
	ChannelShort cs = ChannelsModel_->GetChannelForIndex (i);

	QString hash = cs.ParentURL_ + cs.Title_;
	StorageBackend_->ToggleChannelUnread (cs.ParentURL_, cs.Title_, state);
}

