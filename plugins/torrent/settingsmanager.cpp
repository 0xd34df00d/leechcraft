#include <QDir>
#include <QTimer>
#include <QSettings>
#include <plugininterface/proxy.h>
#include "settingsmanager.h"

Q_GLOBAL_STATIC (SettingsManager, SettingsManagerInstance);

SettingsManager::SettingsManager ()
: SaveScheduled_ (false)
{
	ReadSettings ();
}

SettingsManager::~SettingsManager ()
{
}

void SettingsManager::Release ()
{
	writeSettings ();
}

SettingsManager* SettingsManager::Instance ()
{
	return SettingsManagerInstance ();
}

SettingsItemInfo SettingsManager::GetInfoFor (const QString& propName) const
{
}

void SettingsManager::ScheduleSave ()
{
	if (SaveScheduled_)
		return;

	SaveScheduled_ = true;
	QTimer::singleShot (20, this, SLOT (writeSettings ()));
}

void SettingsManager::ReadSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup ("Torrent");
	LastTorrentDirectory_	= settings.value ("LastTorrentDirectory", QDir::homePath ()).toString ();
	LastSaveDirectory_		= settings.value ("LastSaveDirectory", QDir::homePath ()).toString ();
	settings.endGroup ();
}

void SettingsManager::writeSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup ("Torrent");
	settings.setValue ("LastTorrentDirectory", LastTorrentDirectory_);
	settings.setValue ("LastSaveDirectory", LastSaveDirectory_);
	settings.endGroup ();
	SaveScheduled_ = false;
}

QString SettingsManager::GetLastTorrentDirectory () const
{
	return LastTorrentDirectory_;
}

void SettingsManager::SetLastTorrentDirectory (const QString& val)
{
	LastTorrentDirectory_ = val;
	ScheduleSave ();
}

QString SettingsManager::GetLastSaveDirectory () const
{
	return LastSaveDirectory_;
}

void SettingsManager::SetLastSaveDirectory (const QString& val)
{
	LastSaveDirectory_ = val;
	ScheduleSave ();
}

