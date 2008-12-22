#ifndef TORRENTPLUIGN_H
#define TORRENTPLUIGN_H
#include <memory>
#include <deque>
#include <QMainWindow>
#include <interfaces/interfaces.h>
#include <plugininterface/tagscompleter.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "ui_tabwidget.h"
#include "torrentinfo.h"

class AddTorrent;
class QTimer;
class QToolBar;
class QSortFilterProxyModel;
class QTabWidget;
class RepresentationModel;
class QTranslator;

class TorrentPlugin : public QObject
                    , public IInfo
					, public IDownload
                    , public IJobHolder
					, public IImportExport
					, public ITaggableJobs
{
    Q_OBJECT

    Q_INTERFACES (IInfo IDownload IJobHolder IImportExport ITaggableJobs);

    std::auto_ptr<XmlSettingsDialog> XmlSettingsDialog_;
	std::auto_ptr<AddTorrent> AddTorrentDialog_;
	std::auto_ptr<QTimer> OverallStatsUpdateTimer_;
	std::auto_ptr<QTime> LastPeersUpdate_;
	std::auto_ptr<RepresentationModel> FilterModel_;
    bool TorrentSelectionChanged_;
	std::auto_ptr<LeechCraft::Util::TagsCompleter> TagsChangeCompleter_,
		TagsAddDiaCompleter_;
	std::auto_ptr<QTabWidget> TabWidget_;
	Ui::TabWidget Ui_;
	std::auto_ptr<QToolBar> Toolbar_;
	std::auto_ptr<QAction> OpenTorrent_,
		RemoveTorrent_,
		Preferences_,
		Resume_,
		Stop_,
		CreateTorrent_,
		MoveUp_,
		MoveDown_,
		MoveToTop_,
		MoveToBottom_,
		ForceReannounce_,
		ForceRecheck_,
		OpenMultipleTorrents_,
		MoveFiles_,
		ChangeTrackers_,
		Import_,
		Export_;
	std::auto_ptr<QTranslator> Translator_;
public:
    // IInfo
    void Init ();
	virtual ~TorrentPlugin ();
    QString GetName () const;
    QString GetInfo () const;
    QStringList Provides () const;
    QStringList Needs () const;
    QStringList Uses () const;
    void SetProvider (QObject*, const QString&);
    void Release ();
    QIcon GetIcon () const;

    // IDownload
    qint64 GetDownloadSpeed () const;
    qint64 GetUploadSpeed () const;
    void StartAll ();
    void StopAll ();
	bool CouldDownload (const QByteArray&, LeechCraft::TaskParameters) const;
	int AddJob (const LeechCraft::DownloadParams&, LeechCraft::TaskParameters);

    // IJobHolder
    QAbstractItemModel* GetRepresentation () const;
	LeechCraft::Util::HistoryModel* GetHistory () const;
	QWidget* GetControls () const;
	QWidget* GetAdditionalInfo () const;
	void ItemSelected (const QModelIndex&);

	// IImportExport
	void ImportSettings (const QByteArray&);
	void ImportData (const QByteArray&);
	QByteArray ExportSettings () const;
	QByteArray ExportData () const;

	// ITaggableJobs
	QStringList GetTags (int) const;
	QStringList GetHistoryTags (int) const;
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
	void on_MoveUp__triggered (const std::deque<int>&);
	void on_MoveDown__triggered (const std::deque<int>&);
	void on_MoveToTop__triggered (const std::deque<int>&);
	void on_MoveToBottom__triggered (const std::deque<int>&);
    void on_ForceReannounce__triggered (int);
	void on_ForceRecheck__triggered (int);
    void on_Preferences__triggered ();
	void on_ChangeTrackers__triggered ();
    void on_OverallDownloadRateController__valueChanged (int);
    void on_OverallUploadRateController__valueChanged (int);
    void on_DesiredRating__valueChanged (double);
    void on_TorrentDownloadRateController__valueChanged (int);
    void on_TorrentUploadRateController__valueChanged (int);
    void on_TorrentDesiredRating__valueChanged (double);
	void on_TorrentManaged__stateChanged (int);
	void on_TorrentSequentialDownload__stateChanged (int);
	void on_TorrentSuperSeeding__stateChanged (int);
    void on_CaseSensitiveSearch__stateChanged (int);
    void on_DownloadingTorrents__valueChanged (int);
    void on_UploadingTorrents__valueChanged (int);
    void on_TorrentTags__editingFinished ();
    void on_MoveFiles__triggered (int = 0);
	void on_Import__triggered ();
	void on_Export__triggered ();
    void setActionsEnabled ();
    void showError (QString);
    void updateOverallStats ();
    void doLogMessage (const QString&);
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

