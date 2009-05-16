#include "cstp.h"
#include <boost/bind.hpp>
#include <boost/logic/tribool.hpp>
#include <QMenu>
#include <QTranslator>
#include <QLocale>
#include <QFileInfo>
#include <QTabWidget>
#include <QToolBar>
#include <QMessageBox>
#include <QModelIndex>
#include <QDir>
#include <QUrl>
#include <QTextCodec>
#include <QTranslator>
#include <plugininterface/util.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "mainviewdelegate.h"

CSTP::~CSTP ()
{
}

void CSTP::Init (ICoreProxy_ptr coreProxy)
{
	Core::Instance ().SetNetworkAccessManager (coreProxy->GetNetworkAccessManager ());
	Translator_.reset (LeechCraft::Util::InstallTranslator ("cstp"));

	XmlSettingsDialog_.reset (new LeechCraft::Util::XmlSettingsDialog ());
	XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (), ":/cstpsettings.xml");

	SetupTabWidget ();
	SetupToolbar ();

	Core::Instance ().SetToolbar (Toolbar_.get ());

	connect (&Core::Instance (),
			SIGNAL (taskFinished (int)),
			this,
			SIGNAL (jobFinished (int)));
	connect (&Core::Instance (),
			SIGNAL (taskRemoved (int)),
			this,
			SIGNAL (jobRemoved (int)));
	connect (&Core::Instance (),
			SIGNAL (taskError (int, IDownload::Error)),
			this,
			SIGNAL (jobError (int, IDownload::Error)));
	connect (&Core::Instance (),
			SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)),
			this,
			SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)));
	connect (&Core::Instance (),
			SIGNAL (downloadFinished (const QString&)),
			this,
			SIGNAL (downloadFinished (const QString&)));
	connect (&Core::Instance (),
			SIGNAL (error (const QString&)),
			this,
			SIGNAL (log (const QString&)));
}

void CSTP::Release ()
{
	Core::Instance ().Release ();
	XmlSettingsManager::Instance ().Release ();
	XmlSettingsDialog_.reset ();
	Toolbar_.reset ();
	Translator_.reset ();
}

QString CSTP::GetName () const
{
	return "CSTP";
}

QString CSTP::GetInfo () const
{
	return "Common Stream Transfer Protocols";
}

QStringList CSTP::Provides () const
{
	return QStringList ("http") << "https" << "remoteable" << "resume";
}

QStringList CSTP::Needs () const
{
	return QStringList ();
}

QStringList CSTP::Uses () const
{
	return QStringList ();
}

void CSTP::SetProvider (QObject*, const QString&)
{
}

QIcon CSTP::GetIcon () const
{
	return QIcon ();
}

qint64 CSTP::GetDownloadSpeed () const
{
	return Core::Instance ().GetTotalDownloadSpeed ();
}

qint64 CSTP::GetUploadSpeed () const
{
	return 0;
}

void CSTP::StartAll ()
{
	Core::Instance ().startAllTriggered ();
}

void CSTP::StopAll ()
{
	Core::Instance ().stopAllTriggered ();
}

bool CSTP::CouldDownload (const LeechCraft::DownloadEntity& e) const
{
	return Core::Instance ().CouldDownload (e);
}

int CSTP::AddJob (LeechCraft::DownloadEntity e)
{
	return Core::Instance ().AddTask (e);
}

QAbstractItemModel* CSTP::GetRepresentation () const
{
	return Core::Instance ().GetRepresentationModel ();
}

void CSTP::ItemSelected (const QModelIndex&)
{
}

boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> CSTP::GetSettingsDialog () const
{
	return XmlSettingsDialog_;
}

template<typename T>
void CSTP::ApplyCore2Selection (void (Core::*temp) (const QModelIndex&), T view)
{
	QModelIndexList indexes = view->selectionModel ()->
		selectedRows ();
	std::for_each (indexes.begin (), indexes.end (),
			boost::bind (temp,
				&Core::Instance (),
				_1));
}

void CSTP::SetupTabWidget ()
{
}

void CSTP::SetupToolbar ()
{
	Toolbar_.reset (new QToolBar);
	Toolbar_->setMovable (false);
	Toolbar_->setFloatable (false);

	QAction *remove = Toolbar_->addAction (tr ("Remove"));
	remove->setProperty ("Slot", "removeTriggered");
	remove->setProperty ("Object",
			QVariant::fromValue<QObject*> (&Core::Instance ()));
	remove->setProperty ("ActionIcon", "cstp_remove");

	QAction *removeAll = Toolbar_->addAction (tr ("Remove all"));
	removeAll->setProperty ("Slot", "removeAllTriggered");
	removeAll->setProperty ("Object",
			QVariant::fromValue<QObject*> (&Core::Instance ()));
	removeAll->setProperty ("ActionIcon", "cstp_removeall");

	Toolbar_->addSeparator ();

	QAction *start = Toolbar_->addAction (tr ("Start"));
	start->setProperty ("Slot", "startTriggered");
	start->setProperty ("Object",
			QVariant::fromValue<QObject*> (&Core::Instance ()));
	start->setProperty ("ActionIcon", "cstp_start");

	QAction *stop = Toolbar_->addAction (tr ("Stop"));
	stop->setProperty ("Slot", "stopTriggered");
	stop->setProperty ("Object",
			QVariant::fromValue<QObject*> (&Core::Instance ()));
	stop->setProperty ("ActionIcon", "cstp_stop");

	QAction *startAll = Toolbar_->addAction (tr ("Start all"));
	startAll->setProperty ("Slot", "startAllTriggered");
	startAll->setProperty ("Object",
			QVariant::fromValue<QObject*> (&Core::Instance ()));
	startAll->setProperty ("ActionIcon", "cstp_startall");

	QAction *stopAll = Toolbar_->addAction (tr ("Stop all"));
	stopAll->setProperty ("Slot", "stopAllTriggered");
	stopAll->setProperty ("Object",
			QVariant::fromValue<QObject*> (&Core::Instance ()));
	stopAll->setProperty ("ActionIcon", "cstp_stopall");

	Toolbar_->addSeparator ();
}

void CSTP::handleFileExists (boost::logic::tribool *remove)
{
	QMessageBox::StandardButton userReply = QMessageBox::warning (0,
			tr ("File exists"), tr ("File %1 already exists, continue download?"),
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	if (userReply == QMessageBox::Yes)
		*remove = false;
	else if (userReply == QMessageBox::No)
		*remove = true;
	else
		*remove = boost::logic::indeterminate;
}

Q_EXPORT_PLUGIN2 (leechcraft_cstp, CSTP);

