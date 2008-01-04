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
    SaveDownloadedInHistory_ = settings.value ("SaveDownloadedInHistory").toBool ();
    settings.endGroup ();
    settings.endGroup ();
}

void SettingsManager::WriteSettings ()
{
    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
    settings.beginGroup (Globals::Name);
    settings.beginGroup ("mainsettings");
    settings.setValue ("Mirrors", Mirrors_);
    settings.setValue ("SaveDownloadedInHistory", SaveDownloadedInHistory_);
    settings.endGroup ();
    settings.endGroup ();
}

void SettingsManager::InitializeMap ()
{
	SettingsItemInfo mirrorsInfo = SettingsItemInfo (tr ("Update mirrors"), tr ("General options"));
	PropertyInfo_ ["Mirrors"] = mirrorsInfo;

	SettingsItemInfo saveDownloadedInHistory = SettingsItemInfo (tr ("Save downloaded files in history"), tr ("General options"));
	PropertyInfo_ ["SaveDownloadedInHistory"] = saveDownloadedInHistory;
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

bool SettingsManager::GetSaveDownloadedInHistory () const
{
	return SaveDownloadedInHistory_;
}

void SettingsManager::SetSaveDownloadedInHistory (bool value)
{
	SaveDownloadedInHistory_ = value;
	ScheduleFlush ();
}

SettingsItemInfo SettingsManager::GetInfoFor (const QString& propName) const
{
	return PropertyInfo_ [propName];
}

