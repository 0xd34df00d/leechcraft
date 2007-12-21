#ifndef HTTPPLUGIN_H
#define HTTPPLUGIN_H
#include <QString>
#include <QMainWindow>
#include <QVector>
#include <QStringList>
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
				 , public IVersionable
{
	Q_OBJECT
	Q_INTERFACES (IInfo IWindow IDirectDownload IVersionable);

	int ID_;
	bool IsShown_, SaveChangesScheduled_, CronEnabled_;
	QPushButton *AddJobButton_;
	QSplitter *Splitter_;
	ContextableList *TasksList_, *FinishedList_;
	SettingsDialog *SettingsDialog_;
	JobManager *JobManager_;
	QStringList ProvidesList_, NeedsList_, UsesList_;
	QLabel *SpeedIndicator_;
	QAction *AddJobAction_
		  , *DeleteJobAction_
		  , *StartJobAction_
		  , *StartAllAction_
		  , *StopJobAction_
		  , *StopAllAction_
		  , *GetFileSizeAction_
		  , *ScheduleSelectedAction_
		  , *DeleteFinishedAction_
		  , *CopyFinishedURL_
		  , *PreferencesAction_
		  , *AutoAdjustInterfaceAction_;
	QToolBar *JobManagementToolbar_, *FinishedManagementToolbar_;
public:
	enum TasksListHeaders
	{
		TListID = 0
		, TListLocalName = 1
		, TListURL = 2
		, TListPercent = 3
		, TListSpeed = 4
		, TListDownloaded = 5
		, TListTotal = 6
	};
	enum FinishedListHeaders
	{
		FListLocalName = 0
		, FListURL = 1
		, FListSize = 2
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
	void FillInterface ();
	void SetupJobManagementBar ();
	void SetupJobManagementMenu (QMenu*);
	void SetupToolsBar (QToolBar*);
	void SetupToolsMenu (QMenu*);
	void SetupStatusBarStuff ();
	void SetupMainWidget ();
	QWidget* SetupTasksPart ();
	QWidget* SetupFinishedPart ();
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

	virtual void StartAll ();
	virtual void StopAll ();
	virtual void StartAt (IDownload::JobID_t);
	virtual void StopAt (IDownload::JobID_t);
	virtual void DeleteAt (IDownload::JobID_t);
	void GetFileSizeAt (IDownload::JobID_t);
	void ScheduleAt (IDownload::JobID_t);
	virtual uint GetVersion () const;

	int GetPercentageForRow (int);
public slots:
	virtual void addDownload (const DirectDownloadParams&);
private slots:
	void initiateJobAddition ();
	int handleParams (JobParams*);
	void pushJob (unsigned int);
	void updateJobDisplay (unsigned int);
	void handleGotFileSize (unsigned int);
	void handleJobRemoval (unsigned int);
	void handleJobFinish (unsigned int);
	void handleJobStart (unsigned int);
	void handleJobDelete (unsigned int);
	void handleJobWaiting (unsigned int);
	void startDownloadSelected ();
	void stopDownloadSelected ();
	void deleteDownloadSelected ();
	void deleteDownloadSelectedFinished ();
	void getFileSize ();
	void scheduleSelected ();
	void startDownloadAll ();
	void stopDownloadAll ();
	void showPreferences ();
	void showJobErrorMessage (QString, QString);
	void showStoppedIndicator (unsigned int);
	void handleTotalSpeedUpdate ();
	void autoAdjustInterface ();
	void writeSettings ();
	void copyFinishedURL ();
	void setActionsEnabled ();
	void handleCronEnabled ();
private:
	void ReadSettings ();
	void AddToFinishedList (const FinishedJob*);
	void HandleSelected (JobAction);
protected:
	virtual void closeEvent (QCloseEvent*);
signals:
	void jobAdded (int);
	void jobFinished (int);
	void jobRemoved (int);
	void jobError (int, IDirectDownload::Error);
	void jobProgressUpdated (int, int);
};

#endif

