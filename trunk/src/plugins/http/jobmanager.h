#ifndef JOBMANAGER_H
#define JOBMANAGER_H
#include <QObject>
#include <QVector>
#include <QMap>
#include <QMultiMap>
#include <QPair>
#include <QStack>
#include <QTime>
#include "impbase.h"

class Job;
struct JobParams;
struct JobRepresentation;
class FileExistsDialog;
class QWidget;
class Proxy;

class JobManager : public QObject
{
 Q_OBJECT

 enum { PoolSize_ = 4096 };

 QWidget *TheMain_;

 QVector<Job*> Jobs_;
 QMap<unsigned int, QVector<Job*>::size_type> ID2Pos_;  // It's not position in the QListWidget etc, but position in the Jobs_ vector
 QVector<unsigned int> IDPool_;
 QVector<qint64> JobSpeeds_;

 int TotalDownloads_;
 QMap<QString, int> DownloadsPerHost_;
 typedef QMultiMap<QString, int> MultiHostDict_t;
 MultiHostDict_t ScheduledJobsForHosts_;
 QStack<int> ScheduledJobs_;
 QVector<QPair<int, QTime> > ScheduledStarters_;    // Time of stop
 QMap<QString, QObject*> Providers_;

 int QueryWaitingTimer_;
 FileExistsDialog *FileExists_;
 bool SaveChangesScheduled_, CronEnabled_;
public:
 JobManager (QObject *parent = 0);
 ~JobManager ();
 void Release ();
 void DoDelayedInit ();
 void SetTheMain (QWidget*);
 QWidget* GetTheMain () const;
 JobRepresentation* GetJobRepresentation (unsigned int) const;
 qint64 GetDownloadSpeed () const;
 bool Start (unsigned int);
 void Stop (unsigned int);
 void Delete (unsigned int);
 void GetFileSize (unsigned int);
 void Schedule (unsigned int);
 void StartAll ();
 void StopAll ();
 void SetProvider (QObject*, const QString&);
 void UpdateParams (int, JobParams*);
public slots:
 int addJob (JobParams*);
protected:
 virtual void timerEvent (QTimerEvent*);
signals:
 void jobAdded (unsigned int);
 void jobFinished (unsigned int);
 void jobRemoved (unsigned int);
 void jobStarted (unsigned int);
 void jobWaiting (unsigned int);
 void updateJobDisplay (unsigned int);
 void deleteJob (unsigned int);
 void showError (QString, QString);
 void stopped (unsigned int);
 void gotFileSize (unsigned int);
 void cronEnabled ();
private slots:
 void jobStopHandler (unsigned int);
 void enqueue (unsigned int);
 void handleJobDisplay (unsigned int);
 void saveSettings ();
 void scheduleSave ();
private:
 void TryToStartScheduled ();
 void RehashID2Pos ();
};

#endif

