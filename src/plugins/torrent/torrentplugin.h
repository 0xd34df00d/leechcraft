#ifndef TORRENTPLUIGN_H
#define TORRENTPLUIGN_H
#include <QMainWindow>
#include <interfaces/interfaces.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "ui_mainwindow.h"
#include "torrentinfo.h"

class AddTorrent;
class QTimer;

class TorrentPlugin : public QMainWindow
                    , public IInfo
                    , public IWindow
                    , public IPeer2PeerDownload
                    , private Ui::MainWindow
{
    Q_OBJECT

    Q_INTERFACES (IInfo IWindow IPeer2PeerDownload);

    ID_t ID_;
    bool IsShown_;
    XmlSettingsDialog *XmlSettingsDialog_;
    AddTorrent *AddTorrentDialog_;
    QTimer *OverallStatsUpdateTimer_;
    QMenu *Plugins_;
public:
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
    QIcon GetIcon () const;
    void SetParent (QWidget*);
    void ShowWindow ();
    void ShowBalloonTip ();
    qint64 GetDownloadSpeed () const;
    qint64 GetUploadSpeed () const;
    void StartAll ();
    void StopAll ();
    bool CouldDownload (const QString&) const;
    void AddJob (const QString&);
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
    void on_Preferences__triggered ();
    void on_TorrentView__clicked (const QModelIndex&);
    void on_TorrentView__pressed (const QModelIndex&);
    void on_OverallDownloadRateController__valueChanged (int);
    void on_OverallUploadRateController__valueChanged (int);
    void on_DesiredRating__valueChanged (double);
    void setActionsEnabled ();
    void showError (QString);
    void updateTorrentStats ();
    void updateOverallStats ();
    void doLogMessage (const QString&);
signals:
    void downloadFinished (const QString&);
    void fileDownloaded (const QString&);
};

#endif

