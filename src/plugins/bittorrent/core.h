/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <optional>
#include <QAbstractItemModel>
#include <QList>
#include <QVector>
#include <QFutureInterface>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <interfaces/iinfo.h>
#include <interfaces/structures.h>
#include <interfaces/idownload.h>
#include <util/sll/either.h>
#include "fileinfo.h"
#include "types.h"
#include "alertdispatcher.h"

class QTimer;
class QDomElement;
class QToolBar;
class QStandardItemModel;
class QDataStream;

namespace libtorrent
{
	class session;
};

struct EntityTestHandleResult;

namespace LC
{
namespace Util
{
class ShortcutManager;
}
namespace BitTorrent
{
	class TorrentFilesModel;
	class LiveStreamManager;
	class SessionSettingsManager;
	class CachedStatusKeeper;
	struct SessionStats;
	struct NewTorrentParams;

	using BanRange_t = QPair<QString, QString>;

	class Core : public QAbstractItemModel
	{
		Q_OBJECT

		struct TorrentStruct
		{
			libtorrent::torrent_handle Handle_;
			QByteArray TorrentFileContents_ = {};
			QString TorrentFileName_ = {};
			TorrentState State_ = TSIdle;
			/** Holds the IDs of tags of the torrent.
			 */
			QStringList Tags_;
			bool AutoManaged_ = true;

			TaskParameters Parameters_;

			std::optional<QFutureInterface<IDownload::Result>> Promise_;

			bool PauseAfterCheck_ = false;

			TorrentStruct (libtorrent::torrent_handle handle,
					QStringList tags,
					TaskParameters params)
			: Handle_ { std::move (handle) }
			, Tags_ { std::move (tags) }
			, Parameters_ { params }
			, Promise_ { QFutureInterface<IDownload::Result> {} }
			{
				Promise_->reportStarted ();
			}

			struct NoFuture {};

			TorrentStruct (libtorrent::torrent_handle handle,
					QByteArray torrentFile,
					QString filename,
					QStringList tags,
					bool autoManaged,
					TaskParameters params,
					NoFuture)
			: Handle_ { std::move (handle) }
			, TorrentFileContents_ { std::move (torrentFile) }
			, TorrentFileName_ { std::move (filename) }
			, Tags_ { std::move (tags) }
			, AutoManaged_ { autoManaged }
			, Parameters_ { params }
			{
			}

			TorrentStruct (libtorrent::torrent_handle handle,
					QByteArray torrentFile,
					QString filename,
					QStringList tags,
					bool autoManaged,
					TaskParameters params)
			: TorrentStruct
				{ std::move (handle)
				, std::move (torrentFile)
				, std::move (filename)
				, std::move (tags)
				, autoManaged
				, params
				, NoFuture {}
				}
			{
				Promise_ = QFutureInterface<IDownload::Result> {};
				Promise_->reportStarted ();
			}
		};
		CachedStatusKeeper * const StatusKeeper_;

		libtorrent::session *Session_;
		SessionSettingsManager *SessionSettingsMgr_;

		typedef QList<TorrentStruct> HandleDict_t;
		HandleDict_t Handles_;
		QList<QString> Headers_;
		std::shared_ptr<QTimer> FinishedTimer_, WarningWatchdog_;
		std::shared_ptr<LiveStreamManager> LiveStreamManager_;
		bool SaveScheduled_ = false;
		QToolBar *Toolbar_ = nullptr;
		QWidget *TabWidget_ = nullptr;
		ICoreProxy_ptr Proxy_;
		QMenu *Menu_ = nullptr;
		Util::ShortcutManager *ShortcutMgr_ = nullptr;

		AlertDispatcher Dispatcher_;

		Core ();
	public:
		static Core* Instance ();

		void SetWidgets (QToolBar*, QWidget*);
		void SetMenu (QMenu*);
		void DoDelayedInit ();
		void Release ();

		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;

		AlertDispatcher& GetAlertDispatcher ();

		Util::ShortcutManager* GetShortcutManager () const;

		SessionSettingsManager* GetSessionSettingsManager () const;

		CachedStatusKeeper* GetStatusKeeper () const;

		virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
		virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
		virtual bool setData (const QModelIndex&, const QVariant&, int);
		virtual Qt::ItemFlags flags (const QModelIndex&) const;
		virtual bool hasChildren (const QModelIndex&) const;
		virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
		virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
		virtual QModelIndex parent (const QModelIndex&) const;
		virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

		libtorrent::session& GetSession ();

		/** @brief Adds the  given magnet link to the queue.
			*
			* Fetches the torrent and starts downloading the magnet link to
			* the given path, sets the given tags and takes into the account
			* the given parameters.
			*
			* @param[in] magnet The magnet link.
			* @param[in] path The save path.
			* @param[in] tags The IDs of the tags of the torrent.
			* @param[in] params Task parameters.
			* @return The ID of the task.
			*/
		QFuture<IDownload::Result> AddMagnet (const QString& magnet,
				const QString& path,
				const QStringList& tags,
				LC::TaskParameters params = LC::NoParameters);
		/** @brief Adds the given torrent file from the filename to the
			* queue.
			*
			* Starts downloading the torrent to the given path, sets the
			* passed IDs of the tags of the torrent, marks only selected files
			* for the download and takes into account the given params.
			*
			* @param[in] filename The file name of the torrent.
			* @param[in] path The save path.
			* @param[in] tags The IDs of the tags of the torrent.
			* @param[in] tryLive Try to play this torrent live.
			* @param[in] files The list of initial file selections.
			* @param[in] params Task parameters.
			* @return The ID of the task.
			*/
		QFuture<IDownload::Result> AddFile (const QString& filename,
				const QString& path,
				const QStringList& tags,
				bool tryLive,
				const QVector<bool>& files = QVector<bool> (),
				LC::TaskParameters params = LC::NoParameters);
		void RemoveTorrent (int, bool withFiles = false);
		void PauseTorrent (int);
		void ResumeTorrent (int);
		void ForceReannounce (int);
		void ForceRecheck (int);

		QString GetTorrentDirectory (int) const;
		bool MoveTorrentFiles (const QString&, int);

		void MakeTorrent (const NewTorrentParams&);
		bool CheckValidity (int) const;

		void SaveResumeData (const libtorrent::save_resume_data_alert&) const;
		void HandleMetadata (const libtorrent::metadata_received_alert&);
		void UpdateStatus (const std::vector<libtorrent::torrent_status>&);

		void HandleTorrentChecked (const libtorrent::torrent_handle&);

		void MoveUp (const QList<int>&);
		void MoveDown (const QList<int>&);
		void MoveToTop (const QList<int>&);
		void MoveToBottom (const QList<int>&);
	private:
		HandleDict_t::iterator FindHandle (const libtorrent::torrent_handle&);
		HandleDict_t::const_iterator FindHandle (const libtorrent::torrent_handle&) const;

		void MoveToTop (int);
		void MoveToBottom (int);
		void RestoreTorrents ();
		libtorrent::torrent_handle RestoreSingleTorrent (const QByteArray&,
				const QByteArray&,
				bool,
				bool);

		void HandleSingleFinished (int);

		void ScheduleSave ();

		void ShowError (const QString&);
	private slots:
		void writeSettings ();
		void checkFinished ();
		void scrape ();
		void queryLibtorrent ();
	signals:
		void torrentsStatusesUpdated ();
	};
}
}
