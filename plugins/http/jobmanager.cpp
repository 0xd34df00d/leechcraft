#include <QtDebug>
#include <QMessageBox>
#include <QVariantList>
#include <QSettings>
#include <QTimer>
#include <exceptions/io.h>
#include <exceptions/logic.h>
#include <plugininterface/proxy.h>
#include "httpplugin.h"
#include "jobmanager.h"
#include "job.h"
#include "jobparams.h"
#include "jobrepresentation.h"
#include "fileexistsdialog.h"
#include "finishedjob.h"

JobManager::JobManager (QObject *parent)
: QObject (parent)
, SaveChangesScheduled_ (false)
{
	IDPool_.resize (PoolSize_);
	for (unsigned int i = 0; i < PoolSize_; ++i)
		IDPool_ [i] = PoolSize_ - i;
}

JobManager::~JobManager ()
{
}

void JobManager::Release ()
{
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

int JobManager::addJob (JobParams *params)
{
	Job *job = new Job (params);

	Jobs_.append (job);
	unsigned int id = IDPool_.last ();
	IDPool_.pop_back ();

	job->SetID (id);	
	ID2Pos_ [id] = Jobs_.size () - 1;

	emit jobAdded (id);

	connect (job, SIGNAL (updateDisplays (unsigned int)), this, SLOT (handleJobDisplay (unsigned int)));
	connect (job, SIGNAL (finished (unsigned int)), this, SIGNAL (jobFinished (unsigned int)));
	connect (job, SIGNAL (deleteJob (unsigned int)), this, SIGNAL (deleteJob (unsigned int)));
	connect (job, SIGNAL (addJob (JobParams*)), this, SLOT (addJob (JobParams*)));
	connect (job, SIGNAL (showError (QString, QString)), this, SIGNAL (showError (QString, QString)));
	connect (job, SIGNAL (stopped (unsigned int)), this, SIGNAL (stopped (unsigned int)));

	job->DoDelayedInit ();

	if (params->Autostart_)
	{
		job->Start ();
		emit jobStarted (id);
	}

	if (!SaveChangesScheduled_)
	{
		SaveChangesScheduled_ = true;
		QTimer::singleShot (2000, this, SLOT (saveSettings ()));
	}

	return id;
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
		result += Jobs_ [i]->GetRepresentation ()->Speed_;
	}

	return result;
}

void JobManager::Start (unsigned int id)
{
	try
	{
		Jobs_ [ID2Pos_ [id]]->Start ();
	}
	catch (Exceptions::IO& e)
	{
		QMessageBox::critical (TheMain_, "Job IO error", e.GetReason ().c_str ());
	}
	catch (Exceptions::Logic& e)
	{
		QMessageBox::critical (TheMain_, "Job Logic error", e.GetReason ().c_str () + QString ("; anyway, deleting job, cause logic errors are very bad."));
		DeleteAt (id);
		return;
	}
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

	emit jobRemoved (id);
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

void JobManager::handleJobFinish (unsigned int id)
{
	if (Jobs_ [ID2Pos_ [id]]->GetErrorFlag ())
		QMessageBox::critical (TheMain_, "General job error ", Jobs_ [ID2Pos_ [id]]->GetErrorReason () + QString ("; anyway, keeping job in queue."));

	emit jobFinished (id);
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
		QVariantList qvl = (JobParams (*(Jobs_ [i]->GetRepresentation ()))).ToVariantList ();
		settings.setArrayIndex (i);
		settings.setValue ("jobparamsrepresentation", qvl);
	}
	settings.endArray ();
	settings.endGroup ();
}

void JobManager::RehashID2Pos ()
{
	for (int i = 0; i < Jobs_.size (); ++i)
		ID2Pos_ [Jobs_ [i]->GetID ()] = i;
}

