#include <QtGui>
#include "cron.h"
#include "globals.h"

void Cron::Init ()
{
}

QString Cron::GetName () const
{
	return Globals::Name;
}

QString Cron::GetInfo () const
{
	return tr ("Provides simple scheduling policies.");
}

QString Cron::GetStatusbarMessage () const
{
	return "";
}

IInfo& Cron::SetID (IInfo::ID_t id)
{
	ID_ = id;
	return *this;
}

IInfo::ID_t Cron::GetID () const
{
	return ID_;
}

QStringList Cron::Provides () const
{
	return QStringList ("cron");
}

QStringList Cron::Needs () const
{
	return QStringList ();
}

QStringList Cron::Uses () const
{
	return QStringList ();
}

void Cron::SetProvider (QObject *provider, const QString& feature)
{
	Providers_ [feature] = provider;
}

void Cron::Release ()
{
}

Q_EXPORT_PLUGIN2 (leechcraft_cron, Cron);

