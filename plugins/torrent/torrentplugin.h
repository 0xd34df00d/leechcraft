#ifndef TORRENTPLUGIN_H
#define TORRENTPLUGIN_H
#include <QMainWindow>
#include <QString>
#include <QStringList>
#include <QIcon>
#include <interfaces/interfaces.h>
#include "torrentclient.h"

class Proxy;
class QSplitter;
class QTreeWidget;
class QToolBar;
class AddTorrentDialog;
class QProgressDialog;
class SettingsDialog;
class TorrentClient;
class QCloseEvent;
class QSlider;
class QLabel;

class TorrentPlugin : public QMainWindow
					, public IInfo
					, public IWindow
					, public IDownload
{
	Q_OBJECT
	Q_INTERFACES (IInfo IWindow IDownload);

	int ID_;
	bool IsShown_;
	struct Job
	{
		TorrentClient *TC_;
		QString Filename_;
		QString DestDir_;
	};
	QList<Job> Jobs_;

	QTreeWidget *Downloading_;
	QToolBar *ManagementToolbar_, *ToolsToolbar_, *BottomToolbar_;
	AddTorrentDialog *AddTorrentDialog_;
	QSlider *DownloadLimitSlider_, *UploadLimitSlider_;
	QLabel *DownloadLimitLabel_, *UploadLimitLabel_;
	QString LastOpenDir_, LastSaveDir_;

	SettingsDialog *SettingsDialog_;

	QProgressDialog *QuitDialog_;

	QAction *PreferencesAction_;
	QAction *AddAction_, *DeleteAction_, *PauseAction_, *MoveUpAction_, *MoveDownAction_;

	bool SaveChanges_;

	int JobsStopped_, JobsToStop_;
public:
	virtual void Init ();
	virtual ~TorrentPlugin ();

	enum Columns
	{
		CName = 0
		, CPeersSeeds = 1
		, CProgress = 2
		, CDownloading = 3
		, CUploading = 4
		, CStatus = 5
	};
private:
	void FillInterface ();
	void SetupMainWidget ();
	void SetupToolbars ();
	QWidget *SetupDownloading ();
	void SetupActions ();
	void SetupMenus ();
private slots:
	bool addTorrent (const QString&, const QString&, const QByteArray& resume = QByteArray ());
	bool addTorrent ();
	void removeTorrent ();
	void pauseTorrent ();
	void moveUp ();
	void moveDown ();
	void setActionsEnabled ();
	void setUploadLimit (int);
	void setDownloadLimit (int);
	void torrentStopped ();
	void torrentError (TorrentClient::Error);
	void updatePeerInfo ();
	void updateProgress (int);
	void updateDownloadRate (int);
	void updateUploadRate (int);
	void updateState (TorrentClient::State);
public:
    virtual QString GetName () const;
    virtual QString GetInfo () const;
    virtual QString GetStatusbarMessage () const;
    virtual IInfo& SetID (ID_t);
    virtual ID_t GetID () const;
    virtual QStringList Provides () const;
    virtual QStringList Needs () const;
	virtual QStringList Uses () const;
	virtual void SetProvider (QObject*, const QString&);
    virtual void Release ();

    virtual QIcon GetIcon () const;
    virtual void SetParent (QWidget*);
    virtual void ShowWindow ();
    virtual void ShowBalloonTip ();

	virtual qint64 GetDownloadSpeed () const;
	virtual qint64 GetUploadSpeed () const;
    virtual void StartAll () {}
    virtual void StopAll () {}
    virtual void StartAt (IDownload::JobID_t) {}
    virtual void StopAt (IDownload::JobID_t) {}
    virtual void DeleteAt (IDownload::JobID_t) {}

	const TorrentClient* GetClientForRow (int) const;
	int GetRowOfClient (TorrentClient*) const;
public slots:
	void SaveSettings () const;
	void ReadSettings ();
private slots:
	void showError (QString);
	void showPreferences ();
protected:
	virtual void closeEvent (QCloseEvent*);
};

#endif

