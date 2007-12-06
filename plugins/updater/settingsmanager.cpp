#include <QMutex>
#include <QTimer>
#include <QSettings>
#include <plugininterface/proxy.h>
#include "settingsmanager.h"

SettingsManager *SettingsManager::Instance_ = 0;
QMutex* SettingsManager::InstanceMutex_ = new QMutex;

SettingsManager::SettingsManager ()
: SaveChangesScheduled_ (false)
{
	ReadSettings ();
	InitializeMap ();
}

void SettingsManager::ReadSettings ()
{
    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
    settings.beginGroup (Globals::Name);
    settings.beginGroup ("mainsettings");
    Mirrors_ = settings.value ("Mirrors").toStringList ();
    settings.endGroup ();
    settings.endGroup ();
}

void SettingsManager::WriteSettings ()
{
    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
    settings.beginGroup (Globals::Name);
    settings.beginGroup ("mainsettings");
    settings.setValue ("Mirrors", Mirrors_);
    settings.endGroup ();
    settings.endGroup ();
}

void SettingsManager::InitializeMap ()
{
	SettingsItemInfo mirrorsInfo = SettingsItemInfo (tr ("Update mirrors"), tr ("General options"));
	PropertyInfo_ ["Mirrors"] = mirrorsInfo;
}

void SettingsManager::ScheduleFlush ()
{
	if (!SaveChangesScheduled_)
	{
		SaveChangesScheduled_ = true;
		QTimer::singleShot (1000, this, SLOT (flush ()));
	}
}

void SettingsManager::flush ()
{
	WriteSettings ();
	SaveChangesScheduled_ = false;
}

SettingsManager::~SettingsManager ()
{
}

SettingsManager* SettingsManager::Instance ()
{
	if (!Instance_)
		Instance_ = new SettingsManager ();
	InstanceMutex_->lock ();
	SettingsManager *result = Instance_;
	InstanceMutex_->unlock ();
	return result;
}

void SettingsManager::Release ()
{
	Instance ()->flush ();
	delete Instance_;
	Instance_ = 0;
}

const QStringList& SettingsManager::GetMirrors () const
{
	return Mirrors_;
}

void SettingsManager::SetMirrors (const QStringList& mirrors)
{
	Mirrors_ = mirrors;
	ScheduleFlush ();
}

SettingsItemInfo SettingsManager::GetInfoFor (const QString& propName) const
{
	return PropertyInfo_ [propName];
}

