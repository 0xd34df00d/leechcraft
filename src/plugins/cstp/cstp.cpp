#include "cstp.h"
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <QMenu>
#include <QTranslator>
#include <QLocale>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "addjob.h"
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
	return QStringList ();
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

void CSTP::closeEvent (QCloseEvent*)
{
	IsShown_ = false;
}

void CSTP::handleHidePlugins ()
{
	IsShown_ = false;
	hide ();
}

void CSTP::on_ActionAddTask__triggered ()
{
	AddJob addjob;
	if (addjob.exec () == QDialog::Rejected)
		return;

	AddJob::Job job = addjob.GetJob ();
	Core::Instance ().AddJob (job.URL_,
			job.LocalPath_,
			job.Filename_,
			job.Comment_);
}

void CSTP::on_ActionStart__triggered ()
{
	QModelIndexList indexes = Ui_.MainView_->selectionModel ()->
		selectedRows ();
	using boost::lambda::_1;
	std::for_each (indexes.begin (), indexes.end (),
			boost::lambda::bind (&Core::Start, &Core::Instance (), _1));
}

void CSTP::on_ActionPreferences__triggered ()
{
	XmlSettingsDialog_->show ();
}

void CSTP::on_ActionRemoveItemFromHistory__triggered ()
{
	QModelIndexList indexes = Ui_.HistoryView_->selectionModel ()->
		selectedRows ();
	using boost::lambda::_1;
	std::for_each (indexes.begin (), indexes.end (),
			boost::lambda::bind (&Core::RemoveFromHistory, &Core::Instance (), _1));
}

Q_EXPORT_PLUGIN2 (leechcraft_cstp, CSTP);

