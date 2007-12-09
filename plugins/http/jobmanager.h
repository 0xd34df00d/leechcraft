#ifndef JOBMANAGER_H
#define JOBMANAGER_H
#include <QObject>
#include <QVector>
#include <QMap>
#include <QMultiMap>
#include <QPair>
#include <QStack>
#include <QTime>

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
	QMap<unsigned int, QVector<Job*>::size_type> ID2Pos_;		// It's not position in the QListWidget etc, but position in the Jobs_ vector
	int TotalDownloads_;
	QMap<QString, int> DownloadsPerHost_;
	QMultiMap<QString, int> ScheduledJobsForHosts_;
	QStack<int> ScheduledJobs_;
	QVector<unsigned int> IDPool_;
	QVector<qint64> JobSpeeds_;
	QVector<QPair<int, QTime> > ScheduledStarters_;

	FileExistsDialog *FileExists_;

	bool SaveChangesScheduled_;
public:
	JobManager (QObject *parent = 0);
	~JobManager ();
	void Release ();
	void DoDelayedInit ();
	void SetTheMain (QWidget*);
	JobRepresentation* GetJobRepresentation (unsigned int) const;
	qint64 GetDownloadSpeed () const;
	void Start (unsigned int);
	void Stop (unsigned int);
	void DeleteAt (unsigned int);
	void StartAll ();
	void StopAll ();
public slots:
	int addJob (JobParams*);
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
private slots:
	void jobStopHandler (unsigned int);
	void enqueue (unsigned int);
	void handleJobDisplay (unsigned int);
	void saveSettings ();
	void tryToStart ();
	void scheduleSave ();
private:
	void RehashID2Pos ();
};

#endif

