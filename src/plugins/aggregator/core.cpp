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
#include <boost/foreach.hpp>
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
#include <plugininterface/fileremoveguard.h>
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
#include "opmlparser.h"
#include "opmlwriter.h"
#include "sqlstoragebackend.h"
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
				qRegisterMetaType<Item_ptr> ("Item_ptr");
				qRegisterMetaType<Channel_ptr> ("Channel_ptr");
				qRegisterMetaTypeStreamOperators<Feed> ("LeechCraft::Plugins::Aggregator::Feed");
			}

			Core& Core::Instance ()
			{
				static Core core;
				return core;
			}

			void Core::Release ()
			{
				delete JobHolderRepresentation_;
				delete ChannelsFilterModel_;
				delete ChannelsModel_;
				delete ReprWidget_;

				StorageBackend_.reset ();
				XmlSettingsManager::Instance ()->Release ();
			}

			void Core::SetProxy (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;
			}

			ICoreProxy_ptr Core::GetProxy () const
			{
				return Proxy_;
			}

			Util::IDPool<IDType_t>& Core::GetPool (PoolType type)
			{
				return Pools_ [type];
			}

			bool Core::CouldHandle (const LeechCraft::Entity& e)
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

					QString linkRel = e.Additional_ ["LinkRel"].toString ();
					if (linkRel.size () &&
							linkRel != "alternate")
						return false;
				}

				return true;
			}

			void Core::Handle (LeechCraft::Entity e)
			{
				QUrl url = e.Entity_.toUrl ();
				if (e.Mime_ == "text/x-opml")
				{
					if (url.scheme () == "file")
						StartAddingOPML (url.toLocalFile ());
					else
					{
						QString name = LeechCraft::Util::GetTemporaryName ();

						LeechCraft::Entity e = Util::MakeEntity (url,
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
							ErrorNotification (tr ("Import error"),
									tr ("Could not find plugin to download OPML %1.")
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
				else if (strType == "MySQL")
					type = StorageBackend::SBMysql;
				else
					throw std::runtime_error (qPrintable (QString ("Unknown storage type %1")
								.arg (strType)));

				try
				{
					StorageBackend_ = StorageBackend::Create (type);
				}
				catch (const std::runtime_error& s)
				{
					ErrorNotification (tr ("Storage error"),
							QTextCodec::codecForName ("UTF-8")->
							toUnicode (s.what ()));
					return false;
				}
				catch (...)
				{
					ErrorNotification (tr ("Storage error"),
							tr ("Aggregator: general storage initialization error."));
					return false;
				}

				ChannelsModel_ = new ChannelsModel ();
				ChannelsFilterModel_ = new ChannelsFilterModel ();
				ChannelsFilterModel_->setSourceModel (ChannelsModel_);
				ChannelsFilterModel_->setFilterKeyColumn (0);

				JobHolderRepresentation_ = new JobHolderRepresentation ();

				const int feedsTable = 1;
				const int channelsTable = 1;
				const int itemsTable = 6;

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
					ids_t feeds;
					StorageBackend_->GetFeedsIDs (feeds);
					Q_FOREACH (IDType_t feedId, feeds)
					{
						channels_shorts_t channels;
						StorageBackend_->GetChannels (channels, feedId);
						std::for_each (channels.begin (), channels.end (),
								boost::bind (&ChannelsModel::AddChannel,
									ChannelsModel_,
									_1));
					}

					for (int type = 0; type < PTMAX; ++type)
					{
						Util::IDPool<IDType_t> pool;
						pool.SetID (StorageBackend_->GetHighestID (static_cast<PoolType> (type)) + 1);
						Pools_ [static_cast<PoolType> (type)] = pool;
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
				int interval = XmlSettingsManager::Instance ()->
					property ("UpdateInterval").toInt ();
				if (interval)
				{
					if ((XmlSettingsManager::Instance ()->
								property ("UpdateOnStartup").toBool ()) ||
							(updateDiff > interval * 60))
						QTimer::singleShot (7000,
								this,
								SLOT (updateFeeds ()));
					else
						UpdateTimer_->start (updateDiff * 1000);
				}

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

			void Core::AddFeed (const QString& url, const QStringList& tags,
					boost::shared_ptr<Feed::FeedSettings> fs)
			{
				if (StorageBackend_->FindFeed (url) != static_cast<IDType_t> (-1))
				{
					ErrorNotification (tr ("Feed addition error"),
							tr ("The feed %1 is already added")
							.arg (url));
					return;
				}

				QString name = LeechCraft::Util::GetTemporaryName ();
				LeechCraft::Entity e = LeechCraft::Util::MakeEntity (QUrl (url),
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
					tagIds,
					fs
				};

				int id = -1;
				QObject *pr;
				emit delegateEntity (e, &id, &pr);
				if (id == -1)
				{
					ErrorNotification (tr ("Plugin error"),
							tr ("Could not find plugin to download feed %1.")
								.arg (url),
							false);
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
				catch (const std::exception& e)
				{
					ErrorNotification (tr ("Feed removal error"),
							tr ("Could not remove the feed: %1")
							.arg (e.what ()));
					return;
				}

				channels_shorts_t shorts;
				StorageBackend_->GetChannels (shorts, channel.FeedID_);

				for (size_t i = 0, size = shorts.size (); i < size; ++i)
					ChannelsModel_->RemoveChannel (shorts [i]);
				StorageBackend_->RemoveFeed (channel.FeedID_);

				UpdateUnreadItemsNumber ();
			}

			ItemsWidget* Core::GetReprWidget () const
			{
				return ReprWidget_;
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
				ci.FeedID_ = channel.FeedID_;
				ci.ChannelID_ = channel.ChannelID_;
				ci.Link_ = channel.Link_;

				Channel_ptr rc = StorageBackend_->
						GetChannel (channel.ChannelID_, channel.FeedID_);
				ci.Description_ = rc->Description_;
				ci.Author_ = rc->Author_;

				Feed_ptr feed = StorageBackend_->GetFeed (channel.FeedID_);
				ci.URL_ = feed->URL_;

				items_shorts_t items;
				StorageBackend_->GetItems (items, channel.ChannelID_);
				ci.NumItems_ = items.size ();

				return ci;
			}

			QPixmap Core::GetChannelPixmap (const QModelIndex& i) const
			{
				try
				{
					ChannelShort channel = ChannelsModel_->GetChannelForIndex (i);
					Channel_ptr rc = StorageBackend_->
							GetChannel (channel.ChannelID_, channel.FeedID_);
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
					StorageBackend_->UpdateChannel (channel);
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
					FetchFavicon (StorageBackend_->
							GetChannel (channel.ChannelID_, channel.FeedID_));
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
				StorageBackend_->GetItems (items, cs.ChannelID_);

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
							GetChannelForIndex (index).FeedID_);
				}
				catch (const std::exception& e)
				{
					ErrorNotification (tr ("Aggregator error"),
							tr ("Could not get feed settings: %1")
							.arg (e.what ()));
					throw std::runtime_error (QString ("Could not get feed settings, "
							"inner exception: %1").arg (e.what ()).toStdString ());
				}
			}

			void Core::SetFeedSettings (const Feed::FeedSettings& settings,
					const QModelIndex& index)
			{
				try
				{
					StorageBackend_->SetFeedSettings (settings);
				}
				catch (const std::exception& e)
				{
					ErrorNotification (tr ("Aggregator error"),
							tr ("Could not update feed settings: %1")
							.arg (e.what ()));
				}
			}

			void Core::UpdateFeed (const QModelIndex& si, bool isRepr)
			{
				QModelIndex index = si;

				ChannelShort channel;
				try
				{
					channel = ChannelsModel_->GetChannelForIndex (index);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ()
						<< si
						<< index
						<< isRepr;
					ErrorNotification (tr ("Feed update error"),
							tr ("Could not update feed"),
							false);
					return;
				}
				UpdateFeed (channel.FeedID_);
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
					ErrorNotification (tr ("OPML import error"),
							tr ("Could not open file %1 for reading.")
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
					ErrorNotification (tr ("OPML import error"),
							tr ("XML error, file %1, line %2, column %3, error:<br />%4")
								.arg (filename)
								.arg (errorLine)
								.arg (errorColumn)
								.arg (errorMsg));
					return;
				}

				OPMLParser parser (document);
				if (!parser.IsValid ())
				{
					ErrorNotification (tr ("OPML import error"),
							tr ("OPML from file %1 is not valid.")
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
					int interval = 0;
					if (i->CustomFetchInterval_)
						interval = i->FetchInterval_;
					FeedSettings_ptr s (new Feed::FeedSettings (-1, -1,
							interval, i->MaxArticleNumber_, i->MaxArticleAge_, false));

					AddFeed (i->URL_, tagsList + i->Categories_, s);
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
					ErrorNotification (tr ("OPML export error"),
							tr ("Could not open file %1 for write.").arg (where));
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
					ErrorNotification (tr ("Binary export error"),
							tr ("Could not open file %1 for write.").arg (where));
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
					Channel_ptr channel = StorageBackend_->
							GetChannel (i->ChannelID_, i->FeedID_);
					items_shorts_t items;
					StorageBackend_->GetItems (items, channel->ChannelID_);

					for (items_shorts_t::const_iterator j = items.begin (),
							endJ = items.end (); j != endJ; ++j)
						channel->Items_.push_back (StorageBackend_->
								GetItem (j->ItemID_));

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
				ids_t ids;
				StorageBackend_->GetFeedsIDs (ids);
				Q_FOREACH (IDType_t id, ids)
					StorageBackend_->GetChannels (channels, id);
			}

			void Core::AddFeeds (const feeds_container_t& feeds,
					const QString& tagsString)
			{
				QStringList tags = Proxy_->GetTagsManager ()->Split (tagsString);

				Q_FOREACH (Feed_ptr feed, feeds)
				{
					Q_FOREACH (Channel_ptr channel, feed->Channels_)
					{
						Q_FOREACH (QString tag, tags)
							if (!channel->Tags_.contains (tag))
								channel->Tags_ << tag;

						ChannelsModel_->AddChannel (channel->ToShort ());
					}

					StorageBackend_->AddFeed (feed);
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

			void Core::handleJobFinished (int id)
			{
				if (!PendingJobs_.contains (id))
				{
					if (PendingOPMLs_.contains (id))
					{
						StartAddingOPML (PendingOPMLs_ [id].Filename_);
						PendingOPMLs_.remove (id);
					}
					return;
				}
				PendingJob pj = PendingJobs_ [id];
				PendingJobs_.remove (id);
				ID2Downloader_.remove (id);

				Util::FileRemoveGuard file (pj.Filename_);
				if (!file.open (QIODevice::ReadOnly))
				{
					qWarning () << Q_FUNC_INFO << "could not open file for pj " << pj.Filename_;
					return;
				}
				if (!file.size ())
				{
					if (pj.Role_ != PendingJob::RFeedExternalData)
						ErrorNotification (tr ("Feed error"),
								tr ("Downloaded file from url %1 has null size.").arg (pj.URL_));
					return;
				}

				channels_container_t channels;
				if (pj.Role_ != PendingJob::RFeedExternalData)
				{
					QByteArray data = file.readAll ();
					QDomDocument doc;
					QString errorMsg;
					int errorLine, errorColumn;
					if (!doc.setContent (data, true, &errorMsg, &errorLine, &errorColumn))
					{
						file.copy (QDir::tempPath () + "/failedFile.xml");
						ErrorNotification (tr ("Feed error"),
								tr ("XML file parse error: %1, line %2, column %3, filename %4, from %5")
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
						ErrorNotification (tr ("Feed error"),
								tr ("Could not find parser to parse file %1 from %2")
								.arg (pj.Filename_)
								.arg (pj.URL_));
						return;
					}

					if (pj.Role_ == PendingJob::RFeedAdded)
					{
						Feed_ptr feed (new Feed ());
						feed->URL_ = pj.URL_;
						StorageBackend_->AddFeed (feed);
					}

					IDType_t feedId = StorageBackend_->FindFeed (pj.URL_);

					if (feedId == static_cast<IDType_t> (-1))
					{
						ErrorNotification (tr ("Feed error"),
								tr ("Feed with url %1 not found.").arg (pj.URL_));
						return;
					}

					channels = parser->ParseFeed (doc, feedId);
				}

				if (pj.Role_ == PendingJob::RFeedAdded)
					HandleFeedAdded (channels, pj);
				else if (pj.Role_ == PendingJob::RFeedUpdated)
					HandleFeedUpdated (channels, pj);
				else if (pj.Role_ == PendingJob::RFeedExternalData)
					HandleExternalData (pj.URL_, file);
				UpdateUnreadItemsNumber ();
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
						ErrorNotification (tr ("OPML import error"),
								tr ("Unable to download the OPML file."));
					return;
				}

				PendingJob pj = PendingJobs_ [id];
				Util::FileRemoveGuard file (pj.Filename_);

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
					ErrorNotification (tr ("Download error"),
							msg.arg (pj.URL_));
				}
				PendingJobs_.remove (id);
				ID2Downloader_.remove (id);
			}

			void Core::updateFeeds ()
			{
				ids_t ids;
				StorageBackend_->GetFeedsIDs (ids);
				Q_FOREACH (IDType_t id, ids)
				{
					try
					{
						// It's handled by custom timer.
						if (StorageBackend_->GetFeedSettings (id).UpdateTimeout_)
							continue;
					}
					catch (const StorageBackend::FeedSettingsNotFoundError&)
					{
						// That's ok, we have no settings so we update as always.
					}
					catch (const std::exception& e)
					{
						qWarning () << Q_FUNC_INFO
								<< "error obtaining settings for feed"
								<< id
								<< e.what ();
					}

					UpdateFeed (id);
				}
				XmlSettingsManager::Instance ()->
					setProperty ("LastUpdateDateTime", QDateTime::currentDateTime ());
				int interval = XmlSettingsManager::Instance ()->
					property ("UpdateInterval").toInt ();
				if (interval)
					UpdateTimer_->start (interval * 60 * 1000);
			}

			void Core::fetchExternalFile (const QString& url, const QString& where)
			{
				LeechCraft::Entity e = LeechCraft::Util::MakeEntity (QUrl (url),
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
					ErrorNotification (tr ("Feed error"),
							tr ("Could not find plugin to download external file %1.").arg (url));
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

				cs.Unread_ = StorageBackend_->GetUnreadItems (cs.ChannelID_);
				ChannelsModel_->UpdateChannelData (cs);
				UpdateUnreadItemsNumber ();
			}

			void Core::updateIntervalChanged ()
			{
				int min = XmlSettingsManager::Instance ()->
					property ("UpdateInterval").toInt ();
				if (min)
				{
					if (UpdateTimer_->isActive ())
						UpdateTimer_->setInterval (min * 60 * 1000);
					else
						UpdateTimer_->start (min * 60 * 1000);
				}
				else
					UpdateTimer_->stop ();
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
				ids_t ids;
				StorageBackend_->GetFeedsIDs (ids);
				QDateTime current = QDateTime::currentDateTime ();
				Q_FOREACH (IDType_t id, ids)
				{
					int ut = 0;
					try
					{
						ut = StorageBackend_->GetFeedSettings (id).UpdateTimeout_;
					}
					catch (const StorageBackend::FeedSettingsNotFoundError&)
					{
						continue;
					}
					catch (const std::exception& e)
					{
						qWarning () << Q_FUNC_INFO
								<< "could not get feed settings for"
								<< id
								<< e.what ();
						continue;
					}

					// It's handled by normal timer.
					if (!ut)
						continue;

					if (!Updates_.contains (id) ||
							(Updates_ [id].isValid () &&
							 Updates_ [id].secsTo (current) / 60 > ut))
						UpdateFeed (id);
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
					try
					{
						StorageBackend_->UpdateChannel (data.RelatedChannel_);
					}
					catch (const std::exception& e)
					{
						qWarning () << Q_FUNC_INFO
							<< e.what ();
					}
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

			namespace
			{
				void FixDate (Item_ptr& item)
				{
					if (!item->PubDate_.isValid ())
						item->PubDate_ = QDateTime::currentDateTime ();
				}
			};

			void Core::HandleFeedAdded (const channels_container_t& channels,
					const Core::PendingJob& pj)
			{
				for (size_t i = 0; i < channels.size (); ++i)
				{
					std::for_each (channels [i]->Items_.begin (),
							channels [i]->Items_.end (),
							boost::bind (FixDate,
								_1));

					channels [i]->Tags_ = pj.Tags_;
					ChannelsModel_->AddChannel (channels [i]->ToShort ());
					StorageBackend_->AddChannel (channels [i]);

					std::for_each (channels [i]->Items_.begin (),
							channels [i]->Items_.end (),
							boost::bind (&RegexpMatcherManager::HandleItem,
								&RegexpMatcherManager::Instance (),
								_1));
				}

				for (size_t i = 0; i < channels.size (); ++i)
				{
					FetchPixmap (channels [i]);
					FetchFavicon (channels [i]);
				}

				if (pj.FeedSettings_)
				{
					IDType_t feedId = StorageBackend_->FindFeed (pj.URL_);
					Feed::FeedSettings fs (feedId,
							pj.FeedSettings_->UpdateTimeout_,
							pj.FeedSettings_->NumItems_,
							pj.FeedSettings_->ItemAge_,
							pj.FeedSettings_->AutoDownloadEnclosures_);
					try
					{
						StorageBackend_->SetFeedSettings (fs);
					}
					catch (const std::exception& e)
					{
						qWarning () << Q_FUNC_INFO
								<< "unable to set settings for just added feed"
								<< pj.URL_
								<< e.what ();
					}
				}
			}

			void Core::HandleFeedUpdated (const channels_container_t& channels,
					const Core::PendingJob& pj)
			{
				const QString& url = pj.URL_;
				IDType_t feedId = StorageBackend_->FindFeed (url);
				if (feedId == static_cast<IDType_t> (-1))
				{
					qWarning () << Q_FUNC_INFO
						<< "skipping"
						<< url
						<< "cause seems like it's not in storage yet";
					return;
				}

				const int defaultDays = XmlSettingsManager::Instance ()->
					property ("ItemsMaxAge").toInt ();
				const unsigned defaultIpc = XmlSettingsManager::Instance ()->
					property ("ItemsPerChannel").value<unsigned> ();
				bool downloadEnclosures = false;

				int days = defaultDays;
				unsigned ipc = defaultIpc;
				try
				{
					Feed::FeedSettings settings = StorageBackend_->
							GetFeedSettings (feedId);
					if (settings.ItemAge_)
						days = settings.ItemAge_;
					if (settings.NumItems_)
						ipc = settings.NumItems_;
					downloadEnclosures = settings.AutoDownloadEnclosures_;
				}
				catch (const StorageBackend::FeedSettingsNotFoundError&)
				{
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to get feed settings for"
							<< feedId
							<< url
							<< e.what ();
				}

				QDateTime current = QDateTime::currentDateTime ();
				Q_FOREACH (Channel_ptr channel, channels)
				{
					Channel_ptr ourChannel;
					try
					{
						IDType_t ourChannelID = StorageBackend_->FindChannel (channel->Title_,
								channel->Link_, feedId);
						ourChannel = StorageBackend_->GetChannel (ourChannelID, feedId);
					}
					catch (const StorageBackend::ChannelNotFoundError&)
					{
						size_t truncateAt = (channel->Items_.size () <= ipc) ?
							channel->Items_.size () : ipc;
						for (size_t j = 0; j < channel->Items_.size (); j++)
							if (channel->Items_ [j]->PubDate_.daysTo (current) > days)
							{
			 					truncateAt = std::min (j, truncateAt);
								break;
							}
						channel->Items_.resize (truncateAt);

						ChannelsModel_->AddChannel (channel->ToShort ());
						StorageBackend_->AddChannel (channel);
						QString str = tr ("Added channel \"%1\" (%n item(s))",
								"", channel->Items_.size ())
							.arg (channel->Title_);
						emit gotEntity (Util::MakeNotification ("Aggregator", str, PInfo_));
						continue;
					}

					int newItems = 0;
					int updatedItems = 0;

					GetPool (PTChannel).FreeID (channel->ChannelID_);

					Q_FOREACH (Item_ptr item, channel->Items_)
					{
						Item_ptr ourItem;
						try
						{
							IDType_t ourItemID = StorageBackend_->
									FindItem (item->Title_, item->Link_,
											ourChannel->ChannelID_);
							ourItem = StorageBackend_->GetItem (ourItemID);
						}
						catch (const StorageBackend::ItemNotFoundError&)
						{
							if (item->PubDate_.isValid ())
							{
								if (item->PubDate_.daysTo (current) >= days)
									continue;
							}
							else
								FixDate (item);

							item->ChannelID_ = ourChannel->ChannelID_;
							StorageBackend_->AddItem (item);

							RegexpMatcherManager::Instance ().HandleItem (item);

							if (downloadEnclosures)
								Q_FOREACH (Enclosure e, item->Enclosures_)
								{
									Entity de = Util::MakeEntity (QUrl (e.URL_),
											XmlSettingsManager::Instance ()->
												property ("EnclosuresDownloadPath").toString (),
											0,
											e.Type_);
									de.Additional_ [" Tags"] = channel->Tags_;
									emit gotEntity (de);
								}
							++newItems;
							continue;
						}

						if (!IsModified (ourItem, item))
							continue;

						ourItem->Description_ = item->Description_;
						ourItem->Categories_ = item->Categories_;
						ourItem->NumComments_ = item->NumComments_;
						ourItem->CommentsLink_ = item->CommentsLink_;
						ourItem->CommentsPageLink_ = item->CommentsPageLink_;
						ourItem->Latitude_ = item->Latitude_;
						ourItem->Longitude_ = item->Longitude_;

						Q_FOREACH (Enclosure enc, item->Enclosures_)
						{
							if (ourItem->Enclosures_.contains (enc))
								GetPool (PTEnclosure).FreeID (enc.EnclosureID_);
							else
							{
								enc.ItemID_ = ourItem->ItemID_;
								ourItem->Enclosures_ << enc;
							}
						}

						Q_FOREACH (MRSSEntry entry, item->MRSSEntries_)
						{
							if (ourItem->MRSSEntries_.contains (entry))
							{
								GetPool (PTMRSSEntry).FreeID (entry.MRSSEntryID_);

								Q_FOREACH (MRSSComment comment, entry.Comments_)
									GetPool (PTMRSSComment).FreeID (comment.MRSSCommentID_);
								Q_FOREACH (MRSSCredit credit, entry.Credits_)
									GetPool (PTMRSSCredit).FreeID (credit.MRSSCreditID_);
								Q_FOREACH (MRSSPeerLink peerLink, entry.PeerLinks_)
									GetPool (PTMRSSPeerLink).FreeID (peerLink.MRSSPeerLinkID_);
								Q_FOREACH (MRSSThumbnail thumb, entry.Thumbnails_)
									GetPool (PTMRSSThumbnail).FreeID (thumb.MRSSThumbnailID_);
								Q_FOREACH (MRSSScene scene, entry.Scenes_)
									GetPool (PTMRSSScene).FreeID (scene.MRSSSceneID_);
							}
							else
							{
								entry.ItemID_ = ourItem->ItemID_;
								ourItem->MRSSEntries_ << entry;
							}
						}

						GetPool (PTItem).FreeID (item->ItemID_);

						StorageBackend_->UpdateItem (ourItem);
						++updatedItems;
					}

					QString method = XmlSettingsManager::Instance ()->
							property ("NotificationsFeedUpdateBehavior").toString ();
					bool shouldShow = true;
					if (method == "ShowNo")
						shouldShow = false;
					else if (method == "ShowNew")
						shouldShow = newItems;
					else if (method == "ShowAll")
						shouldShow = newItems + updatedItems;

					if (shouldShow)
					{
						QString str = tr ("Updated channel \"%1\" (%2, %3)").arg (channel->Title_)
							.arg (tr ("%n new item(s)", "Channel update", newItems))
							.arg (tr ("%n updated item(s)", "Channel update", updatedItems));
						emit gotEntity (Util::MakeNotification ("Aggregator", str, PInfo_));
					}

					StorageBackend_->TrimChannel (ourChannel->ChannelID_,
							days, ipc);
				}
			}

			void Core::MarkChannel (const QModelIndex& i, bool state)
			{
				try
				{
					ChannelShort cs = ChannelsModel_->GetChannelForIndex (i);

					StorageBackend_->ToggleChannelUnread (cs.ChannelID_, state);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
					ErrorNotification (tr ("Aggregator error"),
							tr ("Could not mark channel"));
				}
			}

			void Core::UpdateFeed (const IDType_t& id)
			{
				QString url = StorageBackend_->GetFeed (id)->URL_;
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

				LeechCraft::Entity e = LeechCraft::Util::MakeEntity (QUrl (url),
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

				int jobId = -1;
				QObject *pr;
				emit delegateEntity (e, &jobId, &pr);
				if (jobId == -1)
				{
					qWarning () << Q_FUNC_INFO << url << "wasn't delegated";
					emit gotEntity (Util::MakeNotification ("Aggregator",
							tr ("Could not find plugin for feed with URL %1")
								.arg (url), LeechCraft::PCritical_));
					return;
				}

				HandleProvider (pr, jobId);
				PendingJobs_ [jobId] = pj;
				Updates_ [id] = QDateTime::currentDateTime ();
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

			void Core::ErrorNotification (const QString& h, const QString& body, bool wait) const
			{
				Entity e = Util::MakeNotification (h, body, PCritical_);
				e.Additional_ ["UntilUserSees"] = wait;
				emit const_cast<Core*> (this)->gotEntity (e);
			}
		};
	};
};

