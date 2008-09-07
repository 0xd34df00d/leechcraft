#ifndef TORRENTPLUIGN_H
#define TORRENTPLUIGN_H
#include <memory>
#include <QMainWindow>
#include <interfaces/interfaces.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "ui_tabwidget.h"
#include "torrentinfo.h"

class AddTorrent;
class QTimer;
class QToolBar;
class QSortFilterProxyModel;
class QTabWidget;
class RepresentationModel;
class TagsCompleter;

class TorrentPlugin : public QObject
                    , public IInfo
					, public IDownload
                    , public IPeer2PeerDownload
                    , public IRemoteable
                    , public IJobHolder
					, public IImportExport
					, public IEmbedModel
					, public ITaggableJobs
{
    Q_OBJECT

    Q_INTERFACES (IInfo IDownload IPeer2PeerDownload IRemoteable IJobHolder IImportExport IEmbedModel ITaggableJobs);

    ID_t ID_;
    std::auto_ptr<XmlSettingsDialog> XmlSettingsDialog_;
	std::auto_ptr<AddTorrent> AddTorrentDialog_;
	std::auto_ptr<QTimer> OverallStatsUpdateTimer_;
	std::auto_ptr<QTime> LastPeersUpdate_;
	std::auto_ptr<RepresentationModel> FilterModel_;
    QMenu *Plugins_;
    bool IgnoreTimer_;
    bool TorrentSelectionChanged_;
	std::auto_ptr<TagsCompleter> TagsChangeCompleter_,
		TagsAddDiaCompleter_;
	std::auto_ptr<QTabWidget> TabWidget_;
	Ui::TabWidget Ui_;
	std::auto_ptr<QAction> OpenTorrent_,
		RemoveTorrent_,
		Preferences_,
		Resume_,
		Stop_,
		CreateTorrent_,
		ForceReannounce_,
		ForceRecheck_,
		OpenMultipleTorrents_,
		MoveFiles_;
	std::auto_ptr<QToolBar> Toolbar_;
public:
    // IInfo
    void Init ();
	virtual ~TorrentPlugin ();
    QString GetName () const;
    QString GetInfo () const;
    QString GetStatusbarMessage () const;
    IInfo& SetID (ID_t);
    ID_t GetID () const;
    QStringList Provides () const;
    QStringList Needs () const;
    QStringList Uses () const;
    void SetProvider (QObject*, const QString&);
    void PushMainWindowExternals (const MainWindowExternals&);
    void Release ();
    QIcon GetIcon () const;

    // IDownload
    qint64 GetDownloadSpeed () const;
    qint64 GetUploadSpeed () const;
    void StartAll ();
    void StopAll ();
	bool CouldDownload (const QString&, LeechCraft::TaskParameters) const;
	int AddJob (const QString&, LeechCraft::TaskParameters);

    // IRemoteable
    QList<QVariantList> GetAll () const;
    AddJobType GetAddJobType () const;
    void AddJob (const QByteArray&, const QString&);
    void StartAt (int);
    void StopAt (int);
    void DeleteAt (int);

    // IJobHolder
    QAbstractItemModel* GetRepresentation () const;
	QWidget* GetControls () const;
	QWidget* GetAdditionalInfo () const;

	// IImportExport
	void ImportSettings (const QByteArray&);
	void ImportData (const QByteArray&);
	QByteArray ExportSettings () const;
	QByteArray ExportData () const;

	// IEmbedModel
	void ItemSelected (const QModelIndex&);

	// ITaggableJobs
	QStringList GetTags (int) const;
	void SetTags (int, const QStringList&);
public slots:
    void updateTorrentStats ();
private slots:
    void on_OpenTorrent__triggered ();
    void on_OpenMultipleTorrents__triggered ();
    void on_CreateTorrent__triggered ();
    void on_RemoveTorrent__triggered (int);
    void on_Resume__triggered (int);
    void on_Stop__triggered (int);
    void on_ForceReannounce__triggered (int);
	void on_ForceRecheck__triggered (int);
    void on_Preferences__triggered ();
    void on_OverallDownloadRateController__valueChanged (int);
    void on_OverallUploadRateController__valueChanged (int);
    void on_DesiredRating__valueChanged (double);
    void on_TorrentDownloadRateController__valueChanged (int);
    void on_TorrentUploadRateController__valueChanged (int);
    void on_TorrentDesiredRating__valueChanged (double);
    void on_CaseSensitiveSearch__stateChanged (int);
    void on_DownloadingTorrents__valueChanged (int);
    void on_UploadingTorrents__valueChanged (int);
    void on_TorrentTags__editingFinished ();
    void on_MoveFiles__triggered (int = 0);
    void setActionsEnabled ();
    void showError (QString);
    void updateOverallStats ();
    void restartTimers ();
    void doLogMessage (const QString&);
    void addToHistory (const QString&, const QString&, quint64, QDateTime);
private:
    void UpdateDashboard ();
	void UpdateTorrentControl ();
    void UpdateTorrentPage ();
    void UpdateFilesPage ();
    void UpdatePeersPage ();
    void UpdatePiecesPage ();
	void SetupTabWidget ();
    void SetupCore ();
    void SetupTorrentView ();
    void SetupStuff ();
	void SetupActions ();
signals:
    void downloadFinished (const QString&);
    void fileDownloaded (const QString&);
	void jobFinished (int);
	void jobRemoved (int);
};

#endif

