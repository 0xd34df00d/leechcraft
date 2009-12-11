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

#include "core.h"
#include <memory>
#include <numeric>
#include <typeinfo>
#include <QFile>
#include <QProgressDialog>
#include <QDir>
#include <QFileInfo>
#include <QTimerEvent>
#include <QSettings>
#include <QToolBar>
#include <QTimer>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QtDebug>
#include <QThreadPool>
#include <QApplication>
#include <QRunnable>
#include <QDomElement>
#include <QDomDocument>
#include <QXmlStreamWriter>
#include <QTemporaryFile>
#include <QMessageBox>
#include <QUrl>
#include <QTextCodec>
#include <libtorrent/bencode.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/create_torrent.hpp>
#include <libtorrent/extensions/metadata_transfer.hpp>
#include <libtorrent/extensions/ut_metadata.hpp>
#include <libtorrent/extensions/ut_pex.hpp>
#include <libtorrent/extensions/smart_ban.hpp>
#include <libtorrent/file_pool.hpp>
#include <libtorrent/hasher.hpp>
#include <libtorrent/storage.hpp>
#include <libtorrent/file.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <plugininterface/tagscompletionmodel.h>
#include <plugininterface/util.h>
#include "xmlsettingsmanager.h"
#include "piecesmodel.h"
#include "peersmodel.h"
#include "torrentfilesmodel.h"
#include "representationmodel.h"
#include "config.h"

using namespace LeechCraft::Util;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			Core::HandleFinder::HandleFinder (const libtorrent::torrent_handle& h)
			: Handle_ (h)
			{
			}
			
			bool Core::HandleFinder::operator() (const Core::TorrentStruct& ts) const
			{
				return ts.Handle_ == Handle_;
			}
			
			Core::PerTrackerAccumulator::PerTrackerAccumulator (Core::pertrackerstats_t& stats)
			: Stats_ (stats)
			{
			}
			
			int Core::PerTrackerAccumulator::operator() (int,
					const Core::TorrentStruct& str)
			{
				libtorrent::torrent_status s = str.Handle_.status ();
				QString domain = QUrl (s.current_tracker.c_str ()).host ();
				if (domain.size ())
				{
					Stats_ [domain].DownloadRate_ += s.download_payload_rate;
					Stats_ [domain].UploadRate_ += s.upload_payload_rate;
				}
				return 0;
			}
			
			Core::PerTrackerStats::PerTrackerStats ()
			: DownloadRate_ (0)
			, UploadRate_ (0)
			{
			}
			
			Core* Core::Instance ()
			{
				static Core core;
				return &core;
			}
			
			Core::Core ()
			: CurrentTorrent_ (-1)
			, SettingsSaveTimer_ (new QTimer ())
			, FinishedTimer_ (new QTimer ())
			, WarningWatchdog_ (new QTimer ())
			, ScrapeTimer_ (new QTimer ())
			, PiecesModel_ (new PiecesModel ())
			, PeersModel_ (new PeersModel ())
			, TorrentFilesModel_ (new TorrentFilesModel (false))
			, WebSeedsModel_ (new QStandardItemModel ())
			, SaveScheduled_ (false)
			, Toolbar_ (0)
			, TabWidget_ (0)
			, Menu_ (0)
			{
				setObjectName ("BitTorrent Core");
				ExternalAddress_ = tr ("Unknown");
				WebSeedsModel_->setHorizontalHeaderLabels (QStringList (tr ("URL"))
						<< tr ("Standard"));
			}
			
			Core::~Core ()
			{
			}
			
			void Core::SetWidgets (QToolBar *tool, QWidget *tab)
			{
				Toolbar_ = tool;
				TabWidget_ = tab;
			}

			void Core::SetMenu (QMenu *menu)
			{
				Menu_ = menu;
			}
			
			void Core::DoDelayedInit ()
			{
				try
				{
					QString peerIDstring = XmlSettingsManager::Instance ()->
						property ("PeerIDString").toString ();

					QString ver;
					if (XmlSettingsManager::Instance ()->
							property ("OverridePeerIDVersion").toBool ())
						ver = XmlSettingsManager::Instance ()->
							property ("PeerIDVersion").toString ();
					else
					{
						// Build peer_id
						// Get the tag name.
						ver = LEECHCRAFT_VERSION;
						// Get the part before the '-'.
						ver = ver.split ('-', QString::SkipEmptyParts).at (0);
						QStringList vers = ver.split ('.', QString::SkipEmptyParts);
						if (vers.size () != 3)
							throw std::runtime_error ("Malformed version string "
									"(could not split it to three parts)");
						ver = QString ("%1%2")
							.arg (vers.at (1).toInt (),
									2, 10, QChar ('0'))
							.arg (vers.at (2).toInt (),
									2, 10, QChar ('0'));
					}


					if (ver.size () != 4)
						ver = "1111";
					Session_ = new libtorrent::session (libtorrent::fingerprint
							(peerIDstring.toLatin1 ().constData (),
							 ver.at (0).digitValue (),
							 ver.at (1).digitValue (), 
							 ver.at (2).digitValue (),
							 ver.at (3).digitValue ()));
					setLoggingSettings ();
					QList<QVariant> ports = XmlSettingsManager::Instance ()->property ("TCPPortRange").toList ();
					Session_->listen_on (std::make_pair (ports.at (0).toInt (), ports.at (1).toInt ()));
					Session_->add_extension (&libtorrent::create_metadata_plugin);
					Session_->add_extension (&libtorrent::create_ut_pex_plugin);
					Session_->add_extension (&libtorrent::create_ut_metadata_plugin);
					Session_->add_extension (&libtorrent::create_smart_ban_plugin);
					if (XmlSettingsManager::Instance ()->property ("DHTEnabled").toBool ())
						Session_->start_dht (libtorrent::entry ());
					Session_->set_max_uploads (XmlSettingsManager::Instance ()->
							property ("MaxUploads").toInt ());
					Session_->set_max_connections (XmlSettingsManager::Instance ()->
							property ("MaxConnections").toInt ());
					setProxySettings ();
					setGeneralSettings ();
					setScrapeInterval ();
					setDHTSettings ();
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO << typeid (e).name () << e.what ();
				}
			
				Headers_ << tr ("Name")
					<< tr ("Progress")
					<< tr ("State");
			
				connect (SettingsSaveTimer_.get (),
						SIGNAL (timeout ()),
						this,
						SLOT (writeSettings ()));
				SettingsSaveTimer_->start (XmlSettingsManager::Instance ()->
						property ("AutosaveInterval").toInt () * 1000);
			
				connect (FinishedTimer_.get (),
						SIGNAL (timeout ()),
						this,
						SLOT (checkFinished ()));
				FinishedTimer_->start (100);
			
				connect (WarningWatchdog_.get (),
						SIGNAL (timeout ()),
						this,
						SLOT (queryLibtorrentForWarnings ()));
				WarningWatchdog_->start (100);
			
				connect (ScrapeTimer_.get (),
						SIGNAL (timeout ()),
						this,
						SLOT (scrape ()));
			
				ManipulateSettings ();
			}
			
			void Core::Release ()
			{
				Session_->pause ();
				writeSettings ();
			
				SettingsSaveTimer_.reset ();
				FinishedTimer_.reset ();
				WarningWatchdog_.reset ();
				ScrapeTimer_.reset ();
				PiecesModel_.reset ();
				PeersModel_.reset ();
				TorrentFilesModel_.reset ();
				WebSeedsModel_.reset ();
			
				QObjectList kids = children ();
				for (int i = 0; i < kids.size (); ++i)
				{
					delete kids.at (i);
					kids [i] = 0;
				}
			
				Session_->stop_dht ();
				delete Session_;
				Session_ = 0;
			}
			
			void Core::SetProxy (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;
			}
			
			ICoreProxy_ptr Core::GetProxy () const
			{
				return Proxy_;
			}
			
			bool Core::CouldDownload (const DownloadEntity& e) const
			{
				if (e.Entity_.canConvert<QUrl> ())
				{
					QUrl url = e.Entity_.toUrl ();
					if (url.scheme () == "magnet")
					{
						QList<QPair<QString, QString> > queryItems = url.queryItems ();
						for (QList<QPair<QString, QString> >::const_iterator i = queryItems.begin (),
								end = queryItems.end (); i != end; ++i)
							if (i->first == "xt" &&
									i->second.startsWith ("urn:btih:"))
								return true;
						return false;
					}
					else if (url.scheme () == "file")
					{
						QString str = url.toLocalFile ();
						QFile file (str);
						if (file.exists () &&
								file.open (QIODevice::ReadOnly))
						{
							if (file.size () > XmlSettingsManager::Instance ()->
									property ("MaxAutoTorrentSize").toInt () * 1024 * 1024)
							{
								emit logMessage (QString ("Rejecting file %1 because it's "
											"bigger than current auto limit.").arg (str));
								return false;
							}
							else
								return IsValidTorrent (file.readAll ());
						}
						else
							return false;
					}
					else
						return false;
				}
				else if (e.Entity_.canConvert<QByteArray> ())
					return IsValidTorrent (e.Entity_.toByteArray ());
				else
					return false;
			}

			PiecesModel* Core::GetPiecesModel ()
			{
				return PiecesModel_.get ();
			}
			
			void Core::ClearPieces ()
			{
				PiecesModel_->Clear ();
			}
			
			void Core::UpdatePieces ()
			{
				if (!CheckValidity (CurrentTorrent_))
				{
					ClearPieces ();
					return;
				}
			
				std::vector<libtorrent::partial_piece_info> queue;
				Handles_.at (CurrentTorrent_).Handle_.get_download_queue (queue);
				PiecesModel_->Update (queue);
			}
			
			PeersModel* Core::GetPeersModel ()
			{
				return PeersModel_.get ();
			}

			QAbstractItemModel* Core::GetWebSeedsModel ()
			{
				return WebSeedsModel_.get ();
			}
			
			void Core::ClearPeers ()
			{
				PeersModel_->Clear ();
				WebSeedsModel_->clear ();
			}
			
			void Core::UpdatePeers ()
			{
				if (!CheckValidity (CurrentTorrent_))
				{
					ClearPeers ();
					return;
				}
			
				PeersModel_->Update (GetPeers (), CurrentTorrent_);

				if (CheckValidity (CurrentTorrent_) &&
						!WebSeedsModel_->rowCount ())
				{
					Q_FOREACH (std::string url,
							Handles_.at (CurrentTorrent_).Handle_.url_seeds ())
					{
						QList<QStandardItem*> items;
						items << new QStandardItem (QString::fromStdString (url));
						items << new QStandardItem ("BEP 19");
						WebSeedsModel_->appendRow (items);
					}
					Q_FOREACH (std::string url,
							Handles_.at (CurrentTorrent_).Handle_.http_seeds ())
					{
						QList<QStandardItem*> items;
						items << new QStandardItem (QString::fromStdString (url));
						items << new QStandardItem ("BEP 17");
						WebSeedsModel_->appendRow (items);
					}
				}
			}
			
			TorrentFilesModel* Core::GetTorrentFilesModel ()
			{
				return TorrentFilesModel_.get ();
			}
			
			void Core::ClearFiles ()
			{
				TorrentFilesModel_->Clear ();
			}
			
			void Core::UpdateFiles ()
			{
				if (!CheckValidity (CurrentTorrent_))
				{
					ClearFiles ();
					return;
				}
			
				try
				{
					TorrentFilesModel_->UpdateFiles (GetTorrentFiles ());
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO << e.what ();
					TorrentFilesModel_->Clear ();
				}
			}
			
			void Core::ResetFiles ()
			{
				if (!CheckValidity (CurrentTorrent_))
				{
					ClearFiles ();
					return;
				}
			
				try
				{
					TorrentFilesModel_->ResetFiles (GetTorrentFiles ());
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO << e.what ();
					TorrentFilesModel_->Clear ();
				}
			}
			
			int Core::columnCount (const QModelIndex&) const
			{
				return Headers_.size ();
			}
			
			QVariant Core::data (const QModelIndex& index, int role) const
			{
				if (role == RoleControls)
					return QVariant::fromValue<QToolBar*> (Toolbar_);
				if (role == RoleAdditionalInfo)
					return QVariant::fromValue<QWidget*> (TabWidget_);
				if (role == RoleContextMenu)
					return QVariant::fromValue<QMenu*> (Menu_);
				int row = index.row (),
					column = index.column ();
			
				if (!CheckValidity (row))
					return QVariant ();
			
				const libtorrent::torrent_handle& h = Handles_.at (row).Handle_;
				libtorrent::torrent_status status = h.status ();
			
				switch (role)
				{
					case Qt::DisplayRole:
						switch (column)
						{
							case ColumnName:
								return QString::fromUtf8 (h.name ().c_str ());
							case ColumnState:
								{
									QString result = status.paused ?
										tr ("Idle") : GetStringForState (status.state);
									if (status.state == libtorrent::torrent_status::downloading)
									{
										result = QString ("%1 (ETA: %2)")
											.arg (result)
											.arg (Util::MakeTimeFromLong
													(
													 static_cast<double> (status.total_wanted - status.total_wanted_done) /
													 status.download_rate
													));
									}
									return result;
								}
							case ColumnProgress:
								return QString (tr ("%1% (%2 of %3 at %4)")
										.arg (status.progress * 100, 0, 'f', 2)
										.arg (Util::MakePrettySize (status.total_wanted_done))
										.arg (Util::MakePrettySize (status.total_wanted)))
										.arg (Util::MakePrettySize (status.download_payload_rate) +
												tr ("/s"));
							default:
								return QVariant ();
						}
					case Qt::ToolTipRole:
						{
							QString result;
							result += tr ("Name:") + " " + QString::fromUtf8 (h.name ().c_str ()) + "\n";
							result += tr ("Destination:") + " " +
								QString::fromUtf8 (h.save_path ().directory_string ().c_str ()) + "\n";
							result += tr ("Progress:") + " " +
								QString (tr ("%1% (%2 of %3)")
										.arg (status.progress * 100, 0, 'f', 2)
										.arg (Util::MakePrettySize (status.total_wanted_done))
										.arg (Util::MakePrettySize (status.total_wanted))) +
								tr ("; status:") + " " +
								(status.paused ? tr ("Idle") : GetStringForState (status.state)) + "\n";
							result += tr ("Downloading speed:") + " " +
								Util::MakePrettySize (status.download_payload_rate) + tr ("/s") +
								tr ("; uploading speed:") + " " +
								Util::MakePrettySize (status.upload_payload_rate) + tr ("/s") + "\n";
							result += tr ("Peers/seeds: %1/%2").arg (status.num_peers).arg (status.num_seeds);
							return result;
						}
					case RoleTags:
						return Handles_.at (row).Tags_;
					default:
						return QVariant ();
				}
			}
			
			Qt::ItemFlags Core::flags (const QModelIndex&) const
			{
				return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
			}
			
			bool Core::hasChildren (const QModelIndex& index) const
			{
				return !index.isValid ();
			}
			
			QModelIndex Core::index (int row, int column, const QModelIndex&) const
			{
				if (!hasIndex (row, column))
					return QModelIndex ();
			
				return createIndex (row, column);
			} 
			
			QVariant Core::headerData (int column, Qt::Orientation orient, int role) const
			{
				if (orient == Qt::Vertical)
					return QVariant ();
			
				if (role != Qt::DisplayRole)
					return QVariant ();
			
				return Headers_ [column];
			}
			
			QModelIndex Core::parent (const QModelIndex&) const
			{
				return QModelIndex ();
			}
			
			int Core::rowCount (const QModelIndex& index) const
			{
				if (index.isValid ())
					return 0;
			
				return Handles_.size ();
			}
			
			libtorrent::torrent_info Core::GetTorrentInfo (const QString& filename)
			{
				QFile file (filename);
				if (!file.open (QIODevice::ReadOnly))
				{
					emit error (tr ("Could not open file %1 for read: %2").arg (filename).arg (file.errorString ()));
					return libtorrent::torrent_info (libtorrent::sha1_hash ());
				}
				return GetTorrentInfo (file.readAll ());
			}
			
			libtorrent::torrent_info Core::GetTorrentInfo (const QByteArray& data)
			{
				try
				{
					libtorrent::torrent_info result (data.constData (), data.size ());
					return result;
				}
				catch (const libtorrent::libtorrent_exception& e)
				{
					HandleLibtorrentException (e);
					return libtorrent::torrent_info (libtorrent::sha1_hash ());
				}
			}
			
			bool Core::IsValidTorrent (const QByteArray& torrentData) const
			{
				try
				{
					libtorrent::torrent_info result (torrentData.constData (), torrentData.size ());
				}
				catch (...)
				{
					return false;
				}
				return true;
			}
			
			std::auto_ptr<TorrentInfo> Core::GetTorrentStats () const
			{
				if (!CheckValidity (CurrentTorrent_))
					throw std::runtime_error ("Invalid torrent for stats");
			
				const libtorrent::torrent_handle& handle = Handles_.at (CurrentTorrent_).Handle_;
			
				std::auto_ptr<TorrentInfo> result (new TorrentInfo);
				result->Info_.reset (new libtorrent::torrent_info (handle.get_torrent_info ()));
				result->Status_ = handle.status ();
				result->Destination_ = QString::fromUtf8 (handle.save_path ().directory_string ().c_str ());
				result->State_ = result->Status_.paused ? tr ("Idle") : GetStringForState (result->Status_.state);
				return result;
			}
			
			libtorrent::session_status Core::GetOverallStats () const
			{
				return Session_->status ();
			}
			
			void Core::GetPerTracker (Core::pertrackerstats_t& stats) const
			{
				std::accumulate (Handles_.begin (), Handles_.end (), 0,
						PerTrackerAccumulator (stats));
			}
			
			int Core::GetListenPort () const
			{
				return Session_->listen_port ();
			}
			
			libtorrent::cache_status Core::GetCacheStats () const
			{
				return Session_->get_cache_status ();
			}
			
			QList<PeerInfo> Core::GetPeers () const
			{
				if (!CheckValidity (CurrentTorrent_))
					return QList<PeerInfo> ();
			
				QList<PeerInfo> result;
				std::vector<libtorrent::peer_info> peerInfos;
				Handles_.at (CurrentTorrent_).Handle_.get_peer_info (peerInfos);
			
				libtorrent::bitfield localPieces = Handles_.at (CurrentTorrent_).Handle_.status ().pieces;
			
				for (size_t i = 0; i < peerInfos.size (); ++i)
				{
					const libtorrent::peer_info& pi = peerInfos [i];
			
					int interesting = 0;
					for (size_t j = 0; j < localPieces.size (); ++j)
						interesting += (pi.pieces [j] && !localPieces [j]);
			
					PeerInfo ppi =
					{
						QString::fromStdString (pi.ip.address ().to_string ()),
						QString::fromUtf8 (pi.client.c_str ()),
						interesting,
						boost::shared_ptr<libtorrent::peer_info> (new libtorrent::peer_info (pi))
					};
					result << ppi;
				}
			
				return result;
			}
			
			QStringList Core::GetTagsForIndex (int torrent) const
			{
				if (torrent != -1)
					return GetTagsForIndexImpl (torrent);
				else
					return GetTagsForIndexImpl (CurrentTorrent_);
			}
			
			void Core::UpdateTags (const QStringList& tags, int torrent)
			{
				if (torrent != -1)
					UpdateTagsImpl (tags, torrent);
				else
					UpdateTagsImpl (tags, CurrentTorrent_);
			}
			
			int Core::AddMagnet (const QString& magnet,
					const QString& path,
					const QStringList& tags,
					TaskParameters params)
			{
				libtorrent::torrent_handle handle;
				try
				{
					libtorrent::add_torrent_params atp;
					atp.auto_managed = true;
					atp.storage_mode = libtorrent::storage_mode_sparse;
					atp.paused = (params & NoAutostart);
					atp.save_path = boost::filesystem::path (std::string (path.toUtf8 ().constData ()));
					atp.duplicate_is_error = true;
			
					handle = libtorrent::add_magnet_uri (*Session_,
							magnet.toStdString (),
							atp);
				}
				catch (const libtorrent::libtorrent_exception& e)
				{
					HandleLibtorrentException (e);
					return -1;
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO << e.what ();
					return -1;
				}
			
				TorrentStruct tmp =
				{
					std::vector<int> (),
					handle,
					QByteArray (),
					QString (),
					TSIdle,
			   		0,
					tags,
					true,
					Proxy_->GetID (),
					params
				};
				beginInsertRows (QModelIndex (), Handles_.size (), Handles_.size ());
				Handles_ << tmp;
				endInsertRows ();
				return tmp.ID_;
			}
			
			int Core::AddFile (const QString& filename,
					const QString& path,
					const QStringList& tags,
					const QVector<bool>& files,
					TaskParameters params)
			{
				if (!QFileInfo (filename).exists () || !QFileInfo (filename).isReadable ())
				{
					emit error (tr ("File %1 doesn't exist or could not be read").arg (filename));
					return -1;
				}
			
				libtorrent::torrent_handle handle;
				try
				{
					boost::intrusive_ptr<libtorrent::torrent_info> tinfo (
							new libtorrent::torrent_info (GetTorrentInfo (filename))
							);
					libtorrent::add_torrent_params atp;
					atp.ti = new libtorrent::torrent_info (GetTorrentInfo (filename));
					atp.auto_managed = true;
					atp.storage_mode = libtorrent::storage_mode_sparse;
					atp.paused = (params & NoAutostart);
					atp.save_path = boost::filesystem::path (std::string (path.toUtf8 ().constData ()));
					atp.duplicate_is_error = true;
					handle = Session_->add_torrent (atp);
				}
				catch (const libtorrent::libtorrent_exception& e)
				{
					HandleLibtorrentException (e);
					return -1;
				}
				catch (const std::runtime_error& e)
				{
					emit error (tr ("Runtime error"));
					return -1;
				}
			
				std::vector<int> priorities;
				priorities.resize (handle.get_torrent_info ().num_files ());
				for (size_t i = 0; i < priorities.size (); ++i)
					priorities [i] = 1;
			
				if (files.size ())
				{
					for (int i = 0; i < files.size (); ++i)
						priorities [i] = files [i];
			
					handle.prioritize_files (priorities);
				}
				QFile file (filename);
				file.open (QIODevice::ReadOnly);
				QByteArray contents = file.readAll ();
				file.close ();
			
				handle.auto_managed (true);
			
				beginInsertRows (QModelIndex (), Handles_.size (), Handles_.size ());
				QString torrentFileName = QString::fromUtf8 (handle.name ().c_str ());
				if (!torrentFileName.endsWith (".torrent"))
					torrentFileName.append (".torrent");
				TorrentStruct tmp =
				{
					priorities,
					handle,
					contents,
					torrentFileName,
					TSIdle,
			   		0,
					tags,
					true,
					Proxy_->GetID (),
					params
				};
				Handles_.append (tmp);
				endInsertRows ();
			
				ScheduleSave ();
				return tmp.ID_;
			}

			void Core::KillTask (int id)
			{
				for (int i = 0, size = Handles_.size (); i != size; ++i)
					if (Handles_.at (i).ID_ == id)
					{
						RemoveTorrent (id);
						return;
					}
				qWarning () << Q_FUNC_INFO
					<< "not found"
					<< id
					<< Handles_.size ();
			}
			
			void Core::RemoveTorrent (int pos)
			{
				if (!CheckValidity (pos))
					return;
			
				beginRemoveRows (QModelIndex (), pos, pos);
				Session_->remove_torrent (Handles_.at (pos).Handle_);
				int id = Handles_.at (pos).ID_;
				Handles_.removeAt (pos);
				Proxy_->FreeID (id);
				endRemoveRows ();
			
				ScheduleSave ();
				emit taskRemoved (id);
			}
			
			void Core::PauseTorrent (int pos)
			{
				if (!CheckValidity (pos))
					return;
			
				Handles_.at (pos).Handle_.pause ();
				Handles_.at (pos).Handle_.auto_managed (false);
				checkFinished ();
			}
			
			void Core::ResumeTorrent (int pos)
			{
				if (!CheckValidity (pos))
					return;
			
				Handles_.at (pos).Handle_.resume ();
				Handles_ [pos].State_ = TSIdle;
				Handles_.at (pos).Handle_.auto_managed (Handles_.at (pos).AutoManaged_);
				checkFinished ();
			}
			
			void Core::ForceReannounce (int pos)
			{
				if (!CheckValidity (pos))
					return;
			
				try
				{
					Handles_.at (pos).Handle_.force_reannounce ();
				}
				catch (const libtorrent::libtorrent_exception& e)
				{
					HandleLibtorrentException (e);
					emit error (tr ("Torrent %1 could not be reannounced at the "
								"moment, try again later.").arg (pos));
				}
			}
			
			void Core::ForceRecheck (int pos)
			{
				if (!CheckValidity (pos))
					return;
				
				Handles_.at (pos).Handle_.force_recheck ();
			}
			
			void Core::SetOverallDownloadRate (int val)
			{
				Session_->set_download_rate_limit (val == 0 ? -1 : val * 1024);
				XmlSettingsManager::Instance ()->setProperty ("DownloadRateLimit", val);
			}
			
			void Core::SetOverallUploadRate (int val)
			{
				Session_->set_upload_rate_limit (val == 0 ? -1 : val * 1024);
				XmlSettingsManager::Instance ()->setProperty ("UploadRateLimit", val);
			}
			
			void Core::SetMaxDownloadingTorrents (int val)
			{
				XmlSettingsManager::Instance ()->setProperty ("MaxDownloadingTorrents", val);
				libtorrent::session_settings settings = Session_->settings ();
				settings.active_downloads = val;
				Session_->set_settings (settings);
			}
			
			void Core::SetMaxUploadingTorrents (int val)
			{
				XmlSettingsManager::Instance ()->setProperty ("MaxUploadingTorrents", val);
				libtorrent::session_settings settings = Session_->settings ();
				settings.active_seeds = val;
				Session_->set_settings (settings);
			}
			
			void Core::SetDesiredRating (double val)
			{
				for (int i = 0; i < Handles_.size (); ++i)
				{
					if (!CheckValidity (i))
						continue;
			
					Handles_.at (i).Handle_.set_ratio (val ? 1/val : 0);
				}
			
				XmlSettingsManager::Instance ()->setProperty ("DesiredRating", val);
			}
			
			int Core::GetOverallDownloadRate () const
			{
				return XmlSettingsManager::Instance ()->property ("DownloadRateLimit").toInt ();
			}
			
			int Core::GetOverallUploadRate () const
			{
				return XmlSettingsManager::Instance ()->property ("UploadRateLimit").toInt ();
			}
			
			int Core::GetMaxDownloadingTorrents () const
			{
				return XmlSettingsManager::Instance ()->Property ("MaxDownloadingTorrents", -1).toInt ();
			}
			
			int Core::GetMaxUploadingTorrents () const
			{
				return XmlSettingsManager::Instance ()->Property ("MaxUploadingTorrents", -1).toInt ();
			}
			
			double Core::GetDesiredRating () const
			{
				return XmlSettingsManager::Instance ()->property ("DesiredRating").toInt ();
			}
			
			void Core::SetTorrentDownloadRate (int val)
			{
				if (CheckValidity (CurrentTorrent_))
					Handles_.at (CurrentTorrent_).Handle_.set_download_limit (val == 0 ? -1 : val * 1024);
			}
			
			void Core::SetTorrentUploadRate (int val)
			{
				if (CheckValidity (CurrentTorrent_))
					Handles_.at (CurrentTorrent_).Handle_.set_upload_limit (val == 0 ? -1 : val * 1024);
			}
			
			void Core::SetTorrentDesiredRating (double val)
			{
				if (CheckValidity (CurrentTorrent_))
				{
					Handles_.at (CurrentTorrent_).Handle_.set_ratio (val ? 1/val : 0);
					Handles_ [CurrentTorrent_].Ratio_ = val;
				}
			}
			
			int Core::GetTorrentDownloadRate () const
			{
				if (CheckValidity (CurrentTorrent_))
					return Handles_.at (CurrentTorrent_).Handle_.download_limit () / 1024;
				else
					return -1;
			}
			
			int Core::GetTorrentUploadRate () const
			{
				if (CheckValidity (CurrentTorrent_))
					return Handles_.at (CurrentTorrent_).Handle_.upload_limit () / 1024;
				else
					return -1;
			}
			
			double Core::GetTorrentDesiredRating () const
			{
				if (CheckValidity (CurrentTorrent_))
					return Handles_.at (CurrentTorrent_).Ratio_;
				else
					return -1;
			}

			void Core::AddPeer (const QString& ip, int port)
			{
				if (!CheckValidity (CurrentTorrent_))
					return;

				Handles_.at (CurrentTorrent_).Handle_.connect_peer (
							libtorrent::tcp::endpoint (
								libtorrent::address::from_string (ip.toStdString ()),
								port
								)
							);
			}

			void Core::AddWebSeed (const QString& ws, bool url)
			{
				if (!CheckValidity (CurrentTorrent_))
					return;

				if (url)
					Handles_.at (CurrentTorrent_).Handle_.add_url_seed (ws.toStdString ());
				else
					Handles_.at (CurrentTorrent_).Handle_.add_http_seed (ws.toStdString ());
				WebSeedsModel_->clear ();
				UpdatePeers ();
			}
			
			void Core::RemoveWebSeed (const QString& ws, bool url)
			{
				if (!CheckValidity (CurrentTorrent_))
					return;

				if (url)
					Handles_.at (CurrentTorrent_).Handle_.remove_url_seed (ws.toStdString ());
				else
					Handles_.at (CurrentTorrent_).Handle_.remove_http_seed (ws.toStdString ());
				WebSeedsModel_->clear ();
				UpdatePeers ();
			}
			
			void Core::SetFilePriority (int file, int priority)
			{
				if (!CheckValidity (CurrentTorrent_))
					return;
			
				if (priority > 7)
					priority = 7;
				else if (priority < 0)
					priority = 0;
			
				try
				{
					Handles_ [CurrentTorrent_].FilePriorities_.at (file) = priority;
					Handles_.at (CurrentTorrent_).Handle_.prioritize_files (Handles_.at (CurrentTorrent_).FilePriorities_);
				}
				catch (...)
				{
					qWarning () << Q_FUNC_INFO
						<< QString ("index for torrent %1, file %2 is out of bounds")
							.arg (CurrentTorrent_).arg (file);
				}
			}

			void Core::SetFilename (int index, const QString& name)
			{
				if (!CheckValidity (CurrentTorrent_))
					return;

				Handles_ [CurrentTorrent_].Handle_.rename_file (index, std::string (name.toUtf8 ().data ()));

				ResetFiles ();
			}
			
			QStringList Core::GetTrackers () const
			{
				if (!CheckValidity (CurrentTorrent_))
					return QStringList ();
			
				std::vector<libtorrent::announce_entry> an = Handles_.at (CurrentTorrent_).Handle_.trackers ();
				QStringList result;
				for (size_t i = 0; i < an.size (); ++i)
					result.append (QString::fromStdString (an [i].url));
				return result;
			}
			
			QStringList Core::GetTrackers (int row) const
			{
				int old = CurrentTorrent_;
				CurrentTorrent_ = row;
				QStringList trackers = GetTrackers ();
				CurrentTorrent_ = old;
				return trackers;
			}

			void Core::SetTrackers (const QStringList& trackers)
			{
				if (!CheckValidity (CurrentTorrent_))
					return;
			
				std::vector<libtorrent::announce_entry> announces;
				for (int i = 0; i < trackers.size (); ++i)
					announces.push_back (libtorrent::announce_entry (trackers.at (i).toStdString ()));
				Handles_ [CurrentTorrent_].Handle_.replace_trackers (announces);
				Handles_ [CurrentTorrent_].Handle_.force_reannounce ();
			}
			
			void Core::SetTrackers (int row, const QStringList& trackers)
			{
				int old = CurrentTorrent_;
				CurrentTorrent_ = row;
				SetTrackers (trackers);
				CurrentTorrent_ = old;
			}
			
			QString Core::GetMagnetLink () const
			{
				if (!CheckValidity (CurrentTorrent_))
					return QString ();

				std::string result = libtorrent::make_magnet_uri (Handles_ [CurrentTorrent_].Handle_);
				return QString::fromStdString (result);
			}
			
			QString Core::GetTorrentDirectory () const
			{
				if (!CheckValidity (CurrentTorrent_))
					return QString ();
			
				return QString::fromUtf8 (Handles_.at (CurrentTorrent_).Handle_.save_path ().string ().c_str ());
			}
			
			bool Core::MoveTorrentFiles (const QString& newDir)
			{
				if (!CheckValidity (CurrentTorrent_) || newDir == GetTorrentDirectory ())
					return false;
			
				Handles_.at (CurrentTorrent_).Handle_.move_storage (newDir.toUtf8 ().constData ());
				return true;
			}
			
			void Core::SetCurrentTorrent (int torrent)
			{
				CurrentTorrent_ = torrent;
			}
			
			int Core::GetCurrentTorrent () const
			{
				return CurrentTorrent_;
			}
			
			bool Core::IsTorrentManaged () const
			{
				if (!CheckValidity (CurrentTorrent_))
					return false;
			
				return Handles_.at (CurrentTorrent_).Handle_.is_auto_managed ();
			};
			
			void Core::SetTorrentManaged (bool man)
			{
				if (!CheckValidity (CurrentTorrent_))
					return;
			
				Handles_.at (CurrentTorrent_).Handle_.auto_managed (man);
				Handles_ [CurrentTorrent_].AutoManaged_ = man;
			}
			
			bool Core::IsTorrentSequentialDownload () const
			{
				if (!CheckValidity (CurrentTorrent_))
					return false;
			
				return Handles_.at (CurrentTorrent_).Handle_.is_sequential_download ();
			}
			
			void Core::SetTorrentSequentialDownload (bool seq)
			{
				if (!CheckValidity (CurrentTorrent_))
					return;
			
				Handles_.at (CurrentTorrent_).Handle_.set_sequential_download (seq);
			}
			
			bool Core::IsTorrentSuperSeeding () const
			{
				if (!CheckValidity (CurrentTorrent_))
					return false;
			
				return Handles_.at (CurrentTorrent_).Handle_.super_seeding ();
			}
			
			void Core::SetTorrentSuperSeeding (bool sup)
			{
				if (!CheckValidity (CurrentTorrent_))
					return;
				
				Handles_.at (CurrentTorrent_).Handle_.super_seeding (sup);
			}
			
			namespace
			{
				bool FileFilter (const boost::filesystem::path& filename)
				{
					if (filename.leaf () [0] == '.')
						return false;
					return true;
				}
			
				void UpdateProgress (int i, QProgressDialog *pd)
				{
					pd->setValue (i);
				}
			}
			
			void Core::MakeTorrent (NewTorrentParams params) const
			{
				boost::filesystem::path::default_name_check (boost::filesystem::no_check);
			
				libtorrent::file_storage fs;
				boost::filesystem::path fullPath =
					boost::filesystem::complete (params.Path_.toUtf8 ().constData ());
				libtorrent::add_files (fs, fullPath, FileFilter);
				libtorrent::create_torrent ct (fs, params.PieceSize_);
			
				ct.set_creator (qPrintable (QString ("LeechCraft BitTorrent %1")
							.arg (LEECHCRAFT_VERSION)));
				if (!params.Comment_.isEmpty ())
					ct.set_comment (params.Comment_.toUtf8 ());
				for (int i = 0; i < params.URLSeeds_.size (); ++i)
					ct.add_url_seed (params.URLSeeds_.at (0).toStdString ());
				ct.set_priv (!params.DHTEnabled_);
			
				if (params.DHTEnabled_)
					for (int i = 0; i < params.DHTNodes_.size (); ++i)
					{
						QStringList splitted = params.DHTNodes_.at (i).split (":");
						ct.add_node (std::pair<std::string, int> (splitted [0].trimmed ().toStdString (),
									splitted [1].trimmed ().toInt ()));
					}
			
				ct.add_tracker (params.AnnounceURL_.toStdString ());
			
				std::auto_ptr<QProgressDialog> pd (new QProgressDialog ());
				pd->setMaximum (ct.num_pieces ());
			
				libtorrent::set_piece_hashes (ct, fullPath.branch_path (),
						boost::bind (&UpdateProgress, _1, pd.get ()));
			
				libtorrent::entry e = ct.generate ();
				std::deque<char> outbuf;
				libtorrent::bencode (std::back_inserter (outbuf), e);
			
				QString filename = params.OutputDirectory_;
				if (!filename.endsWith ("/"))
					filename.append ("/");
				filename.append (params.TorrentName_);
				filename.append (".torrent");
				QFile file (filename);
				if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
				{
					emit error (tr ("Could not open file %1 for write!").arg (filename));
					return;
				}
				for (size_t i = 0; i < outbuf.size (); ++i)
					file.write (&outbuf.at (i), 1);
				file.close ();
			}
			
			void Core::LogMessage (const QString& message)
			{
				emit logMessage (message);
			}
			
			void Core::SetExternalAddress (const QString& address)
			{
				ExternalAddress_ = address;
			}
			
			QString Core::GetExternalAddress () const
			{
				return ExternalAddress_;
			}
			
			void Core::Import (const QString& filename)
			{
				QFile file (filename);
				if (!file.open (QIODevice::ReadOnly))
				{
					emit error (tr ("Could not open file %1 for reading").arg (filename));
					return;
				}
			
				QString errorString;
				int line, column;
				QDomDocument document;
				if (!document.setContent (file.readAll (), false, &errorString, &line, &column))
				{
					emit error (tr ("Could not parse document from file %1.<br />"
								"%1 at %2:%3").arg (errorString).arg (line).arg (column));
					return;
				}
			
				QDomElement root = document.documentElement ();
				QDomNodeList storages = root.elementsByTagName ("storage");
				if (storages.size ())
				{
					if (storages.size () > 1)
						emit error (tr ("There should be only one storage section."));
					else
						ParseStorage (storages.at (0).toElement ());
				}
			}
			
			void Core::ParseStorage (const QDomElement& storage)
			{
				if (storage.attribute ("version") == "1")
				{
					QDomNodeList torrents = storage.elementsByTagName ("torrent");
					for (int i = 0; i < torrents.size (); ++i)
					{
						QDomElement torrent = torrents.at (i).toElement ();
						QString filename = torrent.attribute ("filename");
					}
				}
				else
					emit error (tr ("Unknown storage version"));
			}
			
			void Core::Export (const QString& filename, bool, bool) const
			{
				QFile file (filename);
				if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
				{
					emit error (tr ("Could not open file %1 for writing").arg (filename));
					return;
				}
			
				QByteArray result;
			
				QXmlStreamWriter writer (&result);
				writer.setAutoFormatting (true);
				writer.writeStartDocument ();
				writer.writeStartElement ("storage");
				writer.writeAttribute ("version", "1");
				for (HandleDict_t::const_iterator i = Handles_.begin (),
						end = Handles_.end (); i != end; ++i)
				{
					writer.writeStartElement ("torrent");
						writer.writeAttribute ("filename",
								i->TorrentFileName_);
						writer.writeAttribute ("ratio",
								QString::number (i->Ratio_));
						writer.writeAttribute ("sequential",
								i->Handle_.is_sequential_download () ? "1" : "0");
						writer.writeAttribute ("managed",
								i->AutoManaged_ ? "1" : "0");
						writer.writeAttribute ("uploadlimit",
								QString::number (i->Handle_.upload_limit ()));
						writer.writeAttribute ("downloadlimit",
								QString::number (i->Handle_.download_limit ()));
			
						writer.writeStartElement ("trackers");
							QStringList trackers =
								GetTrackers (std::distance (Handles_.begin (), i));
							for (QStringList::const_iterator tracker = trackers.begin (),
									trackerEnd = trackers.end ();
									tracker != trackerEnd; ++tracker)
							{
								writer.writeStartElement ("tracker");
								writer.writeCharacters (*tracker);
								writer.writeEndElement ();
							}
						writer.writeEndElement ();
			
						writer.writeStartElement ("tags");
							for (QStringList::const_iterator tag = i->Tags_.begin (),
									tagEnd = i->Tags_.end (); tag != tagEnd; ++tag)
							{
								writer.writeStartElement ("tag");
								writer.writeCharacters (Proxy_->GetTagsManager ()->GetTag (*tag));
								writer.writeEndElement ();
							}
						writer.writeEndElement ();
			
						writer.writeStartElement ("prioritites");
							for (std::vector<int>::const_iterator prio = i->FilePriorities_.begin (),
									prioEnd = i->FilePriorities_.end (); prio != prioEnd; ++prio)
								writer.writeCharacters (QString::number (*prio));
						writer.writeEndElement ();
			
						writer.writeStartElement ("bencoded");
							writer.writeCharacters (i->TorrentFileContents_);
						writer.writeEndElement ();
			
						writer.writeStartElement ("parameters");
							if (i->Parameters_ & NoAutostart)
							{
								writer.writeStartElement ("parameter");
								writer.writeCharacters ("noautostart");
								writer.writeEndElement ();
							}
							if (i->Parameters_ & DoNotSaveInHistory)
							{
								writer.writeStartElement ("parameter");
								writer.writeCharacters ("donotsaveinhistory");
								writer.writeEndElement ();
							}
							if (i->Parameters_ & FromUserInitiated)
							{
								writer.writeStartElement ("parameter");
								writer.writeCharacters ("fromcommondialog");
								writer.writeEndElement ();
							}
							if (i->Parameters_ & DoNotNotifyUser)
							{
								writer.writeStartElement ("parameter");
								writer.writeCharacters ("donotnotifyuser");
								writer.writeEndElement ();
							}
							if (i->Parameters_ & Internal)
							{
								writer.writeStartElement ("parameter");
								writer.writeCharacters ("internal");
								writer.writeEndElement ();
							}
							if (i->Parameters_ & NotPersistent)
							{
								writer.writeStartElement ("parameter");
								writer.writeCharacters ("notpersistent");
								writer.writeEndElement ();
							}
						writer.writeEndElement ();
			
					writer.writeEndElement ();
				}
				writer.writeEndElement ();
				writer.writeEndDocument ();
			
				file.write (result);
			}

			void Core::BanPeers (const Core::BanRange_t& peers, bool block)
			{
				libtorrent::ip_filter filter = Session_->get_ip_filter ();
				filter.add_rule (libtorrent::address::from_string (peers.first.toStdString ()),
						libtorrent::address::from_string (peers.second.toStdString ()),
						block ?
							libtorrent::ip_filter::blocked :
							0);
				Session_->set_ip_filter (filter);

				ScheduleSave ();
			}

			void Core::ClearFilter ()
			{
				Session_->set_ip_filter (libtorrent::ip_filter ());
				ScheduleSave ();
			}

			QMap<Core::BanRange_t, bool> Core::GetFilter () const
			{
				boost::tuple<std::vector<libtorrent::ip_range<libtorrent::address_v4> >,
					std::vector<libtorrent::ip_range<libtorrent::address_v6> > > both =
						Session_->get_ip_filter ().export_filter ();
				std::vector<libtorrent::ip_range<libtorrent::address_v4> > v4 = both.get<0> ();
				std::vector<libtorrent::ip_range<libtorrent::address_v6> > v6 = both.get<1> ();

				QMap<Core::BanRange_t, bool> result;
				Q_FOREACH (libtorrent::ip_range<libtorrent::address_v4> range, v4)
					result [BanRange_t (QString::fromStdString (range.first.to_string ()),
							QString::fromStdString (range.last.to_string ()))] =
						range.flags;
				Q_FOREACH (libtorrent::ip_range<libtorrent::address_v6> range, v6)
					result [BanRange_t (QString::fromStdString (range.first.to_string ()),
							QString::fromStdString (range.last.to_string ()))] =
						range.flags;
				return result;
			}
			
			void Core::SaveResumeData (const libtorrent::save_resume_data_alert& a) const
			{
				HandleDict_t::const_iterator torrent =
					std::find_if (Handles_.begin (), Handles_.end (),
							HandleFinder (a.handle));
				if (torrent == Handles_.end ())
				{
					qWarning () << Q_FUNC_INFO
						<< "this torrent doesn't exist anymore";
					return;
				}

				QFile file (QDir::homePath () +
						"/.leechcraft/bittorrent/" +
						torrent->TorrentFileName_ +
						".resume");
			
				if (!file.open (QIODevice::WriteOnly))
				{
					qWarning () << QString ("Could not open file %1 for write: %2")
						.arg (file.fileName ())
						.arg (file.errorString ());
					return;
				}
			
				std::deque<char> outbuf;
				libtorrent::bencode (std::back_inserter (outbuf), *a.resume_data.get ());
			
				for (size_t i = 0; i < outbuf.size (); ++i)
					file.write (&outbuf.at (i), 1);
			}
			
			void Core::HandleMetadata (const libtorrent::metadata_received_alert& a)
			{
				HandleDict_t::iterator torrent =
					std::find_if (Handles_.begin (), Handles_.end (),
							HandleFinder (a.handle));
				if (torrent == Handles_.end ())
				{
					qWarning () << Q_FUNC_INFO
						<< "this torrent doesn't exist anymore";
					return;
				}

				libtorrent::torrent_info info = a.handle.get_torrent_info ();
				torrent->TorrentFileName_ = QString::fromUtf8 (info.name ().c_str ()) + ".torrent";
				torrent->FilePriorities_
					.resize (std::distance (info.begin_files (), info.end_files ()));
				std::fill (torrent->FilePriorities_.begin (),
						torrent->FilePriorities_.end (), 1);
			
				boost::shared_array<char> metadata = info.metadata ();
				std::copy (metadata.get (), metadata.get () + info.metadata_size (),
						std::back_inserter (torrent->TorrentFileContents_));

				qDebug () << "HandleMetadata"
					<< std::distance (Handles_.begin (), torrent)
					<< torrent->TorrentFileName_;
			
				ScheduleSave ();
			}

			void Core::FileFinished (const libtorrent::torrent_handle& th, int fi)
			{
				QList<TorrentStruct>::const_iterator sit =
					std::find_if (Handles_.begin (), Handles_.end (),
							HandleFinder (th));
				if (sit == Handles_.end ())
				{
					qWarning () << Q_FUNC_INFO
						<< "wtf? not found the handle";
					return;
				} 

				TorrentStruct torrent = *sit;
				libtorrent::torrent_info info = torrent.Handle_
					.get_torrent_info ();

				libtorrent::torrent_info::file_iterator fit = info.begin_files ();
				std::advance (fit, fi);

				QString name = QTextCodec::codecForLocale ()->
					toUnicode ((torrent.Handle_.save_path () / fit->path).string ().c_str ());

				QString string = tr ("File finished: %1").arg (name);
				emit torrentFinished (string);

				DownloadEntity e;
				e.Entity_ = QUrl::fromLocalFile (name);
				e.Parameters_ = LeechCraft::IsDownloaded |
					LeechCraft::ShouldQuerySource;
				e.Location_ = torrent.TorrentFileName_;
				e.Additional_ [" Tags"] = torrent.Tags_;
				emit fileFinished (e);
			}
			
			void Core::MoveUp (const std::deque<int>& selections)
			{
				if (!selections.size ())
					return;
			
				for (std::deque<int>::const_iterator i = selections.begin (),
						end = selections.end (); i != end; ++i)
					if (*i <= 0 || !CheckValidity (*i))
						return;
			
				for (std::deque<int>::const_iterator i = selections.begin (),
						end = selections.end (); i != end; ++i)
				{
					Handles_.at (*i).Handle_.queue_position_up ();
					std::swap (Handles_ [*i],
							Handles_ [*i - 1]);
			
					emit dataChanged (index (*i - 1, 0),
							index (*i, columnCount () - 1));
				}
			}
			
			void Core::MoveDown (const std::deque<int>& selections)
			{
				if (!selections.size ())
					return;
			
				for (std::deque<int>::const_iterator i = selections.begin (),
						end = selections.end (); i != end; ++i)
					if (*i < 0 || !CheckValidity (*i) || !CheckValidity (*i + 1))
						return;
			
				for (std::deque<int>::const_reverse_iterator i = selections.rbegin (),
						end = selections.rend (); i != end; ++i)
				{
					Handles_.at (*i).Handle_.queue_position_down ();
					std::swap (Handles_ [*i],
							Handles_ [*i + 1]);
			
					emit dataChanged (index (*i, 0),
							index (*i + 1, columnCount () - 1));
				}
			}
			
			void Core::MoveToTop (const std::deque<int>& selections)
			{
				if (!selections.size ())
					return;
			
				for (std::deque<int>::const_iterator i = selections.begin (),
						end = selections.end (); i != end; ++i)
					if (*i <= 0 || !CheckValidity (*i))
						return;
			
				for (std::deque<int>::const_reverse_iterator i = selections.rbegin (),
						end = selections.rend (); i != end; ++i)
					MoveToTop (*i);
			}
			
			void Core::MoveToBottom (const std::deque<int>& selections)
			{
				if (!selections.size ())
					return;
			
				for (std::deque<int>::const_iterator i = selections.begin (),
						end = selections.end (); i != end; ++i)
					if (*i < 0 || !CheckValidity (*i))
						return;
			
				for (std::deque<int>::const_iterator i = selections.begin (),
						end = selections.end (); i != end; ++i)
					MoveToBottom (*i);
			}

			void Core::SetPreset (SettingsPreset sp)
			{
				switch (sp)
				{
					case SPMinMemoryUsage:
						// TODO file_checks_delay_per_block = 15
						// max_paused_peerlist_size = 50
						// recv_socket_buffer_size = 16 * 1024
						// send_socket_buffer_size = 16 * 1024
						// optimize_hashing_for_speed = false
						// coalesce_reads = false
						// coalesce_writes = false
						XmlSettingsManager::Instance ()->
							setProperty ("WholePiecesThreshold", 2);
						XmlSettingsManager::Instance ()->
							setProperty ("UseParoleMode", false);
						XmlSettingsManager::Instance ()->
							setProperty ("PrioritizePartialPieces", true);
						XmlSettingsManager::Instance ()->
							setProperty ("FilePoolSize", 4);
						XmlSettingsManager::Instance ()->
							setProperty ("AllowMultipleConnectionsPerIP", false);
						XmlSettingsManager::Instance ()->
							setProperty ("MaxFailcount", 2);
						XmlSettingsManager::Instance ()->
							setProperty ("InactivityTimeout", 120);
						XmlSettingsManager::Instance ()->
							setProperty ("MaxOutstandingDiskBytesPerConnection", 1);
						XmlSettingsManager::Instance ()->
							setProperty ("UPNPIgnoreNonrouters", true);
						XmlSettingsManager::Instance ()->
							setProperty ("SendBufferWatermark", 9);
						XmlSettingsManager::Instance ()->
							setProperty ("CacheSize", 0);
						XmlSettingsManager::Instance ()->
							setProperty ("CacheBufferChunkSize", 1);
						XmlSettingsManager::Instance ()->
							setProperty ("UseReadCache", false);
						XmlSettingsManager::Instance ()->
							setProperty ("CloseRedundantConnections", true);
						XmlSettingsManager::Instance ()->
							setProperty ("MaxPeerListSize", 500);
						XmlSettingsManager::Instance ()->
							setProperty ("PreferUDPTrackers", true);
						XmlSettingsManager::Instance ()->
							setProperty ("MaxRejects", 10);
						break;
					case SPHighPerfSeed:
						// TODO read_cache_line_size = 512
						// write_cache_line_size = 512
						// optimize_hashing_for_speed = true
						XmlSettingsManager::Instance ()->
							setProperty ("FilePoolSize", 500);
						XmlSettingsManager::Instance ()->
							setProperty ("AllowMultipleConnectionsPerIP", true);
						XmlSettingsManager::Instance ()->
							setProperty ("CacheSize", 512);
						XmlSettingsManager::Instance ()->
							setProperty ("UseReadCache", true);
						XmlSettingsManager::Instance ()->
							setProperty ("CacheBufferChunkSize", 128);
						XmlSettingsManager::Instance ()->
							setProperty ("CacheExpiry", 60 * 60);
						XmlSettingsManager::Instance ()->
							setProperty ("CloseRedundantConnections", true);
						XmlSettingsManager::Instance ()->
							setProperty ("MaxRejects", 10);
						XmlSettingsManager::Instance ()->
							setProperty ("RequestTimeout", 10);
						XmlSettingsManager::Instance ()->
							setProperty ("PeerTimeout", 20);
						XmlSettingsManager::Instance ()->
							setProperty ("InactivityTimeout", 20);
						XmlSettingsManager::Instance ()->
							setProperty ("AutoUploadSlots", false);
						XmlSettingsManager::Instance ()->
							setProperty ("MaxFailcount", 1);
						break;
					default:
						break;
				}
				setGeneralSettings ();
			}
			
			QList<FileInfo> Core::GetTorrentFiles () const
			{
				if (!CheckValidity (CurrentTorrent_))
					return QList<FileInfo> ();
			
				QList<FileInfo> result;
				const libtorrent::torrent_handle& handle = Handles_.at (CurrentTorrent_).Handle_;
				libtorrent::torrent_info info = handle.get_torrent_info ();
				std::vector<libtorrent::size_type> prbytes;
				
				int flags = 0;
				if (!XmlSettingsManager::Instance ()->
						property ("AccurateFileProgress").toBool ())
					flags |= libtorrent::torrent_handle::piece_granularity;
				handle.file_progress (prbytes, flags);
				for (libtorrent::torrent_info::file_iterator i = info.begin_files (); i != info.end_files (); ++i)
				{
					FileInfo fi;
					fi.Path_ = i->path;
					fi.Size_ = i->size;
					fi.Priority_ = Handles_.at (CurrentTorrent_).FilePriorities_.at (i - info.begin_files ());
					fi.Progress_ = static_cast<float> (prbytes.at (i - info.begin_files ())) / 
						static_cast<float> (fi.Size_);
					result << fi;
				}
			
				return result;
			}
			
			void Core::MoveToTop (int row)
			{
				Handles_.at (row).Handle_.queue_position_top ();
			
				beginRemoveRows (QModelIndex (), row, row);
				TorrentStruct tmp = Handles_.takeAt (row);
				endRemoveRows ();
			
				beginInsertRows (QModelIndex (), 0, 0);
				Handles_.push_front (tmp);
				endInsertRows ();
			}
			
			void Core::MoveToBottom (int row)
			{
				Handles_.at (row).Handle_.queue_position_bottom ();
			
				beginRemoveRows (QModelIndex (), row, row);
				TorrentStruct tmp = Handles_.takeAt (row);
				endRemoveRows ();
			
				beginInsertRows (QModelIndex (), Handles_.size (), Handles_.size ());
				Handles_.push_back (tmp);
				endInsertRows ();
			}
			
			QString Core::GetStringForState (libtorrent::torrent_status::state_t state) const
			{
				switch (state)
				{
					case libtorrent::torrent_status::queued_for_checking:
						return tr ("Queued for checking");
					case libtorrent::torrent_status::checking_files:
						return tr ("Checking files");
					case libtorrent::torrent_status::downloading_metadata:
						return tr ("Downloading metadata");
					case libtorrent::torrent_status::downloading:
						return tr ("Downloading");
					case libtorrent::torrent_status::finished:
						return tr ("Finished");
					case libtorrent::torrent_status::seeding:
						return tr ("Seeding");
					case libtorrent::torrent_status::allocating:
						return tr ("Allocating");
					case libtorrent::torrent_status::checking_resume_data:
						return tr ("Checking resume data");
				}
				return "Uninitialized?!";
			}
			
			void Core::RestoreTorrents ()
			{
				QSettings settings (QCoreApplication::organizationName (),
						QCoreApplication::applicationName () + "_Torrent");
				settings.beginGroup ("Core");
				int torrents = settings.beginReadArray ("AddedTorrents");
				for (int i = 0; i < torrents; ++i)
				{
					settings.setArrayIndex (i);
					boost::filesystem::path path = settings.value ("SavePath").toString ().toStdString ();
					QString filename = settings.value ("Filename").toString ();
					QFile torrent (QDir::homePath () + "/.leechcraft/bittorrent/" + filename);
					if (!torrent.open (QIODevice::ReadOnly))
					{
						emit error (tr ("Could not open saved torrent %1 for read.").arg (filename));
						continue;
					}
					QByteArray data = torrent.readAll ();
					torrent.close ();
					if (data.isEmpty ())
						continue;
			
					QFile resumeDataFile (QDir::homePath () + "/.leechcraft/bittorrent/" +
							filename + ".resume");
					QByteArray resumed;
					if (resumeDataFile.open (QIODevice::ReadOnly))
					{
						resumed = resumeDataFile.readAll ();
						resumeDataFile.close ();
					}
			
					bool automanaged = settings.value ("AutoManaged", true).toBool ();
					TaskParameters taskParameters = static_cast<TaskParameters> (settings
								.value ("Parameters").toInt ());
			
					libtorrent::torrent_handle handle =
						RestoreSingleTorrent (data,
								resumed,
								path,
								automanaged,
								taskParameters & NoAutostart);
					if (!handle.is_valid ())
						continue;
			
					std::vector<int> priorities;
					QByteArray prioritiesLine = settings.value ("Priorities").toByteArray ();
					std::copy (prioritiesLine.begin (), prioritiesLine.end (),
							std::back_inserter (priorities));
			
					if (!priorities.size ())
					{
						priorities.resize (handle.get_torrent_info ().num_files ());
						std::fill (priorities.begin (), priorities.end (), 1);
					}
			
					handle.prioritize_files (priorities);
					QStringList trackers = settings.value ("TrackersOverride").toStringList ();
					if (!trackers.isEmpty ())
					{
						std::vector<libtorrent::announce_entry> announces;
						for (int i = 0; i < trackers.size (); ++i)
							announces.push_back (libtorrent::announce_entry (trackers.at (i).toStdString ()));
						handle.replace_trackers (announces);
					}
			
					TorrentStruct tmp =
					{
						priorities,
						handle,
						data,
						filename,
						TSIdle,
						0,
						settings.value ("Tags").toStringList (),
						automanaged,
						Proxy_->GetID (),
						taskParameters
				   	};
					beginInsertRows (QModelIndex (), Handles_.size (), Handles_.size ());
					Handles_.append (tmp);
					endInsertRows ();
				}
				settings.endArray ();

				int filters = settings.beginReadArray ("IPFilter");
				for (int i = 0; i < filters; ++i)
				{
					settings.setArrayIndex (i);
					BanRange_t range (settings.value ("First").toString (),
							settings.value ("Last").toString ());
					bool block = settings.value ("Block").toBool ();
					BanPeers (range, block);
				}
				settings.endArray ();
				settings.endGroup ();
			}
			
			libtorrent::torrent_handle Core::RestoreSingleTorrent (const QByteArray& data,
					const QByteArray& resumeData,
					const boost::filesystem::path& path,
					bool automanaged,
					bool pause)
			{
				libtorrent::lazy_entry e;
			
				libtorrent::torrent_handle handle;
				if (libtorrent::lazy_bdecode (data.constData (),
							data.constData () + data.size (), e))
				{
					emit error (tr ("Bad bencoding in saved torrent data"));
					return handle;
				}
			
				try
				{
					libtorrent::add_torrent_params atp;
					atp.ti = new libtorrent::torrent_info (e);
					atp.storage_mode = libtorrent::storage_mode_sparse;
					atp.save_path = path;
					atp.auto_managed = automanaged;
					atp.paused = pause;
					atp.resume_data = new std::vector<char>;
					atp.duplicate_is_error = true;
					std::copy (resumeData.constData (),
							resumeData.constData () + resumeData.size (),
							std::back_inserter (*atp.resume_data));
			
					handle = Session_->add_torrent (atp);
				}
				catch (const libtorrent::libtorrent_exception& e)
				{
					HandleLibtorrentException (e);
				}
			
				return handle;
			}
			
			void Core::HandleSingleFinished (int i)
			{
				TorrentStruct torrent = Handles_.at (i);
				libtorrent::torrent_info info = torrent.Handle_
					.get_torrent_info ();
			
				QString name = QString::fromStdString (info.name ());
				QString string = tr ("Torrent finished: %1").arg (name);
				emit torrentFinished (string);
			
				for (libtorrent::torrent_info::file_iterator i = info.begin_files (),
						end = info.end_files (); i != end; ++i)
				{
					DownloadEntity e;
					e.Entity_ = QTextCodec::codecForLocale ()->
						toUnicode ((torrent.Handle_.save_path () / i->path).string ().c_str ()).toUtf8 ();
					e.Parameters_ = LeechCraft::IsDownloaded |
						LeechCraft::ShouldQuerySource;
					e.Location_ = torrent.TorrentFileName_;
					e.Additional_ [" Tags"] = torrent.Tags_;
					emit fileFinished (e);
				}

				emit taskFinished (torrent.ID_);
			}
			
			void Core::ManipulateSettings ()
			{
				SetOverallDownloadRate (XmlSettingsManager::Instance ()->
						Property ("DownloadRateLimit", 5000).toInt ());
				SetOverallUploadRate (XmlSettingsManager::Instance ()->
						Property ("UploadRateLimit", 5000).toInt ());
				SetMaxDownloadingTorrents (XmlSettingsManager::Instance ()->
						Property ("MaxDownloadingTorrents", -1).toInt ());
				SetMaxUploadingTorrents (XmlSettingsManager::Instance ()->
						Property ("MaxUploadingTorrents", -1).toInt ());
				SetDesiredRating (XmlSettingsManager::Instance ()->
						Property ("DesiredRating", 0).toInt ());
			
				XmlSettingsManager::Instance ()->RegisterObject ("TCPPortRange",
						this, "tcpPortRangeChanged");
				XmlSettingsManager::Instance ()->RegisterObject ("DHTEnabled",
						this, "dhtStateChanged");
				XmlSettingsManager::Instance ()->RegisterObject ("AutosaveInterval",
						this, "autosaveIntervalChanged");
				XmlSettingsManager::Instance ()->RegisterObject ("MaxUploads",
						this, "maxUploadsChanged");
				XmlSettingsManager::Instance ()->RegisterObject ("MaxConnections",
						this, "maxConnectionsChanged");
			
				QList<QByteArray> proxySettings;
				proxySettings << "TrackerProxyEnabled"
					<< "TrackerProxyHost"
					<< "TrackerProxyPort"
					<< "TrackerProxyAuth"
					<< "PeerProxyEnabled"
					<< "PeerProxyHost"
					<< "PeerProxyPort"
					<< "PeerProxyAuth";
				XmlSettingsManager::Instance ()->RegisterObject (proxySettings,
						this, "setProxySettings");
			
				QList<QByteArray> generalSettings;
				generalSettings << "UserAgent"
					<< "TrackerCompletionTimeout"
					<< "TrackerReceiveTimeout"
					<< "StopTrackerTimeout"
					<< "TrackerMaximumResponseLength"
					<< "PieceTimeout"
					<< "RequestQueueTime"
					<< "MaxAllowedInRequestQueue"
					<< "MaxOutRequestQueue"
					<< "WholePiecesThreshold"
					<< "PeerTimeout"
					<< "UrlSeedTimeout"
					<< "UrlSeedPipelineSize"
					<< "SeedingPieceQuota"
					<< "UrlSeedWaitRetry"
					<< "FilePoolSize"
					<< "AllowMultipleConnectionsPerIP"
					<< "MaxFailcount"
					<< "MinReconnectTime"
					<< "PeerConnectTimeout"
					<< "IgnoreLimitsOnLocalNetwork"
					<< "ConnectionSpeed"
					<< "SendRedundantHave"
					<< "LazyBitfields"
					<< "InactivityTimeout"
					<< "UnchokeInterval"
					<< "OptimisticUnchokeMultiplier"
					<< "AnnounceIP"
					<< "NumWant"
					<< "InitialPickerThreshold"
					<< "AllowedFastSetSize"
					<< "MaxOutstandingDiskBytesPerConnection"
					<< "HandshakeTimeout"
					<< "UseDHTAsFallback"
					<< "FreeTorrentHashes"
					<< "UPNPIgnoreNonrouters"
					<< "SendBufferWatermark"
					<< "AutoUploadSlots"
					<< "UseParoleMode"
					<< "CacheSize"
					<< "CacheExpiry"
					<< "OutgoingPorts"
					<< "PeerTOS"
					<< "DontCountSlowTorrents"
					<< "AutoManageInterval"
					<< "ShareRatioLimit"
					<< "SeedTimeRatioLimit"
					<< "SeedTimeLimit"
					<< "CloseRedundantConnections"
					<< "AutoScrapeInterval"
					<< "AutoScrapeMinInterval"
					<< "MaxPeerListSize"
					<< "MinAnnounceInterval"
					<< "PrioritizePartialPieces"
					<< "AnnounceToAllTrackers"
					<< "PreferUDPTrackers"
					<< "StrictSuperSeeding";
				XmlSettingsManager::Instance ()->RegisterObject (generalSettings,
						this, "setGeneralSettings");
			
				QList<QByteArray> dhtSettings;
				dhtSettings << "MaxPeersReply"
					<< "SearchBranching"
					<< "ServicePort"
					<< "MaxDHTFailcount";
				XmlSettingsManager::Instance ()->RegisterObject (dhtSettings,
						this, "setDHTSettings");
			
				XmlSettingsManager::Instance ()->RegisterObject ("ScrapeInterval",
						this, "setScrapeInterval");
				XmlSettingsManager::Instance ()->RegisterObject ("ScrapeEnabled",
						this, "setScrapeInterval");
			
				QList<QByteArray> loggingSettings;
				loggingSettings << "PerformanceWarning"
					<< "NotificationError"
					<< "NotificationPeer"
					<< "NotificationPortMapping"
					<< "NotificationStorage"
					<< "NotificationTracker"
					<< "NotificationStatus"
					<< "NotificationProgress"
					<< "NotificationIPBlock";
				XmlSettingsManager::Instance ()->RegisterObject (loggingSettings,
						this, "setLoggingSettings");
			
				RestoreTorrents ();
			}
			
			QStringList Core::GetTagsForIndexImpl (int torrent) const
			{
				if (!CheckValidity (torrent))
					return QStringList ();
			
				QStringList result;
				Q_FOREACH (QString id, Handles_.at (torrent).Tags_)
					result << Proxy_->GetTagsManager ()->GetTag (id);
				return result;
			}
			
			void Core::UpdateTagsImpl (const QStringList& tags, int torrent)
			{
				if (!CheckValidity (torrent))
					return;
			
				Handles_ [torrent].Tags_.clear ();
				Q_FOREACH (QString tag, tags)
					Handles_ [torrent].Tags_ << Proxy_->GetTagsManager ()->GetID (tag);
			}
			
			void Core::ScheduleSave ()
			{
				if (SaveScheduled_)
					return;
			
				QTimer::singleShot (500,
						this,
						SLOT (writeSettings ()));
			
				SaveScheduled_ = true;
			}
			
			void Core::HandleLibtorrentException (const libtorrent::libtorrent_exception& e)
			{
				emit error (tr ("Error code %1 of category:<blockquote>%2</blockquote>"
							"error message:<blockquote>%3</blockquote>"
							"raw exception message:<blockquote>%4</blockquote>")
						.arg (e.error ().value ())
						.arg (e.error ().category ().name ())
						.arg (QString::fromUtf8 (e.error ().message ().c_str ()))
						.arg (e.what ()));
			}
			
			void Core::writeSettings ()
			{
				SaveScheduled_ = false;
				QDir home = QDir::home ();
				if (!home.exists (".leechcraft/bittorrent"))
					if (!home.mkdir (".leechcraft/bittorrent"))
					{
						emit error (QDir::toNativeSeparators (tr ("Could not create path %1/.leechcraft/bittorrent"))
								.arg (QDir::toNativeSeparators (QDir::homePath ())));
						return;
					}
			
				QSettings settings (QCoreApplication::organizationName (),
						QCoreApplication::applicationName () + "_Torrent");
				settings.beginGroup ("Core");
				settings.beginWriteArray ("AddedTorrents");
				settings.remove ("");
				for (int i = 0; i < Handles_.size (); ++i)
				{
					settings.setArrayIndex (i);
					if (!CheckValidity (i))
					{
						qWarning () << Q_FUNC_INFO
							<< "invalid torrent"
							<< i;
						continue;
					}
					if (Handles_.at (i).TorrentFileName_.isEmpty ())
					{
						qWarning () << Q_FUNC_INFO
							<< "empty file name"
							<< i;
						continue;
					}
					settings.remove ("");
					int oldCurrent = CurrentTorrent_;
					CurrentTorrent_ = i;
					try
					{
						QFile file_info (QDir::homePath () +
								"/.leechcraft/bittorrent/" +
								Handles_.at (i).TorrentFileName_);
						if (!file_info.open (QIODevice::WriteOnly))
							emit error (QString ("Cannot write settings! "
										"Cannot open file %1 for write!")
									.arg (Handles_.at (i).TorrentFileName_));
						else
						{
							file_info.write (Handles_.at (i).TorrentFileContents_);
							file_info.close ();
				
							Handles_.at (i).Handle_.save_resume_data ();
				
							settings.setValue ("SavePath",
									QString::fromStdString (Handles_.at (i).Handle_.save_path ().string ()));
							settings.setValue ("Filename", Handles_.at (i).TorrentFileName_);
							settings.setValue ("TrackersOverride", GetTrackers ());
							settings.setValue ("Tags", Handles_.at (i).Tags_);
							settings.setValue ("ID", Handles_.at (i).ID_);
							settings.setValue ("Parameters", static_cast<int> (Handles_.at (i).Parameters_));
							settings.setValue ("AutoManaged", Handles_.at (i).AutoManaged_);
				
							QByteArray prioritiesLine;
							std::copy (Handles_.at (i).FilePriorities_.begin (),
									Handles_.at (i).FilePriorities_.end (),
									std::back_inserter (prioritiesLine));
							settings.setValue ("Priorities", prioritiesLine);
						}
					}
					catch (const std::exception& e)
					{
						qWarning () << Q_FUNC_INFO << e.what ();
					}
					catch (...)
					{
						qWarning () << Q_FUNC_INFO << "unknown exception";
					}
					CurrentTorrent_ = oldCurrent;
				}
				settings.endArray ();

				settings.beginWriteArray ("IPFilter");
				settings.remove ("");
				QMap<BanRange_t, bool> filter = GetFilter ();
				QList<BanRange_t> keys = filter.keys ();
				int i = 0;
				Q_FOREACH (BanRange_t key, keys)
				{
					settings.setArrayIndex (i++);
					settings.setValue ("First", key.first);
					settings.setValue ("Last", key.second);
					settings.setValue ("Block", filter [key]);
				}
				settings.endArray ();
				settings.endGroup ();
			
				Session_->wait_for_alert (libtorrent::time_duration (5));
			
				queryLibtorrentForWarnings ();
			}
			
			void Core::checkFinished ()
			{
				for (int i = 0; i < Handles_.size (); ++i)
				{
					if (Handles_.at (i).State_ == TSSeeding)
						continue;
			
					libtorrent::torrent_status status = Handles_.at (i).Handle_.status ();
					libtorrent::torrent_status::state_t state = status.state;
			
					if (status.paused)
					{
						Handles_ [i].State_ = TSIdle;
						continue;
					}
			
					switch (state)
					{
						case libtorrent::torrent_status::queued_for_checking:
						case libtorrent::torrent_status::checking_files:
						case libtorrent::torrent_status::checking_resume_data:
						case libtorrent::torrent_status::allocating:
						case libtorrent::torrent_status::downloading_metadata:
							Handles_ [i].State_ = TSPreparing;
							break;
						case libtorrent::torrent_status::downloading:
							Handles_ [i].State_ = TSDownloading;
							break;
							break;
						case libtorrent::torrent_status::finished:
						case libtorrent::torrent_status::seeding:
							TorrentState oldState = Handles_ [i].State_;
							Handles_ [i].State_ = TSSeeding;
							if (oldState == TSDownloading)
							{
								HandleSingleFinished (i);
								ScheduleSave ();
							}
							break;
					}
				}
			}
			
#if defined(Q_CC_GNU)
# define __LLEECHCRAFT_API __attribute__ ((visibility("default")))
#elif defined(Q_CC_MSVC)
# define __LLEECHCRAFT_API __declspec(dllexport)
#else
# define __LLEECHCRAFT_API
#endif
			
			struct __LLEECHCRAFT_API SimpleDispatcher
			{
				void operator() (const libtorrent::external_ip_alert& a) const
				{
					Core::Instance ()->SetExternalAddress (QString::
							fromStdString (a.external_address.to_string ()));
				}
			
				void operator() (const libtorrent::save_resume_data_alert& a) const
				{
					Core::Instance ()->SaveResumeData (a);
				}

				void operator() (const libtorrent::save_resume_data_failed_alert& a) const
				{
					QMessageBox::warning (0,
							QObject::tr ("LeechCraft"),
							QObject::tr ("Saving resume data failed for torrent:<br />%1<br />%2")
								.arg (QString::fromUtf8 (a.handle.name ().c_str ()))
								.arg (QString::fromUtf8 (a.error.message ().c_str ())));
				}
			
				void operator() (const libtorrent::storage_moved_alert& a) const
				{
					QMessageBox::information (0,
							QObject::tr ("LeechCraft"),
							QObject::tr ("Storage for torrent:<br />%1<br />moved successfully to:<br />%2")
								.arg (QString::fromUtf8 (a.handle.name ().c_str ()))
								.arg (QString::fromUtf8 (a.path.c_str ())));
				}

				void operator() (const libtorrent::storage_moved_failed_alert& a) const
				{
					QMessageBox::critical (0,
							QObject::tr ("LeechCraft"),
							QObject::tr ("Storage move failure:<br />%2<br />for torrent:<br />%1")
								.arg (QString::fromUtf8 (a.handle.name ().c_str ()))
								.arg (QString::fromUtf8 (a.error.message ().c_str ())));
				}
			
				void operator() (const libtorrent::metadata_received_alert& a) const
				{
					Core::Instance ()->HandleMetadata (a);
				}

				void operator() (const libtorrent::file_error_alert& a) const
				{
					QMessageBox::critical (0,
							QObject::tr ("LeechCraft"),
							QObject::tr ("File error for torrent:<br />%1<br />"
								"file:<br />%2<br />error:<br />%3")
								.arg (QString::fromUtf8 (a.handle.name ().c_str ()))
								.arg (QString::fromUtf8 (a.file.c_str ()))
								.arg (QString::fromUtf8 (a.error.message ().c_str ())));
				}

				void operator() (const libtorrent::file_rename_failed_alert& a) const
				{
					QMessageBox::critical (0,
							QObject::tr ("LeechCraft"),
							QObject::tr ("File rename failed for torrent:<br />%1<br />"
								"file %2, error:<br />%3")
								.arg (QString::fromUtf8 (a.handle.name ().c_str ()))
								.arg (QString::number (a.index))
								.arg (QString::fromUtf8 (a.error.message ().c_str ())));
				}

				void operator() (const libtorrent::torrent_delete_failed_alert& a) const
				{
					QMessageBox::critical (0,
							QObject::tr ("LeechCraft"),
							QObject::tr ("Failed to delete torrent:<br />%1<br />error:<br />%2")
								.arg (QString::fromUtf8 (a.handle.name ().c_str ()))
								.arg (QString::fromUtf8 (a.error.message ().c_str ())));
				}
				
				void operator() (const libtorrent::file_completed_alert&) const
				{
//					Core::Instance ()->FileFinished (a.handle, a.index);
				}
			};
			
#undef __LLEECHCRAFT_API
			
			void Core::queryLibtorrentForWarnings ()
			{
				std::auto_ptr<libtorrent::alert> a (Session_->pop_alert ());
				SimpleDispatcher sd;
				while (a.get ())
				{
					try
					{
						libtorrent::handle_alert<
							libtorrent::external_ip_alert
							, libtorrent::save_resume_data_alert
							, libtorrent::save_resume_data_failed_alert
							, libtorrent::storage_moved_alert
							, libtorrent::storage_moved_failed_alert
							, libtorrent::metadata_received_alert
							, libtorrent::file_error_alert
							, libtorrent::file_rename_failed_alert
							, libtorrent::file_completed_alert
							>::handle_alert (a, sd);
					}
					catch (const libtorrent::libtorrent_exception& e)
					{
					}
					catch (const std::exception& e)
					{
					}
			
					try
					{
						QString logmsg = QString::fromStdString (a->message ());
						Core::Instance ()->LogMessage (QDateTime::currentDateTime ().toString () + " " + logmsg);
			
						qDebug () << "<libtorrent>" << logmsg;
					}
					catch (const std::exception& e)
					{
						qWarning () << Q_FUNC_INFO << typeid (e).name ();
					}
			
					a = Session_->pop_alert ();
				}
			}
			
			void Core::scrape ()
			{
				for (HandleDict_t::iterator i = Handles_.begin (),
						end = Handles_.end (); i != end; ++i)
					i->Handle_.scrape_tracker ();
			}
			
			bool Core::CheckValidity (int pos) const
			{
				if (pos >= Handles_.size () || pos < 0)
					return false;
				if (!Handles_.at (pos).Handle_.is_valid ())
				{
					qWarning () << QString ("Torrent with position %1 found in The List, but is invalid").arg (pos);
					return false;
				}
				return true;
			}
			
			void Core::tcpPortRangeChanged ()
			{
				QList<QVariant> ports = XmlSettingsManager::Instance ()->property ("TCPPortRange").toList ();
				Session_->listen_on (std::make_pair (ports.at (0).toInt (), ports.at (1).toInt ()));
			}
			
			void Core::dhtStateChanged ()
			{
				if (XmlSettingsManager::Instance ()->property ("DHTEnabled").toBool ())
					Session_->start_dht (libtorrent::entry ());
				else
					Session_->stop_dht ();
			}
			
			void Core::autosaveIntervalChanged ()
			{
				SettingsSaveTimer_->stop ();
				SettingsSaveTimer_->start (XmlSettingsManager::Instance ()->
						property ("AutosaveInterval").toInt () * 1000);
			}
			
			void Core::maxUploadsChanged ()
			{
				Session_->set_max_uploads (XmlSettingsManager::Instance ()->
						property ("MaxUploads").toInt ());
			}
			
			void Core::maxConnectionsChanged ()
			{
				Session_->set_max_connections (XmlSettingsManager::Instance ()->
						property ("MaxConnections").toInt ());
			}
			
			void Core::setProxySettings ()
			{
				libtorrent::proxy_settings trackerProxySettings, peerProxySettings;
				if (XmlSettingsManager::Instance ()->property ("TrackerProxyEnabled").toBool ())
				{
					trackerProxySettings.hostname = XmlSettingsManager::Instance ()->
						property ("TrackerProxyAddress").toString ().toStdString ();
					trackerProxySettings.port = XmlSettingsManager::Instance ()->
						property ("TrackerProxyPort").toInt ();
					QStringList auth = XmlSettingsManager::Instance ()->
						property ("TrackerProxyAuth").toString ().split ('@');
					if (auth.size ())
						trackerProxySettings.username = auth.at (0).toStdString ();
					if (auth.size () > 1)
						trackerProxySettings.password = auth.at (1).toStdString ();
					bool passworded = trackerProxySettings.username.size ();
					QString pt = XmlSettingsManager::Instance ()->property ("TrackerProxyType").toString ();
					if (pt == "http")
						trackerProxySettings.type = passworded ?
							libtorrent::proxy_settings::http_pw :
							libtorrent::proxy_settings::http;
					else if (pt == "socks4")
						trackerProxySettings.type = libtorrent::proxy_settings::socks4;
					else if (pt == "socks5")
						trackerProxySettings.type = passworded ?
							libtorrent::proxy_settings::socks5_pw :
							libtorrent::proxy_settings::socks5;
					else
						trackerProxySettings.type = libtorrent::proxy_settings::none;
				}
				else
					trackerProxySettings.type = libtorrent::proxy_settings::none;
			
				if (XmlSettingsManager::Instance ()->property ("PeerProxyEnabled").toBool ())
				{
					peerProxySettings.hostname = XmlSettingsManager::Instance ()->
						property ("PeerProxyAddress").toString ().toStdString ();
					peerProxySettings.port = XmlSettingsManager::Instance ()->
						property ("PeerProxyPort").toInt ();
					QStringList auth = XmlSettingsManager::Instance ()->
						property ("PeerProxyAuth").toString ().split ('@');
					if (auth.size ())
						peerProxySettings.username = auth.at (0).toStdString ();
					if (auth.size () > 1)
						peerProxySettings.password = auth.at (1).toStdString ();
					bool passworded = peerProxySettings.username.size ();
					QString pt = XmlSettingsManager::Instance ()->property ("PeerProxyType").toString ();
					if (pt == "http")
						peerProxySettings.type = passworded ?
							libtorrent::proxy_settings::http_pw :
							libtorrent::proxy_settings::http;
					else if (pt == "socks4")
						peerProxySettings.type = libtorrent::proxy_settings::socks4;
					else if (pt == "socks5")
						peerProxySettings.type = passworded ?
							libtorrent::proxy_settings::socks5_pw :
							libtorrent::proxy_settings::socks5;
					else
						peerProxySettings.type = libtorrent::proxy_settings::none;
				}
				else
					peerProxySettings.type = libtorrent::proxy_settings::none;
			
				Session_->set_peer_proxy (peerProxySettings);
				Session_->set_web_seed_proxy (peerProxySettings);
				Session_->set_tracker_proxy (trackerProxySettings);
			}

			void Core::setGeneralSettings ()
			{
				libtorrent::session_settings settings = Session_->settings ();
			
				settings.user_agent = XmlSettingsManager::Instance ()->
					property ("UserAgent").toString ().toStdString ();
				settings.tracker_completion_timeout = XmlSettingsManager::Instance ()->
					property ("TrackerCompletionTimeout").toInt ();
				settings.tracker_receive_timeout = XmlSettingsManager::Instance ()->
					property ("TrackerReceiveTimeout").toInt ();
				settings.stop_tracker_timeout = XmlSettingsManager::Instance ()->
					property ("StopTrackerTimeout").toInt ();
				settings.tracker_maximum_response_length = XmlSettingsManager::Instance ()->
					property ("TrackerMaximumResponseLength").toInt () * 1024;
				settings.piece_timeout = XmlSettingsManager::Instance ()->
					property ("PieceTimeout").toInt ();
				settings.request_timeout = XmlSettingsManager::Instance ()->
					property ("RequestTimeout").toInt ();
				settings.request_queue_time = XmlSettingsManager::Instance ()->
					property ("RequestQueueTime").toInt ();
				settings.max_allowed_in_request_queue = XmlSettingsManager::Instance ()->
					property ("MaxAllowedInRequestQueue").toInt ();
				settings.max_out_request_queue = XmlSettingsManager::Instance ()->
					property ("MaxOutRequestQueue").toInt ();
				settings.whole_pieces_threshold = XmlSettingsManager::Instance ()->
					property ("WholePiecesThreshold").toInt ();
				settings.peer_timeout = XmlSettingsManager::Instance ()->
					property ("PeerTimeout").toInt ();
				settings.urlseed_timeout = XmlSettingsManager::Instance ()->
					property ("UrlSeedTimeout").toInt ();
				settings.urlseed_pipeline_size = XmlSettingsManager::Instance ()->
					property ("UrlSeedPipelineSize").toInt ();
				settings.urlseed_wait_retry = XmlSettingsManager::Instance ()->
					property ("UrlSeedWaitRetry").toInt ();
				settings.file_pool_size = XmlSettingsManager::Instance ()->
					property ("FilePoolSize").toInt ();
				settings.allow_multiple_connections_per_ip = XmlSettingsManager::Instance ()->
					property ("AllowMultipleConnectionsPerIP").toBool ();
				settings.max_failcount = XmlSettingsManager::Instance ()->
					property ("MaxFailcount").toInt ();
				settings.min_reconnect_time = XmlSettingsManager::Instance ()->
					property ("MinReconnectTime").toInt ();
				settings.peer_connect_timeout = XmlSettingsManager::Instance ()->
					property ("PeerConnectTimeout").toInt ();
				settings.ignore_limits_on_local_network = XmlSettingsManager::Instance ()->
					property ("IgnoreLimitsOnLocalNetwork").toBool ();
				settings.connection_speed = XmlSettingsManager::Instance ()->
					property ("ConnectionSpeed").toInt ();
				settings.send_redundant_have = XmlSettingsManager::Instance ()->
					property ("SendRedundantHave").toBool ();
				settings.lazy_bitfields = XmlSettingsManager::Instance ()->
					property ("LazyBitfields").toBool ();
				settings.inactivity_timeout = XmlSettingsManager::Instance ()->
					property ("InactivityTimeout").toInt ();
				settings.unchoke_interval = XmlSettingsManager::Instance ()->
					property ("UnchokeInterval").toInt ();
				settings.optimistic_unchoke_interval = XmlSettingsManager::Instance ()->
					property ("OptimisticUnchokeMultiplier").toInt ();
				try
				{
					if (XmlSettingsManager::Instance ()->property ("AnnounceIP").toString ().isEmpty ())
						settings.announce_ip = boost::asio::ip::address ();
					else
						settings.announce_ip = boost::asio::ip::address::from_string (XmlSettingsManager::Instance ()->property ("AnnounceIP").toString ().toStdString ());
				}
				catch (...)
				{
					error (tr ("Wrong announce address %1")
							.arg (XmlSettingsManager::Instance ()->property ("AnnounceIP").toString ()));
				}
				settings.num_want = XmlSettingsManager::Instance ()->
					property ("NumWant").toInt ();
				settings.initial_picker_threshold = XmlSettingsManager::Instance ()->
					property ("InitialPickerThreshold").toInt ();
				settings.allowed_fast_set_size = XmlSettingsManager::Instance ()->
					property ("AllowedFastSetSize").toInt ();
				settings.max_queued_disk_bytes = XmlSettingsManager::Instance ()->
					property ("MaxOutstandingDiskBytesPerConnection").toInt () * 1024;
				settings.handshake_timeout = XmlSettingsManager::Instance ()->
					property ("HandshakeTimeout").toInt ();
				settings.use_dht_as_fallback = XmlSettingsManager::Instance ()->
					property ("UseDHTAsFallback").toBool ();
				settings.free_torrent_hashes = XmlSettingsManager::Instance ()->
					property ("FreeTorrentHashes").toBool ();
				settings.upnp_ignore_nonrouters = XmlSettingsManager::Instance ()->
					property ("UPNPIgnoreNonrouters").toBool ();
				settings.send_buffer_watermark = XmlSettingsManager::Instance ()->
					property ("SendBufferWatermark").toInt () * 1024;
				settings.auto_upload_slots = XmlSettingsManager::Instance ()->
					property ("AutoUploadSlots").toBool ();
				settings.use_parole_mode = XmlSettingsManager::Instance ()->
					property ("UseParoleMode").toBool ();
				settings.cache_size = 1048576 / 16384 * XmlSettingsManager::Instance ()->
					property ("CacheSize").value<long int> ();
				settings.cache_buffer_chunk_size = XmlSettingsManager::Instance ()->
					property ("CacheBufferChunkSize").toInt ();
				settings.cache_expiry = XmlSettingsManager::Instance ()->
					property ("CacheExpiry").toInt ();
				QList<QVariant> ports = XmlSettingsManager::Instance ()->
					property ("OutgoingPorts").toList ();
				if (ports.size () == 2)
					settings.outgoing_ports = std::make_pair (ports.at (0).toInt (),
							ports.at (1).toInt ());
				settings.use_read_cache = XmlSettingsManager::Instance ()->
					property ("UseReadCache").toBool ();
				settings.peer_tos = XmlSettingsManager::Instance ()->
					property ("PeerTOS").toInt ();
				settings.auto_manage_prefer_seeds = XmlSettingsManager::Instance ()->
					property ("AutoManagePreferSeeds").toBool ();
				settings.dont_count_slow_torrents = XmlSettingsManager::Instance ()->
					property ("DontCountSlowTorrents").toBool ();
				settings.auto_manage_interval = XmlSettingsManager::Instance ()->
					property ("AutoManageInterval").toInt ();
				settings.share_ratio_limit = XmlSettingsManager::Instance ()->
					property ("ShareRatioLimit").toDouble ();
				settings.seed_time_ratio_limit = XmlSettingsManager::Instance ()->
					property ("SeedTimeRatioLimit").toDouble ();
				settings.seed_time_limit = XmlSettingsManager::Instance ()->
					property ("SeedTimeLimit").toULongLong () * 60;
				settings.peer_turnover = XmlSettingsManager::Instance ()->
					property ("PeerTurnover").toDouble ();
				settings.close_redundant_connections = XmlSettingsManager::Instance ()->
					property ("CloseRedundantConnections").toBool ();
				settings.auto_scrape_interval = XmlSettingsManager::Instance ()->
					property ("AutoScrapeInterval").toInt () * 60;
				settings.auto_scrape_min_interval = XmlSettingsManager::Instance ()->
					property ("AutoScrapeMinInterval").toInt ();
				settings.max_peerlist_size = XmlSettingsManager::Instance ()->
					property ("MaxPeerListSize").toInt ();
				settings.min_announce_interval = XmlSettingsManager::Instance ()->
					property ("MinAnnounceInterval").toInt ();
				settings.prioritize_partial_pieces = XmlSettingsManager::Instance ()->
					property ("PrioritizePartialPieces").toBool ();
				settings.announce_to_all_trackers = XmlSettingsManager::Instance ()->
					property ("AnnounceToAllTrackers").toBool ();
				settings.announce_to_all_tiers = XmlSettingsManager::Instance ()->
					property ("AnnounceToAllTiers").toBool ();
				settings.prefer_udp_trackers = XmlSettingsManager::Instance ()->
					property ("PreferUDPTrackers").toBool ();
				settings.strict_super_seeding = XmlSettingsManager::Instance ()->
					property ("StrictSuperSeeding").toBool ();
				settings.seeding_piece_quota = XmlSettingsManager::Instance ()->
					property ("SeedingPieceQuota").toInt ();
				settings.auto_manage_startup = XmlSettingsManager::Instance ()->
					property ("AutoManageStartup").toInt ();
				settings.lock_disk_cache = XmlSettingsManager::Instance ()->
					property ("LockDiskCache").toBool ();
				settings.max_rejects = XmlSettingsManager::Instance ()->
					property ("MaxRejects").toInt ();
			
				settings.active_limit = 16384;
			
				Session_->set_settings (settings);
			}
			
			void Core::setDHTSettings ()
			{
				libtorrent::dht_settings settings;
			
				settings.max_peers_reply = XmlSettingsManager::Instance ()->property ("MaxPeersReply").toInt ();
				settings.search_branching = XmlSettingsManager::Instance ()->property ("SearchBranching").toInt ();
				settings.service_port = XmlSettingsManager::Instance ()->property ("ServicePort").toInt ();
				settings.max_fail_count = XmlSettingsManager::Instance ()->property ("MaxDHTFailcount").toInt ();
			
				Session_->set_dht_settings (settings);
			}
			
			void Core::setLoggingSettings ()
			{
				int mask = 0;
				if (XmlSettingsManager::Instance ()->property ("PerformanceWarning").toBool ())
					mask |= libtorrent::alert::performance_warning;
				if (XmlSettingsManager::Instance ()->property ("NotificationError").toBool ())
					mask |= libtorrent::alert::error_notification;
				if (XmlSettingsManager::Instance ()->property ("NotificationPeer").toBool ())
					mask |= libtorrent::alert::peer_notification;
				if (XmlSettingsManager::Instance ()->property ("NotificationPortMapping").toBool ())
					mask |= libtorrent::alert::port_mapping_notification;
				if (XmlSettingsManager::Instance ()->property ("NotificationStorage").toBool ())
					mask |= libtorrent::alert::storage_notification;
				if (XmlSettingsManager::Instance ()->property ("NotificationTracker").toBool ())
					mask |= libtorrent::alert::tracker_notification;
				if (XmlSettingsManager::Instance ()->property ("NotificationStatus").toBool ())
					mask |= libtorrent::alert::status_notification;
				if (XmlSettingsManager::Instance ()->property ("NotificationProgress").toBool ())
					mask |= libtorrent::alert::progress_notification;
				if (XmlSettingsManager::Instance ()->property ("NotificationIPBlock").toBool ())
					mask |= libtorrent::alert::ip_block_notification;
			
				Session_->set_alert_mask (mask);
			}
			
			void Core::setScrapeInterval ()
			{
				bool scrapeEnabled = XmlSettingsManager::Instance ()->property ("ScrapeEnabled").toBool ();
				if (scrapeEnabled)
				{
					ScrapeTimer_->stop ();
					ScrapeTimer_->start (XmlSettingsManager::Instance ()->
							property ("ScrapeInterval").toInt () * 1000);
				}
				else
					ScrapeTimer_->stop ();
			}
			
		};
	};
};

