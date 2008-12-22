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
#include <QTranslator>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <plugininterface/util.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "addtask.h"
#include "mainviewdelegate.h"
#include "ui_tabwidget.h"

CSTP::~CSTP ()
{
}

void CSTP::Init ()
{
	Translator_.reset (LeechCraft::Util::InstallTranslator ("cstp"));

	XmlSettingsDialog_.reset (new XmlSettingsDialog ());
	XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (), ":/cstpsettings.xml");

	SetupTabWidget ();
	SetupToolbar ();

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
			SIGNAL (fileDownloaded (const QString&)),
			this,
			SIGNAL (fileDownloaded (const QString&)));
	connect (&Core::Instance (),
			SIGNAL (downloadFinished (const QString&)),
			this,
			SIGNAL (downloadFinished (const QString&)));
	connect (&Core::Instance (),
			SIGNAL (error (const QString&)),
			this,
			SLOT (handleError (const QString&)));
}

void CSTP::Release ()
{
	Core::Instance ().Release ();
	XmlSettingsManager::Instance ().Release ();
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

bool CSTP::CouldDownload (const QByteArray& url, LeechCraft::TaskParameters tp) const
{
	return Core::Instance ().CouldDownload (QString (url), tp);
}

int CSTP::AddJob (const LeechCraft::DownloadParams& ddp, LeechCraft::TaskParameters tp)
{
	if (tp & LeechCraft::FromCommonDialog)
	{
		AddTask at (ddp.Resource_, ddp.Location_);
		if (at.exec () == QDialog::Rejected)
			return -1;

		AddTask::Task task = at.GetTask ();

		return Core::Instance ().AddTask (task.URL_,
				task.LocalPath_,
				task.Filename_,
				task.Comment_,
				tp);
	}
	else
	{
		QFileInfo fi (ddp.Location_);
		QString dir = fi.dir ().path (), file = fi.fileName ();

		if (!(tp & LeechCraft::Internal))
		{
			if (fi.isDir ())
			{
				dir = ddp.Location_;
				file = QFileInfo (QUrl (ddp.Resource_).path ()).fileName ();
				if (file.isEmpty ())
					file = "index";
			}
			else if (fi.isFile ());
			else
				return -1;
		}

		return Core::Instance ().AddTask (ddp.Resource_,
				dir, file, QString (), tp);
	}
}

QAbstractItemModel* CSTP::GetRepresentation () const
{
	return Core::Instance ().GetRepresentationModel ();
}

LeechCraft::Util::HistoryModel* CSTP::GetHistory () const
{
	return 0;
}

QWidget* CSTP::GetControls () const
{
	return Toolbar_.get ();
}

QWidget* CSTP::GetAdditionalInfo () const
{
	return TabWidget_.get ();
}

void CSTP::ItemSelected (const QModelIndex&)
{
}

void CSTP::on_ActionRemoveItemFromHistory__triggered ()
{
	ApplyCore2Selection (&Core::RemoveFromHistory, UiTabWidget_->HistoryView_);
}

void CSTP::showSettings (int)
{
	XmlSettingsDialog_->show ();
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
	UiTabWidget_.reset (new Ui::TabWidget);
	TabWidget_.reset (new QTabWidget);
	UiTabWidget_->setupUi (TabWidget_.get ());
	UiTabWidget_->HistoryView_->setModel (Core::Instance ().GetHistoryModel ());
	UiTabWidget_->HistoryView_->addAction (UiTabWidget_->ActionRemoveItemFromHistory_);

	connect (UiTabWidget_->ActionRemoveItemFromHistory_,
			SIGNAL (triggered ()),
			this,
			SLOT (on_ActionRemoveItemFromHistory__triggered ()));
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
	remove->setProperty ("ActionIcon", "cstp_removeall");

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
	startAll->setProperty ("ActionIcon", "cstp_stopall");

	Toolbar_->addSeparator ();

	QAction *settings = Toolbar_->addAction (tr ("Settings"));
	settings->setProperty ("Slot", "showSettings");
	settings->setProperty ("Object",
			QVariant::fromValue<QObject*> (this));
	settings->setProperty ("ActionIcon", "cstp_preferences");
}

void CSTP::handleError (const QString& text)
{
	UiTabWidget_->Logger_->append (text + QString ("<br />"));
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

