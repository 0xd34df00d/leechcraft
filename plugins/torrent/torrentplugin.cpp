#include <QtGui>
#include <settingsdialog/settingsdialog.h>
#include <plugininterface/proxy.h>
#include "torrentplugin.h"
#include "torrentmanager.h"
#include "addtorrentdialog.h"
#include "globals.h"
#include "settingsmanager.h"
#include "ratecontroller.h"
#include "torrentviewdelegate.h"
#include "torrentclient.h"

void TorrentPlugin::Init ()
{
	Q_INIT_RESOURCE (resources);
	QTranslator *transl = new QTranslator (this);
	QString localeName = QLocale::system ().name ();
	transl->load (QString (":/leechcraft_torrent_") + localeName);
	qApp->installTranslator (transl);

	IsShown_ = false;
	SaveChanges_ = false;

	QuitDialog_ = 0;

	setWindowTitle ("BitTorrent client");
	setWindowIcon (QIcon (":/resources/images/bittorrent.png"));

	FillInterface ();
	ReadSettings ();

	connect (Downloading_, SIGNAL (itemSelectionChanged ()), this, SLOT (setActionsEnabled ()));
	connect (UploadLimitSlider_, SIGNAL (valueChanged (int)), this, SLOT (setUploadLimit (int)));
	connect (DownloadLimitSlider_, SIGNAL (valueChanged (int)), this, SLOT (setDownloadLimit (int)));

	SettingsDialog_ = new SettingsDialog (this);
	SettingsDialog_->RegisterObject (SettingsManager::Instance ());

	setActionsEnabled ();
}

TorrentPlugin::~TorrentPlugin ()
{
}

void TorrentPlugin::FillInterface ()
{
	AddTorrentDialog_ = new AddTorrentDialog ();

	SetupMainWidget ();
	SetupToolbars ();
	SetupActions ();
}

void TorrentPlugin::SetupMainWidget ()
{
	setCentralWidget (SetupDownloading ());
}

void TorrentPlugin::SetupToolbars ()
{
	ManagementToolbar_ = addToolBar (tr ("Main toolbar"));
	ToolsToolbar_ = addToolBar (tr ("Tools"));
	BottomToolbar_ = new QToolBar (tr ("Rate control"));
	addToolBar (Qt::BottomToolBarArea, BottomToolbar_);
	BottomToolbar_->setMovable (false);
}

QWidget* TorrentPlugin::SetupDownloading ()
{
	Downloading_ = new QTreeWidget ();
	QStringList dLabels;
	dLabels << tr ("Name") << tr ("Peers/seeds") << tr ("Progress") << tr ("Downloaded") << tr ("Uploaded") << tr ("Status");
	Downloading_->setHeaderLabels (dLabels);
	Downloading_->setItemDelegate (new TorrentViewDelegate (this));
	Downloading_->setSelectionBehavior (QAbstractItemView::SelectRows);
	Downloading_->setRootIsDecorated (false);

	QFontMetrics fm = fontMetrics ();
	QHeaderView *header = Downloading_->header ();
	header->resizeSection (CName, fm.width ("whattorrentsdoyouusuallyseehere?.torrent"));
	header->resizeSection (CPeersSeeds, fm.width (dLabels [CPeersSeeds] + QString ("  ")));
	header->resizeSection (CProgress, fm.width (dLabels [CProgress] + QString ("  ")));
	header->resizeSection (CDownloading, qMax (fm.width (dLabels [CDownloading] + QString ("  ")), fm.width ("1234.0 KB/s")));
	header->resizeSection (CUploading, qMax (fm.width (dLabels [CUploading] + QString ("  ")), fm.width ("1234.0 KB/s")));
	header->resizeSection (CStatus, qMax (fm.width (dLabels [CStatus] + QString ("  ")), fm.width (tr ("Downloading") + "  ")));

	return Downloading_;
}

void TorrentPlugin::SetupActions ()
{
	PreferencesAction_ = ToolsToolbar_->addAction (QIcon (":/resources/images/preferences.png"), tr ("Preferences..."), this, SLOT (showPreferences ()));
	PreferencesAction_->setShortcut (tr ("Ctrl+Shift+P"));

	AddAction_ = ManagementToolbar_->addAction (QIcon (":/resources/images/addjob.png"), tr ("Add job..."), this, SLOT (addTorrent ()));
	AddAction_->setShortcut (Qt::Key_Insert);
	DeleteAction_ = ManagementToolbar_->addAction (QIcon (":/resources/images/deletejob.png"), tr ("Delete job"), this, SLOT (removeTorrent ()));
	DeleteAction_->setShortcut (Qt::Key_Delete);

	ManagementToolbar_->addSeparator ();

	PauseAction_ = ManagementToolbar_->addAction (QIcon (":/resources/images/stopjob.png"), tr ("Stop job"), this, SLOT (pauseTorrent ()));
	PauseAction_->setShortcut (tr ("Ctrl+P"));

	ManagementToolbar_->addSeparator ();

	MoveUpAction_ = ManagementToolbar_->addAction (QIcon (":/resources/images/moveup.png"), tr ("Move up"), this, SLOT (moveUp ()));
	MoveDownAction_ = ManagementToolbar_->addAction (QIcon (":/resources/images/movedown.png"), tr ("Move down"), this, SLOT (moveDown ()));

	DownloadLimitSlider_ = new QSlider (Qt::Horizontal);
	DownloadLimitSlider_->setRange (0, 1000);
	BottomToolbar_->addWidget (new QLabel (tr ("Max download:")));
	BottomToolbar_->addWidget (DownloadLimitSlider_);
	BottomToolbar_->addWidget (DownloadLimitLabel_ = new QLabel (tr ("0 kb/s")));
	DownloadLimitLabel_->setFixedSize (QSize (fontMetrics ().width (tr ("9999 kb/s")), fontMetrics ().lineSpacing ()));
	DownloadLimitLabel_->setAlignment (Qt::AlignRight);
	BottomToolbar_->addSeparator ();
	UploadLimitSlider_ = new QSlider (Qt::Horizontal);
	UploadLimitSlider_->setRange (0, 1000);
	BottomToolbar_->addWidget (new QLabel (tr ("Max upload:")));
	BottomToolbar_->addWidget (UploadLimitSlider_);
	BottomToolbar_->addWidget (UploadLimitLabel_ = new QLabel (tr ("0 kb/s")));
	UploadLimitLabel_->setFixedSize (QSize (fontMetrics ().width (tr ("9999 kb/s")), fontMetrics ().lineSpacing ()));
	UploadLimitLabel_->setAlignment (Qt::AlignRight);

	SetupMenus ();
}

void TorrentPlugin::SetupMenus ()
{
	QMenu *jobs = menuBar ()->addMenu (tr ("&Jobs"));

	jobs->addAction (AddAction_);
	jobs->addAction (DeleteAction_);
	jobs->addSeparator ();
	jobs->addAction (PauseAction_);
	jobs->addSeparator ();
	jobs->addAction (MoveUpAction_);
	jobs->addAction (MoveDownAction_);

	QMenu *tools = menuBar ()->addMenu (tr ("&Tools"));
	tools->addAction (PreferencesAction_);
}


bool TorrentPlugin::addTorrent (const QString& filename, const QString& dest, const QByteArray& resume)
{
	foreach (Job job, Jobs_)
		if (job.Filename_ == filename && job.DestDir_ == dest)
		{
			QMessageBox::warning (this, tr ("Already in list"), tr ("This torrent is already in job list."));
			return false;
		}

	TorrentClient *tc = new TorrentClient (this);
	if (!tc->SetTorrent (filename))
	{
		QMessageBox::warning (this, tr ("Error"), tr ("Selected torrent could not be opened/parsed."));
		delete tc;
		return false;
	}

	tc->SetDestinationFolder (dest);
	tc->SetDumpedState (resume);

	connect (tc, SIGNAL (stateChanged (TorrentClient::State)), this, SLOT (updateState (TorrentClient::State)));
	connect (tc, SIGNAL (peerInfoUpdated ()), this, SLOT (updatePeerInfo ()));
	connect (tc, SIGNAL (progressUpdated (int)), this, SLOT (updateProgress (int)));
	connect (tc, SIGNAL (downloadRateUpdated (int)), this, SLOT (updateDownloadRate (int)));
	connect (tc, SIGNAL (uploadRateUpdated (int)), this, SLOT (updateUploadRate (int)));
	connect (tc, SIGNAL (stopped ()), this, SLOT (torrentStopped ()));
	connect (tc, SIGNAL (error (TorrentClient::Error)), this, SLOT (torrentError (TorrentClient::Error)));

	Job job;
	job.TC_ = tc;
	job.Filename_ = filename;
	job.DestDir_ = dest;
	Jobs_ << job;

	QTreeWidgetItem *item = new QTreeWidgetItem (Downloading_);
	QString basefilename = QFileInfo (filename).fileName ();
	if (basefilename.toLower ().endsWith (".torrent"))
		basefilename.remove (basefilename.size () - 8);

	item->setText (CName, basefilename);
	item->setToolTip (CName, tr ("Torrent: %1<br>Destination: %2<br>").arg (basefilename).arg (dest));
	item->setText (CPeersSeeds, "0/0");
	item->setText (CProgress, "0");
	item->setText (CDownloading, "0.0 KB/s");
	item->setText (CUploading, "0.0 KB/s");
	item->setFlags (item->flags () & ~Qt::ItemIsEditable);
	item->setTextAlignment (CPeersSeeds, Qt::AlignHCenter);

	if (!SaveChanges_)
	{
		SaveChanges_ = true;
		QTimer::singleShot (1000, this, SLOT (SaveSettings ()));
	}
	tc->start ();
	return true;
}

bool TorrentPlugin::addTorrent ()
{
	AddTorrentDialog_->Reinit (LastOpenDir_, LastSaveDir_);
	if (!AddTorrentDialog_->exec ())
		return false;

	QString filename = AddTorrentDialog_->GetFilename (),
			destDir = AddTorrentDialog_->GetDestDir ();

	LastOpenDir_ = QFileInfo (filename).absolutePath ();
	LastSaveDir_ = QFileInfo (destDir).absolutePath ();

	if (!addTorrent (filename, destDir))
		return false;

	if (!SaveChanges_)
	{
		SaveChanges_ = true;
		QTimer::singleShot (1000, this, SLOT (SaveSettings ()));
	}
	return true;
}

void TorrentPlugin::removeTorrent ()
{
	int row = Downloading_->indexOfTopLevelItem (Downloading_->currentItem ());
	TorrentClient *tc = Jobs_.at (row).TC_;

	tc->disconnect ();
	connect (tc, SIGNAL (stopped ()), this, SLOT (torrentStopped ()));
	tc->stop ();

	delete Downloading_->takeTopLevelItem (row);
	Jobs_.removeAt (row);
	setActionsEnabled ();
	SaveChanges_ = true;
	SaveSettings ();
}

void TorrentPlugin::pauseTorrent ()
{
	int row = Downloading_->indexOfTopLevelItem (Downloading_->currentItem ());
	TorrentClient *tc = Jobs_.at (row).TC_;
	tc->setPaused (tc->GetState () != TorrentClient::StatePaused);
	setActionsEnabled ();
}

void TorrentPlugin::moveUp ()
{
	QTreeWidgetItem *item = Downloading_->currentItem ();
	int row = Downloading_->indexOfTopLevelItem (item);
	if (!row)
		return;

	Job tmp = Jobs_.at (row - 1);
	Jobs_ [row - 1] = Jobs_ [row];
	Jobs_ [row] = tmp;

	QTreeWidgetItem *itemAbove = Downloading_->takeTopLevelItem (row - 1);
	Downloading_->insertTopLevelItem (row, itemAbove);
	setActionsEnabled ();
}

void TorrentPlugin::moveDown ()
{
	QTreeWidgetItem *item = Downloading_->currentItem ();
	int row = Downloading_->indexOfTopLevelItem (item);
	if (row == Jobs_.size () - 1)
		return;

	Job tmp = Jobs_.at (row + 1);
	Jobs_ [row + 1] = Jobs_ [row];
	Jobs_ [row] = tmp;

	QTreeWidgetItem *itemAbove = Downloading_->takeTopLevelItem (row + 1);
	Downloading_->insertTopLevelItem (row, itemAbove);
	setActionsEnabled ();
}

void TorrentPlugin::setActionsEnabled ()
{
	QTreeWidgetItem *item = 0;
	if (!Downloading_->selectedItems ().isEmpty ())
		item = Downloading_->selectedItems ().first ();

	TorrentClient *tc = item ? Jobs_.at (Downloading_->indexOfTopLevelItem (item)).TC_ : 0;
	bool pauseEnabled = tc && ((tc->GetState () == TorrentClient::StatePaused) || (tc->GetState () == TorrentClient::StatePreparing));

	DeleteAction_->setEnabled (item);
	PauseAction_->setEnabled (item && pauseEnabled);

	if (tc && tc->GetState () == TorrentClient::StatePaused)
	{
		PauseAction_->setIcon (QIcon (":/resources/images/startjob.png"));
		PauseAction_->setText (tr ("Start/resume"));
	}
	else
	{
		PauseAction_->setIcon (QIcon (":/resources/images/stopjob.png"));
		PauseAction_->setText (tr ("Pause"));
	}

	int row = Downloading_->indexOfTopLevelItem (item);
	MoveUpAction_->setEnabled (item && row);
	MoveDownAction_->setEnabled (item && row != Jobs_.size () - 1);
}

void TorrentPlugin::setUploadLimit (int value)
{
	QString size = Proxy::Instance ()->MakePrettySize (value);
	UploadLimitLabel_->setText (size);
	RateController::Instance ()->SetUploadLimit (value);
}

void TorrentPlugin::setDownloadLimit (int value)
{
	QString size = Proxy::Instance ()->MakePrettySize (value);
	DownloadLimitLabel_->setText (size);
	RateController::Instance ()->SetDownloadLimit (value);
}

void TorrentPlugin::torrentStopped ()
{
	TorrentClient *tc = qobject_cast<TorrentClient*> (sender ());
	if (!tc)
	{
		qDebug () << Q_FUNC_INFO << "sender's type is not TorrentClient*, that's strange";
		return;
	}

	tc->deleteLater ();
	if (QuitDialog_)
		if (++JobsStopped_ == JobsToStop_)
			QuitDialog_->close ();
}

void TorrentPlugin::torrentError (TorrentClient::Error)
{
	TorrentClient *tc = qobject_cast<TorrentClient*> (sender ());
	if (!tc)
	{
		qDebug () << Q_FUNC_INFO << "sender's type is not TorrentClient*, that's strange";
		return;
	}
	int row = GetRowOfClient (tc);
	QString filename = Jobs_.at (row).Filename_;
	Jobs_.removeAt (row);

	QMessageBox::warning (this, tr ("Error"),
						tr ("An error occurred while downloading %0: %1")
						.arg (filename)
						.arg (tc->GetErrorString ()));

	delete Downloading_->takeTopLevelItem (row);
	tc->deleteLater ();
}

void TorrentPlugin::updatePeerInfo ()
{
	TorrentClient *tc = qobject_cast<TorrentClient*> (sender ());
	if (!tc)
	{
		qDebug () << Q_FUNC_INFO << "sender's type is not TorrentClient*, that's strange";
		return;
	}

	Downloading_->topLevelItem (GetRowOfClient (tc))->setText (CPeersSeeds, tr ("%1/%2").arg (tc->GetConnectedPeerCount ()).arg (tc->GetSeedCount ()));
}

void TorrentPlugin::updateProgress (int percent)
{
	TorrentClient *tc = qobject_cast<TorrentClient*> (sender ());
	if (!tc)
	{
		qDebug () << Q_FUNC_INFO << "sender's type is not TorrentClient*, that's strange";
		return;
	}

	QTreeWidgetItem *item = Downloading_->topLevelItem (GetRowOfClient (tc));
	if (item)
		item->setText (CProgress, QString::number (percent));
}

void TorrentPlugin::updateDownloadRate (int bps)
{
	TorrentClient *tc = qobject_cast<TorrentClient*> (sender ());
	if (!tc)
	{
		qDebug () << Q_FUNC_INFO << "sender's type is not TorrentClient*, that's strange";
		return;
	}

	QString speed;
	speed.sprintf ("%.1f KB/s", bps/1024.0);
	Downloading_->topLevelItem (GetRowOfClient (tc))->setText (CDownloading, speed);
}

void TorrentPlugin::updateUploadRate (int bps)
{
	TorrentClient *tc = qobject_cast<TorrentClient*> (sender ());
	if (!tc)
	{
		qDebug () << Q_FUNC_INFO << "sender's type is not TorrentClient*, that's strange";
		return;
	}

	QString speed;
	speed.sprintf ("%.1f KB/s", bps/1024.0);
	Downloading_->topLevelItem (GetRowOfClient (tc))->setText (CUploading, speed);
}

void TorrentPlugin::updateState (TorrentClient::State)
{
	TorrentClient *tc = qobject_cast<TorrentClient*> (sender ());
	if (!tc)
	{
		qDebug () << Q_FUNC_INFO << "sender's type is not TorrentClient*, that's strange";
		return;
	}

	int row = GetRowOfClient (tc);
	QTreeWidgetItem *item = Downloading_->topLevelItem (row);
	if (!item)
		return;

	item->setToolTip (CName, tr ("Torrent: %1<br>Destination: %2<br>State: %3").arg (Jobs_.at (row).Filename_).arg (Jobs_.at (row).DestDir_).arg (tc->GetStateString ()));
	item->setText (CStatus, tc->GetStateString ());

	setActionsEnabled ();
}

QString TorrentPlugin::GetName () const
{
	return Globals::Name;
}

QString TorrentPlugin::GetInfo () const
{
	return tr ("A simple plugin trying to implement the BitTorrent protocol.");
}

QString TorrentPlugin::GetStatusbarMessage () const
{
	return "BitTorrent :)";
}

IInfo& TorrentPlugin::SetID (IInfo::ID_t id)
{
	ID_ = id;
	return *this;
}

IInfo::ID_t TorrentPlugin::GetID () const
{
	return ID_;
}

QStringList TorrentPlugin::Provides () const
{
	QStringList result;
	result << "torrent" << "resume";
	return result;
}

QStringList TorrentPlugin::Needs () const
{
	QStringList result;
	return result;
}

QStringList TorrentPlugin::Uses () const
{
	QStringList result;
	return result;
}

void TorrentPlugin::SetProvider (QObject*, const QString&)
{
}

void TorrentPlugin::Release ()
{
	SaveSettings ();

	if (Jobs_.isEmpty ())
		return;

	SaveChanges_ = false;

	QuitDialog_ = new QProgressDialog (tr ("Disconnecting from trackers"), tr ("Abort"), 0, JobsToStop_, this);
	JobsToStop_ = 0;
	JobsStopped_ = 0;
	foreach (Job job, Jobs_)
	{
		++JobsToStop_;
		TorrentClient *tc = job.TC_;
		tc->disconnect ();
		connect (tc, SIGNAL (stopped ()), this, SLOT (torrentStopped ()));
		tc->stop ();
		delete Downloading_->takeTopLevelItem (0);
	}

	if (JobsToStop_ > JobsStopped_)
		QuitDialog_->exec ();
	QuitDialog_->deleteLater ();
	QuitDialog_ = 0;
	if (SettingsManager::Instance ())
		SettingsManager::Instance ()->Release ();
}

QIcon TorrentPlugin::GetIcon () const
{
	return QIcon (":/resources/images/bittorrent.png");
}

void TorrentPlugin::SetParent (QWidget *w)
{
	setParent (w);
}

void TorrentPlugin::ShowWindow ()
{
	IsShown_ = 1 - IsShown_;
	IsShown_ ? show () : hide ();
}

void TorrentPlugin::ShowBalloonTip ()
{
}

qint64 TorrentPlugin::GetDownloadSpeed () const
{
	return 0;
}

qint64 TorrentPlugin::GetUploadSpeed () const
{
	return 0;
}

const TorrentClient* TorrentPlugin::GetClientForRow (int row) const
{
	return Jobs_ [row].TC_;
}

int TorrentPlugin::GetRowOfClient (TorrentClient *tc) const
{
	int row = 0;
	foreach (Job job, Jobs_)
	{
		if (job.TC_ == tc)
			return row;
		++row;
	}
	return -1;
}

void TorrentPlugin::SaveSettings () const
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup (GetName ());
	settings.beginGroup ("geometry");

	settings.setValue ("size", size ());
	settings.setValue ("pos", pos ());

	settings.endGroup ();
	settings.beginGroup ("system");
	settings.setValue ("lastOpenDir", LastOpenDir_);
	settings.setValue ("lastSaveDir", LastSaveDir_);
	settings.setValue ("uploadLimit", UploadLimitSlider_->value ());
	settings.setValue ("downloadLimit", DownloadLimitSlider_->value ());

	settings.beginWriteArray ("torrents");
	for (int i = 0; i < Jobs_.size (); ++i)
	{
		settings.setArrayIndex (i);
		settings.setValue ("sourceFilename", Jobs_ [i].Filename_);
		settings.setValue ("destinationFolder", Jobs_ [i].DestDir_);
		settings.setValue ("uploadedBytes", Jobs_ [i].TC_->GetUploadedBytes ());
		settings.setValue ("downloadedBytes", Jobs_ [i].TC_->GetDownloadedBytes ());
		settings.setValue ("resumeState", Jobs_ [i].TC_->GetDumpedState ());
	}
	settings.endArray ();
	settings.endGroup ();
	settings.endGroup ();
}

void TorrentPlugin::ReadSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup (GetName ());

	settings.beginGroup ("geometry");
	resize (settings.value ("size", QSize (800, 600)).toSize ());
	move (settings.value ("pos", QPoint (20, 20)).toPoint ());
	settings.endGroup ();

	settings.beginGroup ("system");
	LastOpenDir_ = settings.value ("lastOpenDirectory").toString ();
	if (LastOpenDir_.isEmpty ())
		LastOpenDir_ = QDir::currentPath ();
	LastSaveDir_ = settings.value ("lastSaveDirectory").toString ();
	if (LastSaveDir_.isEmpty ())
		LastSaveDir_ = QDir::currentPath ();
	UploadLimitSlider_->setValue (settings.value ("uploadLimit", 512).toInt ());
	DownloadLimitSlider_->setValue (settings.value ("downloadLimit", 1024).toInt ());

	int size = settings.beginReadArray ("torrents");
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex (i);
		QByteArray resumeState = settings.value ("resumeState").toByteArray ();
		QString filename = settings.value ("sourceFilename").toString (),
				dest = settings.value ("destinationFolder").toString ();

		if (addTorrent (filename, dest, resumeState))
		{
			TorrentClient *client = Jobs_.last ().TC_;
			client->SetDownloadedBytes (settings.value ("downloadedBytes").toLongLong ());
			client->SetUploadedBytes (settings.value ("uploadedBytes").toLongLong ());
		}
	}
	settings.endArray ();
	settings.endGroup ();

	settings.endGroup ();
}

void TorrentPlugin::showError (QString str)
{
	QMessageBox::critical (this, "Error!", str);
}

void TorrentPlugin::showPreferences ()
{
	SettingsDialog_->show ();
	SettingsDialog_->setWindowTitle (windowTitle () + tr (": Preferences"));
}

void TorrentPlugin::closeEvent (QCloseEvent*)
{
}

Q_EXPORT_PLUGIN2 (leechcraft_torrent, TorrentPlugin);

