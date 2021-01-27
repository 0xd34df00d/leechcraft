/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"
#include <memory>
#include <numeric>
#include <typeinfo>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QToolBar>
#include <QTimer>
#include <QMenu>
#include <QtDebug>
#include <QApplication>
#include <QStandardItemModel>
#include <QXmlStreamWriter>
#include <QTemporaryFile>
#include <QMessageBox>
#include <QUrl>
#include <QTextCodec>
#include <QDesktopServices>
#include <QUrlQuery>
#include <libtorrent/bencode.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/file.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/version.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/lazy_entry.hpp>
#include <libtorrent/announce_entry.hpp>
#include <libtorrent/read_resume_data.hpp>
#include <libtorrent/write_resume_data.hpp>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/ijobholder.h>
#include <interfaces/an/constants.h>
#include <util/tags/tagscompletionmodel.h>
#include <util/shortcuts/shortcutmanager.h>
#include <util/util.h>
#include <util/xpc/util.h>
#include <util/xpc/notificationactionhandler.h>
#include <util/sll/util.h>
#include <util/sll/prelude.h>
#include <util/sll/either.h>
#include <util/sys/paths.h>
#include <util/threads/futures.h>
#include "xmlsettingsmanager.h"
#include "piecesmodel.h"
#include "torrentfilesmodel.h"
#include "livestreammanager.h"
#include "torrentmaker.h"
#include "sessionsettingsmanager.h"
#include "cachedstatuskeeper.h"
#include "sessionstats.h"
#include "ltutils.h"

Q_DECLARE_METATYPE (QMenu*)
Q_DECLARE_METATYPE (QToolBar*)

using namespace LC::Util;

namespace LC
{
namespace BitTorrent
{
	Core* Core::Instance ()
	{
		static Core core;
		return &core;
	}

	namespace
	{
		auto BuildFingerprint (const ICoreProxy_ptr& proxy)
		{
			auto ver = proxy->GetVersion ().section ('-', 0, 0);
			const auto& vers = ver.splitRef ('.', Qt::SkipEmptyParts);
			if (vers.size () != 3)
				throw std::runtime_error ("Malformed version string " + ver.toStdString ());
			ver = QStringLiteral ("%1%2")
					.arg (vers.at (1).toInt (), 2, 10, QChar ('0'))
					.arg (vers.at (2).toInt (), 2, 10, QChar ('0'));

			if (ver.size () != 4)
				ver = QStringLiteral ("1111");

			auto dig = [&ver] (int pos) { return ver.at (pos).digitValue (); };

			return libtorrent::generate_fingerprint ("LC", dig (0), dig (1), dig (2), dig (3));
		}

		auto CreateSession ()
		{
			libtorrent::settings_pack pack;
			pack.set_str (libtorrent::settings_pack::peer_fingerprint, BuildFingerprint (GetProxyHolder ()));
			return new libtorrent::session (pack, {});
		}
	}

	Core::Core ()
	: StatusKeeper_ { new CachedStatusKeeper { this } }
	, Session_ { CreateSession () }
	, FinishedTimer_ { new QTimer }
	, WarningWatchdog_ { new QTimer }
	, TorrentIcon_ { GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon () }
	, Holder_ { *Session_ }
	{
		setObjectName ("BitTorrent Core");
		ExternalAddress_ = tr ("Unknown");
	}

	SessionHolder& Core::GetSessionHolder ()
	{
		return Holder_;
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

	namespace
	{
		bool DecodeEntry (const QByteArray& data, libtorrent::bdecode_node& e)
		{
			libtorrent::error_code ec;
			e = libtorrent::bdecode (libtorrent::span { data.constData (), data.size () }, ec);
			if (ec)
			{
				qWarning () << Q_FUNC_INFO
						<< "bad bencoding in saved torrent data"
						<< ec.message ().c_str ();
				return false;
			}

			return true;
		}
	}

	void Core::DoDelayedInit ()
	{
		try
		{
			SessionSettingsMgr_ = new SessionSettingsManager { Session_, Proxy_, this };

			auto sstateVariant = XmlSettingsManager::Instance ()->
					property ("SessionState");
			if (sstateVariant.isValid () &&
					!sstateVariant.toByteArray ().isEmpty ())
			{
				libtorrent::bdecode_node state;
				if (DecodeEntry (sstateVariant.toByteArray (), state))
					Session_->load_state (state);
			}
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO << typeid (e).name () << e.what ();
		}

		Headers_ = QStringList
		{
			"#",
			tr ("Name"),
			tr ("State"),
			tr ("Progress"),
			tr ("Down speed"),
			tr ("Up speed"),
			tr ("Leechers"),
			tr ("Seeders"),
			tr ("Size"),
			tr ("Total downloaded"),
			tr ("Total uploaded"),
			tr ("Ratio")
		};

		connect (FinishedTimer_.get (),
				SIGNAL (timeout ()),
				this,
				SLOT (checkFinished ()));
		FinishedTimer_->start (10000);

		connect (WarningWatchdog_.get (),
				SIGNAL (timeout ()),
				this,
				SLOT (queryLibtorrent ()));
		WarningWatchdog_->start (2000);

		connect (SessionSettingsMgr_,
				SIGNAL (scrapeRequested ()),
				this,
				SLOT (scrape ()));
		connect (SessionSettingsMgr_,
				SIGNAL (saveSettingsRequested ()),
				this,
				SLOT (writeSettings ()));

		RestoreTorrents ();
	}

	void Core::Release ()
	{
		Session_->pause ();
		writeSettings ();

		FinishedTimer_.reset ();
		WarningWatchdog_.reset ();

		qDeleteAll (children ());

		delete Session_;
		Session_ = 0;
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
		ShortcutMgr_ = new ShortcutManager (proxy, this);
		LiveStreamManager_ = std::make_shared<LiveStreamManager> (StatusKeeper_);
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	Util::ShortcutManager* Core::GetShortcutManager () const
	{
		return ShortcutMgr_;
	}

	SessionSettingsManager* Core::GetSessionSettingsManager () const
	{
		return SessionSettingsMgr_;
	}

	EntityTestHandleResult Core::CouldDownload (const Entity& e) const
	{
		if (e.Entity_.canConvert<QUrl> ())
		{
			QUrl url = e.Entity_.toUrl ();
			if (url.scheme () == "magnet")
			{
				const auto& items = QUrlQuery { url }.queryItems ();
				const bool hasMagnet = std::any_of (items.begin (), items.end (),
						[] (const auto& item) { return item.first == "xt" && item.second.startsWith ("urn:btih:"); });
				return hasMagnet ?
						EntityTestHandleResult { EntityTestHandleResult::PIdeal } :
						EntityTestHandleResult {};
			}
			else if (url.scheme () == "file")
			{
				QString str = url.toLocalFile ();
				QFile file (str);
				if (!file.exists () ||
						!file.open (QIODevice::ReadOnly))
					return EntityTestHandleResult ();

				if (file.size () > XmlSettingsManager::Instance ()->
						property ("MaxAutoTorrentSize").toInt () * 1024 * 1024)
				{
					if (str.endsWith (".torrent", Qt::CaseInsensitive) &&
							XmlSettingsManager::Instance ()->
								property ("NotifyAboutTooBig").toBool ())
					{
						const auto& msg = tr ("Rejecting file %1 because it's "
								"bigger than current auto limit.").arg (str);
						const auto& entity = Util::MakeNotification ("BitTorrent", msg, Priority::Warning);
						Proxy_->GetEntityManager ()->HandleEntity (entity);
					}
					return EntityTestHandleResult ();
				}
				else
					return IsValidTorrent (file.readAll ()) ?
							EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
							EntityTestHandleResult ();
			}
			else
				return EntityTestHandleResult ();
		}
		else if (e.Entity_.canConvert<QByteArray> ())
			return IsValidTorrent (e.Entity_.toByteArray ()) ?
					EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
					EntityTestHandleResult ();
		else
			return EntityTestHandleResult ();
	}

	PiecesModel* Core::GetPiecesModel (int idx)
	{
		return idx >= 0 ? new PiecesModel (idx) : 0;
	}

	QAbstractItemModel* Core::GetWebSeedsModel (int idx)
	{
		if (idx < 0)
			return 0;

		auto model = new QStandardItemModel;
		model->setHorizontalHeaderLabels ({tr ("URL"), tr ("Standard") });
		for (const auto& url : Handles_.at (idx).Handle_.url_seeds ())
			model->appendRow ({
						new QStandardItem (QString::fromUtf8 (url.c_str ())),
						new QStandardItem ("BEP 19")
					});
		for (const auto& url : Handles_.at (idx).Handle_.http_seeds ())
			model->appendRow ({
						new QStandardItem (QString::fromUtf8 (url.c_str ())),
						new QStandardItem ("BEP 17")
					});
		return model;
	}

	TorrentFilesModel* Core::GetTorrentFilesModel (int idx)
	{
		if (idx < 0)
			return 0;

		const auto tfm = new TorrentFilesModel (idx);
		connect (this,
				SIGNAL (fileRenamed (int, int, QString)),
				tfm,
				SLOT (handleFileRenamed (int, int, QString)));
		return tfm;
	}

	CachedStatusKeeper* Core::GetStatusKeeper () const
	{
		return StatusKeeper_;
	}

	int Core::columnCount (const QModelIndex&) const
	{
		return Headers_.size ();
	}

	namespace
	{
		QString GetStringForState (libtorrent::torrent_status::state_t state)
		{
			switch (state)
			{
				case libtorrent::torrent_status::checking_files:
					return Core::tr ("Checking files");
				case libtorrent::torrent_status::downloading_metadata:
					return Core::tr ("Downloading metadata");
				case libtorrent::torrent_status::downloading:
					return Core::tr ("Downloading");
				case libtorrent::torrent_status::finished:
					return Core::tr ("Finished");
				case libtorrent::torrent_status::seeding:
					return Core::tr ("Seeding");
				case libtorrent::torrent_status::allocating:
					return Core::tr ("Allocating");
				case libtorrent::torrent_status::checking_resume_data:
					return Core::tr ("Checking resume data");
			}
			return "Uninitialized?!";
		}

		bool IsPaused (const libtorrent::torrent_status& status)
		{
			return status.flags & libtorrent::torrent_flags::paused;
		}

		bool IsAutoManaged (const libtorrent::torrent_status& status)
		{
			return status.flags & libtorrent::torrent_flags::auto_managed;
		}

		QString GetStringForStatus (const libtorrent::torrent_status& status)
		{
			const auto& stateStr = GetStringForState (status.state);
			if (status.state == libtorrent::torrent_status::downloading)
			{
				if (status.errc)
				{
					static const auto errorStr = Core::tr ("Error");
					return errorStr;
				}

				if (IsPaused (status))
				{
					static const auto pausedStr = Core::tr ("Paused");
					return pausedStr;
				}

				const auto remaining = status.total_wanted - status.total_wanted_done;
				const auto time = static_cast<double> (remaining) / status.download_rate;
				return QString ("%1 (ETA: %2)")
					.arg (stateStr)
					.arg (Util::MakeTimeFromLong (time));
			}
			else if (IsPaused (status))
			{
				static const auto idleStr = Core::tr ("Idle");
				return idleStr;
			}
			else
				return stateStr;
		}
	}

	QVariant Core::data (const QModelIndex& index, int role) const
	{
		if (role == RoleControls)
			return QVariant::fromValue<QToolBar*> (Toolbar_);
		if (role == RoleAdditionalInfo)
			return QVariant::fromValue<QWidget*> (TabWidget_);
		if (role == RoleContextMenu)
			return QVariant::fromValue<QMenu*> (Menu_);
		if (role == HandleIndex)
			return index.row ();

		const int row = index.row ();
		const int column = index.column ();

		if (!CheckValidity (row))
			return QVariant ();

		const auto& h = Handles_.at (row).Handle_;
		const auto& status = StatusKeeper_->GetStatus (h,
				libtorrent::torrent_handle::query_name |
				libtorrent::torrent_handle::query_save_path);

		switch (role)
		{
		case Qt::DecorationRole:
			if (column != ColumnName)
				return {};

			if (status.errc)
				return QIcon::fromTheme ("dialog-error");

			if (IsPaused (status))
				return QIcon::fromTheme ("media-playback-stop");

			switch (status.state)
			{
			case libtorrent::torrent_status::checking_files:
			case libtorrent::torrent_status::checking_resume_data:
				return QIcon::fromTheme ("tools-check-spelling");
			case libtorrent::torrent_status::downloading:
			case libtorrent::torrent_status::downloading_metadata:
				return QIcon::fromTheme ("arrow-down");
			case libtorrent::torrent_status::allocating:
				return QIcon::fromTheme ("media-playback-start");
			case libtorrent::torrent_status::finished:
				return QIcon::fromTheme ("arrow-up");
			case libtorrent::torrent_status::seeding:
				return QIcon::fromTheme ("arrow-up-double");
			}

			return {};
		case Roles::SortRole:
			switch (column)
			{
			case ColumnID:
				return row + 1;
			case ColumnName:
				return QString::fromStdString (status.name);
			case ColumnState:
				return IsPaused (status) ?
						-1 :
						static_cast<int> (status.state);
			case ColumnProgress:
				return status.progress;
			case ColumnDownSpeed:
				return status.download_payload_rate;
			case ColumnUpSpeed:
				return status.upload_payload_rate;
			case ColumnLeechers:
				return status.num_peers - status.num_seeds;
			case ColumnSeeders:
				return status.num_seeds;
			case ColumnDownloaded:
				return static_cast<quint64> (status.all_time_download);
			case ColumnSize:
				return static_cast<quint64> (status.total_wanted);
			case ColumnUploaded:
				return static_cast<quint64> (status.all_time_upload);
			case ColumnRatio:
				if (status.all_time_download)
					return static_cast<double> (status.all_time_upload) / status.all_time_download;

				return status.all_time_upload ?
						std::numeric_limits<double>::max () :
						0;
			default:
				return {};
			}
		case Roles::FullLengthText:
		case Qt::DisplayRole:
			switch (column)
			{
			case ColumnID:
				return row + 1;
			case ColumnName:
				return QString::fromStdString (status.name);
			case ColumnState:
				return GetStringForStatus (status);
			case ColumnProgress:
				if (role == Roles::FullLengthText)
				{
					if (status.state == libtorrent::torrent_status::downloading)
					{
						static const auto templ = tr ("%1% (%2 of %3 at %4 from %5 peers)");
						return templ
								.arg (status.progress * 100, 0, 'f', 2)
								.arg (Util::MakePrettySize (status.total_wanted_done))
								.arg (Util::MakePrettySize (status.total_wanted))
								.arg (Util::MakePrettySize (status.download_payload_rate) +
										tr ("/s"))
								.arg (status.num_peers);
					}
					else if (!IsPaused (status) &&
								(status.state == libtorrent::torrent_status::finished ||
								status.state == libtorrent::torrent_status::seeding))
					{
						auto total = status.num_incomplete;
						if (total <= 0)
							total = status.list_peers - status.list_seeds;
						static const auto templ = tr ("%1, seeding at %2 to %3 leechers (of around %4)");
						return templ
								.arg (Util::MakePrettySize (status.total_wanted))
								.arg (Util::MakePrettySize (status.upload_payload_rate) +
										tr ("/s"))
								.arg (status.num_peers - status.num_seeds)
								.arg (total);
					}
					else
					{
						static const auto templ = tr ("%1% (%2 of %3)");
						return templ
								.arg (status.progress * 100, 0, 'f', 2)
								.arg (Util::MakePrettySize (status.total_wanted_done))
								.arg (Util::MakePrettySize (status.total_wanted));
					}
				}
				else
				{
					if (status.state == libtorrent::torrent_status::downloading)
					{
						static const auto templ = tr ("%1% (%2 of %3)");
						return templ
								.arg (status.progress * 100, 0, 'f', 2)
								.arg (Util::MakePrettySize (status.total_wanted_done))
								.arg (Util::MakePrettySize (status.total_wanted));
					}
					else if (!IsPaused (status) &&
								(status.state == libtorrent::torrent_status::finished ||
								status.state == libtorrent::torrent_status::seeding))
					{
						static const auto templ = QString { "100% (%1)" };
						return templ
								.arg (Util::MakePrettySize (status.total_wanted));
					}
					else
					{
						static const auto templ = tr ("%1% (%2 of %3)");
						return templ
								.arg (status.progress * 100, 0, 'f', 2)
								.arg (Util::MakePrettySize (status.total_wanted_done))
								.arg (Util::MakePrettySize (status.total_wanted));
					}
				}
			case ColumnDownSpeed:
				return Util::MakePrettySize (status.download_payload_rate) + tr ("/s");
			case ColumnUpSpeed:
				return Util::MakePrettySize (status.upload_payload_rate) + tr ("/s");
			case ColumnLeechers:
				return QString::number (status.num_peers - status.num_seeds);
			case ColumnSeeders:
				return QString::number (status.num_seeds);
			case ColumnDownloaded:
				return Util::MakePrettySize (status.all_time_download);
			case ColumnSize:
				return Util::MakePrettySize (status.total_wanted);
			case ColumnUploaded:
				return Util::MakePrettySize (status.all_time_upload);
			case ColumnRatio:
				if (status.all_time_download)
				{
					const auto ratio = static_cast<double> (status.all_time_upload) / status.all_time_download;
					return QString::number (ratio, 'f', 2);
				}

				return status.all_time_upload ?
						QString::fromUtf8 ("∞") :
						"0";
			default:
				return QVariant ();
			}
		case Qt::ToolTipRole:
		{
			QString result;
			const auto& name = QString::fromStdString (status.name);
			result += tr ("Name:") + " " + name + "\n";
			result += tr ("Destination:") + " " +
				QString::fromStdString (status.save_path) + "\n";
			result += tr ("Progress:") + " " +
				QString (tr ("%1% (%2 of %3)")
						.arg (status.progress * 100, 0, 'f', 2)
						.arg (Util::MakePrettySize (status.total_wanted_done))
						.arg (Util::MakePrettySize (status.total_wanted))) + "\n";
			result += tr ("Status:") + " " + GetStringForStatus (status);
			if (status.errc)
				result += " (" + QString::fromStdString (status.errc.message ()) + ")";
			result += "\n";

			result += tr ("Downloading speed:") + " " +
				Util::MakePrettySize (status.download_payload_rate) + tr ("/s") +
				tr ("; uploading speed:") + " " +
				Util::MakePrettySize (status.upload_payload_rate) + tr ("/s") + "\n";
			result += tr ("Peers/seeds: %1/%2").arg (status.num_peers).arg (status.num_seeds);
			return result;
		}
		case RoleTags:
			return Handles_.at (row).Tags_;
		case CustomDataRoles::RoleJobHolderRow:
			return QVariant::fromValue<JobHolderRow> (JobHolderRow::DownloadProgress);
		case JobHolderRole::ProcessState:
		{
			ProcessStateInfo::State state = ProcessStateInfo::State::Running;
			if (status.errc)
				state = ProcessStateInfo::State::Error;
			else if (IsPaused (status))
				state = ProcessStateInfo::State::Paused;

			return QVariant::fromValue<ProcessStateInfo> ({
					status.total_wanted_done,
					status.total_wanted,
					Handles_.at (row).Parameters_,
					state
				});
		}
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

	QIcon Core::GetTorrentIcon (int) const
	{
		return TorrentIcon_;
	}

	libtorrent::torrent_handle Core::GetTorrentHandle (int idx) const
	{
		if (idx >= Handles_.size ())
			return {};

		return Handles_.at (idx).Handle_;
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

	std::unique_ptr<TorrentInfo> Core::GetTorrentStats (int idx) const
	{
		if (!CheckValidity (idx))
			throw std::runtime_error ("Invalid torrent for stats");

		const auto& handle = Handles_.at (idx).Handle_;

		auto result = std::make_unique<TorrentInfo> ();
		result->Status_ = StatusKeeper_->GetStatus (handle, CachedStatusKeeper::AllFlags);
		if (const auto info = handle.torrent_file ())
			result->Info_.reset (new libtorrent::torrent_info (*info));
		result->Destination_ = QString::fromStdString (result->Status_.save_path);
		result->State_ = GetStringForStatus (result->Status_);
		if (result->Status_.errc)
			result->State_ = " (" + QString::fromStdString (result->Status_.errc.message ()) + ")";
		return result;
	}

	SessionStats Core::GetSessionStats () const
	{
		const auto& status = Session_->status ();

		libtorrent::cache_status cacheStatus;
		Session_->get_cache_info (&cacheStatus, {}, libtorrent::session::disk_cache_no_pieces);

		return
		{
			{ status.download_rate, status.upload_rate },
			{ status.ip_overhead_download_rate, status.ip_overhead_upload_rate },
			{ status.dht_download_rate, status.dht_upload_rate },
			{ status.tracker_download_rate, status.tracker_upload_rate },

			{ status.total_download, status.total_upload },
			{ status.total_ip_overhead_download, status.total_ip_overhead_upload },
			{ status.total_dht_download, status.total_dht_upload },
			{ status.total_tracker_download, status.total_tracker_upload },
			{ status.total_payload_download, status.total_payload_upload },

			status.num_peers,
			status.dht_global_nodes,
			status.dht_nodes,
			status.dht_torrents,

			status.total_failed_bytes,
			status.total_redundant_bytes,

			cacheStatus.blocks_written,
			cacheStatus.writes,
			cacheStatus.blocks_read,
			cacheStatus.blocks_read_hit,
			cacheStatus.cache_size,
			cacheStatus.read_cache_size,
		};
	}

	void Core::GetPerTracker (Core::pertrackerstats_t& stats) const
	{
		for (const auto& handle : Handles_)
		{
			const auto& s = handle.Handle_.status ({});
			QString domain = QUrl (s.current_tracker.c_str ()).host ();
			if (domain.size ())
			{
				stats [domain].DownloadRate_ += s.download_payload_rate;
				stats [domain].UploadRate_ += s.upload_payload_rate;
			}
		}
	}

	int Core::GetListenPort () const
	{
		return Session_->listen_port ();
	}

	QStringList Core::GetTagsForIndex (int torrent) const
	{
		return GetTagsForIndexImpl (torrent);
	}

	void Core::UpdateTags (const QStringList& tags, int torrent)
	{
		UpdateTagsImpl (tags, torrent);
	}

	namespace
	{
		libtorrent::storage_mode_t GetCurrentStorageMode ()
		{
			auto sm = XmlSettingsManager::Instance ()->property ("AllocationMode").toString ();
			if (sm == "full")
				return libtorrent::storage_mode_allocate;
			else
				return libtorrent::storage_mode_sparse;
		}

		QFuture<IDownload::Result> MakeErrorResult (const QString& msg)
		{
			return Util::MakeReadyFuture (IDownload::Result::Left ({ IDownload::Error::Type::LocalError, msg }));
		}
	}

#define ATP_FLAG(name) libtorrent::torrent_flags::name;

	QFuture<IDownload::Result> Core::AddMagnet (const QString& magnet,
			const QString& path,
			const QStringList& tags,
			TaskParameters params)
	{
		libtorrent::torrent_handle handle;
		try
		{
			libtorrent::add_torrent_params atp;
			libtorrent::error_code ec;
			libtorrent::parse_magnet_uri (magnet.toStdString (), atp, ec);
			if (ec)
			{
				ShowError (tr ("libtorrent error: %1").arg (QString::fromStdString (ec.message ())));
				return MakeErrorResult ("Torrent error");
			}
			atp.storage_mode = GetCurrentStorageMode ();
			atp.save_path = std::string (path.toUtf8 ().constData ());
			if (params & NoAutostart)
				atp.flags |= ATP_FLAG (paused);
			atp.flags |= ATP_FLAG (duplicate_is_error);
			handle = Session_->add_torrent (atp);
		}
		catch (const std::exception& e)
		{
			HandleLibtorrentException (e);
			return MakeErrorResult ("Torrent error");
		}

		beginInsertRows ({}, Handles_.size (), Handles_.size ());
		Handles_.append ({ handle, tags, params });
		Holder_.AddHandle (handle);
		endInsertRows ();

		return Handles_.back ().Promise_->future ();
	}

	namespace
	{
		void ToggleFlag (libtorrent::torrent_handle& h, libtorrent::torrent_flags_t flags, bool set)
		{
			set ? h.set_flags (flags) : h.unset_flags (flags);
		}

		QByteArray TryReadResumed (const QString& filename)
		{
			const auto& torrentsDir = Util::CreateIfNotExists ("bittorrent");

			QFile resumeDataFile (torrentsDir.filePath (filename + ".resume"));
			if (resumeDataFile.open (QIODevice::ReadOnly))
				return resumeDataFile.readAll ();
			else
				return {};
		}
	}

	QFuture<IDownload::Result> Core::AddFile (const QString& filename,
			const QString& path,
			const QStringList& tags,
			bool tryLive,
			const QVector<bool>& files,
			TaskParameters params)
	{
		QFile file (filename);
		if (!file.open (QIODevice::ReadOnly))
		{
			ShowError (tr ("File %1 could not be read: %2.")
					.arg (filename)
					.arg (file.errorString ()));
			return MakeErrorResult ("Cannot read file");
		}
		const auto& contents = file.readAll ();

		libtorrent::torrent_handle handle;
		bool autoManaged = !(params & NoAutostart);

		libtorrent::add_torrent_params atp;
		try
		{
			if (const auto& resumeData = TryReadResumed (filename);
					!resumeData.isEmpty ())
				atp = libtorrent::read_resume_data (libtorrent::span { resumeData.constData (), resumeData.size () });
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to reuse torrent resume data for"
					<< filename
					<< "although it's been found; ignoring";
		}

		try
		{
			atp.ti = std::make_shared<libtorrent::torrent_info> (contents.constData (), contents.size ());
			atp.storage_mode = GetCurrentStorageMode ();
			atp.save_path = std::string (path.toUtf8 ().constData ());
			if (!autoManaged)
				atp.flags &= ~ATP_FLAG (auto_managed);
			if (tryLive || (params & NoAutostart))
				atp.flags |= ATP_FLAG (paused);
			atp.flags |= ATP_FLAG (duplicate_is_error);

			handle = Session_->add_torrent (atp);
		}
		catch (const std::exception& e)
		{
			HandleLibtorrentException (e);
			return MakeErrorResult ("Torrent error");
		}

		std::vector<libtorrent::download_priority_t > priorities (atp.ti->num_files (), libtorrent::default_priority);
		if (!files.isEmpty ())
		{
			for (int i = 0; i < files.size (); ++i)
				priorities [i] = files [i] ? libtorrent::default_priority : libtorrent::dont_download;

			handle.prioritize_files (priorities);
		}

		ToggleFlag (handle, libtorrent::torrent_flags::auto_managed, autoManaged);

		beginInsertRows (QModelIndex (), Handles_.size (), Handles_.size ());
		auto torrentFileName = QString::fromStdString (handle
					.status (libtorrent::torrent_handle::query_name).name);
		if (!torrentFileName.endsWith (".torrent"))
			torrentFileName.append (".torrent");

		Handles_.append ({
				priorities,
				handle,
				contents,
				torrentFileName,
				tags,
				autoManaged,
				params
			});
		Holder_.AddHandle (handle);
		endInsertRows ();

		if (tryLive)
		{
			LiveStreamManager_->EnableOn (handle);
			handle.resume ();
		}

		ScheduleSave ();
		return Handles_.back ().Promise_->future ();
	}

	void Core::RemoveTorrent (int pos, bool withFiles)
	{
		if (!CheckValidity (pos))
			return;

		beginRemoveRows (QModelIndex (), pos, pos);

		libtorrent::remove_flags_t options;
		if (withFiles)
			options |= libtorrent::session_handle::delete_files;
		Session_->remove_torrent (Handles_.at (pos).Handle_, options);

		Handles_.removeAt (pos);
		Holder_.RemoveHandleAt (pos);

		endRemoveRows ();

		ScheduleSave ();
	}

	void Core::PauseTorrent (int pos)
	{
		if (!CheckValidity (pos))
			return;

		Handles_.at (pos).Handle_.pause ();
		ToggleFlag (Handles_ [pos].Handle_, libtorrent::torrent_flags::auto_managed, false);
		checkFinished ();
	}

	void Core::ResumeTorrent (int pos)
	{
		if (!CheckValidity (pos))
			return;

		Handles_.at (pos).Handle_.resume ();
		Handles_ [pos].State_ = TSIdle;
		ToggleFlag (Handles_ [pos].Handle_, libtorrent::torrent_flags::auto_managed, Handles_.at (pos).AutoManaged_);
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
		catch (const std::exception& e)
		{
			HandleLibtorrentException (e);
			ShowError (tr ("Torrent %1 could not be reannounced at the "
						"moment, try again later.").arg (pos));
		}
	}

	void Core::ForceRecheck (int pos)
	{
		if (!CheckValidity (pos))
			return;

		const auto& handle = Handles_.at (pos).Handle_;
		const auto& status = handle.status ({});
		switch (status.state)
		{
		case libtorrent::torrent_status::checking_files:
		case libtorrent::torrent_status::checking_resume_data:
			return;
		default:
			break;
		}

		handle.force_recheck ();

		if (IsPaused (status) && !IsAutoManaged (status))
		{
			handle.resume ();
			Handles_ [pos].PauseAfterCheck_ = true;
		}
	}

	void Core::AddWebSeed (const QString& ws, WebSeedType type, int idx)
	{
		if (!CheckValidity (idx))
			return;

		auto& handle = Handles_.at (idx).Handle_;
		switch (type)
		{
		case WebSeedType::Bep17:
			handle.add_http_seed (ws.toStdString ());
			break;
		case WebSeedType::Bep19:
			handle.add_url_seed (ws.toStdString ());
			break;
		}
	}

	void Core::RemoveWebSeed (const QString& ws, WebSeedType type, int idx)
	{
		if (!CheckValidity (idx))
			return;

		auto& handle = Handles_.at (idx).Handle_;
		switch (type)
		{
		case WebSeedType::Bep17:
			handle.remove_http_seed (ws.toStdString ());
			break;
		case WebSeedType::Bep19:
			handle.remove_url_seed (ws.toStdString ());
			break;
		}
	}

	void Core::SetFilePriority (int file, int priority, int idx)
	{
		if (!CheckValidity (idx))
			return;

		if (priority > 7)
			priority = 7;
		else if (priority < 0)
			priority = 0;

		try
		{
			Handles_ [idx].FilePriorities_.at (file) = priority;
			Handles_.at (idx).Handle_.prioritize_files (Handles_.at (idx).FilePriorities_);
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
				<< QString ("index for torrent %1, file %2 is out of bounds")
					.arg (idx).arg (file);
		}
	}

	void Core::SetFilename (int index, const QString& name, int idx)
	{
		if (!CheckValidity (idx))
			return;

		Handles_ [idx].Handle_.rename_file (index, std::string (name.toUtf8 ().data ()));
	}

	std::vector<libtorrent::announce_entry> Core::GetTrackers (int row) const
	{
		if (!CheckValidity (row))
			return {};

		return Handles_.at (row).Handle_.trackers ();
	}

	void Core::SetTrackers (const std::vector<libtorrent::announce_entry>& trackers, int row)
	{
		if (!CheckValidity (row))
			return;

		Handles_ [row].Handle_.replace_trackers (trackers);
		Handles_ [row].Handle_.force_reannounce ();
	}

	QString Core::GetMagnetLink (int idx) const
	{
		if (!CheckValidity (idx))
			return QString ();

		const std::string& result = libtorrent::make_magnet_uri (Handles_ [idx].Handle_);
		return QString::fromStdString (result);
	}

	QString Core::GetTorrentDirectory (int idx) const
	{
		if (!CheckValidity (idx))
			return QString ();

		const auto& handle = Handles_.at (idx).Handle_;
		const auto& path = StatusKeeper_->GetStatus (handle,
					libtorrent::torrent_handle::query_save_path).save_path;
		return QString::fromStdString (path);
	}

	bool Core::MoveTorrentFiles (const QString& newDir, int idx)
	{
		if (!CheckValidity (idx) || newDir == GetTorrentDirectory (idx))
			return false;

		Handles_.at (idx).Handle_.move_storage (newDir.toUtf8 ().constData ());
		return true;
	}

	bool Core::IsTorrentManaged (int idx) const
	{
		if (!CheckValidity (idx))
			return false;

		return IsAutoManaged (StatusKeeper_->GetStatus (Handles_.at (idx).Handle_));
	}

	void Core::SetTorrentManaged (bool man, int idx)
	{
		if (!CheckValidity (idx))
			return;

		ToggleFlag (Handles_ [idx].Handle_, libtorrent::torrent_flags::auto_managed, man);
		Handles_ [idx].AutoManaged_ = man;
	}

	bool Core::IsTorrentSequentialDownload (int idx) const
	{
		if (!CheckValidity (idx))
			return false;

		const auto& status = StatusKeeper_->GetStatus (Handles_.at (idx).Handle_);
		return status.flags & libtorrent::torrent_flags::sequential_download;
	}

	void Core::SetTorrentSequentialDownload (bool seq, int idx)
	{
		if (!CheckValidity (idx))
			return;

		ToggleFlag (Handles_ [idx].Handle_, libtorrent::torrent_flags::sequential_download, seq);
	}

	bool Core::IsTorrentSuperSeeding (int idx) const
	{
		if (!CheckValidity (idx))
			return false;

		const auto& status = StatusKeeper_->GetStatus (Handles_.at (idx).Handle_);
		return status.flags & libtorrent::torrent_flags::super_seeding;
	}

	void Core::SetTorrentSuperSeeding (bool sup, int idx)
	{
		if (!CheckValidity (idx))
			return;

		ToggleFlag (Handles_ [idx].Handle_, libtorrent::torrent_flags::super_seeding, sup);
	}

	void Core::MakeTorrent (const NewTorrentParams& params) const
	{
		const auto tm = new TorrentMaker { Proxy_ };
		tm->Start (params);
	}

	void Core::SetExternalAddress (const QString& address)
	{
		ExternalAddress_ = address;
	}

	QString Core::GetExternalAddress () const
	{
		return ExternalAddress_;
	}

	void Core::SaveResumeData (const libtorrent::save_resume_data_alert& a) const
	{
		const auto torrent = FindHandle (a.handle);
		if (torrent == Handles_.end ())
		{
			qWarning () << Q_FUNC_INFO
				<< "this torrent doesn't exist anymore";
			return;
		}

		const auto& status = a.handle.status ({});
		if (status.errc)
		{
			qWarning () << Q_FUNC_INFO
					<< "not saving erroneous torrent:"
					<< StatusKeeper_->GetStatus (a.handle, libtorrent::torrent_handle::query_name)
							.name.c_str ();
			return;
		}

		const auto& filePath = Util::CreateIfNotExists ("bittorrent")
				.filePath (torrent->TorrentFileName_ + ".resume");
		QFile file { filePath };

		if (!file.open (QIODevice::WriteOnly))
		{
			qWarning () << QString ("Could not open file %1 for write: %2")
				.arg (file.fileName ())
				.arg (file.errorString ());
			return;
		}

		const auto& buf = libtorrent::write_resume_data_buf (a.params);
		file.write (buf.data (), buf.size ());
	}

	void Core::HandleMetadata (const libtorrent::metadata_received_alert& a)
	{
		const auto torrent = FindHandle (a.handle);
		if (torrent == Handles_.end ())
		{
			qWarning () << Q_FUNC_INFO
				<< "this torrent doesn't exist anymore";
			return;
		}

		const auto& file = a.handle.torrent_file ();
		if (!file)
		{
			qWarning () << Q_FUNC_INFO
					<< "torrent doesn't have a torrent file yet";
			return;
		}

		const auto& info = *file;
		torrent->TorrentFileName_ = QString::fromUtf8 (info.name ().c_str ()) + ".torrent";
		torrent->FilePriorities_.resize (info.num_files ());
		std::fill (torrent->FilePriorities_.begin (),
				torrent->FilePriorities_.end (), 1);

		libtorrent::error_code ec;
		const libtorrent::span metadata { info.metadata ().get (), info.metadata_size () };
		libtorrent::entry infoE = libtorrent::bdecode (metadata, ec);
		if (ec)
		{
			qWarning () << Q_FUNC_INFO
					 << "unable to bdecode"
					 << torrent->TorrentFileName_;
			return;
		}
		libtorrent::entry e;
		e ["info"] = infoE;
		libtorrent::bencode (std::back_inserter (torrent->TorrentFileContents_), e);

		qDebug () << "HandleMetadata"
			<< std::distance (Handles_.begin (), torrent)
			<< torrent->TorrentFileName_;

		ScheduleSave ();
	}

	void Core::PieceRead (const libtorrent::read_piece_alert& a)
	{
		LiveStreamManager_->PieceRead (a);
	}

	void Core::UpdateStatus (const std::vector<libtorrent::torrent_status>& statuses)
	{
		if (statuses.empty ())
			return;

		for (const auto& status : statuses)
		{
			StatusKeeper_->HandleStatusUpdatePosted (status);
			const auto pos = FindHandle (status.handle);
			if (pos == Handles_.end ())
			{
				qWarning () << Q_FUNC_INFO
						<< "unknown handle";
				continue;
			}

			const auto row = std::distance (Handles_.begin (), pos);
			emit dataChanged (index (row, 0), index (row, columnCount () - 1));
		}

		emit torrentsStatusesUpdated ();
	}

	void Core::HandleTorrentChecked (const libtorrent::torrent_handle& h)
	{
		const auto pos = FindHandle (h);
		if (pos == Handles_.end ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown torrent handle"
					<< StatusKeeper_->GetStatus (h, libtorrent::torrent_handle::query_name)
							.name.c_str ();
			return;
		}

		if (!pos->PauseAfterCheck_)
			return;

		pos->PauseAfterCheck_ = false;
		h.pause ();
	}

	void Core::MoveUp (const std::vector<int>& selections)
	{
		if (!selections.size ())
			return;

		for (auto i = selections.begin (),
				end = selections.end (); i != end; ++i)
			if (*i <= 0 || !CheckValidity (*i))
				return;

		for (auto i = selections.begin (),
				end = selections.end (); i != end; ++i)
		{
			Handles_.at (*i).Handle_.queue_position_up ();
			std::swap (Handles_ [*i],
					Handles_ [*i - 1]);
			Holder_.MoveUp (*i);

			emit dataChanged (index (*i - 1, 0),
					index (*i, columnCount () - 1));
		}
	}

	void Core::MoveDown (const std::vector<int>& selections)
	{
		if (!selections.size ())
			return;

		for (auto i = selections.begin (),
				end = selections.end (); i != end; ++i)
			if (*i < 0 || !CheckValidity (*i) || !CheckValidity (*i + 1))
				return;

		for (auto i = selections.rbegin (),
				end = selections.rend (); i != end; ++i)
		{
			Handles_.at (*i).Handle_.queue_position_down ();
			std::swap (Handles_ [*i],
					Handles_ [*i + 1]);
			Holder_.MoveDown (*i);

			emit dataChanged (index (*i, 0),
					index (*i + 1, columnCount () - 1));
		}
	}

	void Core::MoveToTop (const std::vector<int>& selections)
	{
		if (!selections.size ())
			return;

		for (auto i = selections.begin (),
				end = selections.end (); i != end; ++i)
			if (*i <= 0 || !CheckValidity (*i))
				return;

		for (auto i = selections.rbegin (),
				end = selections.rend (); i != end; ++i)
			MoveToTop (*i);
	}

	void Core::MoveToBottom (const std::vector<int>& selections)
	{
		if (!selections.size ())
			return;

		for (auto i = selections.begin (),
				end = selections.end (); i != end; ++i)
			if (*i < 0 || !CheckValidity (*i))
				return;

		for (auto i = selections.begin (),
				end = selections.end (); i != end; ++i)
			MoveToBottom (*i);
	}

	QList<FileInfo> Core::GetTorrentFiles (int idx) const
	{
		if (!CheckValidity (idx))
			return {};

		QList<FileInfo> result;
		const auto& handle = Handles_.at (idx).Handle_;
		const auto& infoPtr = StatusKeeper_->GetStatus (handle,
					libtorrent::torrent_handle::query_torrent_file).torrent_file.lock ();
		if (!infoPtr)
			return {};

		const auto& info = *infoPtr;

		std::vector<std::int64_t> prbytes;

		int flags = 0;
		if (!XmlSettingsManager::Instance ()->
				property ("AccurateFileProgress").toBool ())
			flags |= libtorrent::torrent_handle::piece_granularity;
		handle.file_progress (prbytes, flags);

		for (int i = 0, numFiles = info.num_files (); i < numFiles; ++i)
		{
			FileInfo fi;
			fi.Path_ = info.files ().file_path (i);
			fi.Size_ = info.files ().file_size (i);
			fi.Priority_ = Handles_.at (idx).FilePriorities_.at (i);
			fi.Progress_ = fi.Size_ ?
					prbytes.at (i) / static_cast<float> (fi.Size_) :
					1;
			result << fi;
		}

		return result;
	}

	auto Core::FindHandle (const libtorrent::torrent_handle& h) -> HandleDict_t::iterator
	{
		return std::find_if (Handles_.begin (), Handles_.end (),
				[&h] (const TorrentStruct& ts) { return ts.Handle_ == h; });
	}

	auto Core::FindHandle (const libtorrent::torrent_handle& h) const -> HandleDict_t::const_iterator
	{
		return std::find_if (Handles_.begin (), Handles_.end (),
				[&h] (const TorrentStruct& ts) { return ts.Handle_ == h; });
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

		Holder_.MoveToTop (row);
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

		Holder_.MoveToBottom (row);
	}

	void Core::RestoreTorrents ()
	{
		const auto& torrentsDir = Util::CreateIfNotExists ("bittorrent");

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Torrent");
		settings.beginGroup ("Core");
		int torrents = settings.beginReadArray ("AddedTorrents");
		qDebug () << Q_FUNC_INFO << "gonna restore" << torrents << "torrents";
		for (int i = 0; i < torrents; ++i)
		{
			settings.setArrayIndex (i);
			QString filename = settings.value ("Filename").toString ();
			QFile torrent (torrentsDir.filePath (filename));
			if (!torrent.open (QIODevice::ReadOnly))
			{
				ShowError (tr ("Could not open saved torrent %1 for read.").arg (filename));
				continue;
			}
			QByteArray data = torrent.readAll ();
			torrent.close ();
			if (data.isEmpty ())
			{
				qWarning () << Q_FUNC_INFO
						<< "empty torrent data for"
						<< filename;
				continue;
			}

			bool automanaged = settings.value ("AutoManaged", true).toBool ();
			TaskParameters taskParameters = static_cast<TaskParameters> (settings
						.value ("Parameters").toInt ());

			auto handle = RestoreSingleTorrent (data,
					TryReadResumed (filename),
					automanaged,
					taskParameters & NoAutostart);
			if (!handle.is_valid ())
			{
				qWarning () << Q_FUNC_INFO
						<< "got invalid handle for"
						<< filename;
				continue;
			}

			const auto& prioritiesLine = settings.value ("Priorities").toByteArray ();
			std::vector<libtorrent::download_priority_t> priorities;
			priorities.reserve (prioritiesLine.size ());
			for (const auto ch : prioritiesLine)
				priorities.emplace_back (ch);

			if (priorities.empty ())
			{
				const auto& infoPtr = StatusKeeper_->GetStatus (handle,
							libtorrent::torrent_handle::query_torrent_file).torrent_file.lock ();
				priorities.resize (infoPtr ? infoPtr->num_files () : 0, libtorrent::default_priority);
			}

			handle.prioritize_files (priorities);

			beginInsertRows ({}, Handles_.size (), Handles_.size ());
			Handles_.append ({
					priorities,
					handle,
					data,
					filename,
					settings.value ("Tags").toStringList (),
					automanaged,
					taskParameters,
					TorrentStruct::NoFuture {}
				});
			Holder_.AddHandle (handle);
			endInsertRows ();
			qDebug () << "restored a torrent";
		}
		settings.endArray ();
		settings.endGroup ();

		RestoreFilter (*Session_);
	}

	libtorrent::torrent_handle Core::RestoreSingleTorrent (const QByteArray& data,
			const QByteArray& resumeData,
			bool automanaged,
			bool pause)
	{
		libtorrent::torrent_handle handle;

		libtorrent::bdecode_node e;
		if (!DecodeEntry (data, e))
			return handle;

		try
		{
			auto atp = libtorrent::read_resume_data (libtorrent::span { resumeData.constData (), resumeData.size () });
			atp.ti = std::make_shared<libtorrent::torrent_info> (e);

			if (!automanaged)
				atp.flags &= ~ATP_FLAG (auto_managed);
			if (pause)
				atp.flags |= ATP_FLAG (paused);
			atp.flags |= ATP_FLAG (duplicate_is_error);

			handle = Session_->add_torrent (atp);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO << e.what ();
			HandleLibtorrentException (e);
		}

		return handle;
	}

	void Core::HandleSingleFinished (int i)
	{
		TorrentStruct torrent = Handles_.at (i);
		const auto& status = StatusKeeper_->GetStatus (torrent.Handle_,
				libtorrent::torrent_handle::query_save_path |
				libtorrent::torrent_handle::query_torrent_file |
				libtorrent::torrent_handle::query_name |
				libtorrent::torrent_handle::query_pieces);

		const auto& name = QString::fromStdString (status.name);

		const auto filePtr = status.torrent_file.lock ();
		if (!filePtr)
		{
			qWarning () << Q_FUNC_INFO
					<< "torrent"
					<< name
					<< "has finished, but we don't have its info";
			return;
		}

		const auto& info = *filePtr;

		if (LiveStreamManager_->IsEnabledOn (torrent.Handle_) &&
				status.num_pieces != info.num_pieces ())
			return;

		auto notifyE = Util::MakeAN ("BitTorrent",
				tr ("Torrent finished: %1").arg (name),
				Priority::Info,
				"org.LeechCraft.BitTorrent",
				AN::CatDownloads,
				AN::TypeDownloadFinished,
				"org.LC.Plugins.BitTorrent.DLFinished/" + name,
				QStringList (name));

		const auto& savePath = status.save_path;
		const auto& savePathStr = QString::fromUtf8 (savePath.c_str ());

		const auto iem = Proxy_->GetEntityManager ();

		auto nah = new Util::NotificationActionHandler (notifyE);
		if (info.files ().num_files () == 1)
		{
			const QByteArray path { (savePath + '/' + info.files ().file_path (0)).c_str () };
			nah->AddFunction (tr ("Open..."),
					[iem, path]
					{
						iem->HandleEntity (Util::MakeEntity (QUrl::fromLocalFile (path),
								{},
								FromUserInitiated));
					});
		}
		nah->AddFunction (tr ("Show folder"),
				[savePathStr]
				{
					const auto& dirPath = QFileInfo (savePathStr).absolutePath ();
					QDesktopServices::openUrl (QUrl::fromLocalFile (dirPath));
				});
		iem->HandleEntity (notifyE);

		auto localeCodec = QTextCodec::codecForLocale ();
		Entity e;
		e.Parameters_ = IsDownloaded;
		e.Location_ = torrent.TorrentFileName_;
		e.Additional_ [" Tags"] = torrent.Tags_;
		e.Additional_ ["IgnorePlugins"] = QStringList ("org.LeechCraft.BitTorrent");

		for (int i = 0, numFiles = info.num_files (); i < numFiles; ++i)
		{
			const QByteArray path { (savePath + '/' + info.files ().file_path (i)).c_str () };
			e.Entity_ = QUrl::fromLocalFile (localeCodec->toUnicode (path));
			iem->HandleEntity (e);
		}

		if (torrent.Promise_)
			Util::ReportFutureResult (*torrent.Promise_, IDownload::Result::Right ({}));
	}

	void Core::HandleFileRenamed (const libtorrent::file_renamed_alert& a)
	{
		const auto pos = FindHandle (a.handle);
		if (pos == Handles_.end ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown handle";
			return;
		}

		const auto& newName = QString::fromUtf8 (a.new_name ());
		emit fileRenamed (std::distance (Handles_.begin (), pos), a.index, newName);
	}

	QStringList Core::GetTagsForIndexImpl (int torrent) const
	{
		if (!CheckValidity (torrent))
			return {};

		return Util::Map (Handles_.at (torrent).Tags_,
				[this] (const QString& id) { return Proxy_->GetTagsManager ()->GetTag (id); });
	}

	void Core::UpdateTagsImpl (const QStringList& tags, int torrent)
	{
		if (!CheckValidity (torrent))
			return;

		Handles_ [torrent].Tags_ = Util::Map (tags,
				[this] (const QString& tag) { return Proxy_->GetTagsManager ()->GetID (tag); });
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

	void Core::HandleLibtorrentException (const std::exception& e)
	{
		ShowError (tr ("libtorrent error: %1")
				.arg (e.what ()));
	}

	void Core::ShowError (const QString& msg)
	{
		const auto& e = Util::MakeNotification ("BitTorrent", msg, Priority::Critical);
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}

	void Core::writeSettings ()
	{
		SaveScheduled_ = false;

		const auto& torrentsDir = Util::CreateIfNotExists ("bittorrent");

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Torrent");
		settings.beginGroup ("Core");
		settings.beginWriteArray ("AddedTorrents");
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
			try
			{
				QFile file_info (torrentsDir.filePath (Handles_.at (i).TorrentFileName_));
				if (!file_info.open (QIODevice::WriteOnly))
					ShowError (QString ("Cannot write settings! "
								"Cannot open file %1 for write!")
							.arg (Handles_.at (i).TorrentFileName_));
				else
				{
					file_info.write (Handles_.at (i).TorrentFileContents_);
					file_info.close ();

					const auto& handle = Handles_.at (i).Handle_;
					if (handle.need_save_resume_data ())
						handle.save_resume_data ();

					const auto& savePath = StatusKeeper_->GetStatus (handle,
								libtorrent::torrent_handle::query_save_path).save_path;
					settings.setValue ("SavePath", QString::fromUtf8 (savePath.c_str ()));
					settings.setValue ("Filename", Handles_.at (i).TorrentFileName_);
					settings.setValue ("Tags", Handles_.at (i).Tags_);
					settings.setValue ("Parameters", static_cast<int> (Handles_.at (i).Parameters_));
					settings.setValue ("AutoManaged", Handles_.at (i).AutoManaged_);

					QByteArray prioritiesLine;
					prioritiesLine.reserve (Handles_.at (i).FilePriorities_.size ());
					for (const auto prio : Handles_.at (i).FilePriorities_)
						prioritiesLine.push_back (static_cast<uint8_t> (prio));
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
		}
		settings.endArray ();

		settings.endGroup ();

		constexpr auto saveflags = libtorrent::save_state_flags_t::all ();
		libtorrent::entry sessionState;
		Session_->save_state (sessionState, saveflags);

		QByteArray sessionStateBA;
		libtorrent::bencode (std::back_inserter (sessionStateBA), sessionState);
		XmlSettingsManager::Instance ()->setProperty ("SessionState", sessionStateBA);

		Session_->wait_for_alert (libtorrent::time_duration (5));

		queryLibtorrent ();
	}

	void Core::checkFinished ()
	{
		for (int i = 0; i < Handles_.size (); ++i)
		{
			if (Handles_.at (i).State_ == TSSeeding)
				continue;

			const auto& status = Handles_.at (i).Handle_.status ({});
			libtorrent::torrent_status::state_t state = status.state;

			if (IsAutoManaged (status))
			{
				Handles_ [i].State_ = TSIdle;
				continue;
			}

			switch (state)
			{
			case libtorrent::torrent_status::checking_files:
			case libtorrent::torrent_status::checking_resume_data:
			case libtorrent::torrent_status::allocating:
			case libtorrent::torrent_status::downloading_metadata:
				Handles_ [i].State_ = TSPreparing;
				break;
			case libtorrent::torrent_status::downloading:
				Handles_ [i].State_ = TSDownloading;
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

	struct SimpleDispatcher
	{
		bool NeedToLog_ = true;

		IEntityManager * const IEM_;
		Core& Core_;

		SimpleDispatcher (Core& core, const ICoreProxy_ptr& proxy)
		: IEM_ { proxy->GetEntityManager () }
		, Core_ { core }
		{
		}

		void operator() (const libtorrent::external_ip_alert& a) const
		{
			const auto& extAddrStr = QString::fromStdString (a.external_address.to_string ());
			Core_.SetExternalAddress (extAddrStr);
		}

		void operator() (const libtorrent::save_resume_data_alert& a) const
		{
			Core_.SaveResumeData (a);
		}

		void operator() (const libtorrent::save_resume_data_failed_alert& a) const
		{
			const auto& text = QObject::tr ("Saving resume data failed for torrent:<br />%1<br />%2")
					.arg (GetTorrentName (a.handle))
					.arg (QString::fromUtf8 (a.error.message ().c_str ()));
			IEM_->HandleEntity (Util::MakeNotification ("BitTorrent", text, Priority::Warning));
		}

		void operator() (const libtorrent::storage_moved_alert& a) const
		{
			const auto& text = QObject::tr ("Storage for torrent:<br />%1"
						"<br />moved successfully to:<br />%2")
					.arg (GetTorrentName (a.handle))
					.arg (QString::fromUtf8 (a.storage_path ()));
			IEM_->HandleEntity (Util::MakeNotification ("BitTorrent", text, Priority::Info));
		}

		void operator() (const libtorrent::storage_moved_failed_alert& a) const
		{
			const auto& text = QObject::tr ("Storage move failure:<br />%2<br />for torrent:<br />%1")
					.arg (GetTorrentName (a.handle))
					.arg (QString::fromUtf8 (a.error.message ().c_str ()));
			IEM_->HandleEntity (Util::MakeNotification ("BitTorrent", text, Priority::Critical));
		}

		void operator() (const libtorrent::metadata_received_alert& a) const
		{
			Core_.HandleMetadata (a);
		}

		void operator() (const libtorrent::file_renamed_alert& a) const
		{
			Core_.HandleFileRenamed (a);
		}

		void operator() (const libtorrent::file_rename_failed_alert& a) const
		{
			const auto& text = QObject::tr ("File rename failed for torrent:<br />%1<br />"
						"file %2, error:<br />%3")
					.arg (GetTorrentName (a.handle))
					.arg (QString::number (a.index))
					.arg (QString::fromUtf8 (a.error.message ().c_str ()));
			IEM_->HandleEntity (Util::MakeNotification ("BitTorrent", text, Priority::Critical));
		}

		void operator() (const libtorrent::torrent_delete_failed_alert& a) const
		{
			const auto& text = QObject::tr ("Failed to delete torrent:<br />%1<br />error:<br />%2")
					.arg (GetTorrentName (a.handle))
					.arg (QString::fromUtf8 (a.error.message ().c_str ()));
			IEM_->HandleEntity (Util::MakeNotification ("BitTorrent", text, Priority::Critical));
		}

		void operator() (const libtorrent::read_piece_alert& a) const
		{
			Core_.PieceRead (a);
		}

		void operator() (const libtorrent::state_update_alert& a)
		{
			Core_.UpdateStatus (a.status);
			NeedToLog_ = false;
		}

		void operator() (const libtorrent::torrent_paused_alert& a) const
		{
			Core_.UpdateStatus ({ a.handle.status () });
		}

		void operator() (const libtorrent::torrent_resumed_alert& a) const
		{
			Core_.UpdateStatus ({ a.handle.status () });
		}

		void operator() (const libtorrent::torrent_checked_alert& a) const
		{
			Core_.HandleTorrentChecked (a.handle);
			Core_.UpdateStatus ({ a.handle.status () });
		}

		void operator() (const libtorrent::dht_announce_alert& a)
		{
			qDebug () << "<libtorrent> <DHT>"
					<< "got announce from"
					<< a.ip.to_string ().c_str ()
					<< ":"
					<< a.port
					<< "; the SHA1 hash is"
					<< QByteArray::fromStdString (a.info_hash.to_string ());
			NeedToLog_ = false;
		}

		void operator() (const libtorrent::dht_reply_alert& a)
		{
			qDebug () << "<libtorrent> <DHT>"
					<< "got reply with"
					<< a.num_peers
					<< "peers";
			NeedToLog_ = false;
		}

		void operator() (const libtorrent::dht_bootstrap_alert& a)
		{
			qDebug () << "<libtorrent> <DHT>"
					<< "bootstrapped; "
					<< a.message ().c_str ();
			NeedToLog_ = false;
		}

		void operator() (const libtorrent::dht_get_peers_alert& a)
		{
			qDebug () << "<libtorrent> <DHT>"
					<< "got peers for"
					<< QByteArray::fromStdString (a.info_hash.to_string ());
			NeedToLog_ = false;
		}

		void operator() (const libtorrent::file_error_alert& a) const
		{
			const auto& text = QObject::tr ("File error for torrent:<br />%1<br />"
						"file:<br />%2<br />error:<br />%3")
					.arg (GetTorrentName (a.handle))
					.arg (QString::fromUtf8 (a.filename ()))
					.arg (QString::fromUtf8 (a.error.message ().c_str ()));
			IEM_->HandleEntity (Util::MakeNotification ("BitTorrent", text, Priority::Critical));
		}

		void operator() (const libtorrent::torrent_error_alert& a) const
		{
			Core_.UpdateStatus ({ a.handle.status () });
		}

		void operator() (const libtorrent::piece_finished_alert&) const
		{
		}

		void operator() (const libtorrent::torrent_removed_alert&) const
		{
		}

		void operator() (const libtorrent::torrent_deleted_alert&) const
		{
		}
	private:
		QString GetTorrentName (const libtorrent::torrent_handle& handle) const
		{
			const auto& status = Core_.GetStatusKeeper ()->GetStatus (handle, libtorrent::torrent_handle::query_name);
			return QString::fromStdString (status.name);
		}
	};

	namespace
	{
		template<typename Dispatcher, typename... Types>
		struct HandleAlertImpl;

		template<typename Dispatcher>
		struct HandleAlertImpl<Dispatcher>
		{
			const int Info_;
			Dispatcher& D_;

			void operator() (libtorrent::alert *alert) const
			{
				qDebug () << Q_FUNC_INFO
						<< "unhandled alert type"
						<< Info_
						<< ":"
						<< alert->message ().c_str ();
			}
		};

		template<typename Dispatcher, typename Head, typename... Tail>
		struct HandleAlertImpl<Dispatcher, Head, Tail...>
		{
			const int Info_;
			Dispatcher& D_;

			void operator() (libtorrent::alert *alert) const
			{
				if (Info_ == Head::alert_type)
					D_ (*static_cast<Head*> (alert));
				else
					HandleAlertImpl<Dispatcher, Tail...> { Info_, D_ } (alert);
			}
		};

		template<typename... Types, typename Dispatcher>
		void HandleAlert (libtorrent::alert *alert, Dispatcher& dispatcher)
		{
			HandleAlertImpl<Dispatcher, Types...> { alert->type (), dispatcher } (alert);
		}
	}

	void Core::queryLibtorrent ()
	{
		Session_->post_torrent_updates ();

		std::vector<libtorrent::alert*> alerts;
		Session_->pop_alerts (&alerts);
		for (const auto alert : alerts)
		{
			SimpleDispatcher sd { *this, Proxy_ };
			try
			{
				HandleAlert<
					libtorrent::external_ip_alert
					, libtorrent::save_resume_data_alert
					, libtorrent::save_resume_data_failed_alert
					, libtorrent::storage_moved_alert
					, libtorrent::storage_moved_failed_alert
					, libtorrent::metadata_received_alert
					, libtorrent::file_error_alert
					, libtorrent::file_renamed_alert
					, libtorrent::file_rename_failed_alert
					, libtorrent::read_piece_alert
					, libtorrent::state_update_alert
					, libtorrent::torrent_paused_alert
					, libtorrent::torrent_resumed_alert
					, libtorrent::torrent_checked_alert
					, libtorrent::dht_announce_alert
					, libtorrent::dht_reply_alert
					, libtorrent::dht_bootstrap_alert
					, libtorrent::dht_get_peers_alert
					, libtorrent::torrent_error_alert
					, libtorrent::piece_finished_alert
					, libtorrent::torrent_removed_alert
					, libtorrent::torrent_deleted_alert
					> (alert, sd);
			}
			catch (const std::exception&)
			{
			}

			try
			{
				if (sd.NeedToLog_)
				{
					const auto& logmsg = QString::fromUtf8 (alert->message ().c_str ());
					qDebug () << "<libtorrent>" << logmsg;
				}
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO << typeid (e).name ();
			}
		}
	}

	void Core::scrape ()
	{
		for (const auto& item : Handles_)
			item.Handle_.scrape_tracker ();
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
}
}
