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
#include <interfaces/core/iiconthememanager.h>
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
#include "livestreammanager.h"
#include "torrentmaker.h"
#include "sessionsettingsmanager.h"
#include "cachedstatuskeeper.h"
#include "sessionstats.h"
#include "ltutils.h"
#include "newtorrentparams.h"

Q_DECLARE_METATYPE (QMenu*)
Q_DECLARE_METATYPE (QToolBar*)
Q_DECLARE_METATYPE (const libtorrent::torrent_handle*)

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
	, Dispatcher_ { *Session_ }
	{
		setObjectName ("BitTorrent Core");

		using namespace libtorrent;
		auto iem = GetProxyHolder ()->GetEntityManager ();
		Dispatcher_.RegisterHandler ([this] (const save_resume_data_alert& a) { SaveResumeData (a); });
		Dispatcher_.RegisterHandler ([iem] (const save_resume_data_failed_alert& a)
				{
					const auto& text = tr ("Saving resume data failed for torrent:<br />%1<br />%2")
							.arg (a.torrent_name (),
								  a.error.message ().c_str ());
					iem->HandleEntity (Util::MakeNotification ("BitTorrent", text, Priority::Warning));
				});
		Dispatcher_.RegisterHandler ([iem] (const storage_moved_alert& a)
				{
					const auto& text = tr ("Storage for torrent:<br />%1<br />moved successfully to:<br />%2")
							.arg (a.torrent_name (),
								  a.storage_path ());
					iem->HandleEntity (Util::MakeNotification ("BitTorrent", text, Priority::Info));
				});
		Dispatcher_.RegisterHandler ([iem] (const storage_moved_failed_alert& a)
				{
					const auto& text = tr ("Storage move failure:<br />%2<br />for torrent:<br />%1")
							.arg (a.torrent_name (),
								  a.error.message ().c_str ());
					iem->HandleEntity (Util::MakeNotification ("BitTorrent", text, Priority::Critical));
				});
		Dispatcher_.RegisterHandler ([this] (const metadata_received_alert& a) { HandleMetadata (a); });
		Dispatcher_.RegisterHandler ([iem] (const file_rename_failed_alert& a)
				{
					const auto& text = tr ("File rename failed for torrent:<br />%1<br />file %2, error:<br />%3")
							.arg (a.torrent_name (),
								  QString::number (a.index),
								  a.error.message ().c_str ());
					iem->HandleEntity (Util::MakeNotification ("BitTorrent", text, Priority::Critical));
				});
		Dispatcher_.RegisterHandler ([iem] (const torrent_delete_failed_alert& a)
				{
					const auto& text = tr ("Failed to delete torrent:<br />%1<br />error:<br />%2")
							.arg (a.torrent_name (),
								  a.error.message ().c_str ());
					iem->HandleEntity (Util::MakeNotification ("BitTorrent", text, Priority::Critical));
				});
		Dispatcher_.RegisterHandler ([iem] (const file_error_alert& a)
				{
					const auto& text = tr ("File error for torrent:<br />%1<br />file:<br />%2<br />error:<br />%3")
							.arg (a.torrent_name (),
								  a.filename (),
								  a.error.message ().c_str ());
					iem->HandleEntity (Util::MakeNotification ("BitTorrent", text, Priority::Critical));
				});
		Dispatcher_.RegisterHandler ([this] (const state_update_alert& a)
				{
					UpdateStatus (a.status);
					return false;
				});
		Dispatcher_.RegisterHandler ([this] (const torrent_paused_alert& a) { UpdateStatus ({ a.handle.status () }); });
		Dispatcher_.RegisterHandler ([this] (const torrent_resumed_alert& a) { UpdateStatus ({ a.handle.status () }); });
		Dispatcher_.RegisterHandler ([this] (const state_changed_alert& a) { UpdateStatus ({ a.handle.status () }); });
		Dispatcher_.RegisterHandler ([this] (const torrent_error_alert& a) { UpdateStatus ({ a.handle.status () }); });
		Dispatcher_.RegisterHandler ([this] (const torrent_checked_alert& a)
				{
					HandleTorrentChecked (a.handle);
					UpdateStatus ({ a.handle.status () });
				});

		Dispatcher_.Swallow (torrent_finished_alert::alert_type, false);
		Dispatcher_.Swallow (file_completed_alert::alert_type, false);
		Dispatcher_.Swallow (tracker_announce_alert::alert_type, false);
		Dispatcher_.Swallow (cache_flushed_alert::alert_type, false);
		Dispatcher_.Swallow (torrent_removed_alert::alert_type, true);
		Dispatcher_.Swallow (torrent_deleted_alert::alert_type, true);
		Dispatcher_.Swallow (listen_succeeded_alert::alert_type, true);
		Dispatcher_.Swallow (dht_announce_alert::alert_type, true);
		Dispatcher_.Swallow (dht_reply_alert::alert_type, true);
		Dispatcher_.Swallow (dht_bootstrap_alert::alert_type, true);
		Dispatcher_.Swallow (dht_get_peers_alert::alert_type, true);
		Dispatcher_.Swallow (external_ip_alert::alert_type, true);
	}

	AlertDispatcher& Core::GetAlertDispatcher ()
	{
		return Dispatcher_;
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
			SessionSettingsMgr_ = new SessionSettingsManager { Session_, this };

			auto sstateVariant = XmlSettingsManager::Instance ()->property ("SessionState");
			if (sstateVariant.isValid () &&
					!sstateVariant.toByteArray ().isEmpty ())
			{
				libtorrent::bdecode_node state;
				if (DecodeEntry (sstateVariant.toByteArray (), state))
				{
					Session_->load_state (state);

					libtorrent::settings_pack settings;
					settings.set_int (libtorrent::settings_pack::alert_queue_size, 10000);
					{
						namespace cat = libtorrent::alert_category;
						settings.set_int (libtorrent::settings_pack::alert_mask,
								cat::all &
								~cat::block_progress &
								~cat::peer &
								~cat::upload &
								~cat::stats &
								~cat::connect &
								~cat::piece_progress &
								~cat::peer_log &
								~cat::torrent_log &
								~cat::dht_log &
								~cat::port_mapping_log &
								~cat::picker_log &
								~cat::session_log &
								~libtorrent::alert::progress_notification);
					}
					Session_->apply_settings (settings);
				}
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
		LiveStreamManager_ = std::make_shared<LiveStreamManager> (*StatusKeeper_, Dispatcher_);
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
			case libtorrent::torrent_status::checking_resume_data:
				return Core::tr ("Checking resume data");
			}
			return {};
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

		TorrentInfo GetTorrentStats (const libtorrent::torrent_handle& handle, CachedStatusKeeper& keeper)
		{
			auto status = keeper.GetStatus (handle, CachedStatusKeeper::AllFlags);
			TorrentInfo info
			{
				.Destination_ = QString::fromStdString (status.save_path),
				.State_ = GetStringForStatus (status),
				.Status_ = status,
				.Info_ = handle.torrent_file () ? *handle.torrent_file () : std::optional<libtorrent::torrent_info> {},
			};
			if (status.errc)
				info.State_ += " (" + QString::fromStdString (status.errc.message ()) + ")";
			return info;

		}

		void ToggleFlag (libtorrent::torrent_handle& h, libtorrent::torrent_flags_t flags, bool set)
		{
			set ? h.set_flags (flags) : h.unset_flags (flags);
		}
	}

	bool Core::setData (const QModelIndex& index, const QVariant& data, int role)
	{
		auto& ts = Handles_ [index.row ()];
		switch (role)
		{
		case Roles::TorrentTags:
			ts.Tags_ = Proxy_->GetTagsManager ()->GetIDs (data.toStringList ());
			return true;
		case Roles::IsManaged:
			ts.AutoManaged_ = data.toBool ();
			return true;
		case Roles::IsSequentialDownloading:
			ToggleFlag (ts.Handle_, libtorrent::torrent_flags::sequential_download, data.toBool ());
			return true;
		case Roles::IsSuperSeeding:
			ToggleFlag (ts.Handle_, libtorrent::torrent_flags::super_seeding, data.toBool ());
			return true;
		}

		return false;
	}

	QVariant Core::data (const QModelIndex& index, int role) const
	{
		const int row = index.row ();
		const int column = index.column ();
		const auto& h = Handles_.at (row).Handle_;

		switch (role)
		{
		case RoleControls:
			return QVariant::fromValue<QToolBar*> (Toolbar_);
		case RoleAdditionalInfo:
			return QVariant::fromValue<QWidget*> (TabWidget_);
		case RoleContextMenu:
			return QVariant::fromValue<QMenu*> (Menu_);
		case Roles::HandleIndex:
			return index.row ();
		case Roles::TorrentHandle:
			return QVariant::fromValue (&h);
		case Roles::IsManaged:
			return Handles_ [row].AutoManaged_;
		case Roles::IsSequentialDownloading:
			return static_cast<bool> (StatusKeeper_->GetStatus (h).flags & libtorrent::torrent_flags::sequential_download);
		case Roles::IsSuperSeeding:
			return static_cast<bool> (StatusKeeper_->GetStatus (h).flags & libtorrent::torrent_flags::super_seeding);
		case Roles::TorrentStats:
			return QVariant::fromValue (GetTorrentStats (h, *StatusKeeper_));
		}

		const auto& status = StatusKeeper_->GetStatus (h,
				libtorrent::torrent_handle::query_name |
				libtorrent::torrent_handle::query_save_path);

		switch (role)
		{
		case Roles::IsSeeding:
			if (IsPaused (status))
				return false;
			return status.state == libtorrent::torrent_status::seeding ||
					status.state == libtorrent::torrent_status::finished;
		case Roles::IsLeeching:
			if (IsPaused (status))
				return false;
			return status.state == libtorrent::torrent_status::downloading ||
					status.state == libtorrent::torrent_status::downloading_metadata;
		case Qt::DecorationRole:
			if (column != Columns::ColumnName)
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
			case libtorrent::torrent_status::finished:
				return QIcon::fromTheme ("arrow-up");
			case libtorrent::torrent_status::seeding:
				return QIcon::fromTheme ("arrow-up-double");
			}

			return {};
		case Roles::SortRole:
			switch (column)
			{
			case Columns::ColumnID:
				return row + 1;
			case Columns::ColumnName:
				return QString::fromStdString (status.name);
			case Columns::ColumnState:
				return IsPaused (status) ?
						-1 :
						static_cast<int> (status.state);
			case Columns::ColumnProgress:
				return status.progress;
			case Columns::ColumnDownSpeed:
				return status.download_payload_rate;
			case Columns::ColumnUpSpeed:
				return status.upload_payload_rate;
			case Columns::ColumnLeechers:
				return status.num_peers - status.num_seeds;
			case Columns::ColumnSeeders:
				return status.num_seeds;
			case Columns::ColumnDownloaded:
				return static_cast<quint64> (status.all_time_download);
			case Columns::ColumnSize:
				return static_cast<quint64> (status.total_wanted);
			case Columns::ColumnUploaded:
				return static_cast<quint64> (status.all_time_upload);
			case Columns::ColumnRatio:
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
			case Columns::ColumnID:
				return row + 1;
			case Columns::ColumnName:
				return QString::fromStdString (status.name);
			case Columns::ColumnState:
				return GetStringForStatus (status);
			case Columns::ColumnProgress:
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
			case Columns::ColumnDownSpeed:
				return Util::MakePrettySize (status.download_payload_rate) + tr ("/s");
			case Columns::ColumnUpSpeed:
				return Util::MakePrettySize (status.upload_payload_rate) + tr ("/s");
			case Columns::ColumnLeechers:
				return QString::number (status.num_peers - status.num_seeds);
			case Columns::ColumnSeeders:
				return QString::number (status.num_seeds);
			case Columns::ColumnDownloaded:
				return Util::MakePrettySize (status.all_time_download);
			case Columns::ColumnSize:
				return Util::MakePrettySize (status.total_wanted);
			case Columns::ColumnUploaded:
				return Util::MakePrettySize (status.all_time_upload);
			case Columns::ColumnRatio:
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
		case Roles::TorrentTags:
			return Proxy_->GetTagsManager ()->GetTags (Handles_.at (row).Tags_);
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

	libtorrent::session& Core::GetSession ()
	{
		return *Session_;
	}

	libtorrent::torrent_handle Core::GetTorrentHandle (int idx) const
	{
		if (idx >= Handles_.size ())
			return {};

		return Handles_.at (idx).Handle_;
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
		endInsertRows ();

		return Handles_.back ().Promise_->future ();
	}

	namespace
	{
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
				handle,
				contents,
				torrentFileName,
				tags,
				autoManaged,
				params
			});
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

	void Core::SetFilePriority (int file, int priority, int idx)
	{
		if (!CheckValidity (idx))
			return;

		Handles_.at (idx).Handle_.file_priority (file, std::clamp (priority, 0, 7));
	}

	void Core::SetFilename (int index, const QString& name, int idx)
	{
		if (!CheckValidity (idx))
			return;

		Handles_ [idx].Handle_.rename_file (index, std::string (name.toUtf8 ().data ()));
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

	void Core::MakeTorrent (const NewTorrentParams& params)
	{
		if (const auto result = CreateTorrent (params))
			AddFile (*result, params.Path_, {}, false);
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

	void Core::MoveUp (const QList<int>& selections)
	{
		if (selections.isEmpty ())
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

			emit dataChanged (index (*i - 1, 0),
					index (*i, columnCount () - 1));
		}
	}

	void Core::MoveDown (const QList<int>& selections)
	{
		if (selections.isEmpty ())
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

			emit dataChanged (index (*i, 0),
					index (*i + 1, columnCount () - 1));
		}
	}

	void Core::MoveToTop (const QList<int>& selections)
	{
		if (selections.isEmpty ())
			return;

		for (auto i = selections.begin (),
				end = selections.end (); i != end; ++i)
			if (*i <= 0 || !CheckValidity (*i))
				return;

		for (auto i = selections.rbegin (),
				end = selections.rend (); i != end; ++i)
			MoveToTop (*i);
	}

	void Core::MoveToBottom (const QList<int>& selections)
	{
		if (selections.isEmpty ())
			return;

		for (auto i = selections.begin (),
				end = selections.end (); i != end; ++i)
			if (*i < 0 || !CheckValidity (*i))
				return;

		for (auto i = selections.begin (),
				end = selections.end (); i != end; ++i)
			MoveToBottom (*i);
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

			beginInsertRows ({}, Handles_.size (), Handles_.size ());
			Handles_.append ({
					handle,
					data,
					filename,
					settings.value ("Tags").toStringList (),
					automanaged,
					taskParameters,
					TorrentStruct::NoFuture {}
				});
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

	void Core::queryLibtorrent ()
	{
		Session_->post_torrent_updates ();
		Dispatcher_.PollAlerts ();
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
