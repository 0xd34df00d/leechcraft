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
#include "settingsmanager.h"
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

	IDPool_.resize (PoolSize_);
	for (unsigned int i = 0; i < PoolSize_; ++i)
		IDPool_ [i] = PoolSize_ - i;
}

JobManager::~JobManager ()
{
}

void JobManager::Release ()
{
	killTimer (QueryWaitingTimer_);
	saveSettings ();
	StopAll ();

	for (int i = 0; i < Jobs_.size (); ++i)
	{
		Jobs_ [i]->Release ();
		delete Jobs_ [i];
	}
	Jobs_.clear ();
}

void JobManager::DoDelayedInit ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup (qobject_cast<HttpPlugin*> (parent ())->GetName ());
	int size = settings.beginReadArray ("jobs");
	for (int i = 0; i < size; ++i)
	{
		QVariantList qvl;
		settings.setArrayIndex (i);
		JobParams *jp = new JobParams;
		jp->FeedVariantList (settings.value ("jobparamsrepresentation").toList ());
		addJob (jp);
	}
	settings.endArray ();
	settings.endGroup ();
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
	else if (SettingsManager::Instance ()->GetAutoGetFileSize ())
		GetFileSizeAt (id);

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
	JobRepresentation *jr = GetJobRepresentation (id);
	QString host = QUrl (jr->URL_).host ();
	delete jr;

	if (DownloadsPerHost_ [host] >= SettingsManager::Instance ()->GetMaxConcurrentPerServer ())
	{
		ScheduledJobsForHosts_.insert (host, id);
		emit jobWaiting (id);
		return false;
	}

	if (TotalDownloads_ >= SettingsManager::Instance ()->GetMaxTotalConcurrent ())
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
		DeleteAt (id);
	}
	return false;
}

void JobManager::Stop (unsigned int id)
{
	Jobs_ [ID2Pos_ [id]]->Stop ();
}

void JobManager::DeleteAt (unsigned int id)
{
	Jobs_ [ID2Pos_ [id]]->Release ();;
	delete Jobs_ [ID2Pos_ [id]];
	Jobs_.remove (ID2Pos_ [id]);
	QVector<Job*>::size_type pos = ID2Pos_ [id];
	ID2Pos_.remove (id);

	for (int i = 0; i < ID2Pos_.size (); ++i)
		if (ID2Pos_ [i] > pos)
			--(ID2Pos_ [i]);

	scheduleSave ();

	emit jobRemoved (id);
}

void JobManager::GetFileSizeAt (unsigned int id)
{
	Jobs_ [ID2Pos_ [id]]->GetFileSize ();
}

void JobManager::ScheduleAt (unsigned int id)
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

void JobManager::jobStopHandler (unsigned int id)
{
	JobRepresentation *jr = GetJobRepresentation (id);
	QString host = QUrl (jr->URL_).host ();
	delete jr;

	if (DownloadsPerHost_ [host])
		--DownloadsPerHost_ [host];
	if (TotalDownloads_ > 0)
		--TotalDownloads_;

	if (DownloadsPerHost_ [host] < SettingsManager::Instance ()->GetMaxConcurrentPerServer () &&
		ScheduledJobsForHosts_.contains (host) &&
		ScheduledJobsForHosts_.value (host))
	{
		Start (ScheduledJobsForHosts_.value (host));
		ScheduledJobsForHosts_.remove (host, ScheduledJobsForHosts_.value (host));
		return;
	}

	if (TotalDownloads_ < SettingsManager::Instance ()->GetMaxTotalConcurrent () &&
		!ScheduledJobs_.isEmpty ())
	{
		Start (ScheduledJobs_.pop ());
		return;
	}
}

void JobManager::enqueue (unsigned int id)
{
	Stop (id);
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
	settings.beginGroup (qobject_cast<HttpPlugin*> (parent ())->GetName ());
	settings.beginWriteArray ("jobs");
	for (int i = 0; i < Jobs_.size (); ++i)
	{
		JobRepresentation *jr = Jobs_ [i]->GetRepresentation ();
		QVariantList qvl = JobParams (*jr).ToVariantList ();
		delete jr;
		settings.setArrayIndex (i);
		settings.setValue ("jobparamsrepresentation", qvl);
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
		if (pair.second.msecsTo (QTime::currentTime ()) >= SettingsManager::Instance ()->GetRetryTimeout () && Start (pair.first))
			ScheduledStarters_.remove (i);
	}
}

void JobManager::RehashID2Pos ()
{
	for (int i = 0; i < Jobs_.size (); ++i)
		ID2Pos_ [Jobs_ [i]->GetID ()] = i;
}

