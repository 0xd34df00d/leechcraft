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
: QObject (parent)
, TotalDownloads_ (0)
, SaveChangesScheduled_ (false)
, CronEnabled_ (false)
{
    QueryWaitingTimer_ = startTimer (500);
    Headers_ << tr ("State") << tr ("Local name") << tr ("URL") << tr ("Progress") << tr ("Speed") << tr ("ETA") << tr ("Download time") << tr ("Ready") << tr ("Total");
}

JobManager::~JobManager ()
{
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
    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
    settings.beginGroup ("HTTP and FTP");
    int size = settings.beginReadArray ("Jobs");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex (i);
        Job *job = new Job;
        job->Unserialize (settings.value ("job").toByteArray ());
        Jobs_.append (job);
    }
    settings.endArray ();
    settings.endGroup ();
}

int JobManager::columnCount (const QModelIndex&) const
{
    return Headers_.size ();
}

QVariant JobManager::data (const QModelIndex& index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant ();
    int row = index.row (),
        column = index.column ();

    Job *job = Jobs_.at (row);

    switch (static_cast<TasksListHeaders> (column))
    {
        case TListState:
            return QIcon ();
        case TListLocalName:
            return job->GetLocalName ();
        case TListURL:
            return job->GetURL ();
        case TListPercent:
            return QString::number (job->GetDownloaded () / job->GetTotal ());
        case TListSpeed:
            return job->GetSpeed ();
        case TListDownloadTime:
            return job->GetDownloadTime ();
        case TListRemainingTime:
            return job->GetAverageTime ();
        case TListDownloaded:
            return job->GetDownloaded ();
        case TListTotal:
            return job->GetTotal ();
    }
}

Qt::ItemFlags JobManager::flags (const QModelIndex& index) const
{
    return Qt::ItemIsSelectible | Qt::ItemIsEnabled;
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
    if (!hasIndex (r, c))
        return QModelIndex ();
    return createIndex (r, c);
}

QModelIndex JobManager::parent (const QModelIndex&) const
{
    return QModelIndex ();
}

int JobManager::rowCount (const QModelIndex& parent) const
{
    if (index.isValid ())
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
int JobManager::addJob (JobParams *params)
{
    Job *job = new Job (params, this);

    Jobs_.append (job);
    unsigned int id = IDPool_.last ();
    IDPool_.pop_back ();

    job->SetID (id);    
    ID2Pos_ [id] = Jobs_.size () - 1;

    emit jobAdded (id);

    connect (job, SIGNAL (finished (unsigned int)), this, SLOT (jobStopHandler (unsigned int)));
    connect (job, SIGNAL (deleteJob (unsigned int)), this, SLOT (jobStopHandler (unsigned int)));
    connect (job, SIGNAL (stopped (unsigned int)), this, SLOT (jobStopHandler (unsigned int)));
    connect (job, SIGNAL (enqueue (unsigned int)), this, SLOT (enqueue (unsigned int)));

    connect (job, SIGNAL (updateDisplays (unsigned int)), this, SLOT (handleJobDisplay (unsigned int)));
    connect (job, SIGNAL (finished (unsigned int)), this, SIGNAL (jobFinished (unsigned int)));
    connect (job, SIGNAL (started (unsigned int)), this, SIGNAL (jobStarted (unsigned int)));
    connect (job, SIGNAL (deleteJob (unsigned int)), this, SIGNAL (deleteJob (unsigned int)));
    connect (job, SIGNAL (addJob (JobParams*)), this, SLOT (addJob (JobParams*)));
    connect (job, SIGNAL (showError (QString, QString)), this, SIGNAL (showError (QString, QString)));
    connect (job, SIGNAL (stopped (unsigned int)), this, SIGNAL (stopped (unsigned int)));
    connect (job, SIGNAL (gotFileSize (unsigned int)), this, SIGNAL (gotFileSize (unsigned int)));

    job->DoDelayedInit ();

    QString host = QUrl (params->URL_).host ();
    if (!DownloadsPerHost_.contains (host))
        DownloadsPerHost_ [host ] = 0;

    if (params->Autostart_)
        Start (id);
    else if (XmlSettingsManager::Instance ()->property ("AutoGetFileSize").toBool ())
        GetFileSize (id);

    scheduleSave ();

    return id;
}

void JobManager::timerEvent (QTimerEvent *e)
{
    if (QueryWaitingTimer_ == e->timerId ())
        TryToStartScheduled ();
}

JobRepresentation* JobManager::GetJobRepresentation (unsigned int id) const
{
    return Jobs_ [ID2Pos_ [id]]->GetRepresentation ();
}

qint64 JobManager::GetDownloadSpeed () const
{
    qint64 result = 0;
    for (int i = 0; i < Jobs_.size (); ++i)
    {
        JobRepresentation *jr = Jobs_ [i]->GetRepresentation ();
        result += jr->Speed_;
        delete jr;
    }

    return result;
}

bool JobManager::Start (unsigned int id)
{
    if (Jobs_ [ID2Pos_ [id]]->GetState () != Job::StateIdle)
        return false;

    JobRepresentation *jr = GetJobRepresentation (id);
    QString host = QUrl (jr->URL_).host ();
    delete jr;

    if (DownloadsPerHost_ [host] >= XmlSettingsManager::Instance ()->property ("MaxConcurrentPerServer").toInt ())
    {
        ScheduledJobsForHosts_.insert (host, id);
        emit jobWaiting (id);
        return false;
    }

    if (TotalDownloads_ >= XmlSettingsManager::Instance ()->property ("MaxTotalConcurrent").toInt ())
    {
        ScheduledJobs_.push_back (id);
        emit jobWaiting (id);
        return false;
    }

    try
    {
        Jobs_ [ID2Pos_ [id]]->Start ();
        ++DownloadsPerHost_ [host];
        ++TotalDownloads_;
        emit jobStarted (id);
        return true;
    }
    catch (Exceptions::IO& e)
    {
        QMessageBox::critical (TheMain_, "Job IO error", e.GetReason ().c_str ());
    }
    catch (Exceptions::Logic& e)
    {
        QMessageBox::critical (TheMain_, "Job Logic error", e.GetReason ().c_str () + QString ("; anyway, deleting job, cause logic errors are very bad."));
        Delete (id);
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
    if (Jobs_ [ID2Pos_ [id]]->GetState () == Job::StateDownloading)
    {
        Jobs_ [ID2Pos_ [id]]->Stop ();
        jobStopHandler (id);
    }
    else if (Jobs_ [ID2Pos_ [id]]->GetState () == Job::StateWaiting)
        Jobs_ [ID2Pos_ [id]]->Stop ();
}

void JobManager::Delete (unsigned int id)
{
    disconnect (Jobs_ [ID2Pos_ [id]], 0, 0, 0);
    Jobs_ [ID2Pos_ [id]]->Release ();;
    delete Jobs_ [ID2Pos_ [id]];
    Jobs_.remove (ID2Pos_ [id]);
    QVector<Job*>::size_type pos = ID2Pos_ [id];
    ID2Pos_.remove (id);

    for (int i = 0; i < ID2Pos_.size (); ++i)
        if (ID2Pos_ [i] > pos)
            --(ID2Pos_ [i]);

    scheduleSave ();

    for (MultiHostDict_t::Iterator i = ScheduledJobsForHosts_.begin (); i != ScheduledJobsForHosts_.end (); ++i)
        if (i.value () == static_cast<int> (id))
            i = ScheduledJobsForHosts_.erase (i);

    for (int i = 0; i < ScheduledJobs_.size (); ++i)
        if (ScheduledJobs_ [i] == static_cast<int> (id))
            ScheduledJobs_.remove (i--);

    emit jobRemoved (id);
}

void JobManager::GetFileSize (unsigned int id)
{
    Jobs_ [ID2Pos_ [id]]->GetFileSize ();
}

void JobManager::Schedule (unsigned int id)
{
    emit jobWaiting (id);
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
    Job *job = Jobs_ [ID2Pos_ [id]];
    if (job->GetState () == Job::StateIdle)
        job->UpdateParams (params);
}

void JobManager::jobStopHandler (unsigned int id)
{
    JobRepresentation *jr = GetJobRepresentation (id);
    QString host = QUrl (jr->URL_).host ();
    delete jr;

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

void JobManager::enqueue (unsigned int id)
{
    JobRepresentation *jr = GetJobRepresentation (id);
    QString host = QUrl (jr->URL_).host ();
    delete jr;

    Job *job = Jobs_ [ID2Pos_ [id]];
    connect (job, SIGNAL (stopped (unsigned int)), this, SIGNAL (stopped (unsigned int)));
    connect (job, SIGNAL (stopped (unsigned int)), this, SLOT (jobStopHandler (unsigned int)));
    ScheduledJobsForHosts_.insert (host, id);
    if (DownloadsPerHost_ [host] > 0)
        --DownloadsPerHost_ [host];
    if (TotalDownloads_ > 0)
        --TotalDownloads_;
    emit jobWaiting (id);
}

void JobManager::handleJobDisplay (unsigned int id)
{
    emit updateJobDisplay (id);
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

void JobManager::TryToStartScheduled ()
{
    for (int i = 0; i < ScheduledStarters_.size (); ++i)
    {
        QPair<int, QTime> pair = ScheduledStarters_.at (i);
        if (pair.second.msecsTo (QTime::currentTime ()) >= XmlSettingsManager::Instance ()->property ("RetryTimeout").toInt () && Start (pair.first))
            ScheduledStarters_.remove (i);
    }
}

void JobManager::RehashID2Pos ()
{
    for (int i = 0; i < Jobs_.size (); ++i)
        ID2Pos_ [Jobs_ [i]->GetID ()] = i;
}

