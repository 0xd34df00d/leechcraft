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

#include <stdexcept>
#include <numeric>
#include <boost/bind.hpp>
#include <QtDebug>
#include <QImage>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QTextCodec>
#include <QNetworkReply>
#include <interfaces/iwebbrowser.h>
#include <plugininterface/mergemodel.h>
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
#include "importopml.h"
#include "addfeed.h"
#include "itemswidget.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			Core::Core ()
			: SaveScheduled_ (false)
			, ChannelsModel_ (0)
			, JobHolderRepresentation_ (0)
			, ChannelsFilterModel_ (0)
			, Initialized_ (false)
			{
				qRegisterMetaType<QItemSelection> ("QItemSelection");
				qRegisterMetaType<Item> ("LeechCraft::Plugins::Aggregator::Item");
				qRegisterMetaType<Enclosure> ("LeechCraft::Plugins::Aggregator::Enclosure");

				qRegisterMetaType<Item_ptr> ("Item_ptr");
				qRegisterMetaType<Channel_ptr> ("Channel_ptr");

				qRegisterMetaTypeStreamOperators<Feed> ("LeechCraft::Plugins::Aggregator::Feed");
				qRegisterMetaTypeStreamOperators<Item> ("LeechCraft::Plugins::Aggregator::Item");
				qRegisterMetaTypeStreamOperators<Enclosure> ("LeechCraft::Plugins::Aggregator::Enclosure");
			}
			
			Core& Core::Instance ()
			{
				static Core core;
				return core;
			}
			
			void Core::Release ()
			{
				XmlSettingsManager::Instance ()->Release ();
				StorageBackend_.reset ();
				delete JobHolderRepresentation_;
				delete ChannelsFilterModel_;
				delete ChannelsModel_;
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
				if (!e.Entity_.canConvert<QUrl> () ||
						!Initialized_)
					return false;

				QUrl url = e.Entity_.toUrl ();

				if (e.Mime_ == "text/x-opml")
				{
					if (url.scheme () != "file" &&
							url.scheme () != "http" &&
							url.scheme () != "https")
						return false;
				}
				else
				{
					if (url.scheme () == "feed")
						return true;

					if (url.scheme () != "http" &&
							url.scheme () != "https")
						return false;
				
					if (e.Mime_ != "application/atom+xml" &&
							e.Mime_ != "application/rss+xml")
						return false;
				}
			
				return true;
			}

			void Core::Handle (LeechCraft::DownloadEntity e)
			{
				QUrl url = e.Entity_.toUrl ();
				if (e.Mime_ == "text/x-opml")
				{
					if (url.scheme () == "file")
						StartAddingOPML (url.toLocalFile ());
					else
					{
						QString name = LeechCraft::Util::GetTemporaryName ();

						LeechCraft::DownloadEntity e = Util::MakeEntity (url,
								name, 
								LeechCraft::Internal |
									LeechCraft::DoNotNotifyUser |
									LeechCraft::DoNotSaveInHistory |
									LeechCraft::NotPersistent |
									LeechCraft::DoNotAnnounceEntity);
						PendingOPML po =
						{
							name
						};
					
						int id = -1;
						QObject *pr;
						emit delegateEntity (e, &id, &pr);
						if (id == -1)
						{
							emit error (tr ("Task for OPML %1 wasn't delegated.")
									.arg (url.toString ()));
							return;
						}
					
						HandleProvider (pr, id);
						PendingOPMLs_ [id] = po;
					}

					QMap<QString, QVariant> s = e.Additional_;
					if (s.contains ("ShowTrayIcon"))
						XmlSettingsManager::Instance ()->setProperty ("ShowIconInTray",
								s.value ("ShowIconInTray").toBool ());
					if (s.contains ("UpdateOnStartup"))
						XmlSettingsManager::Instance ()->setProperty ("UpdateOnStartup",
								s.value ("UpdateOnStartup").toBool ());
					if (s.contains ("UpdateTimeout"))
						XmlSettingsManager::Instance ()->setProperty ("UpdateInterval",
								s.value ("UpdateTimeout").toInt ());
					if (s.contains ("MaxArticles"))
						XmlSettingsManager::Instance ()->setProperty ("ItemsPerChannel",
								s.value ("MaxArticles").toInt ());
					if (s.contains ("MaxAge"))
						XmlSettingsManager::Instance ()->setProperty ("ItemsMaxAge",
								s.value ("MaxAge").toInt ());
				}
				else
				{
					QString str = url.toString ();
					if (str.startsWith ("feed://"))
						str.replace (0, 4, "http");
					else if (str.startsWith ("feed:"))
						str.remove  (0, 5);

					LeechCraft::Plugins::Aggregator::AddFeed af (str);
					if (af.exec () == QDialog::Accepted)
						AddFeed (af.GetURL (),
								af.GetTags ());
				}
			}
			
			void Core::StartAddingOPML (const QString& file)
			{
				ImportOPML importDialog (file);
				if (importDialog.exec () == QDialog::Rejected)
					return;
			
				AddFromOPML (importDialog.GetFilename (),
						importDialog.GetTags (),
						importDialog.GetMask ());
			}

			void Core::SetAppWideActions (const AppWideActions& aw)
			{
				AppWideActions_ = aw;
			}

			const AppWideActions& Core::GetAppWideActions () const
			{
				return AppWideActions_;
			}
			
			bool Core::DoDelayedInit ()
			{
				QDir dir = QDir::home ();
				if (!dir.cd (".leechcraft/aggregator") &&
						!dir.mkpath (".leechcraft/aggregator"))
				{
					qCritical () << Q_FUNC_INFO << "could not create neccessary "
						"directories for Aggregator";
					return false;
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
			
				try
				{
					StorageBackend_ = StorageBackend::Create (type);
				}
				catch (const std::runtime_error& s)
				{
					emit error (QTextCodec::codecForName ("UTF-8")->
							toUnicode (s.what ()));
					return false;
				}
				catch (...)
				{
					emit error (tr ("Aggregator: general storage initialization error."));
					return false;
				}

				ItemBucket_.reset (new ItemBucket ());

				ChannelsModel_ = new ChannelsModel ();
				ChannelsFilterModel_ = new ChannelsFilterModel ();
				ChannelsFilterModel_->setSourceModel (ChannelsModel_);
				ChannelsFilterModel_->setFilterKeyColumn (0);
			
				JobHolderRepresentation_ = new JobHolderRepresentation ();
			
				const int feedsTable = 1;
				const int channelsTable = 1;
				const int itemsTable = 5;
			
				bool tablesOK = true;
			
				if (StorageBackend_->UpdateFeedsStorage (XmlSettingsManager::Instance ()->
						Property (strType + "FeedsTableVersion", feedsTable).toInt (),
						feedsTable))
					XmlSettingsManager::Instance ()->setProperty (qPrintable (strType + "FeedsTableVersion"),
							feedsTable);
				else
					tablesOK = false;
			
				if (StorageBackend_->UpdateChannelsStorage (XmlSettingsManager::Instance ()->
						Property (strType + "ChannelsTableVersion", channelsTable).toInt (),
						channelsTable))
					XmlSettingsManager::Instance ()->setProperty (qPrintable (strType + "ChannelsTableVersion"),
							channelsTable);
				else
					tablesOK = false;

				if (StorageBackend_->UpdateItemsStorage (XmlSettingsManager::Instance ()->
						Property (strType + "ItemsTableVersion", itemsTable).toInt (),
						itemsTable))
					XmlSettingsManager::Instance ()->setProperty (qPrintable (strType + "ItemsTableVersion"),
							itemsTable);
				else
					tablesOK = false;
			
				StorageBackend_->Prepare ();
			
				connect (StorageBackend_.get (),
						SIGNAL (channelDataUpdated (Channel_ptr)),
						this,
						SLOT (handleChannelDataUpdated (Channel_ptr)),
						Qt::QueuedConnection);
			
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

				ReprWidget_ = new ItemsWidget ();
				ReprWidget_->SetChannelsFilter (JobHolderRepresentation_);
				ChannelsModel_->SetWidgets (ReprWidget_->GetToolBar (), ReprWidget_);
			
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
				connect (UpdateTimer_,
						SIGNAL (timeout ()),
						this,
						SLOT (updateFeeds ()));
			
				int updateDiff = lastUpdated.secsTo (currentDateTime);
				if ((XmlSettingsManager::Instance ()->
							property ("UpdateOnStartup").toBool ()) ||
					(updateDiff > XmlSettingsManager::Instance ()->
							property ("UpdateInterval").toInt () * 60))
						QTimer::singleShot (7000,
								this,
								SLOT (updateFeeds ()));
				else
					UpdateTimer_->start (updateDiff * 1000);
			
				QTimer *saveTimer = new QTimer (this);
				saveTimer->start (60 * 1000);
				connect (saveTimer,
						SIGNAL (timeout ()),
						this,
						SLOT (scheduleSave ()));
			
				XmlSettingsManager::Instance ()->
					RegisterObject ("UpdateInterval", this, "updateIntervalChanged");
				XmlSettingsManager::Instance ()->
					RegisterObject ("ShowIconInTray", this, "showIconInTrayChanged");
				UpdateUnreadItemsNumber ();
				Initialized_ = true;
				return true;
			}

			void Core::AddFeed (const QString& url, const QString& tagString)
			{
				AddFeed (url, Proxy_->GetTagsManager ()->Split (tagString));
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
				LeechCraft::DownloadEntity e = LeechCraft::Util::MakeEntity (QUrl (url),
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
			
				HandleProvider (pr, id);
				PendingJobs_ [id] = pj;
			}
			
			void Core::RemoveFeed (const QModelIndex& index)
			{
				if (!index.isValid ())
					return;
			
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
			
				UpdateUnreadItemsNumber ();
			}
			
			ItemsWidget* Core::GetReprWidget () const
			{
				return ReprWidget_;
			}

			ItemBucket* Core::GetItemBucket () const
			{
				return ItemBucket_.get ();
			}
			
			ChannelsModel* Core::GetRawChannelsModel () const
			{
				return ChannelsModel_;
			}
			
			QSortFilterProxyModel* Core::GetChannelsModel () const
			{
				return ChannelsFilterModel_;
			}
			
			IWebBrowser* Core::GetWebBrowser () const
			{
				IPluginsManager *pm = Proxy_->GetPluginsManager ();
				QObjectList browsers = pm->Filter<IWebBrowser*> (pm->GetAllPlugins ());
				return browsers.size () ?
					qobject_cast<IWebBrowser*> (browsers.at (0)) :
					0;
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
				QModelIndex index = si;
			
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
				{
					AddFeed (i->URL_, tagsList + i->Categories_);

					try
					{
						int interval = 0;
						if (i->CustomFetchInterval_)
							interval = i->FetchInterval_;
						Feed::FeedSettings s (interval, i->MaxArticleNumber_, i->MaxArticleAge_);
						StorageBackend_->SetFeedSettings (i->URL_, s);
					}
					catch (const std::exception& e)
					{
						emit error (tr ("Could not update feed settings"));
					}
				}
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
			
			StorageBackend* Core::GetStorageBackend () const
			{
				return StorageBackend_.get ();
			}
			
			QWebView* Core::CreateWindow ()
			{
				IWebBrowser *browser = GetWebBrowser ();
				if (!browser)
					return 0;
			
				return browser->CreateWindow ();
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
			
			void Core::SetContextMenu (QMenu *menu)
			{
				ChannelsModel_->SetMenu (menu);
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
				IWebBrowser *browser = GetWebBrowser ();
				if (!browser ||
						XmlSettingsManager::Instance ()->
							property ("AlwaysUseExternalBrowser").toBool ())
				{
					QDesktopServices::openUrl (QUrl (url));
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
				{
					if (!PendingOPMLs_.contains (id))
						return;

					StartAddingOPML (PendingOPMLs_ [id].Filename_);
					PendingOPMLs_.remove (id);
				}
				PendingJob pj = PendingJobs_ [id];
				PendingJobs_.remove (id);
				ID2Downloader_.remove (id);
			
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
				{
					PendingJobs_.remove (id);
					ID2Downloader_.remove (id);
				}
				if (PendingOPMLs_.contains (id))
					PendingOPMLs_.remove (id);
			}
			
			void Core::handleJobError (int id, IDownload::Error ie)
			{
				if (!PendingJobs_.contains (id))
				{
					if (PendingOPMLs_.contains (id))
					{
						emit error (tr ("Unable to download the OPML file."));
						return;
					}
				}
			
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
							msg = tr ("Local error for:<br />%1");
							break;
						default:
							msg = tr ("Unknown error for:<br />%1");
							break;
					}
					emit error (msg.arg (pj.URL_));
				}
				PendingJobs_.remove (id);
				ID2Downloader_.remove (id);
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
				XmlSettingsManager::Instance ()->
					setProperty ("LastUpdateDateTime", QDateTime::currentDateTime ());
				UpdateTimer_->start (XmlSettingsManager::Instance ()->
						property ("UpdateInterval").toInt () * 60 * 1000);
			}
			
			void Core::fetchExternalFile (const QString& url, const QString& where)
			{
				LeechCraft::DownloadEntity e = LeechCraft::Util::MakeEntity (QUrl (url),
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
			
				HandleProvider (pr, id);
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
			}
			
			void Core::updateIntervalChanged ()
			{
				UpdateTimer_->setInterval (XmlSettingsManager::Instance ()->
						property ("UpdateInterval").toInt () * 60 * 1000);
			}
			
			void Core::showIconInTrayChanged ()
			{
				UpdateUnreadItemsNumber ();
			}
			
			void Core::handleSslError (QNetworkReply *reply)
			{
				reply->ignoreSslErrors ();
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
						(*ourItem)->Latitude_ = (*item)->Latitude_;
						(*ourItem)->Longitude_ = (*item)->Longitude_;
						(*ourItem)->MRSSEntries_ = (*item)->MRSSEntries_;
			
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
					std::for_each ((*i)->Items_.begin (), (*i)->Items_.end (),
							boost::bind (&RegexpMatcherManager::HandleItem,
								&RegexpMatcherManager::Instance (),
								_1));

					if (settings.AutoDownloadEnclosures_)
						Q_FOREACH (Item_ptr item, (*i)->Items_)
							Q_FOREACH (Enclosure e, item->Enclosures_)
							{
								DownloadEntity de = Util::MakeEntity (QUrl (e.URL_),
										XmlSettingsManager::Instance ()->
											property ("EnclosuresDownloadPath").toString (),
										0,
										e.Type_);
								de.Additional_ [" Tags"] = (*i)->Tags_;
								emit gotEntity (de);
							}
			
					channels_container_t::const_iterator position =
						std::find_if (ourChannels.begin (), ourChannels.end (),
								ChannelFinder (*i));
			
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

						int maxNew = ipc - (*position)->Items_.size ();
						if (static_cast<int> ((*i)->Items_.size ()) > maxNew)
							(*i)->Items_.resize (maxNew >= 0 ? maxNew : 0);
			
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
				QList<int> keys = PendingJobs_.keys ();
				Q_FOREACH (int key, keys)
					if (PendingJobs_ [key].URL_ == url)
					{
						QObject *provider = ID2Downloader_ [key];
						IDownload *downloader = qobject_cast<IDownload*> (provider);
						if (downloader)
						{
							qWarning () << Q_FUNC_INFO
								<< "stalled task detected from"
								<< downloader
								<< "trying to kill...";
							downloader->KillTask (key);
							ID2Downloader_.remove (key);
							PendingJobs_.remove (key);
							qWarning () << Q_FUNC_INFO
								<< "killed!";
						}
						else
							qWarning () << Q_FUNC_INFO
								<< "provider is not a downloader:"
								<< provider
								<< "; cannot kill the task";
						return;
					}

				QString filename = LeechCraft::Util::GetTemporaryName ();

				LeechCraft::DownloadEntity e = LeechCraft::Util::MakeEntity (QUrl (url),
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
			
				HandleProvider (pr, id);
				PendingJobs_ [id] = pj;
				Updates_ [url] = QDateTime::currentDateTime ();
			}
			
			void Core::HandleProvider (QObject *provider, int id)
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

				ID2Downloader_ [id] = provider;
			}
		};
	};
};

