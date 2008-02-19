#include <QtDebug>
#include <QMessageBox>
#include <QVariantList>
#include <QSettings>
#include <QTimer>
#include <QUrl>
#include <exceptions/io.h>
#include <exceptions/logic.h>
#include <plugininterface/proxy.h>
#include "jobmanager.h"
#include "xmlsettingsmanager.h"
#include "httpplugin.h"
#include "job.h"
#include "jobparams.h"
#include "jobrepresentation.h"
#include "fileexistsdialog.h"
#include "finishedjob.h"

JobManager::JobManager (QObject *parent)
: QAbstractItemModel (parent)
, TotalDownloads_ (0)
, SaveChangesScheduled_ (false)
, CronEnabled_ (false)
{
    QueryWaitingTimer_ = startTimer (500);
    Headers_ << tr ("State") << tr ("Local name") << tr ("URL") << tr ("Progress") << tr ("Speed") << tr ("ETA") << tr ("Download time");
    QTimer *timer = new QTimer;
    connect (timer, SIGNAL (timeout ()), this, SLOT (updateAll ()));
    timer->start (1000);
}

JobManager::~JobManager ()
{
}

JobManager& JobManager::Instance ()
{
    static JobManager TheOnlyOne_;
    return TheOnlyOne_;
}

void JobManager::Release ()
{
    killTimer (QueryWaitingTimer_);
    saveSettings ();

    for (int i = 0; i < Jobs_.size (); ++i)
    {
        Jobs_ [i]->Release ();
        delete Jobs_ [i];
    }
}

void JobManager::DoDelayedInit ()
{
    QTimer::singleShot (500, this, SLOT (readSettings ()));
}

int JobManager::columnCount (const QModelIndex&) const
{
    return Headers_.size ();
}

QVariant JobManager::data (const QModelIndex& index, int role) const
{
    int row = index.row (),
        column = index.column ();
    Job *job = Jobs_.at (row);
    switch (role)
    {
        case Qt::DisplayRole:
            switch (static_cast<TasksListHeaders> (column))
            {
                case TListState:
                    return QVariant ();
                case TListLocalName:
                    return job->GetLocalName ();
                case TListURL:
                    return job->GetURL ();
                case TListPercent:
                    return job->GetTotal () ? job->GetDownloaded () / job->GetTotal () : 0;
                case TListSpeed:
                    return static_cast<quint64> (job->GetSpeed ());
                case TListDownloadTime:
                    return static_cast<quint64> (job->GetDownloadTime () / 1000);
                case TListRemainingTime:
                    return static_cast<quint64> (job->GetAverageTime () / 1000);
                case TListDownloaded:
                    return job->GetDownloaded ();
                case TListTotal:
                    return job->GetTotal ();
            }
        case Qt::DecorationRole:
            switch (static_cast<TasksListHeaders> (column))
            {
                case TListState:
                    switch (job->GetState ())
                    {
                        case Job::StateWaiting:
                            return QIcon (":/resources/images/waiting.png");
                        case Job::StateDownloading:
                            return QIcon (":/resources/images/startjob.png");
                        case Job::StateIdle:
                            return QIcon (":/resources/images/stopjob.png");
                    }
                default:
                    return QVariant ();
            }
        default:
            return QVariant ();
    }
}

Qt::ItemFlags JobManager::flags (const QModelIndex& index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool JobManager::hasChildren (const QModelIndex&) const
{
    return true;
}

QVariant JobManager::headerData (int header, Qt::Orientation orient, int role) const
{
    if (orient == Qt::Vertical)
        return QVariant ();
    if (role != Qt::DisplayRole)
        return QVariant ();

    return Headers_.at (header);
}

QModelIndex JobManager::index (int r, int c, const QModelIndex&) const
{
    if (!hasIndex (r, c) && !hasIndex (r, c - 1) && !hasIndex (r, c - 2))
        return QModelIndex ();
    return createIndex (r, c);
}

QModelIndex JobManager::parent (const QModelIndex&) const
{
    return QModelIndex ();
}

int JobManager::rowCount (const QModelIndex& parent) const
{
    if (parent.isValid ())
        return 0;
    else
        return Jobs_.size ();
}

void JobManager::SetTheMain (QWidget *main)
{
    TheMain_ = main;
    FileExists_ = new FileExistsDialog (TheMain_);
}

QWidget* JobManager::GetTheMain () const
{
    return TheMain_;
} 
void JobManager::addJob (JobParams *params)
{
    Job *job = new Job (params, this);
    connect (job, SIGNAL (finished ()), this, SLOT (handleJobFinish ()));
    connect (job, SIGNAL (deleteJob ()), this, SLOT (jobStopHandler ()));
    connect (job, SIGNAL (deleteJob ()), this, SLOT (removeJob ()));
    connect (job, SIGNAL (stopped ()), this, SLOT (jobStopHandler ()));
    connect (job, SIGNAL (enqueue ()), this, SLOT (enqueue ()));

    connect (job, SIGNAL (updateDisplays ()), this, SLOT (handleJobDisplay ()));
    connect (job, SIGNAL (stopped ()), this, SLOT (handleJobDisplay ()));
    connect (job, SIGNAL (started ()), this, SLOT (handleJobDisplay ()));
    connect (job, SIGNAL (addJob (JobParams*)), this, SLOT (addJob (JobParams*)));
    connect (job, SIGNAL (showError (QString, QString)), this, SIGNAL (showError (QString, QString)));
    connect (job, SIGNAL (gotFileSize ()), this, SLOT (handleJobDisplay ()));

    beginInsertRows (QModelIndex (), Jobs_.size (), Jobs_.size ());

    Jobs_.append (job);

    job->DoDelayedInit ();

    QString host = QUrl (params->URL_).host ();
    if (!DownloadsPerHost_.contains (host))
        DownloadsPerHost_ [host ] = 0;

    if (params->Autostart_)
        Start (Jobs_.size () - 1);
    else if (XmlSettingsManager::Instance ()->property ("AutoGetFileSize").toBool ())
        GetFileSize (Jobs_.size () - 1);
    endInsertRows ();

    scheduleSave ();
}

void JobManager::timerEvent (QTimerEvent *e)
{
    if (QueryWaitingTimer_ == e->timerId ())
        TryToStartScheduled ();
}

qint64 JobManager::GetDownloadSpeed () const
{
    qint64 result = 0;
    for (int i = 0; i < Jobs_.size (); ++i)
        result += Jobs_ [i]->GetCurrentSpeed ();

    return result;
}

bool JobManager::Start (unsigned int id)
{
    if (Jobs_ [id]->GetState () != Job::StateIdle)
        return false;

    QString host = QUrl (Jobs_ [id]->GetURL ()).host ();

    if (DownloadsPerHost_ [host] >= XmlSettingsManager::Instance ()->property ("MaxConcurrentPerServer").toInt ())
    {
        ScheduledJobsForHosts_.insert (host, id);
        emit dataChanged (index (id, 0), index (id, columnCount ()));
        return false;
    }

    if (TotalDownloads_ >= XmlSettingsManager::Instance ()->property ("MaxTotalConcurrent").toInt ())
    {
        ScheduledJobs_.push_back (id);
        emit dataChanged (index (id, 0), index (id, columnCount ()));
        return false;
    }

    try
    {
        Jobs_ [id]->Start ();
        ++DownloadsPerHost_ [host];
        ++TotalDownloads_;
        emit dataChanged (index (id, 0), index (id, columnCount ()));
        return true;
    }
    catch (Exceptions::IO& e)
    {
        QMessageBox::critical (TheMain_, "Job IO error", e.GetReason ().c_str ());
    }
    catch (Exceptions::Logic& e)
    {
        QMessageBox::critical (TheMain_, "Job Logic error", e.GetReason ().c_str ());
    }
    return false;
}

void JobManager::Stop (unsigned int id)
{
    for (QMultiMap<QString, int>::Iterator i = ScheduledJobsForHosts_.begin (); i != ScheduledJobsForHosts_.end (); ++i)
        if (i.value () == static_cast<int> (id))
        {
            i = ScheduledJobsForHosts_.erase (i);
            break;
        }
    if (Jobs_ [id]->GetState () == Job::StateDownloading)
        Jobs_ [id]->Stop ();
    else if (Jobs_ [id]->GetState () == Job::StateWaiting)
        Jobs_ [id]->Stop ();

    emit dataChanged (index (0, id), index (0, id));
}

void JobManager::Delete (unsigned int id)
{
    disconnect (Jobs_ [id], 0, 0, 0);
    Jobs_ [id]->Release ();;
    beginRemoveRows (QModelIndex (), id, id);
    delete Jobs_ [id];
    Jobs_.removeAt (id);
    endRemoveRows ();

    QVector<Job*>::size_type pos = id;
    scheduleSave ();
    for (MultiHostDict_t::Iterator i = ScheduledJobsForHosts_.begin (); i != ScheduledJobsForHosts_.end (); ++i)
        if (i.value () == static_cast<int> (id))
            i = ScheduledJobsForHosts_.erase (i);
    for (int i = 0; i < ScheduledJobs_.size (); ++i)
        if (ScheduledJobs_ [i] == static_cast<int> (id))
            ScheduledJobs_.remove (i--);
}

void JobManager::GetFileSize (unsigned int id)
{
    Jobs_ [id]->GetFileSize ();
}

void JobManager::Schedule (unsigned int id)
{
    emit dataChanged (index (id, 0), index (id, columnCount ()));
}

void JobManager::StartAll ()
{
    for (int i = 0; i < Jobs_.size (); ++i)
        Start (i + 1);
}

void JobManager::StopAll ()
{
    for (int i = 0; i < Jobs_.size (); ++i)
        Stop (i);
}

void JobManager::SetProvider (QObject *object, const QString& feature)
{
    Providers_ [feature] = object;
    if (feature == "cron")
    {
        CronEnabled_ = true;
        emit cronEnabled ();
    }
}

void JobManager::UpdateParams (int id, JobParams *params)
{
    Job *job = Jobs_ [id];
    if (job->GetState () == Job::StateIdle)
        job->UpdateParams (params);
}

void JobManager::jobStopHandler ()
{
    int id = JobPosition (qobject_cast<Job*> (sender ()));
    QString host = QUrl (Jobs_ [id]->GetURL ()).host ();

    if (DownloadsPerHost_ [host] > 0)
        --DownloadsPerHost_ [host];
    if (TotalDownloads_ > 0)
        --TotalDownloads_;

    if (DownloadsPerHost_ [host] < XmlSettingsManager::Instance ()->property ("MaxConcurrentPerServer").toInt () &&
        ScheduledJobsForHosts_.contains (host))
    {
        QList<int> ids = ScheduledJobsForHosts_.values (host);
        qSort (ids);
        int id = ids.takeFirst ();
        ScheduledJobsForHosts_.remove (host, id);
        Start (id);
        return;
    }

    if (TotalDownloads_ < XmlSettingsManager::Instance ()->property ("MaxTotalConcurrent").toInt () &&
        !ScheduledJobs_.isEmpty ())
    {
        Start (ScheduledJobs_.pop ());
        return;
    }
}

void JobManager::addToFinishedList ()
{
    qDebug () << Q_FUNC_INFO;
    Job *job = qobject_cast<Job*> (sender ());
    FinishedJob *fj = new FinishedJob;
    fj->URL_ = job->GetURL ();
    fj->Local_ = job->GetLocalName ();
    fj->Size_ = job->GetTotal ();
    fj->Speed_ = Proxy::Instance ()->MakePrettySize (job->GetSpeed ()) + tr ("/s");
    fj->TimeToComplete_ = Proxy::Instance ()->MakeTimeFromLong (job->GetDownloadTime ()).toString ();

    emit addToFinishedList (fj, IDOnAddition_ [job]);
    delete fj;
}

void JobManager::removeJob ()
{
    int id = JobPosition (qobject_cast<Job*> (sender ()));
    beginRemoveRows (QModelIndex (), id, id);
    Jobs_ [id]->Release ();
    delete Jobs_ [id];
    Jobs_.removeAt (id);
    endRemoveRows ();
}

void JobManager::handleJobFinish ()
{
    qDebug () << Q_FUNC_INFO;
    jobStopHandler ();
    addToFinishedList ();
    removeJob ();
}

void JobManager::enqueue ()
{
    int id = JobPosition (qobject_cast<Job*> (sender ()));
    QString host = QUrl (Jobs_ [id]->GetURL ()).host ();

    Job *job = Jobs_ [id];
    connect (job, SIGNAL (stopped (unsigned int)), this, SIGNAL (stopped (unsigned int)));
    connect (job, SIGNAL (stopped (unsigned int)), this, SLOT (jobStopHandler (unsigned int)));
    ScheduledJobsForHosts_.insert (host, id);
    if (DownloadsPerHost_ [host] > 0)
        --DownloadsPerHost_ [host];
    if (TotalDownloads_ > 0)
        --TotalDownloads_;
    emit dataChanged (index (id, 0), index (id, columnCount ()));
}

void JobManager::handleJobDisplay ()
{
    handleJobDisplay (JobPosition (qobject_cast<Job*> (sender ())));
}

void JobManager::handleJobDisplay (unsigned int id)
{
    emit dataChanged (index (id, 0), index (id, columnCount ()));
}

void JobManager::updateAll ()
{
    emit dataChanged (index (0, 0), index (rowCount (), columnCount ()));
}

void JobManager::saveSettings ()
{
    SaveChangesScheduled_ = false;

    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
    settings.beginGroup ("HTTP and FTP");
    settings.beginWriteArray ("Jobs");
    settings.remove ("");
    for (int i = 0; i < Jobs_.size (); ++i)
    {
        settings.setArrayIndex (i);
        settings.setValue ("job", Jobs_.at (i)->Serialized ());
    }
    settings.endArray ();
    settings.endGroup ();
}

void JobManager::scheduleSave ()
{
    if (!SaveChangesScheduled_)
    {
        SaveChangesScheduled_ = true;
        QTimer::singleShot (500, this, SLOT (saveSettings ()));
    }
}

void JobManager::readSettings ()
{
    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
    settings.beginGroup ("HTTP and FTP");
    int size = settings.beginReadArray ("Jobs");
    if (size)
    {
        beginInsertRows (QModelIndex (), 0, size - 1);
        for (int i = 0; i < size; ++i)
        {
            settings.setArrayIndex (i);
            Job *job = new Job;
            job->Unserialize (settings.value ("job").toByteArray ());
            Jobs_.append (job);
            connect (job, SIGNAL (finished ()), this, SLOT (handleJobFinish ()));
            connect (job, SIGNAL (deleteJob ()), this, SLOT (jobStopHandler ()));
            connect (job, SIGNAL (deleteJob ()), this, SLOT (removeJob ()));
            connect (job, SIGNAL (stopped ()), this, SLOT (jobStopHandler ()));
            connect (job, SIGNAL (enqueue ()), this, SLOT (enqueue ()));

            connect (job, SIGNAL (updateDisplays ()), this, SLOT (handleJobDisplay ()));
            connect (job, SIGNAL (stopped ()), this, SLOT (handleJobDisplay ()));
            connect (job, SIGNAL (started ()), this, SLOT (handleJobDisplay ()));
            connect (job, SIGNAL (addJob (JobParams*)), this, SLOT (addJob (JobParams*)));
            connect (job, SIGNAL (showError (QString, QString)), this, SIGNAL (showError (QString, QString)));
            connect (job, SIGNAL (gotFileSize ()), this, SLOT (handleJobDisplay ()));
        }
        endInsertRows ();
    }
    settings.endArray ();
    settings.endGroup ();
}

void JobManager::TryToStartScheduled ()
{
    for (int i = 0; i < ScheduledStarters_.size (); ++i)
    {
        QPair<int, QTime> pair = ScheduledStarters_.at (i);
        if (pair.second.msecsTo (QTime::currentTime ()) >= XmlSettingsManager::Instance ()->property ("RetryTimeout").toInt () && Start (pair.first))
            ScheduledStarters_.remove (i);
    }
}

uint JobManager::JobPosition (Job *job)
{
    QList<Job*>::const_iterator i = qFind (Jobs_, job);
    return i == Jobs_.end () ? -1 : i - Jobs_.begin ();
}

