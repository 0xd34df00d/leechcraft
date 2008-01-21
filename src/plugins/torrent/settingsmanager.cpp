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
	ProxyEnabled_				= settings.value ("ProxyEnabled").toBool ();
	ProxyAddress_				= settings.value ("ProxyAddress").toString ();
	ProxyPort_					= settings.value ("ProxyPort", 8080).toInt ();
	ProxyLogin_					= settings.value ("ProxyLogin").toString ();
	ProxyPassword_				= settings.value ("ProxyPassword").toString ();
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
	settings.setValue ("ProxyEnabled", ProxyEnabled_);
	settings.setValue ("ProxyAddress", ProxyAddress_);
	settings.setValue ("ProxyPort", ProxyPort_);
	settings.setValue ("ProxyLogin", ProxyLogin_);
	settings.setValue ("ProxyPassword", ProxyPassword_);
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

bool SettingsManager::GetProxyEnabled () const
{
	return ProxyEnabled_;
}

void SettingsManager::SetProxyEnabled (bool val)
{
	ProxyEnabled_ = val;
	ScheduleSave ();
	CallSlots ("ProxyEnabled");
}

QString SettingsManager::GetProxyAddress () const
{
	return ProxyAddress_;
}

void SettingsManager::SetProxyAddress (QString val)
{
	ProxyAddress_ = val;
	ScheduleSave ();
	CallSlots ("ProxyAddress");
}

int SettingsManager::GetProxyPort () const
{
	return ProxyPort_;
}

void SettingsManager::SetProxyPort (int val)
{
	ProxyPort_ = val;
	ScheduleSave ();
	CallSlots ("ProxyPort");
}

QString SettingsManager::GetProxyLogin () const
{
	return ProxyLogin_;
}

void SettingsManager::SetProxyLogin (QString val)
{
	ProxyLogin_ = val;
	ScheduleSave ();
	CallSlots ("ProxyLogin");
}

QString SettingsManager::GetProxyPassword () const
{
	return ProxyPassword_;
}

void SettingsManager::SetProxyPassword (QString val)
{
	ProxyPassword_ = val;
	ScheduleSave ();
	CallSlots ("ProxyPassword");
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

	SettingsItemInfo proxyEnabled = SettingsItemInfo (tr ("Proxy enabled"), tr ("Network options"));
	proxyEnabled.GroupBoxer_ = true;
	proxyEnabled.SubItems_ << "ProxyAddress" << "ProxyPort" << "ProxyLogin" << "ProxyPassword";
	PropInfos_ ["ProxyEnabled"] = proxyEnabled;
	
	SettingsItemInfo proxyAddress = SettingsItemInfo (tr ("Hostname"), tr ("Network options"));
	PropInfos_ ["ProxyAddress"] = proxyAddress;
	
	SettingsItemInfo proxyPort = SettingsItemInfo (tr ("Port"), tr ("Network options"));
	proxyPort.IntRange_ = qMakePair<int, int> (1, 65535);
	PropInfos_ ["ProxyPort"] = proxyPort;

	SettingsItemInfo proxyLogin = SettingsItemInfo (tr ("Login"), tr ("Network options"));
	PropInfos_ ["ProxyLogin"] = proxyLogin;

	SettingsItemInfo proxyPassword = SettingsItemInfo (tr ("Password"), tr ("Network options"));
	PropInfos_ ["ProxyPassword"] = proxyPassword;

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

