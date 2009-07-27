#ifndef PLUGINS_BITTORRENT_CORE_H
#define PLUGINS_BITTORRENT_CORE_H
#include <map>
#include <list>
#include <deque>
#include <memory>
#include <QAbstractItemModel>
#include <QPair>
#include <QList>
#include <QVector>
#include <QStandardItemModel>
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
#include "newtorrentparams.h"

class QTimer;
class QDomElement;
class QToolBar;

namespace libtorrent
{
	class cache_status;
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
				std::auto_ptr<PiecesModel> PiecesModel_;
				std::auto_ptr<PeersModel> PeersModel_;
				std::auto_ptr<TorrentFilesModel> TorrentFilesModel_;
				std::auto_ptr<QStandardItemModel> WebSeedsModel_;
				QString ExternalAddress_;
				bool SaveScheduled_;
				QToolBar *Toolbar_;
				QWidget *TabWidget_;
				ICoreProxy_ptr Proxy_;

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

				static Core* Instance ();
				virtual ~Core ();
				void SetWidgets (QToolBar*, QWidget*);
				void DoDelayedInit ();
				void Release ();

				void SetProxy (ICoreProxy_ptr);
				ICoreProxy_ptr GetProxy () const;

				bool CouldDownload (const LeechCraft::DownloadEntity&) const;
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
				int AddMagnet (const QString&, const QString&, const QStringList&,
						LeechCraft::TaskParameters = LeechCraft::NoParameters);
				int AddFile (const QString&, const QString&, const QStringList&,
						const QVector<bool>& = QVector<bool> (),
						LeechCraft::TaskParameters = LeechCraft::NoParameters);
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
				void SetFilePriority (int, int);
				void SetFilename (int, const QString&);
				QStringList GetTrackers () const;
				QStringList GetTrackers (int) const;
				void SetTrackers (const QStringList&);
				void SetTrackers (int, const QStringList&);
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
				void MakeTorrent (NewTorrentParams) const;
				void LogMessage (const QString&);
				void SetExternalAddress (const QString&);
				QString GetExternalAddress () const;
				void Import (const QString&);
				void Export (const QString&, bool, bool) const;
				bool CheckValidity (int) const;

				void SaveResumeData (const libtorrent::save_resume_data_alert&) const;
				void HandleMetadata (const libtorrent::metadata_received_alert&);
				void FileFinished (const libtorrent::torrent_handle&, int);

				void MoveUp (const std::deque<int>&);
				void MoveDown (const std::deque<int>&);
				void MoveToTop (const std::deque<int>&);
				void MoveToBottom (const std::deque<int>&);
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
				QStringList GetTagsForIndexImpl (int) const;
				void UpdateTagsImpl (const QStringList&, int);
				void ParseStorage (const QDomElement&);
				void ScheduleSave ();
				void HandleLibtorrentException (const libtorrent::libtorrent_exception&);
			private slots:
				void writeSettings ();
				void checkFinished ();
				void queryLibtorrentForWarnings ();
				void scrape ();
			public slots:
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
				void logMessage (const QString&) const;
				void torrentFinished (const QString&);
				void fileFinished (const LeechCraft::DownloadEntity&);
				void addToHistory (const QString&, const QString&, quint64,
						const QDateTime&, const QStringList&);
				void taskFinished (int);
				void taskRemoved (int);
			};
		};
	};
};

#endif

