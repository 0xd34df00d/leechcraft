#ifndef TORRENTPLUIGN_H
#define TORRENTPLUIGN_H
#include <QMainWindow>
#include <interfaces/interfaces.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "ui_mainwindow.h"
#include "torrentinfo.h"

class AddTorrent;
class QTimer;
class QSortFilterProxyModel;

class TorrentPlugin : public QMainWindow
                    , public IInfo
                    , public IWindow
                    , public IPeer2PeerDownload
                    , public IRemoteable
                    , public IJobHolder
                    , private Ui::MainWindow
{
    Q_OBJECT

    Q_INTERFACES (IInfo IWindow IPeer2PeerDownload IRemoteable IJobHolder);

    ID_t ID_;
    bool IsShown_;
    XmlSettingsDialog *XmlSettingsDialog_;
    AddTorrent *AddTorrentDialog_;
    QTimer *OverallStatsUpdateTimer_;
    QTime *LastPeersUpdate_;
    QSortFilterProxyModel *FilterModel_;
    QMenu *Plugins_;
    bool IgnoreTimer_;
    bool TorrentSelectionChanged_;
public:
    // IInfo
    void Init ();
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
    //IWindow
    QIcon GetIcon () const;
    void SetParent (QWidget*);
    void ShowWindow ();
    void ShowBalloonTip ();
    // IDownload
    qint64 GetDownloadSpeed () const;
    qint64 GetUploadSpeed () const;
    void StartAll ();
    void StopAll ();
    bool CouldDownload (const QString&, bool) const;
    void AddJob (const QString&);

    // IRemoteable
    QList<QVariantList> GetAll () const;
    AddJobType GetAddJobType () const;
    void AddJob (const QByteArray&, const QString&);
    void StartAt (int);
    void StopAt (int);
    void DeleteAt (int);

    // IJobHolder
    QAbstractItemModel* GetRepresentation () const;
    QAbstractItemDelegate* GetDelegate () const;
public slots:
    void handleHidePlugins ();
protected:
    virtual void closeEvent (QCloseEvent*);
private slots:
    void on_OpenTorrent__triggered ();
    void on_OpenMultipleTorrents__triggered ();
    void on_CreateTorrent__triggered ();
    void on_RemoveTorrent__triggered ();
    void on_Resume__triggered ();
    void on_Stop__triggered ();
    void on_ForceReannounce__triggered ();
    void on_ChangeTrackers__triggered ();
    void on_Preferences__triggered ();
    void on_TorrentView__clicked (const QModelIndex&);
    void on_TorrentView__pressed (const QModelIndex&);
    void on_OverallDownloadRateController__valueChanged (int);
    void on_OverallUploadRateController__valueChanged (int);
    void on_DesiredRating__valueChanged (double);
    void on_TorrentDownloadRateController__valueChanged (int);
    void on_TorrentUploadRateController__valueChanged (int);
    void on_TorrentDesiredRating__valueChanged (double);
    void on_FilesWidget__currentItemChanged (QTreeWidgetItem*);
    void on_PrioritySpinbox__valueChanged (int);
    void on_CaseSensitiveSearch__stateChanged (int);
    void setActionsEnabled ();
    void showError (QString);
    void updateTorrentStats ();
    void restartTimers ();
    void updateOverallStats ();
    void doLogMessage (const QString&);
signals:
    void downloadFinished (const QString&);
    void fileDownloaded (const QString&);
};

#endif

