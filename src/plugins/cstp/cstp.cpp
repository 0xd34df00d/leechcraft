#include "cstp.h"
#include "core.h"

CSTP::CSTP ()
: IsShown_ (false)
{
}

CSTP::~CSTP ()
{
}

void CSTP::Init ()
{
	Ui_.setupUi (this);
	Core::Instance ();
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

Q_EXPORT_PLUGIN2 (leechcraft_cstp, CSTP);

