#include <QDir>
#include <QTimer>
#include <QSettings>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "settingsmanager.h"

Q_GLOBAL_STATIC (SettingsManager, SettingsManagerInstance);
Q_DECLARE_METATYPE (libtorrent::entry);

SettingsManager::SettingsManager ()
: SaveScheduled_ (false)
{
	qRegisterMetaTypeStreamOperators<IntRange> ("IntRange");
//	qRegisterMetaType<libtorrent::entry> ("libtorrent::entry");
//	qRegisterMetaTypeStreamOperators<libtorrent::entry> ("libtorrent::entry");
	ReadSettings ();
	FillMap ();
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
	return PropInfos_ [propName];
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
	PortRange_				= settings.value ("PortRange", QVariant::fromValue (qMakePair<int, int> (6881, 6889))).value<IntRange> ();
//	DHTState_				= settings.value ("DHTState", QVariant::fromValue (libtorrent::entry ())).value<libtorrent::entry> ();
	settings.endGroup ();
}

void SettingsManager::writeSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup ("Torrent");
	settings.setValue ("LastTorrentDirectory", LastTorrentDirectory_);
	settings.setValue ("LastSaveDirectory", LastSaveDirectory_);
	settings.setValue ("PortRange", QVariant::fromValue (PortRange_.Val ()));
//	settings.setValue ("DHTState", QVariant::fromValue (DHTState_.Val ()));
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

/*
const libtorrent::entry& SettingsManager::GetDHTState () const
{
	return DHTState_;
}

void SettingsManager::SetDHTState (const libtorrent::entry& val)
{
	DHTState_ = val;
	ScheduleSave ();
}
*/

IntRange SettingsManager::GetPortRange () const
{
	return PortRange_;
}

void SettingsManager::SetPortRange (const IntRange& value)
{
	PortRange_ = value;
	ScheduleSave ();
}

void SettingsManager::FillMap ()
{
	SettingsItemInfo portRange = SettingsItemInfo (tr ("Port range"), tr ("Network options"));
	portRange.SpinboxStep_ = 1;
	portRange.IntRange_ = qMakePair<int, int> (1025, 65535);
	PropInfos_ ["PortRange"] = portRange;
}

