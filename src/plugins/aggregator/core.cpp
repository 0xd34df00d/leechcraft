#include <stdexcept>
#include <numeric>
#include <boost/bind.hpp>
#include <QtDebug>
#include <QImage>
#include <QSettings>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QNetworkReply>
#include <interfaces/iwebbrowser.h>
#include <plugininterface/mergemodel.h>
#include <plugininterface/proxy.h>
#include <plugininterface/util.h>
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
#include "jobholderrepresentation.h"
#include "channelsfiltermodel.h"
#include "itemslistmodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			Core::Core ()
			: SaveScheduled_ (false)
			, CurrentItemsModel_ (new ItemsListModel)
			, MergeMode_ (false)
			{
				qRegisterMetaType<QItemSelection> ("QItemSelection");
				QStringList placeholders;
				placeholders << "" << "";
				ItemLists_ = new LeechCraft::Util::MergeModel (placeholders);
				ItemLists_->AddModel (CurrentItemsModel_);
			}
			
			Core& Core::Instance ()
			{
				static Core core;
				return core;
			}
			
			void Core::Release ()
			{
				ItemModel_->saveSettings ();
				saveSettings ();
				XmlSettingsManager::Instance ()->Release ();
				StorageBackend_.reset ();
				delete JobHolderRepresentation_;
				delete ItemModel_;
				delete ChannelsFilterModel_;
				delete ChannelsModel_;
				delete ItemLists_;
				delete CurrentItemsModel_;
			}
			
			void Core::SetProxy (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;
			}
			
			ICoreProxy_ptr Core::GetProxy () const
			{
				return Proxy_;
			}
			
			bool Core::CouldHandle (const LeechCraft::DownloadEntity& e)
			{
				if (QUrl (QString (e.Location_)).scheme () != "http" &&
						QUrl (QString (e.Location_)).scheme () != "https")
					return false;
			
				if (e.Mime_ != "application/atom+xml" &&
						e.Mime_ != "application/rss+xml")
					return false;
			
				return true;
			}
			
			void Core::SetWidgets (QToolBar *bar, QWidget *tab)
			{
				ChannelsModel_->SetWidgets (bar, tab);
			}
			
			void Core::DoDelayedInit ()
			{
				QStringList headers;
				headers << tr ("Name")
					<< tr ("Date");
				ItemLists_->SetHeaders (headers);

				ChannelsModel_ = new ChannelsModel ();
				ChannelsFilterModel_ = new ChannelsFilterModel ();
				ChannelsFilterModel_->setSourceModel (ChannelsModel_);
				ChannelsFilterModel_->setFilterKeyColumn (0);
			
				ItemModel_ = new ItemModel ();
				JobHolderRepresentation_ = new JobHolderRepresentation ();
				qRegisterMetaTypeStreamOperators<Feed> ("Feed");
				qRegisterMetaTypeStreamOperators<Item> ("Item");
			
				QDir dir = QDir::home ();
				if (!dir.cd (".leechcraft/aggregator") &&
						!dir.mkpath (".leechcraft/aggregator"))
				{
					qCritical () << Q_FUNC_INFO << "could not create neccessary "
						"directories for Aggregator";
					return;
				}
			
				StorageBackend::Type type;
				QString strType = XmlSettingsManager::Instance ()->
					property ("StorageType").toString ();
				if (strType == "SQLite")
					type = StorageBackend::SBSQLite;
				else if (strType == "PostgreSQL")
					type = StorageBackend::SBPostgres;
				else
					throw std::runtime_error (qPrintable (QString ("Unknown storage type %1")
								.arg (strType)));
			
				StorageBackend_ = StorageBackend::Create (type);
			
				const int feedsTable = 1;
				const int channelsTable = 1;
				const int itemsTable = 3;
			
				bool tablesOK = true;
			
				if (StorageBackend_->UpdateFeedsStorage (XmlSettingsManager::Instance ()->
						Property (strType + "FeedsTableVersion", feedsTable).toInt (),
						feedsTable))
					XmlSettingsManager::Instance ()->setProperty ("FeedsTableVersion",
							feedsTable);
				else
					tablesOK = false;
			
				if (StorageBackend_->UpdateChannelsStorage (XmlSettingsManager::Instance ()->
						Property (strType + "ChannelsTableVersion", channelsTable).toInt (),
						channelsTable))
					XmlSettingsManager::Instance ()->setProperty ("ChannelsTableVersion",
							channelsTable);
				else
					tablesOK = false;

				if (StorageBackend_->UpdateItemsStorage (XmlSettingsManager::Instance ()->
						Property (strType + "ItemsTableVersion", itemsTable).toInt (),
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
			
				JobHolderRepresentation_->setSourceModel (ChannelsModel_);
			
				CustomUpdateTimer_ = new QTimer (this);
				CustomUpdateTimer_->start (60 * 1000);
				connect (CustomUpdateTimer_,
						SIGNAL (timeout ()),
						this,
						SLOT (handleCustomUpdates ()));
			
				UpdateTimer_ = new QTimer (this);
				UpdateTimer_->setSingleShot (true);
				QDateTime currentDateTime = QDateTime::currentDateTime ();
				QDateTime lastUpdated = XmlSettingsManager::Instance ()->
					Property ("LastUpdateDateTime", currentDateTime).toDateTime ();
				connect (UpdateTimer_, SIGNAL (timeout ()), this, SLOT (updateFeeds ()));
			
				int updateDiff = lastUpdated.secsTo (currentDateTime);
				if ((XmlSettingsManager::Instance ()->property ("UpdateOnStartup").toBool ()) ||
					(updateDiff > XmlSettingsManager::Instance ()->property ("UpdateInterval").toInt () * 60))
						QTimer::singleShot (7000, this, SLOT (updateFeeds ()));
				else
					UpdateTimer_->start (updateDiff * 1000);
			
				QTimer *saveTimer = new QTimer (this);
				saveTimer->start (60 * 1000);
				connect (saveTimer,
						SIGNAL (timeout ()),
						this,
						SLOT (scheduleSave ()));
			
				XmlSettingsManager::Instance ()->RegisterObject ("UpdateInterval", this, "updateIntervalChanged");
				XmlSettingsManager::Instance ()->RegisterObject ("ShowIconInTray", this, "showIconInTrayChanged");
				UpdateUnreadItemsNumber ();
			}
			
			void Core::SetProvider (QObject *provider, const QString& feature)
			{
				Providers_ [feature] = provider;
			}
			
			void Core::AddFeed (const QString& url, const QStringList& tags)
			{
				feeds_urls_t feeds;
				StorageBackend_->GetFeedsURLs (feeds);
				feeds_urls_t::const_iterator pos =
					std::find (feeds.begin (), feeds.end (), url);
				if (pos != feeds.end ())
				{
					emit error (tr ("This feed is already added."));
					return;
				}
			
				QString name = LeechCraft::Util::GetTemporaryName ();
				LeechCraft::DownloadEntity e = LeechCraft::Util::MakeEntity (url.toUtf8 (),
						name, 
						LeechCraft::Internal |
							LeechCraft::DoNotNotifyUser |
							LeechCraft::DoNotSaveInHistory |
							LeechCraft::NotPersistent |
							LeechCraft::DoNotAnnounceEntity);
			
				QStringList tagIds;
				Q_FOREACH (QString tag, tags)
					tagIds << Proxy_->GetTagsManager ()->GetID (tag);

				PendingJob pj =
				{
					PendingJob::RFeedAdded,
					url,
					name,
					tagIds
				};
			
				int id = -1;
				QObject *pr;
				emit delegateEntity (e, &id, &pr);
				if (id == -1)
				{
					emit error (tr ("Job for feed %1 wasn't delegated.")
							.arg (url));
					return;
				}
			
				HandleProvider (pr);
				PendingJobs_ [id] = pj;
			}
			
			void Core::RemoveFeed (const QModelIndex& si, bool representation)
			{
				if (!si.isValid ())
					return;
			
				QModelIndex index;
				if (representation)
					index = JobHolderRepresentation_->mapToSource (si);
				else
					index = ChannelsFilterModel_->mapToSource (si);
			
				ChannelShort channel;
				try
				{
					channel = ChannelsModel_->GetChannelForIndex (index);
				}
				catch (const std::exception&)
				{
					emit error (tr ("Could not remove the feed."));
					return;
				}
			
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
			
				if (!MergeMode_ && feedURL  == CurrentItemsModel_->GetHash ().first)
					CurrentItemsModel_->SetHash (qMakePair (QString (), QString ()));
			
				UpdateUnreadItemsNumber ();
			}
			
			void Core::Selected (const QModelIndex& index)
			{
				QModelIndex mapped = ItemLists_->mapToSource (index);
				static_cast<ItemsListModel*> (*ItemLists_->
						GetModelForRow (index.row ()))->Selected (mapped);
			}
			
			Item_ptr Core::GetItem (const QModelIndex& index) const
			{
				QModelIndex mapped = ItemLists_->mapToSource (index);
				const ItemsListModel *model = static_cast<const ItemsListModel*> (mapped.model ());
				ItemShort item = model->GetItem (mapped);
				return StorageBackend_->GetItem (item.Title_, item.URL_,
						model->GetHash ().first + model->GetHash ().second);
			}
			
			QSortFilterProxyModel* Core::GetChannelsModel () const
			{
				return ChannelsFilterModel_;
			}
			
			QAbstractItemModel* Core::GetItemsModel () const
			{
				return ItemLists_;
			}
			
			IWebBrowser* Core::GetWebBrowser () const
			{
				return qobject_cast<IWebBrowser*> (Providers_ ["webbrowser"]);
			}
			
			void Core::MarkItemAsUnread (const QModelIndex& i)
			{
				QModelIndex mapped = ItemLists_->mapToSource (i);
				static_cast<ItemsListModel*> (*ItemLists_->
						GetModelForRow (i.row ()))->MarkItemAsUnread (mapped);
			}
			
			bool Core::IsItemRead (int item) const
			{
				LeechCraft::Util::MergeModel::const_iterator i = ItemLists_->
					GetModelForRow (item);
				int starting = ItemLists_->GetStartingRow (i);
				return static_cast<ItemsListModel*> (*i)->IsItemRead (item - starting);
			}
			
			bool Core::IsItemCurrent (int item) const
			{
				return !MergeMode_ && CurrentItemsModel_->GetSelectedRow () == item;
			}
			
			void Core::MarkChannelAsRead (const QModelIndex& i)
			{
				MarkChannel (ChannelsFilterModel_->mapToSource (i), false);
			}
			
			void Core::MarkChannelAsUnread (const QModelIndex& i)
			{
				MarkChannel (ChannelsFilterModel_->mapToSource (i), true);
			}
			
			QStringList Core::GetTagsForIndex (int i) const
			{
				try
				{
					QStringList ids = ChannelsModel_->
						GetChannelForIndex (ChannelsModel_->index (i, 0)).Tags_;
					QStringList result;
					Q_FOREACH (QString id, ids)
					{
						QString tag = Proxy_->GetTagsManager ()->GetTag (id);
						if (!tag.isEmpty ())
							result.append (tag);
					}
					return result;
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< "caught"
						<< e.what ();
					return QStringList ();
				}
			}
			
			Core::ChannelInfo Core::GetChannelInfo (const QModelIndex& i) const
			{
				ChannelShort channel;
				try
				{
					channel = ChannelsModel_->GetChannelForIndex (i);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
					return ChannelInfo ();
				}
				ChannelInfo ci;
				ci.URL_ = channel.ParentURL_;
				ci.Link_ = channel.Link_;
			
				Channel_ptr rc = StorageBackend_->
					GetChannel (channel.Title_, channel.ParentURL_);
				ci.Description_ = rc->Description_;
				ci.Author_ = rc->Author_;
				return ci;
			}
			
			QPixmap Core::GetChannelPixmap (const QModelIndex& i) const
			{
				try
				{
					ChannelShort channel = ChannelsModel_->GetChannelForIndex (i);
					Channel_ptr rc = StorageBackend_->
						GetChannel (channel.Title_, channel.ParentURL_);
					return rc->Pixmap_;
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
					return QPixmap ();
				}
			}
			
			void Core::SetTagsForIndex (const QString& tags, const QModelIndex& index)
			{
				try
				{
					ChannelShort channel = ChannelsModel_->GetChannelForIndex (index);
					QStringList tlist = Proxy_->GetTagsManager ()->Split (tags);
					channel.Tags_.clear ();
					Q_FOREACH (QString tag, tlist)
						channel.Tags_.append (Proxy_->GetTagsManager ()->GetID (tag));
					StorageBackend_->UpdateChannel (channel, channel.ParentURL_);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
				}
			}
			
			void Core::UpdateFavicon (const QModelIndex& index)
			{
				try
				{
					ChannelShort channel = ChannelsModel_->GetChannelForIndex (index);
					FetchFavicon (StorageBackend_->GetChannel (channel.Title_,
								channel.ParentURL_));
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
				}
			}
			
			QStringList Core::GetCategories (const QModelIndex& index) const
			{
				ChannelShort cs;
				try
				{
					cs = ChannelsModel_->GetChannelForIndex (index);
				}
				catch (const std::exception& e)
				{
					return QStringList ();
				}
			
				items_shorts_t items;
				StorageBackend_->GetItems (items, cs.ParentURL_ + cs.Title_);
			
				QStringList result;
				for (items_shorts_t::const_iterator i = items.begin (),
						end = items.end (); i != end; ++i)
				{
					QStringList categories = i->Categories_;
					for (QStringList::const_iterator j = categories.begin (),
							endJ = categories.end (); j != endJ; ++j)
						if (!result.contains (*j) && j->size ())
							result << *j;
				}
				std::sort (result.begin (), result.end ());
				return result;
			}
			
			QStringList Core::GetItemCategories (int index) const
			{
				if (!SupplementaryModels_.size ())
					return CurrentItemsModel_->GetCategories (index);
				else
				{
					LeechCraft::Util::MergeModel::const_iterator i = ItemLists_->
						GetModelForRow (index);
					int starting = ItemLists_->GetStartingRow (i);
					return static_cast<ItemsListModel*> (*i)->GetCategories (index - starting);
				}
			}
			
			Feed::FeedSettings Core::GetFeedSettings (const QModelIndex& index) const
			{
				try
				{
					return StorageBackend_->GetFeedSettings (ChannelsModel_->
							GetChannelForIndex (index).ParentURL_);
				}
				catch (const std::exception& e)
				{
					emit error (tr ("Could not get feed settings"));
					return Feed::FeedSettings ();
				}
			}
			
			void Core::SetFeedSettings (const Feed::FeedSettings& settings,
					const QModelIndex& index)
			{
				try
				{
					StorageBackend_->SetFeedSettings (ChannelsModel_->
							GetChannelForIndex (index).ParentURL_, settings);
				}
				catch (const std::exception& e)
				{
					emit error (tr ("Could not update feed settings"));
				}
			}
			
			void Core::UpdateFeed (const QModelIndex& si, bool isRepr)
			{
				QModelIndex index;
				if (isRepr)
					index = JobHolderRepresentation_->mapToSource (si);
				else
					index = ChannelsFilterModel_->mapToSource (si);
			
				ChannelShort channel;
				try
				{
					channel = ChannelsModel_->
						GetChannelForIndex (index);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ()
						<< si
						<< index
						<< isRepr;
					emit error (tr ("Could not update feed"));
					return;
				}
				QString url = channel.ParentURL_;
				if (url.isEmpty ())
				{
					qWarning () << Q_FUNC_INFO << "could not found feed for index" << index;
					return;
				}
				UpdateFeed (url);
			}
			
			QModelIndex Core::GetUnreadChannelIndex () const
			{
				return ChannelsFilterModel_->
					mapFromSource (ChannelsModel_->GetUnreadChannelIndex ());
			}
			
			int Core::GetUnreadChannelsNumber () const
			{
				return ChannelsModel_->GetUnreadChannelsNumber ();
			}
			
			void Core::AddToItemBucket (const QModelIndex& index) const
			{
				ItemModel_->AddItem (GetItem (index));
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
			
				QStringList tagsList = Proxy_->GetTagsManager ()->Split (tags);
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
			
				QByteArray buffer;
				QDataStream data (&buffer, QIODevice::WriteOnly);
			
				int version = 1;
				int magic = 0xd34df00d;
				data <<
					magic <<
					version <<
					title <<
					owner <<
					ownerEmail;
			
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
			
				f.write (qCompress (buffer, 9));
			}
			
			JobHolderRepresentation* Core::GetJobHolderRepresentation () const
			{
				return JobHolderRepresentation_;
			}
			
			ItemModel* Core::GetItemModel () const
			{
				return ItemModel_;
			}
			
			StorageBackend* Core::GetStorageBackend () const
			{
				return StorageBackend_.get ();
			}
			
			void Core::SubscribeToComments (const QModelIndex& index)
			{
				Item_ptr it = GetItem (index);
				QString commentRSS = it->CommentsLink_;
				QStringList tags = it->Categories_;
			
				QStringList addTags = Proxy_->GetTagsManager ()->
					Split (XmlSettingsManager::Instance ()->
							property ("CommentsTags").toString ());
				AddFeed (commentRSS, tags + addTags);
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
				QStringList tags = Proxy_->GetTagsManager ()->Split (tagsString);
			
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
			
			void Core::SetMerge (bool merge)
			{
				MergeMode_ = merge;
				if (MergeMode_)
				{
					for (int i = 0, size = ChannelsFilterModel_->rowCount ();
							i < size; ++i)
					{
						QModelIndex index = ChannelsFilterModel_->index (i, 0);
						ChannelShort cs;
						try
						{
							cs = ChannelsModel_->
								GetChannelForIndex (ChannelsFilterModel_->mapToSource (index));
						}
						catch (const std::exception& e)
						{
							qWarning () << Q_FUNC_INFO
								<< e.what ();
							continue;
						}
						QPair<QString, QString> hash = qMakePair (cs.ParentURL_, cs.Title_);
			
						if (hash == CurrentItemsModel_->GetHash ())
							continue;
			
						boost::shared_ptr<ItemsListModel> ilm (new ItemsListModel);
						ilm->Reset (hash);
						SupplementaryModels_ << ilm;
						ItemLists_->AddModel (ilm.get ());
					}
				}
				else
					while (SupplementaryModels_.size ())
					{
						ItemLists_->RemoveModel (SupplementaryModels_.at (0).get ());
						SupplementaryModels_.removeAt (0);
					}
			}
			
			void Core::CurrentChannelChanged (const QModelIndex& si, bool repr)
			{
				if (MergeMode_)
					return;
			
				QModelIndex index;
				if (repr)
				{
					index = JobHolderRepresentation_->mapToSource (si);
					JobHolderRepresentation_->SelectionChanged (si);
				}
				else
					index = ChannelsFilterModel_->mapToSource (si);
				try
				{
					ChannelShort ch = ChannelsModel_->GetChannelForIndex (index);
					CurrentItemsModel_->Reset (qMakePair (ch.ParentURL_, ch.Title_));
				}
				catch (const std::exception&)
				{
					CurrentItemsModel_->Reset (qMakePair (QString (), QString ()));
				}
				emit currentChannelChanged (index);
			}
			
			void Core::scheduleSave ()
			{
				if (SaveScheduled_)
					return;
				QTimer::singleShot (500, this, SLOT (saveSettings ()));
				SaveScheduled_ = true;
			}
			
			void Core::openLink (const QString& url)
			{
				if (!Providers_.contains ("webbrowser") ||
						XmlSettingsManager::Instance ()->
							property ("AlwaysUseExternalBrowser").toBool ())
				{
					QDesktopServices::openUrl (QUrl (url));
					return;
				}
				IWebBrowser *browser = GetWebBrowser ();
				if (!browser)
				{
					emit error (tr ("Provided web browser is wrong web browser."));
					return;
				}
				browser->Open (url);
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
			
			namespace
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
			};
			
			void Core::handleJobFinished (int id)
			{
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
						StorageBackend_->GetItems (channel2push->Items_,
								pj.URL_ + channel2push->Title_);
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
			
						std::for_each (channels [i]->Items_.begin (),
								channels [i]->Items_.end (),
								boost::bind (&RegexpMatcherManager::HandleItem,
									&RegexpMatcherManager::Instance (),
									_1));
					}
			
					Feed_ptr feed (new Feed ());
					feed->Channels_ = channels;
					feed->URL_ = pj.URL_;
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
			
			void Core::handleJobError (int id, IDownload::Error ie)
			{
				if (!PendingJobs_.contains (id))
					return;
			
				PendingJob pj = PendingJobs_ [id];
				FileRemoval file (pj.Filename_);
			
				if ((!XmlSettingsManager::Instance ()->property ("BeSilent").toBool () &&
							pj.Role_ == PendingJob::RFeedUpdated) ||
						pj.Role_ == PendingJob::RFeedAdded)
				{
					QString msg;
					switch (ie)
					{
						case IDownload::ENotFound:
							msg = tr ("Address not found:<br />%1");
							break;
						case IDownload::EAccessDenied:
							msg = tr ("Access denied:<br />%1");
							break;
						case IDownload::ELocalError:
							msg = tr ("Local errro for:<br />%1");
							break;
						default:
							msg = tr ("Unknown error for:<br />%1");
							break;
					}
					emit error (msg.arg (pj.URL_));
				}
				PendingJobs_.remove (id);
			}
			
			void Core::updateFeeds ()
			{
				feeds_urls_t urls;
				StorageBackend_->GetFeedsURLs (urls);
				for (feeds_urls_t::const_iterator i = urls.begin (),
						end = urls.end (); i != end; ++i)
				{
					// It's handled by custom timer.
					if (StorageBackend_->GetFeedSettings (*i).UpdateTimeout_)
						continue;
			
					UpdateFeed (*i);
				}
				XmlSettingsManager::Instance ()->setProperty ("LastUpdateDateTime", QDateTime::currentDateTime ());
				UpdateTimer_->start (XmlSettingsManager::Instance ()->property ("UpdateInterval").toInt () * 60 * 1000);
			}
			
			void Core::fetchExternalFile (const QString& url, const QString& where)
			{
				LeechCraft::DownloadEntity e = LeechCraft::Util::MakeEntity (url.toUtf8 (),
						where,
						LeechCraft::Internal |
							LeechCraft::DoNotNotifyUser |
							LeechCraft::DoNotSaveInHistory |
							LeechCraft::NotPersistent |
							LeechCraft::DoNotAnnounceEntity);
			
				PendingJob pj =
				{
					PendingJob::RFeedExternalData,
					url,
					where,
					QStringList ()
				};
			
				int id = -1;
				QObject *pr;
				emit delegateEntity (e, &id, &pr);
				if (id == -1)
				{
					if (!XmlSettingsManager::Instance ()->property ("BeSilent").toBool ())
						emit error (tr ("External file %1 wasn't delegated.").arg (url));
					return;
				}
			
				HandleProvider (pr);
				PendingJobs_ [id] = pj;
			}
			
			void Core::saveSettings ()
			{
				SaveScheduled_ = false;
			}
			
			void Core::handleChannelDataUpdated (Channel_ptr channel)
			{
				ChannelShort cs = channel->ToShort ();
			
				cs.Unread_ = StorageBackend_->GetUnreadItems (cs.ParentURL_, cs.Title_);
				ChannelsModel_->UpdateChannelData (cs);
				UpdateUnreadItemsNumber ();
			
				QPair<QString, QString> newHash = qMakePair (cs.ParentURL_, cs.Title_);
			}
			
			void Core::handleItemDataUpdated (Item_ptr item, Channel_ptr channel)
			{
				if (MergeMode_ ||
						(qMakePair (channel->ParentURL_, channel->Title_) !=
						 CurrentItemsModel_->GetHash ()))
					return;
			
				CurrentItemsModel_->ItemDataUpdated (item);
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
			
			void Core::tagsUpdated ()
			{
				if (MergeMode_)
				{
					SetMerge (false);
					SetMerge (true);
				}
			}
			
			void Core::handleCustomUpdates ()
			{
				feeds_urls_t urls;
				StorageBackend_->GetFeedsURLs (urls);
				QDateTime current = QDateTime::currentDateTime ();
				for (feeds_urls_t::const_iterator i = urls.begin (),
						end = urls.end (); i != end; ++i)
				{
					int ut = StorageBackend_->GetFeedSettings (*i).UpdateTimeout_;
					// It's handled by normal timer.
					if (!ut)
						continue;
			
					if (!Updates_.contains (*i) ||
							(Updates_ [*i].isValid () &&
							 Updates_ [*i].secsTo (current) / 60 > ut))
						UpdateFeed (*i);
				}
			}
			
			void Core::UpdateUnreadItemsNumber () const
			{
				emit unreadNumberChanged (ChannelsModel_->GetUnreadItemsNumber ());
			}
			
			void Core::FetchPixmap (const Channel_ptr& channel)
			{
				if (QUrl (channel->PixmapURL_).isValid () &&
						!QUrl (channel->PixmapURL_).isRelative ())
				{
					ExternalData data;
					data.Type_ = ExternalData::TImage;
					data.RelatedChannel_ = channel;
					QString exFName = LeechCraft::Util::GetTemporaryName ();
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
				QString exFName = LeechCraft::Util::GetTemporaryName ();
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
							continue;
			
						if (!IsModified (*ourItem, *item))
							continue;
			
						(*ourItem)->Description_ = (*item)->Description_;
						(*ourItem)->Categories_ = (*item)->Categories_;
						(*ourItem)->NumComments_ = (*item)->NumComments_;
						(*ourItem)->CommentsLink_ = (*item)->CommentsLink_;
						(*ourItem)->CommentsPageLink_ = (*item)->CommentsPageLink_;
			
						StorageBackend_->UpdateItem ((*ourItem),
								(*position)->ParentURL_, (*position)->Title_);
					}
				}
			
				if (!channels.size ())
					return emitString;
			
				Feed::FeedSettings settings = StorageBackend_->
					GetFeedSettings (channels [0]->ParentURL_);
				const int days = settings.ItemAge_ ? settings.ItemAge_ :
					XmlSettingsManager::Instance ()->property ("ItemsMaxAge").toInt ();
				const unsigned ipc = settings.NumItems_ ? settings.NumItems_ :
					XmlSettingsManager::Instance ()->property ("ItemsPerChannel").value<unsigned> ();
			
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
			
					const QDateTime current = QDateTime::currentDateTime ();
			
					if (position == ourChannels.end ())
					{
						size_t truncateAt = ((*i)->Items_.size () <= ipc) ?
							(*i)->Items_.size () : ipc;
						for (size_t j = 0; j < (*i)->Items_.size (); j++)
							if ((*i)->Items_ [j]->PubDate_.daysTo (current) > days)
							{
			 					truncateAt = std::min (j, truncateAt);
								break;
							}
						(*i)->Items_.resize (truncateAt);
			
						ChannelsModel_->AddChannel ((*i)->ToShort ());
						StorageBackend_->AddChannel (*i, pj.URL_);
						emitString += tr ("Added channel \"%1\" (has %2 items)")
							.arg ((*i)->Title_)
							.arg ((*i)->Items_.size ());
					}
					else
					{
						if ((*i)->LastBuild_.isValid ())
							(*position)->LastBuild_ = (*i)->LastBuild_;
						else 
							(*position)->LastBuild_ = QDateTime::currentDateTime ();
			
						for (size_t j = 0; j < (*i)->Items_.size (); j++)
							if ((*i)->Items_ [j]->PubDate_.daysTo (current) > days)
							{
			 					(*i)->Items_.resize (j);
								break;
							}
			
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
			
						// Now cut off old and overwhelming items.
						unsigned removeFrom = (*position)->Items_.size ();
						for (size_t j = 0; j < (*position)->Items_.size (); ++j)
							if ((*position)->Items_ [j]->PubDate_.daysTo (current) > days)
							{
								removeFrom = j;
								break;
							}
						/*if (removeFrom == 0)
							removeFrom = 1;*/
			
						removeFrom = std::min (removeFrom, ipc);
			
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
				try
				{
					ChannelShort cs = ChannelsModel_->GetChannelForIndex (i);
			
					QString hash = cs.ParentURL_ + cs.Title_;
					StorageBackend_->ToggleChannelUnread (cs.ParentURL_, cs.Title_, state);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
					emit error (tr ("Could not mark channel"));
				}
			}
			
			void Core::UpdateFeed (const QString& url)
			{
				QString filename = LeechCraft::Util::GetTemporaryName ();
			
				LeechCraft::DownloadEntity e = LeechCraft::Util::MakeEntity (url.toUtf8 (),
						filename,
						LeechCraft::Internal |
							LeechCraft::DoNotNotifyUser |
							LeechCraft::DoNotSaveInHistory |
							LeechCraft::NotPersistent |
							LeechCraft::DoNotAnnounceEntity);
			
				PendingJob pj =
				{
					PendingJob::RFeedUpdated,
					url,
					filename,
					QStringList ()
				};
			
				int id = -1;
				QObject *pr;
				emit delegateEntity (e, &id, &pr);
				if (id == -1)
				{
					qWarning () << Q_FUNC_INFO << url << "wasn't deleagated";
					return;
				}
			
				HandleProvider (pr);
				PendingJobs_ [id] = pj;
				Updates_ [url] = QDateTime::currentDateTime ();
			}
			
			void Core::HandleProvider (QObject *provider)
			{
				if (Downloaders_.contains (provider))
					return;
			
				Downloaders_ << provider;
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
		};
	};
};

