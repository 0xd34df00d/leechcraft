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

void SettingsManager::RegisterObject (const QString& propName, QObject *obj, const QString& slotName)
{
	Property2Object_ [propName] = qMakePair (obj, slotName);
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
	LastTorrentDirectory_		= settings.value ("LastTorrentDirectory", QDir::homePath ()).toString ();
	LastSaveDirectory_			= settings.value ("LastSaveDirectory", QDir::homePath ()).toString ();
	LastMakeTorrentDirectory_	= settings.value ("LastMakeTorrentDirectory", QDir::homePath ()).toString ();
	LastAddDirectory_			= settings.value ("LastAddDirectory", QDir::homePath ()).toString ();
	PortRange_					= settings.value ("PortRange", QVariant::fromValue (qMakePair<int, int> (6881, 6889))).value<IntRange> ();
	DHTEnabled_					= settings.value ("DHTEnabled", true).toBool ();
	AutosaveInterval_			= settings.value ("AutosaveInterval", 120).toInt ();
//	DHTState_					= settings.value ("DHTState", QVariant::fromValue (libtorrent::entry ())).value<libtorrent::entry> ();
	settings.endGroup ();
}

void SettingsManager::writeSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup ("Torrent");
	settings.setValue ("LastTorrentDirectory", LastTorrentDirectory_);
	settings.setValue ("LastSaveDirectory", LastSaveDirectory_);
	settings.setValue ("LastMakeTorrentDirectory", LastMakeTorrentDirectory_);
	settings.setValue ("LastAddDirectory", LastAddDirectory_);
	settings.setValue ("PortRange", QVariant::fromValue (PortRange_.Val ()));
	settings.setValue ("DHTEnabled", DHTEnabled_);
	settings.setValue ("AutosaveInterval", AutosaveInterval_);
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

QString SettingsManager::GetLastMakeTorrentDirectory () const
{
	return LastMakeTorrentDirectory_;
}

void SettingsManager::SetLastMakeTorrentDirectory (const QString& val)
{
	LastMakeTorrentDirectory_ = val;
	ScheduleSave ();
}

QString SettingsManager::GetLastAddDirectory () const
{
	return LastAddDirectory_;
}

void SettingsManager::SetLastAddDirectory (const QString& val)
{
	LastAddDirectory_ = val;
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
	CallSlots ("PortRange");
}

bool SettingsManager::GetDHTEnabled () const
{
	return DHTEnabled_;
}

void SettingsManager::SetDHTEnabled (bool val)
{
	DHTEnabled_ = val;
	ScheduleSave ();
	CallSlots ("DHTState");
}

int SettingsManager::GetAutosaveInterval () const
{
	return AutosaveInterval_;
}

void SettingsManager::SetAutosaveInterval (int val)
{ 
	AutosaveInterval_ = val;
	ScheduleSave ();
	CallSlots ("AutosaveInterval");
}

void SettingsManager::FillMap ()
{
	SettingsItemInfo portRange = SettingsItemInfo (tr ("Port range"), tr ("Network options"));
	portRange.SpinboxStep_ = 1;
	portRange.IntRange_ = qMakePair<int, int> (1025, 65535);
	PropInfos_ ["PortRange"] = portRange;

	SettingsItemInfo dhtEnabled = SettingsItemInfo (tr ("DHT enabled"), tr ("Network options"));
	PropInfos_ ["DHTEnabled"] = dhtEnabled;

	SettingsItemInfo autosaveInterval = SettingsItemInfo (tr ("Auto save interval"), tr ("Local options"));
	autosaveInterval.SpinboxStep_ = 1;
	autosaveInterval.IntRange_ = qMakePair<int, int> (10, 600);
	autosaveInterval.SpinboxSuffix_ = tr (" s");
	PropInfos_ ["AutosaveInterval"] = autosaveInterval;
}

void SettingsManager::CallSlots (const QString& name)
{
	QObject *obj = Property2Object_ [name].first;
	QString slotName = Property2Object_ [name].second;

	QMetaObject::invokeMethod (obj, slotName.toStdString ().c_str (), Qt::AutoConnection);
}

