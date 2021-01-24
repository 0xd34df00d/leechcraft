/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <list>
#include <memory>
#include <optional>
#include <QAbstractItemModel>
#include <QPair>
#include <QList>
#include <QVector>
#include <QIcon>
#include <QFutureInterface>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/session_status.hpp>
#include <interfaces/iinfo.h>
#include <interfaces/structures.h>
#include <interfaces/idownload.h>
#include <util/tags/tagscompletionmodel.h>
#include <util/sll/either.h>
#include "torrentinfo.h"
#include "fileinfo.h"
#include "peerinfo.h"
#include "types.h"

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
	class PiecesModel;
	class PeersModel;
	class TorrentFilesModel;
	class LiveStreamManager;
	class SessionSettingsManager;
	class CachedStatusKeeper;
	class GeoIP;
	struct SessionStats;
	struct NewTorrentParams;

	using BanRange_t = QPair<QString, QString>;

	class Core : public QAbstractItemModel
	{
		Q_OBJECT

		enum TorrentState
		{
			TSIdle,
			TSPreparing,
			TSDownloading,
			TSSeeding
		};

		struct TorrentStruct
		{
			std::vector<libtorrent::download_priority_t> FilePriorities_ = {};
			libtorrent::torrent_handle Handle_;
			QByteArray TorrentFileContents_ = {};
			QString TorrentFileName_ = {};
			TorrentState State_ = TSIdle;
			double Ratio_ = 0;
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

			TorrentStruct (std::vector<libtorrent::download_priority_t> prios,
					libtorrent::torrent_handle handle,
					QByteArray torrentFile,
					QString filename,
					QStringList tags,
					bool autoManaged,
					TaskParameters params,
					NoFuture)
			: FilePriorities_ { std::move (prios) }
			, Handle_ { std::move (handle) }
			, TorrentFileContents_ { std::move (torrentFile) }
			, TorrentFileName_ { std::move (filename) }
			, Tags_ { std::move (tags) }
			, AutoManaged_ { autoManaged }
			, Parameters_ { params }
			{
			}

			TorrentStruct (std::vector<libtorrent::download_priority_t> prios,
					libtorrent::torrent_handle handle,
					QByteArray torrentFile,
					QString filename,
					QStringList tags,
					bool autoManaged,
					TaskParameters params)
			: TorrentStruct
				{ std::move (prios)
				, std::move (handle)
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

		friend struct SimpleDispatcher;
	public:
		struct PerTrackerStats
		{
			qint64 DownloadRate_ = 0;
			qint64 UploadRate_ = 0;
		};
		typedef QMap<QString, PerTrackerStats> pertrackerstats_t;
	private:
		CachedStatusKeeper * const StatusKeeper_;

		libtorrent::session *Session_ = nullptr;
		SessionSettingsManager *SessionSettingsMgr_ = nullptr;

		typedef QList<TorrentStruct> HandleDict_t;
		HandleDict_t Handles_;
		QList<QString> Headers_;
		mutable int CurrentTorrent_ = -1;
		std::shared_ptr<QTimer> FinishedTimer_, WarningWatchdog_;
		std::shared_ptr<LiveStreamManager> LiveStreamManager_;
		QString ExternalAddress_;
		bool SaveScheduled_ = false;
		QToolBar *Toolbar_ = nullptr;
		QWidget *TabWidget_ = nullptr;
		ICoreProxy_ptr Proxy_;
		QMenu *Menu_ = nullptr;
		Util::ShortcutManager *ShortcutMgr_ = nullptr;

		std::shared_ptr<GeoIP> GeoIP_;

		QIcon TorrentIcon_;

		Core ();
	public:
		enum Columns
		{
			ColumnID,
			ColumnName,
			ColumnState,
			ColumnProgress,  // percentage, Downloaded of Size
			ColumnDownSpeed,
			ColumnUpSpeed,
			ColumnLeechers,
			ColumnSeeders,
			ColumnSize,
			ColumnDownloaded,
			ColumnUploaded,
			ColumnRatio
		};
		enum Roles
		{
			FullLengthText = Qt::UserRole + 1,
			SortRole
		};

		static Core* Instance ();

		void SetWidgets (QToolBar*, QWidget*);
		void SetMenu (QMenu*);
		void DoDelayedInit ();
		void Release ();

		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;

		Util::ShortcutManager* GetShortcutManager () const;

		SessionSettingsManager* GetSessionSettingsManager () const;

		EntityTestHandleResult CouldDownload (const LC::Entity&) const;
		PiecesModel* GetPiecesModel (int);
		PeersModel* GetPeersModel (int);
		QAbstractItemModel* GetWebSeedsModel (int);
		TorrentFilesModel* GetTorrentFilesModel (int);
		CachedStatusKeeper* GetStatusKeeper () const;

		virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
		virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
		virtual Qt::ItemFlags flags (const QModelIndex&) const;
		virtual bool hasChildren (const QModelIndex&) const;
		virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
		virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
		virtual QModelIndex parent (const QModelIndex&) const;
		virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

		QIcon GetTorrentIcon (int) const;

		libtorrent::torrent_handle GetTorrentHandle (int) const;

		bool IsValidTorrent (const QByteArray&) const;
		std::unique_ptr<TorrentInfo> GetTorrentStats (int) const;
		SessionStats GetSessionStats () const;
		void GetPerTracker (pertrackerstats_t&) const;
		int GetListenPort () const;
		QList<PeerInfo> GetPeers (int = -1) const;
		QStringList GetTagsForIndex (int = -1) const;
		void UpdateTags (const QStringList&, int = -1);
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
		void SetTorrentDownloadRate (int, int);
		void SetTorrentUploadRate (int, int);
		int GetTorrentDownloadRate (int) const;
		int GetTorrentUploadRate (int) const;
		void AddPeer (const QString&, unsigned short, int);
		void AddWebSeed (const QString&, WebSeedType, int);
		void RemoveWebSeed (const QString&, WebSeedType, int);
		void SetFilePriority (int, int, int);
		void SetFilename (int, const QString&, int);

		std::vector<libtorrent::announce_entry> GetTrackers (const std::optional<int>& = {}) const;
		void SetTrackers (const std::vector<libtorrent::announce_entry>&, const std::optional<int>& = {});

		QString GetMagnetLink (int) const;

		QString GetTorrentDirectory (int) const;
		bool MoveTorrentFiles (const QString&, int);

		void SetCurrentTorrent (int);
		int GetCurrentTorrent () const;
		bool IsTorrentManaged (int) const;
		void SetTorrentManaged (bool, int);
		bool IsTorrentSequentialDownload (int) const;
		void SetTorrentSequentialDownload (bool, int);
		bool IsTorrentSuperSeeding (int) const;
		void SetTorrentSuperSeeding (bool, int);
		void MakeTorrent (const NewTorrentParams&) const;
		void SetExternalAddress (const QString&);
		QString GetExternalAddress () const;
		void BanPeers (const BanRange_t&, bool = true);
		void ClearFilter ();
		QMap<BanRange_t, bool> GetFilter () const;
		bool CheckValidity (int) const;

		void SaveResumeData (const libtorrent::save_resume_data_alert&) const;
		void HandleMetadata (const libtorrent::metadata_received_alert&);
		void PieceRead (const libtorrent::read_piece_alert&);
		void UpdateStatus (const std::vector<libtorrent::torrent_status>&);

		void HandleTorrentChecked (const libtorrent::torrent_handle&);

		void MoveUp (const std::vector<int>&);
		void MoveDown (const std::vector<int>&);
		void MoveToTop (const std::vector<int>&);
		void MoveToBottom (const std::vector<int>&);

		QList<FileInfo> GetTorrentFiles (int = -1) const;
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
		void HandleFileRenamed (const libtorrent::file_renamed_alert&);

		/** Returns human-readable list of tags for the given torrent.
		 *
		 * @param[in] torrent The ID of the torrent.
		 * @return The human-readable list of tags.
		 */
		QStringList GetTagsForIndexImpl (int torrent) const;
		/** Sets the tags for the given torrent.
		 *
		 * @param[in] tags The human-readable list of tags.
		 * @param[in] torrent The ID of the torrent.
		 */
		void UpdateTagsImpl (const QStringList& tags, int torrent);
		void ScheduleSave ();
		void HandleLibtorrentException (const std::exception&);

		void ShowError (const QString&);
	private slots:
		void writeSettings ();
		void checkFinished ();
		void scrape ();
		void queryLibtorrent ();
	signals:
		void addToHistory (const QString&, const QString&, quint64,
				const QDateTime&, const QStringList&);
		void fileRenamed (int torrent, int file, const QString& newName);
		void torrentsStatusesUpdated ();
	};
}
}
