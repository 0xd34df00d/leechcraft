#ifndef CORE_H
#define CORE_H
#include <QAbstractItemModel>
#include <QPair>
#include <QList>
#include <QVector>
#include <torrent_info.hpp>
#include <torrent_handle.hpp>
#include <session.hpp>
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
        quint64 UploadedBefore_;
        std::vector<int> FilePriorities_;
        libtorrent::torrent_handle Handle_;
        QByteArray TorrentFileContents_;
        QString TorrentFileName_;
        TorrentState State_;
        double Ratio_;
        QStringList Tags_;
    };

    libtorrent::session *Session_;
    typedef QList<TorrentStruct> HandleDict_t;
    HandleDict_t Handles_;
    QList<QString> Headers_;
    int InterfaceUpdateTimer_, CurrentTorrent_;
    QTimer *SettingsSaveTimer_;
    PiecesModel *PiecesModel_;
    PeersModel *PeersModel_;
    TagsCompletionModel *TagsCompletionModel_;
	TorrentFilesModel *TorrentFilesModel_;
public:
    enum Columns
    {
        ColumnName = 0
        , ColumnDownloaded
        , ColumnUploaded
        , ColumnRating
        , ColumnSize
        , ColumnProgress
        , ColumnState
        , ColumnSP
        , ColumnDSpeed
        , ColumnUSpeed 
        , ColumnRemaining 
    };
    static Core* Instance ();
    Core (QObject *parent = 0);
    void DoDelayedInit ();
    void Release ();
    PiecesModel* GetPiecesModel ();
    void ClearPieces ();
    void UpdatePieces (int);
    PeersModel* GetPeersModel ();
    void ClearPeers ();
    void UpdatePeers (int);
	TorrentFilesModel* GetTorrentFilesModel ();
	void ClearFiles ();
	void UpdateFiles (int);
	void ResetFiles (int);

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
    TorrentInfo GetTorrentStats (int) const;
    const std::vector<bool>* GetLocalPieces (int) const;
    OverallStats GetOverallStats () const;
    QList<FileInfo> GetTorrentFiles (int) const;
    QList<PeerInfo> GetPeers (int) const;
    QStringList GetTagsForIndex (int) const;
    void UpdateTags (int, const QStringList&);
    TagsCompletionModel* GetTagsCompletionModel ();
    void AddFile (const QString&, const QString&, const QStringList&, const QVector<bool>& = QVector<bool> ());
    void RemoveTorrent (int);
    void PauseTorrent (int);
    void ResumeTorrent (int);
    void ForceReannounce (int);
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
    void SetTorrentDownloadRate (int, int);
    void SetTorrentUploadRate (int, int);
    void SetTorrentDesiredRating (double, int);
    int GetTorrentDownloadRate (int) const;
    int GetTorrentUploadRate (int) const;
    double GetTorrentDesiredRating (int) const;
    void SetFilePriority (int, int, int);
    int GetFilePriority (int, int) const;
    QStringList GetTrackers (int) const;
    void SetTrackers (int, const QStringList&);
    QString GetTorrentDirectory (int) const;
    bool MoveTorrentFiles (int, const QString&);
    void MakeTorrent (NewTorrentParams) const;
    void SetCurrentTorrent (int);
    void LogMessage (const QString&);
private:
    QString GetStringForState (libtorrent::torrent_status::state_t) const;
    bool CheckValidity (int) const;
    void ReadSettings ();
    void RestoreTorrents ();
    libtorrent::torrent_handle RestoreSingleTorrent (const QByteArray&, const QByteArray&, const boost::filesystem::path&);
    void HandleSingleFinished (const libtorrent::torrent_info&, const QString&);
    int GetCurrentlyDownloading () const;
    int GetCurrentlySeeding () const;
    void ManipulateSettings ();
private slots:
    void writeSettings ();
    void checkFinished ();
    void queryLibtorrentForWarnings ();
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
signals:
    void error (QString) const;
    void logMessage (const QString&);
    void torrentFinished (const QString&);
    void fileFinished (const QString&);
    void addToHistory (const QString&, const QString&, quint64, QDateTime);
};

#endif

