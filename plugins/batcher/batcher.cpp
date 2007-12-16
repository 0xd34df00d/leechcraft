#include <QtGui>
#include "batcher.h"
#include "globals.h"

void Batcher::Init ()
{
	Q_INIT_RESOURCE (resources);
	QTranslator *transl = new QTranslator;
    QString localeName = QLocale::system ().name ();
    transl->load (QString (":/leechcraft_batcher_") + localeName);
    qApp->installTranslator (transl);
}

QString Batcher::GetName () const
{
	return Globals::Name;
}

QString Batcher::GetInfo () const
{
	return tr ("Batch job adder.");
}

QString Batcher::GetStatusbarMessage () const
{
	return "";
}

IInfo& Batcher::SetID (IInfo::ID_t id)
{
	ID_ = id;
}

IInfo::ID_t Batcher::GetID () const
{
	return ID_;
}

QStringList Batcher::Provides () const
{
	return QStringList ();
}

QStringList Batcher::Needs () const
{
	return QStringList ("http") << "ftp";
}

QStringList Batcher::Uses () const
{
	return QStringList ();
}

void Batcher::SetProvider (QObject *obj, const QString& feature)
{
	Providers_ [feature] = obj;
}

void Batcher::Release ()
{
}

QIcon Batcher::GetIcon () const
{
	return QIcon ();
}

void Batcher::SetParent (QWidget *parent)
{
	setParent (parent);
}

void Batcher::ShowWindow ()
{
	IsShown_ = 1 - IsShown_;
	IsShown_ ? show () : hide ();
}

void Batcher::ShowBalloonTip ()
{
}

void Batcher::closeEvent (QCloseEvent*)
{
	IsShown_ = false;
}

Q_EXPORT_PLUGIN2 (leechcraft_batcher, Batcher);
