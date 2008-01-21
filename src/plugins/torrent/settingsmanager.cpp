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
	QTimer::singleShot (1000, this, SLOT (writeSettings ()));
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
	UploadRateLimit_			= settings.value ("UploadRateLimit", -1).toInt ();
	DownloadRateLimit_			= settings.value ("DownloadRateLimit", -1).toInt ();
	DesiredRating_				= settings.value ("DesiredRating", 0).toDouble ();
	MaxUploads_					= settings.value ("MaxUploads", 100).toInt ();
	MaxConnections_				= settings.value ("MaxConnections", 1000).toInt ();
	TrackerProxyEnabled_		= settings.value ("TrackerProxyEnabled").toBool ();
	TrackerProxyAddress_		= settings.value ("TrackerProxyAddress").toString ();
	TrackerProxyPort_			= settings.value ("TrackerProxyPort", 8080).toInt ();
	TrackerProxyLogin_			= settings.value ("TrackerProxyLogin").toString ();
	TrackerProxyPassword_		= settings.value ("TrackerProxyPassword").toString ();
	PeerProxyEnabled_			= settings.value ("PeerProxyEnabled").toBool ();
	PeerProxyAddress_			= settings.value ("PeerProxyAddress").toString ();
	PeerProxyPort_				= settings.value ("PeerProxyPort", 8080).toInt ();
	PeerProxyLogin_				= settings.value ("PeerProxyLogin").toString ();
	PeerProxyPassword_			= settings.value ("PeerProxyPassword").toString ();
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
	settings.setValue ("UploadRateLimit", UploadRateLimit_);
	settings.setValue ("DownloadRateLimit", DownloadRateLimit_);
	settings.setValue ("DesiredRating", DesiredRating_);
	settings.setValue ("MaxUploads", MaxUploads_);
	settings.setValue ("MaxConnections", MaxConnections_);
	settings.setValue ("TrackerProxyEnabled", TrackerProxyEnabled_);
	settings.setValue ("TrackerProxyAddress", TrackerProxyAddress_);
	settings.setValue ("TrackerProxyPort", TrackerProxyPort_);
	settings.setValue ("TrackerProxyLogin", TrackerProxyLogin_);
	settings.setValue ("TrackerProxyPassword", TrackerProxyPassword_);
	settings.setValue ("PeerProxyEnabled", PeerProxyEnabled_);
	settings.setValue ("PeerProxyAddress", PeerProxyAddress_);
	settings.setValue ("PeerProxyPort", PeerProxyPort_);
	settings.setValue ("PeerProxyLogin", PeerProxyLogin_);
	settings.setValue ("PeerProxyPassword", PeerProxyPassword_);
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

int SettingsManager::GetUploadRateLimit () const
{
	return UploadRateLimit_;
}

void SettingsManager::SetUploadRateLimit (int val)
{
	UploadRateLimit_ = val;
	ScheduleSave ();
}

int SettingsManager::GetDownloadRateLimit () const
{
	return DownloadRateLimit_;
}

void SettingsManager::SetDownloadRateLimit (int val)
{
	DownloadRateLimit_ = val;
	ScheduleSave ();
}

double SettingsManager::GetDesiredRating () const
{
	return DesiredRating_;
}

void SettingsManager::SetDesiredRating (double val)
{
	DesiredRating_ = val;
	ScheduleSave ();
}

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

int SettingsManager::GetMaxUploads () const
{
	return MaxUploads_;
}

void SettingsManager::SetMaxUploads (int val)
{
	MaxUploads_ = val;
	ScheduleSave ();
	CallSlots ("MaxUploads");
}

int SettingsManager::GetMaxConnections () const
{
	return MaxConnections_;
}

void SettingsManager::SetMaxConnections (int val)
{
	MaxConnections_ = val;
	ScheduleSave ();
	CallSlots ("MaxConnections");
}

bool SettingsManager::GetTrackerProxyEnabled () const
{
	return TrackerProxyEnabled_;
}

void SettingsManager::SetTrackerProxyEnabled (bool val)
{
	TrackerProxyEnabled_ = val;
	ScheduleSave ();
	CallSlots ("TrackerProxyEnabled");
}

QString SettingsManager::GetTrackerProxyAddress () const
{
	return TrackerProxyAddress_;
}

void SettingsManager::SetTrackerProxyAddress (QString val)
{
	TrackerProxyAddress_ = val;
	ScheduleSave ();
	CallSlots ("TrackerProxyAddress");
}

int SettingsManager::GetTrackerProxyPort () const
{
	return TrackerProxyPort_;
}

void SettingsManager::SetTrackerProxyPort (int val)
{
	TrackerProxyPort_ = val;
	ScheduleSave ();
	CallSlots ("TrackerProxyPort");
}

QString SettingsManager::GetTrackerProxyLogin () const
{
	return TrackerProxyLogin_;
}

void SettingsManager::SetTrackerProxyLogin (QString val)
{
	TrackerProxyLogin_ = val;
	ScheduleSave ();
	CallSlots ("TrackerProxyLogin");
}

QString SettingsManager::GetTrackerProxyPassword () const
{
	return TrackerProxyPassword_;
}

void SettingsManager::SetTrackerProxyPassword (QString val)
{
	TrackerProxyPassword_ = val;
	ScheduleSave ();
	CallSlots ("TrackerProxyPassword");
}

bool SettingsManager::GetPeerProxyEnabled () const
{
	return PeerProxyEnabled_;
}

void SettingsManager::SetPeerProxyEnabled (bool val)
{
	PeerProxyEnabled_ = val;
	ScheduleSave ();
	CallSlots ("PeerProxyEnabled");
}

QString SettingsManager::GetPeerProxyAddress () const
{
	return PeerProxyAddress_;
}

void SettingsManager::SetPeerProxyAddress (QString val)
{
	PeerProxyAddress_ = val;
	ScheduleSave ();
	CallSlots ("PeerProxyAddress");
}

int SettingsManager::GetPeerProxyPort () const
{
	return PeerProxyPort_;
}

void SettingsManager::SetPeerProxyPort (int val)
{
	PeerProxyPort_ = val;
	ScheduleSave ();
	CallSlots ("PeerProxyPort");
}

QString SettingsManager::GetPeerProxyLogin () const
{
	return PeerProxyLogin_;
}

void SettingsManager::SetPeerProxyLogin (QString val)
{
	PeerProxyLogin_ = val;
	ScheduleSave ();
	CallSlots ("PeerProxyLogin");
}

QString SettingsManager::GetPeerProxyPassword () const
{
	return PeerProxyPassword_;
}

void SettingsManager::SetPeerProxyPassword (QString val)
{
	PeerProxyPassword_ = val;
	ScheduleSave ();
	CallSlots ("PeerProxyPassword");
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
	portRange.PageIcon_ = QIcon (":/resources/images/network.png");
	PropInfos_ ["PortRange"] = portRange;

	SettingsItemInfo dhtEnabled = SettingsItemInfo (tr ("DHT enabled"), tr ("Network options"));
	PropInfos_ ["DHTEnabled"] = dhtEnabled;
	
	SettingsItemInfo maxUploads = SettingsItemInfo (tr ("Maximum uploads"), tr ("Network options"), tr ("Limits"));
	maxUploads.IntRange_ = qMakePair<int, int> (1, 10000);
	PropInfos_ ["MaxUploads"] = maxUploads;

	SettingsItemInfo maxConnections = SettingsItemInfo (tr ("Maximum connections"), tr ("Network options"), tr ("Limits"));
	maxConnections.SpinboxStep_ = 5;
	maxConnections.IntRange_ = qMakePair<int, int> (2, 10000);
	PropInfos_ ["MaxConnections"] = maxConnections;

	SettingsItemInfo trackerProxyEnabled = SettingsItemInfo (tr ("Tracker proxy"), tr ("Network options"));
	trackerProxyEnabled.GroupBoxer_ = true;
	trackerProxyEnabled.SubItems_ << "TrackerProxyAddress" << "TrackerProxyPort" << "TrackerProxyLogin" << "TrackerProxyPassword";
	PropInfos_ ["TrackerProxyEnabled"] = trackerProxyEnabled;
	
	SettingsItemInfo trackerProxyAddress = SettingsItemInfo (tr ("Hostname"), tr ("Network options"));
	PropInfos_ ["TrackerProxyAddress"] = trackerProxyAddress;
	
	SettingsItemInfo trackerProxyPort = SettingsItemInfo (tr ("Port"), tr ("Network options"));
	trackerProxyPort.IntRange_ = qMakePair<int, int> (1, 65535);
	PropInfos_ ["TrackerProxyPort"] = trackerProxyPort;

	SettingsItemInfo trackerProxyLogin = SettingsItemInfo (tr ("Login"), tr ("Network options"));
	PropInfos_ ["TrackerProxyLogin"] = trackerProxyLogin;

	SettingsItemInfo trackerProxyPassword = SettingsItemInfo (tr ("Password"), tr ("Network options"));
	PropInfos_ ["TrackerProxyPassword"] = trackerProxyPassword;

	SettingsItemInfo peerProxyEnabled = SettingsItemInfo (tr ("Peer proxy"), tr ("Network options"));
	peerProxyEnabled.GroupBoxer_ = true;
	peerProxyEnabled.SubItems_ << "PeerProxyAddress" << "PeerProxyPort" << "PeerProxyLogin" << "PeerProxyPassword";
	PropInfos_ ["PeerProxyEnabled"] = peerProxyEnabled;
	
	SettingsItemInfo peerProxyAddress = SettingsItemInfo (tr ("Hostname"), tr ("Network options"));
	PropInfos_ ["PeerProxyAddress"] = peerProxyAddress;
	
	SettingsItemInfo peerProxyPort = SettingsItemInfo (tr ("Port"), tr ("Network options"));
	peerProxyPort.IntRange_ = qMakePair<int, int> (1, 65535);
	PropInfos_ ["PeerProxyPort"] = peerProxyPort;

	SettingsItemInfo peerProxyLogin = SettingsItemInfo (tr ("Login"), tr ("Network options"));
	PropInfos_ ["PeerProxyLogin"] = peerProxyLogin;

	SettingsItemInfo peerProxyPassword = SettingsItemInfo (tr ("Password"), tr ("Network options"));
	PropInfos_ ["PeerProxyPassword"] = peerProxyPassword;

	SettingsItemInfo autosaveInterval = SettingsItemInfo (tr ("Auto save interval"), tr ("Local options"));
	autosaveInterval.SpinboxStep_ = 1;
	autosaveInterval.IntRange_ = qMakePair<int, int> (10, 600);
	autosaveInterval.SpinboxSuffix_ = tr (" s");
	autosaveInterval.PageIcon_ = QIcon (":/resources/images/local.png");
	PropInfos_ ["AutosaveInterval"] = autosaveInterval;
}

void SettingsManager::CallSlots (const QString& name)
{
	QObject *obj = Property2Object_ [name].first;
	QString slotName = Property2Object_ [name].second;

	QMetaObject::invokeMethod (obj, slotName.toStdString ().c_str (), Qt::AutoConnection);
}

