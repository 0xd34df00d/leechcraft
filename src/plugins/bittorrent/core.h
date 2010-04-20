/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#ifndef PLUGINS_BITTORRENT_CORE_H
#define PLUGINS_BITTORRENT_CORE_H
#include <map>
#include <list>
#include <deque>
#include <boost/shared_ptr.hpp>
#include <QAbstractItemModel>
#include <QPair>
#include <QList>
#include <QVector>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/session_status.hpp>
#include <interfaces/iinfo.h>
#include <interfaces/structures.h>
#include <plugininterface/tagscompletionmodel.h>
#include "torrentinfo.h"
#include "fileinfo.h"
#include "peerinfo.h"

class QTimer;
class QDomElement;
class QToolBar;
class QStandardItemModel;
class QDataStream;

namespace libtorrent
{
	struct cache_status;
	class session;
};

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class PiecesModel;
			class PeersModel;
			class TorrentFilesModel;
			class RepresentationModel;
			class LiveStreamManager;
			struct NewTorrentParams;

			class Core : public QAbstractItemModel
			{
				Q_OBJECT

				enum TorrentState
				{
					TSIdle
					, TSPreparing
					, TSDownloading
					, TSSeeding
				};

				struct TorrentStruct
				{
					std::vector<int> FilePriorities_;
					libtorrent::torrent_handle Handle_;
					QByteArray TorrentFileContents_;
					QString TorrentFileName_;
					TorrentState State_;
					double Ratio_;
					/** Holds the IDs of tags of the torrent.
					 */
					QStringList Tags_;
					bool AutoManaged_;

					int ID_;
					LeechCraft::TaskParameters Parameters_;
				};

				struct HandleFinder
				{
					const libtorrent::torrent_handle& Handle_;

					HandleFinder (const libtorrent::torrent_handle&);
					bool operator() (const TorrentStruct&) const;
				};
			public:
				struct PerTrackerStats
				{
					qint64 DownloadRate_;
					qint64 UploadRate_;

					PerTrackerStats ();
				};
				typedef std::map<QString, PerTrackerStats> pertrackerstats_t;
			private:
				struct PerTrackerAccumulator
				{
					pertrackerstats_t& Stats_;

					PerTrackerAccumulator (pertrackerstats_t&);
					int operator() (int, const Core::TorrentStruct& str);
				};

				libtorrent::session *Session_;
				typedef QList<TorrentStruct> HandleDict_t;
				HandleDict_t Handles_;
				QList<QString> Headers_;
				mutable int CurrentTorrent_;
				std::auto_ptr<QTimer> SettingsSaveTimer_, FinishedTimer_, WarningWatchdog_, ScrapeTimer_;
				boost::shared_ptr<PiecesModel> PiecesModel_;
				boost::shared_ptr<PeersModel> PeersModel_;
				boost::shared_ptr<TorrentFilesModel> TorrentFilesModel_;
				boost::shared_ptr<QStandardItemModel> WebSeedsModel_;
				boost::shared_ptr<LiveStreamManager> LiveStreamManager_;
				QString ExternalAddress_;
				bool SaveScheduled_;
				QToolBar *Toolbar_;
				QWidget *TabWidget_;
				ICoreProxy_ptr Proxy_;
				QMenu *Menu_;

				Core ();
			public:
				enum Columns
				{
					ColumnName = 0
					, ColumnState
					, ColumnProgress  // percentage, Downloaded of Size
				};
				enum AddType
				{
					Started
					, Paused
				};
				enum SettingsPreset
				{
					SPDefault
					, SPMinMemoryUsage
					, SPHighPerfSeed
				};

				static Core* Instance ();
				virtual ~Core ();
				void SetWidgets (QToolBar*, QWidget*);
				void SetMenu (QMenu*);
				void DoDelayedInit ();
				void Release ();

				void SetProxy (ICoreProxy_ptr);
				ICoreProxy_ptr GetProxy () const;

				bool CouldDownload (const LeechCraft::DownloadEntity&) const;
				bool CouldHandle (const LeechCraft::DownloadEntity&) const;
				void Handle (LeechCraft::DownloadEntity);
				PiecesModel* GetPiecesModel ();
				void ClearPieces ();
				void UpdatePieces ();
				PeersModel* GetPeersModel ();
				QAbstractItemModel* GetWebSeedsModel ();
				void ClearPeers ();
				void UpdatePeers ();
				TorrentFilesModel* GetTorrentFilesModel ();
				void ClearFiles ();
				void UpdateFiles ();
				void ResetFiles ();

				virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
				virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				virtual Qt::ItemFlags flags (const QModelIndex&) const;
				virtual bool hasChildren (const QModelIndex&) const;
				virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
				virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
				virtual QModelIndex parent (const QModelIndex&) const;
				virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

				libtorrent::torrent_info GetTorrentInfo (const QString&);
				libtorrent::torrent_info GetTorrentInfo (const QByteArray&);
				bool IsValidTorrent (const QByteArray&) const;
				std::auto_ptr<TorrentInfo> GetTorrentStats () const;
				libtorrent::session_status GetOverallStats () const;
				void GetPerTracker (pertrackerstats_t&) const;
				int GetListenPort () const;
				libtorrent::cache_status GetCacheStats () const;
				QList<PeerInfo> GetPeers () const;
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
				int AddMagnet (const QString& magnet,
						const QString& path,
						const QStringList& tags,
						LeechCraft::TaskParameters params = LeechCraft::NoParameters);
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
				int AddFile (const QString& filename,
						const QString& path,
						const QStringList& tags,
						bool tryLive,
						const QVector<bool>& files = QVector<bool> (),
						LeechCraft::TaskParameters params = LeechCraft::NoParameters);
				void KillTask (int);
				void RemoveTorrent (int);
				void PauseTorrent (int);
				void ResumeTorrent (int);
				void ForceReannounce (int);
				void ForceRecheck (int);
				void SetOverallDownloadRate (int);
				void SetOverallUploadRate (int);
				void SetMaxDownloadingTorrents (int);
				void SetMaxUploadingTorrents (int);
				void SetDesiredRating (double);
				int GetOverallDownloadRate () const;
				int GetOverallUploadRate () const;
				int GetMaxDownloadingTorrents () const;
				int GetMaxUploadingTorrents () const;
				double GetDesiredRating () const;
				void SetTorrentDownloadRate (int);
				void SetTorrentUploadRate (int);
				void SetTorrentDesiredRating (double);
				int GetTorrentDownloadRate () const;
				int GetTorrentUploadRate () const;
				double GetTorrentDesiredRating () const;
				void AddPeer (const QString&, int);
				void AddWebSeed (const QString&, bool);
				void RemoveWebSeed (const QString&, bool);
				void SetFilePriority (int, int);
				void SetFilename (int, const QString&);
				std::vector<libtorrent::announce_entry> GetTrackers () const;
				std::vector<libtorrent::announce_entry> GetTrackers (int) const;
				void SetTrackers (const std::vector<libtorrent::announce_entry>&);
				void SetTrackers (int, const std::vector<libtorrent::announce_entry>&);
				QString GetMagnetLink () const;
				QString GetTorrentDirectory () const;
				bool MoveTorrentFiles (const QString&);
				void SetCurrentTorrent (int);
				int GetCurrentTorrent () const;
				bool IsTorrentManaged () const;
				void SetTorrentManaged (bool);
				bool IsTorrentSequentialDownload () const;
				void SetTorrentSequentialDownload (bool);
				bool IsTorrentSuperSeeding () const;
				void SetTorrentSuperSeeding (bool);
				void MakeTorrent (const NewTorrentParams&) const;
				void LogMessage (const QString&);
				void SetExternalAddress (const QString&);
				QString GetExternalAddress () const;
				void Import (const QString&);
				void Export (const QString&, bool, bool) const;
				typedef QPair<QString, QString> BanRange_t;
				void BanPeers (const BanRange_t&, bool = true);
				void ClearFilter ();
				QMap<BanRange_t, bool> GetFilter () const;
				bool CheckValidity (int) const;

				void SaveResumeData (const libtorrent::save_resume_data_alert&) const;
				void HandleMetadata (const libtorrent::metadata_received_alert&);
				void FileFinished (const libtorrent::torrent_handle&, int);
				void PieceRead (const libtorrent::read_piece_alert&);

				void MoveUp (const std::deque<int>&);
				void MoveDown (const std::deque<int>&);
				void MoveToTop (const std::deque<int>&);
				void MoveToBottom (const std::deque<int>&);

				void SetPreset (SettingsPreset);
			private:
				QList<FileInfo> GetTorrentFiles () const;
				void MoveToTop (int);
				void MoveToBottom (int);
				QString GetStringForState (libtorrent::torrent_status::state_t) const;
				void RestoreTorrents ();
				libtorrent::torrent_handle RestoreSingleTorrent (const QByteArray&,
						const QByteArray&,
						const boost::filesystem::path&,
						bool,
						bool);
				void HandleSingleFinished (int);
				int GetCurrentlyDownloading () const;
				int GetCurrentlySeeding () const;
				void ManipulateSettings ();
				void CheckDownloadQueue ();
				void CheckUploadQueue ();
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
				void ParseStorage (const QDomElement&);
				void ScheduleSave ();
				void HandleLibtorrentException (const libtorrent::libtorrent_exception&);
			private slots:
				void writeSettings ();
				void checkFinished ();
				void scrape ();
			public slots:
				void queryLibtorrentForWarnings ();
				void tcpPortRangeChanged ();
				void dhtStateChanged ();
				void autosaveIntervalChanged ();
				void maxUploadsChanged ();
				void maxConnectionsChanged ();
				void setProxySettings ();
				void setGeneralSettings ();
				void setDHTSettings ();
				void setLoggingSettings ();
				void setScrapeInterval ();
			signals:
				void error (QString) const;
				void gotEntity (const LeechCraft::DownloadEntity&);
				void addToHistory (const QString&, const QString&, quint64,
						const QDateTime&, const QStringList&);
				void taskFinished (int);
				void taskRemoved (int);
			};
		};
	};
};

namespace libtorrent
{
	QDataStream& operator<< (QDataStream&, const libtorrent::entry&);
	QDataStream& operator>> (QDataStream&, libtorrent::entry&);
};

#endif

