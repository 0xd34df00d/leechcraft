#ifndef JOBMANAGER_H
#define JOBMANAGER_H
#include <QAbstractItemModel>
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
class FinishedJob;

class JobManager : public QAbstractItemModel
{
    Q_OBJECT

    QWidget *TheMain_;

    QList<Job*> Jobs_;
    QVector<qint64> JobSpeeds_;

    QMap<Job*, int> IDOnAddition_;

    int TotalDownloads_;
    QMap<QString, int> DownloadsPerHost_;
    typedef QMultiMap<QString, int> MultiHostDict_t;
    MultiHostDict_t ScheduledJobsForHosts_;
    QStack<int> ScheduledJobs_;
    QVector<QPair<int, QTime> > ScheduledStarters_;                // Time of stop
    QMap<QString, QObject*> Providers_;

    int QueryWaitingTimer_;
    FileExistsDialog *FileExists_;
    bool SaveChangesScheduled_, CronEnabled_;

    QStringList Headers_;
    JobManager (QObject *parent = 0);
    ~JobManager ();
public:
    enum TasksListHeaders
    {
        TListState = 0
        , TListLocalName = 1
        , TListURL = 2
        , TListPercent = 3
        , TListSpeed = 4
        , TListDownloadTime = 5
        , TListRemainingTime = 6
        , TListDownloaded = 7
        , TListTotal = 8
    };

    static JobManager& Instance ();
    void Release ();
    void DoDelayedInit ();

    virtual int columnCount (const QModelIndex& index = QModelIndex ()) const;
    virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual bool hasChildren (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation, int role = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int, const QModelIndex& parent = QModelIndex ()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;

    void SetTheMain (QWidget*);
    QWidget* GetTheMain () const;
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
    void addJob (JobParams*);
protected:
    virtual void timerEvent (QTimerEvent*);
signals:
    void showError (QString, QString);
    void stopped (unsigned int);
    void gotFileSize (unsigned int);
    void cronEnabled ();
    void addToFinishedList (const FinishedJob*, int);
private slots:
    void jobStopHandler ();
    void addToFinishedList ();
    void removeJob ();
    void handleJobFinish ();
    void enqueue ();
    void handleJobDisplay ();
    void handleJobDisplay (unsigned int);
    void updateAll ();
    void saveSettings ();
    void scheduleSave ();
    void readSettings ();
private:
    void TryToStartScheduled ();
    uint JobPosition (Job*);
};

#endif

