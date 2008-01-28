#ifndef HTTPPLUGIN_H
#define HTTPPLUGIN_H
#include <QString>
#include <QMainWindow>
#include <QVector>
#include <QStringList>
#include "ui_mainwindow.h"
#include "interfaces/interfaces.h"
#include "httpimp.h"
#include "globals.h"

class QPushButton;
class QSplitter;
class Job;
class JobManager;
class Proxy;
class FinishedJob;
class SettingsDialog;
class QLabel;
class QAction;
class QToolBar;
class ContextableList;

struct JobParams;

class HttpPlugin : public QMainWindow
				 , public IInfo
				 , public IWindow
				 , public IDirectDownload
{
	Q_OBJECT
	Q_INTERFACES (IInfo IWindow IDirectDownload);

	Ui::MainWindow Ui_;

	int ID_;
	bool IsShown_, SaveChangesScheduled_, CronEnabled_;
	SettingsDialog *SettingsDialog_;
	JobManager *JobManager_;
	QStringList ProvidesList_, NeedsList_, UsesList_, TaskHeaderLabels_, FinishedHeaderLabels_;
	QLabel *SpeedIndicator_;
	QMenu *Plugins_;
public:
	enum TasksListHeaders
	{
		TListID = 0
		, TListLocalName = 1
		, TListURL = 2
		, TListPercent = 3
		, TListSpeed = 4
		, TListDownloadTime = 5
		, TListRemainingTime = 6
		, TListDownloaded = 7
		, TListTotal = 8
	};
	enum FinishedListHeaders
	{
		FListLocalName = 0
		, FListURL = 1
		, FListSize = 2
		, FListSpeed = 3
		, FListTimeToComplete = 4
	};

	enum JobAction
	{
		JAStart
		, JAStop
		, JADelete
		, JAGFS
		, JASchedule
	};

	virtual void Init ();
	virtual ~HttpPlugin ();
private:
	void SetupStatusBarStuff ();
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
	virtual void PushMainWindowExternals (const MainWindowExternals&);
	virtual void Release ();

	virtual QIcon GetIcon () const;
	virtual void SetParent (QWidget*);
	virtual void ShowWindow ();
	virtual void ShowBalloonTip ();

	virtual qint64 GetDownloadSpeed () const;
	virtual qint64 GetUploadSpeed () const;

	virtual void StartAll ();
	virtual void StopAll ();
	
	virtual bool CouldDownload (const QString&) const;
	virtual void AddJob (const QString&);

	int GetPercentageForRow (int);
public slots:
	virtual void addDownload (const DirectDownloadParams&);
	void handleHidePlugins ();
private slots:
	void on_ActionAddJob__triggered ();
	int handleParams (JobParams*);
	void pushJob (unsigned int);
	void updateJobDisplay (unsigned int);
	void handleGotFileSize (unsigned int);
	void handleJobRemoval (unsigned int);
	void handleJobFinish (unsigned int);
	void handleJobStart (unsigned int);
	void handleJobDelete (unsigned int);
	void handleJobWaiting (unsigned int);
	void on_ActionStart__triggered ();
	void on_ActionStop__triggered ();
	void on_ActionRemoveJob__triggered ();
	void on_ActionRemoveFinished__triggered ();
	void on_ActionGetFileSize__triggered ();
	void on_ActionSchedule__triggered ();
	void on_ActionStartAll__triggered ();
	void on_ActionStopAll__triggered ();
	void on_ActionJobProperties__triggered ();
	void on_ActionPreferences__triggered ();
	void showJobErrorMessage (QString, QString);
	void showStoppedIndicator (unsigned int);
	void handleTotalSpeedUpdate ();
	void on_ActionAutoajust__triggered ();
	void writeSettings ();
	void copyFinishedURL ();
	void setActionsEnabled ();
	void handleCronEnabled ();
	void on_ActionActiveColumns__triggered ();
	void on_ActionFinishedColumns__triggered ();
private:
	void ReadSettings ();
	void AddToFinishedList (const FinishedJob*);
	void HandleSelected (JobAction);
protected:
	virtual void closeEvent (QCloseEvent*);
signals:
	void downloadFinished (const QString&);
	void fileDownloaded (const QString&);
	void jobAdded (int);
	void jobFinished (int);
	void jobRemoved (int);
	void jobError (int, IDirectDownload::Error);
	void jobProgressUpdated (int, int);
};

#endif

