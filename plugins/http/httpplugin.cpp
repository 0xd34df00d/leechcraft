#include <QtGui>
#include <QtNetwork>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include <interfaces/interfaces.h>
#include <settingsdialog/settingsdialog.h>
#include "httpplugin.h"
#include "settingsmanager.h"
#include "jobadderdialog.h"
#include "jobparams.h"
#include "job.h"
#include "jobmanager.h"
#include "jobrepresentation.h"
#include "joblistitem.h"
#include "finishedjob.h"
#include "mainviewdelegate.h"
#include "contextablelist.h"

void HttpPlugin::Init ()
{
	qRegisterMetaType<ImpBase::length_t> ("ImpBase::length_t");
	Q_INIT_RESOURCE (resources);
    QTranslator *transl = new QTranslator (this);
    QString localeName = QLocale::system ().name ();
    transl->load (QString (":/leechcraft_http_") + localeName);
    qApp->installTranslator (transl);

	SaveChangesScheduled_ = false;
	ProvidesList_ << "http" << "ftp" << "resume";
	UsesList_ << "cron";

	JobManager_ = new JobManager (this);
	connect (JobManager_, SIGNAL (jobAdded (unsigned int)), this, SLOT (pushJob (unsigned int)));
	connect (JobManager_, SIGNAL (updateJobDisplay (unsigned int)), this, SLOT (updateJobDisplay (unsigned int)));
	connect (JobManager_, SIGNAL (jobRemoved (unsigned int)), this, SLOT (handleJobRemoval (unsigned int)));
	connect (JobManager_, SIGNAL (jobFinished (unsigned int)), this, SLOT (handleJobFinish (unsigned int)));
	connect (JobManager_, SIGNAL (jobStarted (unsigned int)), this, SLOT (handleJobStart (unsigned int)));
	connect (JobManager_, SIGNAL (deleteJob (unsigned int)), this, SLOT (handleJobDelete (unsigned int)));
	connect (JobManager_, SIGNAL (showError (QString, QString)), this, SLOT (showJobErrorMessage (QString, QString)));
	connect (JobManager_, SIGNAL (stopped (unsigned int)), this, SLOT (showStoppedIndicator (unsigned int)));
	connect (JobManager_, SIGNAL (jobWaiting (unsigned int)), this, SLOT (handleJobWaiting (unsigned int)));
	connect (JobManager_, SIGNAL (gotFileSize (unsigned int)), this, SLOT (handleGotFileSize (unsigned int)));
	connect (JobManager_, SIGNAL (cronEnabled ()), this, SLOT (handleCronEnabled ()));

	setWindowTitle (tr ("HTTP/FTP worker 0.2"));
	setWindowIcon (QIcon (":/resources/images/pluginicon.png"));

	SetupMainWidget ();

	FillInterface ();
	ReadSettings ();
	JobManager_->SetTheMain (TasksList_);
	JobManager_->DoDelayedInit ();

	IsShown_ = false;

	SettingsDialog_ = new SettingsDialog (this);
	SettingsDialog_->RegisterObject (SettingsManager::Instance ());

	QTimer *speedUpdTimer = new QTimer (this);
	speedUpdTimer->setInterval (1000);
	connect (speedUpdTimer, SIGNAL (timeout ()), this, SLOT (handleTotalSpeedUpdate ()));
	speedUpdTimer->start ();
	connect (FinishedList_, SIGNAL (itemSelectionChanged ()), this, SLOT (setActionsEnabled ()));
	connect (TasksList_, SIGNAL (itemSelectionChanged ()), this, SLOT (setActionsEnabled ()));
	setActionsEnabled ();
}

HttpPlugin::~HttpPlugin ()
{
}

void HttpPlugin::FillInterface ()
{
	QToolBar *toolstb = addToolBar (tr ("&Tools"));
	toolstb->setIconSize (QSize (16, 16));
	QMenuBar *menuBar = new QMenuBar ();
	setMenuBar (menuBar);
	QMenu *jobsMenu = menuBar->addMenu (tr ("&Jobs"));
	QMenu *toolsMenu = menuBar->addMenu (tr ("&Tools"));

	SetupJobManagementBar ();
	SetupJobManagementMenu (jobsMenu);
	SetupToolsBar (toolstb);
	SetupToolsMenu (toolsMenu);
	SetupStatusBarStuff ();
}

void HttpPlugin::SetupJobManagementBar ()
{
	AddJobAction_ = JobManagementToolbar_->addAction (QIcon (":/resources/images/addjob.png"), tr ("Add job..."), this, SLOT (initiateJobAddition ()));
	AddJobAction_->setShortcut (Qt::Key_Insert);
	DeleteJobAction_ = JobManagementToolbar_->addAction (QIcon (":/resources/images/deletejob.png"), tr ("Delete selected active job"), this, SLOT (deleteDownloadSelected ()));
	DeleteJobAction_->setShortcut (Qt::Key_Delete);
	JobManagementToolbar_->addSeparator ();
	StartJobAction_ = JobManagementToolbar_->addAction (QIcon (":/resources/images/startjob.png"), tr ("Start current"), this, SLOT (startDownloadSelected ()));
	StartJobAction_->setShortcut (tr ("Ctrl+S"));
	StartAllAction_ = JobManagementToolbar_->addAction (QIcon (":/resources/images/startall.png"), tr ("Start all"), this, SLOT (startDownloadAll ()));
	StartAllAction_->setShortcut (tr ("Ctrl+Shift+S"));
	StopJobAction_ = JobManagementToolbar_->addAction (QIcon (":/resources/images/stopjob.png"), tr ("Stop current"), this, SLOT (stopDownloadSelected ()));
	StopJobAction_->setShortcut (tr ("Ctrl+I"));
	StopAllAction_ = JobManagementToolbar_->addAction (QIcon (":/resources/images/stopall.png"), tr ("Stop all"), this, SLOT (stopDownloadAll ()));
	StopAllAction_->setShortcut (tr ("Ctrl+Shift+I"));
	GetFileSizeAction_ = JobManagementToolbar_->addAction (QIcon (":/resources/images/getfilesize.png"), tr ("Get file size"), this, SLOT (getFileSize ()));
	JobManagementToolbar_->addSeparator ();
	ScheduleSelectedAction_ = JobManagementToolbar_->addAction (QIcon (":/resources/images/schedule.png"), tr ("Schedule selected"), this, SLOT (scheduleSelected ()));
	DeleteFinishedAction_ = FinishedManagementToolbar_->addAction (QIcon (":/resources/images/deletejob.png"), tr ("Delete selected finished job"), this, SLOT (deleteDownloadSelectedFinished ()));
	DeleteFinishedAction_->setShortcut (Qt::Key_Delete & Qt::ShiftModifier);
}

void HttpPlugin::SetupJobManagementMenu (QMenu *jobsMenu)
{
	jobsMenu->addAction (AddJobAction_);
	jobsMenu->addAction (DeleteJobAction_);
	jobsMenu->addSeparator ();
	jobsMenu->addAction (DeleteFinishedAction_);
	CopyFinishedURL_ = jobsMenu->addAction (tr ("Copy URL to clipboard"), this, SLOT (copyFinishedURL ()));
	jobsMenu->addSeparator ();
	jobsMenu->addAction (StartJobAction_);
	jobsMenu->addAction (StartAllAction_);
	jobsMenu->addAction (StopJobAction_);
	jobsMenu->addAction (StopAllAction_);
	jobsMenu->addAction (GetFileSizeAction_);
	jobsMenu->addSeparator ();
	jobsMenu->addAction (ScheduleSelectedAction_);

	TasksList_->AddAction (DeleteJobAction_);
	TasksList_->AddAction (StartJobAction_);
	TasksList_->AddAction (StopJobAction_);
	TasksList_->AddAction (GetFileSizeAction_);

	FinishedList_->AddAction (DeleteFinishedAction_);
	FinishedList_->AddAction (CopyFinishedURL_);
}

void HttpPlugin::SetupToolsBar (QToolBar *toolstb)
{
	AutoAdjustInterfaceAction_ = toolstb->addAction (QIcon (":/resources/images/autoadjustiface.png"), tr ("Autoadjust interface"), this, SLOT (autoAdjustInterface ()));

	PreferencesAction_ = toolstb->addAction (QIcon (":/resources/images/preferences.png"), tr ("Preferences..."), this, SLOT (showPreferences ()));
	PreferencesAction_->setShortcut (tr ("Ctrl+P"));
}

void HttpPlugin::SetupToolsMenu (QMenu *toolsMenu)
{
	toolsMenu->addAction (AutoAdjustInterfaceAction_);
	toolsMenu->addAction (PreferencesAction_);
}

void HttpPlugin::SetupStatusBarStuff ()
{
	SpeedIndicator_ = new QLabel ("0");
	SpeedIndicator_->setMinimumWidth (70);
	SpeedIndicator_->setAlignment (Qt::AlignRight);
	statusBar ()->addPermanentWidget (SpeedIndicator_);
}

void HttpPlugin::SetupMainWidget ()
{
	Splitter_ = new QSplitter (Qt::Vertical, this);
	setCentralWidget (Splitter_);

	Splitter_->addWidget (SetupTasksPart ());
	Splitter_->addWidget (SetupFinishedPart ());
	Splitter_->show ();
}

QWidget* HttpPlugin::SetupTasksPart ()
{
	TasksList_ = new ContextableList (this);
	TasksList_->setItemDelegate (new MainViewDelegate (this));
	TasksList_->header ()->setClickable (false);
	TasksList_->setRootIsDecorated (false);
	TasksList_->setSelectionMode (QAbstractItemView::ExtendedSelection);
	TasksList_->setEditTriggers (QAbstractItemView::NoEditTriggers);
	QStringList taskHeaderLabels;
	taskHeaderLabels << tr ("ID") << tr ("Local name") << tr ("URL") << tr ("%") << tr ("Speed") << tr ("Downloaded size") << tr ("Total size");
	TasksList_->setHeaderLabels (taskHeaderLabels);
	TasksList_->header ()->setStretchLastSection (true);
	TasksList_->header ()->setHighlightSections (false);
	TasksList_->header ()->setDefaultAlignment (Qt::AlignLeft);

	JobManagementToolbar_ = new QToolBar;
	JobManagementToolbar_->setIconSize (QSize (16, 16));
	QWidget *container = new QWidget;
	QVBoxLayout *lay = new QVBoxLayout;
	container->setLayout (lay);
	lay->setSpacing (0);
	lay->addWidget (JobManagementToolbar_);
	lay->addWidget (TasksList_);

	return container;
}

QWidget* HttpPlugin::SetupFinishedPart ()
{
	FinishedList_ = new ContextableList (this);
	FinishedList_->setRootIsDecorated (false);
	FinishedList_->setSelectionMode (QAbstractItemView::SingleSelection);
	FinishedList_->setEditTriggers (QAbstractItemView::NoEditTriggers);
	QStringList finishedHeaderLabels;
	finishedHeaderLabels << tr ("Local name") << tr ("URL") << tr ("Size");
	FinishedList_->setHeaderLabels (finishedHeaderLabels);
	FinishedList_->header ()->setStretchLastSection (true);
	FinishedList_->header ()->setHighlightSections (false);
	FinishedList_->header ()->setDefaultAlignment (Qt::AlignLeft);

	FinishedManagementToolbar_ = new QToolBar;
	FinishedManagementToolbar_->setIconSize (QSize (16, 16));
	QWidget *container = new QWidget;
	QVBoxLayout *lay = new QVBoxLayout;
	container->setLayout (lay);
	lay->setSpacing (0);
	lay->addWidget (FinishedManagementToolbar_);
	lay->addWidget (FinishedList_);

	return container;
}

QString HttpPlugin::GetName () const
{
	return "HTTP & FTP";
}

QString HttpPlugin::GetInfo () const
{
	return tr ("Simple HTTP and FTP plugin, providing basic functionality.");
}

QString HttpPlugin::GetStatusbarMessage () const
{
	return "Yeah, that works!";
}

IInfo& HttpPlugin::SetID (IInfo::ID_t id)
{
	ID_ = id;
	return *this;
}

IInfo::ID_t HttpPlugin::GetID () const
{
	return ID_;
}

QStringList HttpPlugin::Provides () const
{
	return ProvidesList_;
}

QStringList HttpPlugin::Needs () const
{
	return NeedsList_;
}

QStringList HttpPlugin::Uses () const
{
	return UsesList_;
}

void HttpPlugin::SetProvider (QObject *object, const QString& feature)
{
	JobManager_->SetProvider (object, feature);
}

void HttpPlugin::Release ()
{
	JobManager_->Release ();
	writeSettings ();
	SettingsManager::Instance ()->Release ();

	delete JobManager_;
	JobManager_ = 0;
}

QIcon HttpPlugin::GetIcon () const
{
	return QIcon (":/resources/images/pluginicon.png");
}

void HttpPlugin::SetParent (QWidget *parent)
{
	setParent (parent);
}

void HttpPlugin::ShowWindow ()
{
	IsShown_ = 1 - IsShown_;
	IsShown_ ? show () : hide ();
}

void HttpPlugin::ShowBalloonTip ()
{
}

void HttpPlugin::StartAll ()
{
	JobManager_->StartAll ();
}

void HttpPlugin::StopAll ()
{
	JobManager_->StopAll ();
}

void HttpPlugin::StartAt (IDownload::JobID_t id)
{
	JobManager_->Start (id);
}

void HttpPlugin::StopAt (IDownload::JobID_t id)
{
	JobManager_->Stop (id);
}

void HttpPlugin::DeleteAt (IDownload::JobID_t id)
{
	JobManager_->DeleteAt (id);
}

void HttpPlugin::GetFileSizeAt (IDownload::JobID_t id)
{
	JobManager_->GetFileSizeAt (id);
}

uint HttpPlugin::GetVersion () const
{
	return QDateTime (QDate (2007, 11, 30), QTime (10, 11)).toTime_t ();
}

int HttpPlugin::GetPercentageForRow (int row)
{
	JobRepresentation *jr = JobManager_->GetJobRepresentation (dynamic_cast<JobListItem*> (TasksList_->topLevelItem (row))->GetID ());
	int result = jr->Size_ ? jr->Downloaded_ * 100 / jr->Size_ : 0;
	delete jr;
	return result;
}

qint64 HttpPlugin::GetDownloadSpeed () const
{
	return JobManager_->GetDownloadSpeed ();
}

qint64 HttpPlugin::GetUploadSpeed () const
{
	return 0;
}

void HttpPlugin::addDownload (const DirectDownloadParams& params)
{
	JobParams *jp = new JobParams;
	jp->IsFullName_				= true;
	jp->URL_					= params.Resource_;
	jp->LocalName_				= params.Location_;
	jp->Autostart_				= params.Autostart_;
	jp->ShouldBeSavedInHistory_	= params.ShouldBeSavedInHistory_;
	emit jobAdded (handleParams (jp));
}

void HttpPlugin::initiateJobAddition ()
{
	JobAdderDialog *dia = new JobAdderDialog ();
	connect (dia, SIGNAL (gotParams (JobParams*)), this, SLOT (handleParams (JobParams*)));
	dia->exec ();
	delete dia;
}

int HttpPlugin::handleParams (JobParams *params)
{
	return JobManager_->addJob (params);
}

void HttpPlugin::pushJob (unsigned int id)
{
	JobRepresentation *jr = JobManager_->GetJobRepresentation (id);
	JobListItem *item = new JobListItem;
	item->SetID (jr->ID_);
	item->setIcon (TListID,			QIcon (":/resources/images/stopjob.png"));
	item->setText (TListLocalName,	QFileInfo (jr->LocalName_).fileName ());
	item->setText (TListURL,		jr->URL_);
	item->setText (TListPercent,	"0");
	item->setText (TListSpeed,		"0.0");
	item->setText (TListDownloaded,	"");
	item->setText (TListTotal,		"");
	TasksList_->addTopLevelItem (item);
	delete jr;

	setActionsEnabled ();
}

void HttpPlugin::updateJobDisplay (unsigned int id)
{
	JobRepresentation *jr = JobManager_->GetJobRepresentation (id);

	int rowCount = TasksList_->topLevelItemCount ();
	for (int i = 0; i < rowCount; ++i)
		if (dynamic_cast<JobListItem*> (TasksList_->topLevelItem (i))->GetID () == id)
		{
			QTreeWidgetItem *item = TasksList_->topLevelItem (i);
			item->setText (TListLocalName, QFileInfo (jr->LocalName_).fileName ());
			if (jr->Size_)
				item->setText (TListPercent, QString::number (jr->Downloaded_ * 100 / jr->Size_));
			else
				item->setText (TListPercent, QString (tr ("0")));
			item->setText (TListSpeed, Proxy::Instance ()->MakePrettySize (jr->Speed_) + tr ("/s"));
			item->setText (TListDownloaded, Proxy::Instance ()->MakePrettySize (jr->Downloaded_));
			item->setText (TListTotal, Proxy::Instance ()->MakePrettySize (jr->Size_));
		}

	delete jr;
}

void HttpPlugin::handleGotFileSize (unsigned int id)
{
	JobRepresentation *jr = JobManager_->GetJobRepresentation (id);

	int rowCount = TasksList_->topLevelItemCount ();
	for (int i = 0; i < rowCount; ++i)
		if (dynamic_cast<JobListItem*> (TasksList_->topLevelItem (i))->GetID () == id)
		{
			QTreeWidgetItem *item = TasksList_->topLevelItem (i);
			item->setText (TListPercent, QString::number (jr->Downloaded_ * 100 / jr->Size_));
			item->setText (TListDownloaded, Proxy::Instance ()->MakePrettySize (jr->Downloaded_));
			item->setText (TListTotal, Proxy::Instance ()->MakePrettySize (jr->Size_));
		}

	delete jr;
}

void HttpPlugin::handleJobRemoval (unsigned int id)
{
	int rowCount = TasksList_->topLevelItemCount ();
	for (int i = 0; i < rowCount; ++i)
		if (dynamic_cast<JobListItem*> (TasksList_->topLevelItem (i))->GetID () == id)
		{
			delete TasksList_->takeTopLevelItem (i);
			setActionsEnabled ();
			return;
		}
}

void HttpPlugin::handleJobFinish (unsigned int id)
{
	emit jobFinished (id);

	JobRepresentation *jr = JobManager_->GetJobRepresentation (id);
	JobManager_->DeleteAt (id);

	if (jr->ShouldBeSavedInHistory_)
	{
		FinishedJob fj (*jr);
		AddToFinishedList (&fj);
	}

	delete jr;

	if (!SaveChangesScheduled_)
	{
		SaveChangesScheduled_ = true;
		QTimer::singleShot (100, this, SLOT (writeSettings ()));
	}
}

void HttpPlugin::handleJobStart (unsigned int id)
{
	int rowCount = TasksList_->topLevelItemCount ();
	for (int i = 0; i < rowCount; ++i)
		if (dynamic_cast<JobListItem*> (TasksList_->topLevelItem (i))->GetID () == id)
			dynamic_cast<JobListItem*> (TasksList_->topLevelItem (i))->setIcon (TListID, QIcon (":/resources/images/startjob.png"));
}

void HttpPlugin::handleJobDelete (unsigned int id)
{
	JobManager_->DeleteAt (id);
}

void HttpPlugin::handleJobWaiting (unsigned int id)
{
	int rowCount = TasksList_->topLevelItemCount ();
	for (int i = 0; i < rowCount; ++i)
		if (dynamic_cast<JobListItem*> (TasksList_->topLevelItem (i))->GetID () == id)
			dynamic_cast<JobListItem*> (TasksList_->topLevelItem (i))->setIcon (TListID, QIcon (":/resources/images/waiting.png"));
}

void HttpPlugin::startDownloadSelected ()
{
	HandleSelected (JAStart);
}

void HttpPlugin::stopDownloadSelected ()
{
	HandleSelected (JAStop);
}

void HttpPlugin::deleteDownloadSelected ()
{
	HandleSelected (JADelete);
}

void HttpPlugin::deleteDownloadSelectedFinished ()
{
	QList<QTreeWidgetItem*> items = FinishedList_->selectedItems ();

	for (int i = 0; i < items.size (); ++i)
		delete items [i];

	setActionsEnabled ();
}

void HttpPlugin::getFileSize ()
{
	HandleSelected (JAGFS);
}

void HttpPlugin::startDownloadAll ()
{
	StartAll ();
}

void HttpPlugin::stopDownloadAll ()
{
	StopAll ();
}

void HttpPlugin::showPreferences ()
{
	SettingsDialog_->show ();
	SettingsDialog_->setWindowTitle (windowTitle () + tr (": Preferences"));
}

void HttpPlugin::showJobErrorMessage (QString url, QString message)
{
	QMessageBox::warning (this, tr ("Job error"), tr ("Job with URL %1 signals about following error:<br /><code>%2</code>").arg (url).arg (message));
}

void HttpPlugin::showStoppedIndicator (unsigned int id)
{
	for (int i = 0; i < TasksList_->topLevelItemCount (); ++i)
	{
		JobListItem *item = dynamic_cast<JobListItem*> (TasksList_->topLevelItem (i));
		if (item->GetID () == id)
			item->setIcon (TListID, QIcon (":/resources/images/stopjob.png"));
	}
}

void HttpPlugin::handleTotalSpeedUpdate ()
{
	SpeedIndicator_->setText (Proxy::Instance ()->MakePrettySize (GetDownloadSpeed ()) + tr ("/s"));
}

void HttpPlugin::autoAdjustInterface ()
{
	if (TasksList_->topLevelItemCount ())
		for (int i = 0; i < TasksList_->columnCount (); ++i)
			TasksList_->resizeColumnToContents (i);
	if (FinishedList_->topLevelItemCount ())
		for (int i = 0; i < FinishedList_->columnCount (); ++i)
			FinishedList_->resizeColumnToContents (i);
}

void HttpPlugin::writeSettings ()
{
	SaveChangesScheduled_ = false;

	QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup (GetName ());
	settings.beginGroup ("geometry");
	settings.setValue ("size", size ());
	settings.setValue ("pos", pos ());
	settings.setValue ("jobListHeadersState", TasksList_->header ()->saveState ());
	settings.setValue ("finishedListHeadersState", FinishedList_->header ()->saveState ());
	settings.setValue ("splitterState", Splitter_->saveState ());
	settings.endGroup ();

	settings.beginWriteArray ("finished");
	for (int i = 0; i < FinishedList_->topLevelItemCount (); ++i)
	{
		settings.setArrayIndex (i);
		QByteArray arr;
		QDataStream str (&arr, QIODevice::WriteOnly);
		FinishedList_->topLevelItem (i)->write (str);
		settings.setValue ("representation", arr);
	}
	settings.endArray ();
	settings.endGroup ();
}

void HttpPlugin::copyFinishedURL ()
{
	QList<QTreeWidgetItem*> item = FinishedList_->selectedItems ();
	QApplication::clipboard ()->setText (item.at (0)->text (FListURL));
}

void HttpPlugin::setActionsEnabled ()
{
	QList<QTreeWidgetItem*> finishedItems = FinishedList_->selectedItems (),
							tasksItems = TasksList_->selectedItems ();
	DeleteFinishedAction_->setEnabled (finishedItems.size ());
	CopyFinishedURL_->setEnabled (finishedItems.size ());

	DeleteJobAction_->setEnabled (tasksItems.size ());
	StartJobAction_->setEnabled (tasksItems.size ());
	StopJobAction_->setEnabled (tasksItems.size ());
	GetFileSizeAction_->setEnabled (tasksItems.size ());

	StartAllAction_->setEnabled (TasksList_->topLevelItemCount ());
	StopAllAction_->setEnabled (TasksList_->topLevelItemCount ());
}

void HttpPlugin::handleCronEnabled ()
{
}

void HttpPlugin::ReadSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup (GetName ());
	settings.beginGroup ("geometry");
	resize (settings.value ("size", QSize (640, 480)).toSize ());
	move   (settings.value ("pos",  QPoint (10, 10)).toPoint ());

	QByteArray splitterArr = settings.value ("splitterState").toByteArray ();
	if (!splitterArr.isEmpty () && !splitterArr.isNull ())
		Splitter_->restoreState (splitterArr);

	QByteArray jobListArr = settings.value ("jobListHeadersState").toByteArray ();
	if (!jobListArr.isEmpty () && !jobListArr.isNull ())
		TasksList_->header ()->restoreState (jobListArr);
	else if (TasksList_->topLevelItemCount ())
	{
		for (int i = 0; i < TasksList_->columnCount (); ++i)
			TasksList_->resizeColumnToContents (i);
	}

	QByteArray finishedListArr = settings.value ("finishedListHeadersState").toByteArray ();
	if (!finishedListArr.isEmpty () && !finishedListArr.isNull ())
		FinishedList_->header ()->restoreState (finishedListArr);
	else if (FinishedList_->topLevelItemCount ())
	{
		for (int i = 0; i < TasksList_->columnCount (); ++i)
			FinishedList_->resizeColumnToContents (i);
	}

	settings.endGroup ();

	int size = settings.beginReadArray ("finished");
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex (i);
		QTreeWidgetItem item;
		QDataStream str (settings.value ("representation").toByteArray ());
		item.read (str);
		FinishedList_->addTopLevelItem (new QTreeWidgetItem (item));
	}
	setActionsEnabled ();
	settings.endArray ();
	settings.endGroup ();
}

void HttpPlugin::AddToFinishedList (const FinishedJob *fj)
{
	QTreeWidgetItem *item = new QTreeWidgetItem;
	item->setText (FListLocalName, QFileInfo (fj->GetLocal ()).fileName ());
	item->setText (FListURL, fj->GetURL ());
	item->setText (FListSize, Proxy::Instance ()->MakePrettySize (fj->GetSize ()));

	FinishedList_->addTopLevelItem (item);

	setActionsEnabled ();
}

void HttpPlugin::HandleSelected (JobAction ja)
{
	if (!TasksList_->topLevelItemCount ())
		return;
	QList<QTreeWidgetItem*> items = TasksList_->selectedItems ();

	for (int i = 0; i < items.size (); ++i)
	{
		JobListItem *item = dynamic_cast<JobListItem*> (items [i]);
		switch (ja)
		{
			case JAStart:
				StartAt (item->GetID ());
				item->setIcon (TListID, QIcon (":/resources/images/startjob.png"));
				break;
			case JAStop:
				StopAt (item->GetID ());
				item->setIcon (TListID, QIcon (":/resources/images/stopjob.png"));
				break;
			case JADelete:
				DeleteAt (item->GetID ());
				break;
			case JAGFS:
				GetFileSizeAt (item->GetID ());
				break;
		}
	}
}

void HttpPlugin::closeEvent (QCloseEvent*)
{
	IsShown_ = false;
}

Q_EXPORT_PLUGIN2 (leechcraft_http, HttpPlugin);

