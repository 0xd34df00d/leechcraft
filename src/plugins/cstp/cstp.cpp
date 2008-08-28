#include "cstp.h"
#include <boost/bind.hpp>
#include <boost/logic/tribool.hpp>
#include <QMenu>
#include <QTranslator>
#include <QLocale>
#include <QFileInfo>
#include <QMessageBox>
#include <QDir>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "addtask.h"
#include "mainviewdelegate.h"

CSTP::CSTP ()
: IsShown_ (false)
{
}

CSTP::~CSTP ()
{
}

void CSTP::Init ()
{
    QTranslator *transl = new QTranslator (this);
    QString localeName = QString(::getenv ("LANG")).left (2);
    if (localeName.isNull () || localeName.isEmpty ())
        localeName = QLocale::system ().name ();
    transl->load (QString (":/leechcraft_cstp_") + localeName);
    qApp->installTranslator (transl);

	XmlSettingsDialog_ = new XmlSettingsDialog (this);
	XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (), ":/cstpsettings.xml");

	Ui_.setupUi (this);
	Ui_.MainView_->setItemDelegate (new MainViewDelegate (Ui_.MainView_));
	Ui_.MainView_->setModel (&Core::Instance ());
	Ui_.HistoryView_->setModel (Core::Instance ().GetHistoryModel ());
	Ui_.HistoryView_->addAction (Ui_.ActionRemoveItemFromHistory_);

	connect (&Core::Instance (),
			SIGNAL (taskFinished (int)),
			this,
			SIGNAL (jobFinished (int)));
	connect (&Core::Instance (),
			SIGNAL (taskRemoved (int)),
			this,
			SIGNAL (jobRemoved (int)));
	connect (&Core::Instance (),
			SIGNAL (taskError (int, IDirectDownload::Error)),
			this,
			SIGNAL (jobError (int, IDirectDownload::Error)));
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
}

QString CSTP::GetName () const
{
	return "CSTP";
}

QString CSTP::GetInfo () const
{
	return "Common Stream Transfer Protocols";
}

QString CSTP::GetStatusbarMessage () const
{
	return QString ();
}

IInfo& CSTP::SetID (long unsigned int id)
{
	ID_ = id;
	return *this;
}

unsigned long int CSTP::GetID () const
{
	return ID_;
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

void CSTP::PushMainWindowExternals (const MainWindowExternals& externals)
{
	Plugins_ = externals.RootMenu_->addMenu ("&CSTP");
}

QIcon CSTP::GetIcon () const
{
	return QIcon ();
}

void CSTP::SetParent (QWidget *parent)
{
	setParent (parent);
}

void CSTP::ShowWindow ()
{
	IsShown_ = 1 - IsShown_;
	IsShown_ ? show () : hide ();
}

void CSTP::ShowBalloonTip ()
{
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
	on_ActionStartAll__triggered ();
}

void CSTP::StopAll ()
{
	on_ActionStopAll__triggered ();
}

bool CSTP::CouldDownload (const QString& url, LeechCraft::TaskParameters tp) const
{
	return Core::Instance ().CouldDownload (url, tp);
}

int CSTP::AddJob (const DirectDownloadParams& ddp, LeechCraft::TaskParameters tp)
{
	QFileInfo fi (ddp.Location_);
	return Core::Instance ().AddTask (ddp.Resource_,
			fi.dir ().path (), fi.fileName (),
			QString (), tp);
}

QAbstractItemModel* CSTP::GetRepresentation () const
{
	return Core::Instance ().GetRepresentationModel ();
}

void CSTP::closeEvent (QCloseEvent*)
{
	IsShown_ = false;
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

void CSTP::handleHidePlugins ()
{
	IsShown_ = false;
	hide ();
}

void CSTP::on_ActionAddTask__triggered ()
{
	AddTask at;
	if (at.exec () == QDialog::Rejected)
		return;

	AddTask::Task task = at.GetTask ();
	Core::Instance ().AddTask (task.URL_,
			task.LocalPath_,
			task.Filename_,
			task.Comment_);
}

void CSTP::on_ActionRemoveTask__triggered ()
{
	ApplyCore2Selection (&Core::RemoveTask, Ui_.MainView_);
}

void CSTP::on_ActionStart__triggered ()
{
	ApplyCore2Selection (&Core::Start, Ui_.MainView_);
}

void CSTP::on_ActionStop__triggered ()
{
	ApplyCore2Selection (&Core::Stop, Ui_.MainView_);
}

void CSTP::on_ActionRemoveItemFromHistory__triggered ()
{
	ApplyCore2Selection (&Core::RemoveFromHistory, Ui_.HistoryView_);
}

void CSTP::on_ActionRemoveAll__triggered ()
{
	Core::Instance ().RemoveAll ();
}

void CSTP::on_ActionStartAll__triggered ()
{
	Core::Instance ().StartAll ();
}

void CSTP::on_ActionStopAll__triggered ()
{
	Core::Instance ().StopAll ();
}

void CSTP::on_ActionPreferences__triggered ()
{
	XmlSettingsDialog_->show ();
}

void CSTP::handleError (const QString& text)
{
	if (XmlSettingsManager::Instance ().property ("AlertAboutErrors").toBool ())
		QMessageBox::warning (this, tr ("Error"), text);
	Ui_.Logger_->append (QString ("<br />") + text);
}

void CSTP::handleFileExists (boost::logic::tribool *remove)
{
	QMessageBox::StandardButton userReply = QMessageBox::warning (this,
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

