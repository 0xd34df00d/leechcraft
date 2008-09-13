#ifndef CORE_H
#define CORE_H
#include <list>
#include <memory>
#include <QAbstractItemModel>
#include <QPair>
#include <QList>
#include <QVector>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <interfaces/interfaces.h>
#include "torrentinfo.h"
#include "overallstats.h"
#include "fileinfo.h"
#include "peerinfo.h"
#include "newtorrentparams.h"

class QTimer;
class PiecesModel;
class PeersModel;
class TagsCompletionModel;
class TorrentFilesModel;
class RepresentationModel;

namespace libtorrent
{
	class save_resume_data_alert;
	class cache_status;
	class session;
};

class Core : public QAbstractItemModel
{
    Q_OBJECT

private:
    enum TorrentState
    {
        TSIdle
        , TSWaiting2Download
        , TSWaiting2Seed
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

		int ID_;
		LeechCraft::TaskParameters Parameters_;
    };

	struct HandleFinder
	{
		const libtorrent::torrent_handle& Handle_;

		HandleFinder (const libtorrent::torrent_handle&);
		bool operator() (const TorrentStruct&) const;
	};

    libtorrent::session *Session_;
    typedef QList<TorrentStruct> HandleDict_t;
    HandleDict_t Handles_;
    QList<QString> Headers_;
    int InterfaceUpdateTimer_;
	mutable int CurrentTorrent_;
	std::auto_ptr<QTimer> SettingsSaveTimer_, FinishedTimer_, WarningWatchdog_, ScrapeTimer_;
	std::auto_ptr<PiecesModel> PiecesModel_;
	std::auto_ptr<PeersModel> PeersModel_;
	std::auto_ptr<TagsCompletionModel> TagsCompletionModel_;
	std::auto_ptr<TorrentFilesModel> TorrentFilesModel_;

	std::list<quint16> IDPool_;

	QString ExternalAddress_;

    Core ();
public:
    enum Columns
    {
        ColumnName = 0
        , ColumnState
        , ColumnProgress  // percentage, Downloaded of Size
        , ColumnDSpeed
        , ColumnUSpeed 
        , ColumnUploaded
        , ColumnRating
        , ColumnSP
        , ColumnRemaining 
    };
	enum AddType
	{
		Started
		, Paused
	};
    static Core* Instance ();
	virtual ~Core ();
    void DoDelayedInit ();
    void Release ();
    PiecesModel* GetPiecesModel ();
    void ClearPieces ();
    void UpdatePieces ();
    PeersModel* GetPeersModel ();
    void ClearPeers ();
    void UpdatePeers ();
	TorrentFilesModel* GetTorrentFilesModel ();
	void ClearFiles ();
	void UpdateFiles ();
	void ResetFiles ();

    virtual int columnCount (const QModelIndex&) const;
    virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual bool hasChildren (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation, int role = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int, const QModelIndex& parent = QModelIndex ()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;

    libtorrent::torrent_info GetTorrentInfo (const QString&);
    libtorrent::torrent_info GetTorrentInfo (const QByteArray&);
    bool IsValidTorrent (const QByteArray&) const;
    TorrentInfo GetTorrentStats () const;
	libtorrent::bitfield GetLocalPieces () const;
    OverallStats GetOverallStats () const;
	libtorrent::cache_status GetCacheStats () const;
    QList<FileInfo> GetTorrentFiles () const;
    QList<PeerInfo> GetPeers () const;
    QStringList GetTagsForIndex (int = -1) const;
    void UpdateTags (const QStringList&, int = -1);
    TagsCompletionModel* GetTagsCompletionModel () const;
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
    void SetFilePriority (int, int);
    int GetFilePriority (int) const;
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

    void MakeTorrent (NewTorrentParams) const;
    void LogMessage (const QString&);

	void SetExternalAddress (const QString&);
	QString GetExternalAddress () const;

	void ImportData (const QByteArray&);
	QByteArray ExportData () const;

    bool CheckValidity (int) const;
	void SaveResumeData (const libtorrent::save_resume_data_alert&) const;
private:
    QString GetStringForState (libtorrent::torrent_status::state_t) const;
    void ReadSettings ();
    void RestoreTorrents ();
    libtorrent::torrent_handle RestoreSingleTorrent (const QByteArray&,
			const QByteArray&, const boost::filesystem::path&);
    void HandleSingleFinished (int);
    int GetCurrentlyDownloading () const;
    int GetCurrentlySeeding () const;
    void ManipulateSettings ();
	void CheckDownloadQueue ();
	void CheckUploadQueue ();
	QStringList GetTagsForIndexImpl (int) const;
	void UpdateTagsImpl (const QStringList&, int);
private slots:
    void writeSettings ();
    void checkFinished ();
    void queryLibtorrentForWarnings ();
	void scrape ();
protected:
    virtual void timerEvent (QTimerEvent*);
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
    void logMessage (const QString&);
    void torrentFinished (const QString&);
    void fileFinished (const QString&);
    void addToHistory (const QString&, const QString&, quint64, QDateTime);
	void taskFinished (int);
	void taskRemoved (int);
};

#endif

