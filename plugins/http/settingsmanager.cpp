#include <QSettings>
#include <QtDebug>
#include <QMutex>
#include <QTimer>
#include <QMutexLocker>
#include <plugininterface/proxy.h>
#include <settingsdialog/typelist.h>
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
	ConnectTimeout_				= settings.value ("ConnectTimeout", 10000).toInt ();
	DefaultTimeout_				= settings.value ("DefaultTimeout", 2000).toInt ();
	ProxyEnabled_				= settings.value ("ProxyEnabled", false).toBool ();
	ProxyAddress_				= settings.value ("ProxyAddress", "").toString ();
	ProxyPort_					= settings.value ("ProxyPort", 8080).toInt ();
	Login_						= settings.value ("FTPLogin", "anonymous").toString ();
	Password_					= settings.value ("FTPPassword", "some@email.com").toString ();
	CacheSize_					= settings.value ("CacheSize", 256).toInt ();
	StopTimeout_				= settings.value ("StopTimeout", 10000).toInt ();
	DownloadDir_				= settings.value ("DownloadDir", "").toString ();
	AutostartChildren_			= settings.value ("AutostartChildren", true).toBool ();
	UserAgent_.Val ().first		= settings.value ("UserAgent.First").toStringList ();
	UserAgent_.Val ().second	= settings.value ("UserAgent.Second", 3).toInt ();
	MaxConcurrentPerServer_		= settings.value ("MaxConcurrentPerServer", 5).toInt ();
	MaxTotalConcurrent_			= settings.value ("MaxTotalConcurrent", 5).toInt ();
	RetryTimeout_				= settings.value ("RetryTimeout", 30).toInt ();
	AutoGetFileSize_			= settings.value ("AutoGetFileSize", false).toBool ();
	InterfaceUpdateTimeout_		= settings.value ("InterfaceUpdateTimeout", 1000).toInt ();
	settings.endGroup ();
	settings.endGroup ();

	if (UserAgent_.Val ().first.isEmpty ())
		UserAgent_.Val ().first << "LeechCraft HTTP (0.2)"
								<< "Mozilla/4.0 (Compatible; MSIE 7.0; Windows NT 5.1)"
								<< "Mozilla/4.0 (compatible; MSIE 5.01; Windows 98)"
								<< "Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.8.1.9)"
								<< "Opera/9.50 (Windows NT 6.0; U; en)"
								<< "Links (2.1pre28; Linux 2.6.23-gentoo-r5 i686)"
								<< "Wget/1.10.2"
								<< "Mozilla/4.0 (compatible; AvantGo 6.0; FreeBSD)";
}

void SettingsManager::WriteSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
	settings.beginGroup (Globals::Name);
	settings.beginGroup ("mainsettings");
	settings.setValue ("ConnectTimeout", ConnectTimeout_);
	settings.setValue ("DefaultTimeout", DefaultTimeout_);
	settings.setValue ("ProxyEnabled", ProxyEnabled_);
	settings.setValue ("ProxyAddress", ProxyAddress_);
	settings.setValue ("ProxyPort", ProxyPort_);
	settings.setValue ("FTPLogin", Login_);
	settings.setValue ("FTPPassword", Password_);
	settings.setValue ("CacheSize", CacheSize_);
	settings.setValue ("StopTimeout", StopTimeout_);
	settings.setValue ("DownloadDir", DownloadDir_);
	settings.setValue ("AutostartChildren", AutostartChildren_);
	settings.setValue ("UserAgent.First", UserAgent_.Val ().first);
	settings.setValue ("UserAgent.Second", UserAgent_.Val ().second);
	settings.setValue ("MaxConcurrentPerServer", MaxConcurrentPerServer_);
	settings.setValue ("MaxTotalConcurrent", MaxTotalConcurrent_);
	settings.setValue ("RetryTimeout", RetryTimeout_);
	settings.setValue ("AutoGetFileSize", AutoGetFileSize_);
	settings.setValue ("InterfaceUpdateTimeout", InterfaceUpdateTimeout_);
	settings.endGroup ();
	settings.endGroup ();
}

void SettingsManager::InitializeMap ()
{
	SettingsItemInfo downloadDirInfo = SettingsItemInfo (tr ("Default download directory"), tr ("Local options"));
	downloadDirInfo.BrowseButton_ = true;
	PropertyInfo_ ["DownloadDir"] = downloadDirInfo;

	SettingsItemInfo maxConcurrentPerServer = SettingsItemInfo (tr ("Max concurrent jobs per server"), tr ("Network options"));
	maxConcurrentPerServer.IntRange_ = qMakePair (1, 99);	
	PropertyInfo_ ["MaxConcurrentPerServer"] = maxConcurrentPerServer;

	SettingsItemInfo maxTotalConcurrent = SettingsItemInfo (tr ("Max total concurrent jobs"), tr ("Network options"));
	maxTotalConcurrent.IntRange_ = qMakePair (1, 99);	
	PropertyInfo_ ["MaxTotalConcurrent"] = maxTotalConcurrent;

	SettingsItemInfo retryTimeout = SettingsItemInfo (tr ("Retry timeout"), tr ("Network options"));
	retryTimeout.IntRange_ = qMakePair (10, 600);
	retryTimeout.SpinboxSuffix_ = tr (" s");
	retryTimeout.SpinboxStep_ = 10;
	PropertyInfo_ ["RetryTimeout"] = retryTimeout;

	SettingsItemInfo connectTimeoutInfo = SettingsItemInfo (tr ("Connection timeout"), tr ("Network options"));
	connectTimeoutInfo.SpinboxSuffix_ = tr (" ms");
	connectTimeoutInfo.SpinboxStep_ = 100;
	connectTimeoutInfo.IntRange_ = qMakePair (0, 120000);
	PropertyInfo_ ["ConnectTimeout"] = connectTimeoutInfo;

	SettingsItemInfo defaultTimeoutInfo = SettingsItemInfo (tr ("Timeout for other operations"), tr ("Network options"));
	defaultTimeoutInfo.SpinboxSuffix_ = tr (" ms");
	defaultTimeoutInfo.SpinboxStep_ = 100;
	defaultTimeoutInfo.IntRange_ = qMakePair (0, 120000);
	PropertyInfo_ ["DefaultTimeout"] = defaultTimeoutInfo;

	SettingsItemInfo stopTimeout = SettingsItemInfo (tr ("Stop timeout"), tr ("Network options"));
	stopTimeout.SpinboxSuffix_ = tr (" ms");
	stopTimeout.SpinboxStep_ = 250;
	stopTimeout.IntRange_ = qMakePair (1000, 60000);
	PropertyInfo_ ["StopTimeout"] = stopTimeout;

	SettingsItemInfo proxyEnabled = SettingsItemInfo (tr ("Proxy enabled"), tr ("Network options"), tr ("Proxy"));
	PropertyInfo_ ["ProxyEnabled"] = proxyEnabled;

	SettingsItemInfo proxyAddr = SettingsItemInfo (tr ("Proxy address"), tr ("Network options"), tr ("Proxy"));
	proxyAddr.BrowseButton_ = false;
	PropertyInfo_ ["ProxyAddress"] = proxyAddr;

	SettingsItemInfo proxyPort = SettingsItemInfo (tr ("Proxy port"), tr ("Network options"), tr ("Proxy"));
	PropertyInfo_ ["ProxyPort"] = proxyPort;

	SettingsItemInfo resourceLogin = SettingsItemInfo (tr ("Default login"), tr ("FTP options"));
	resourceLogin.BrowseButton_ = false;
	PropertyInfo_ ["ResourceLogin"] = resourceLogin;

	SettingsItemInfo resourcePassword = SettingsItemInfo (tr ("Default password"), tr ("FTP options"));
	resourcePassword.BrowseButton_ = false;
	PropertyInfo_ ["ResourcePassword"] = resourcePassword;

	SettingsItemInfo cacheSize = SettingsItemInfo (tr ("Cache size"), tr ("Local options"), tr ("IO"));
	cacheSize.SpinboxSuffix_ = tr (" kb");
	cacheSize.SpinboxStep_ = 4;
	cacheSize.IntRange_ = qMakePair (0, 1024);
	PropertyInfo_ ["CacheSize"] = cacheSize;

	SettingsItemInfo autostartChildren = SettingsItemInfo (tr ("Autostart spawned jobs"), tr ("Local options"));
	PropertyInfo_ ["AutostartChildren"] = autostartChildren;

	SettingsItemInfo autoGetFileSize = SettingsItemInfo (tr ("Get file size on job addition"), tr ("Local options"));
	PropertyInfo_ ["AutoGetFileSize"] = autoGetFileSize;

	SettingsItemInfo userAgent = SettingsItemInfo (tr ("Mask as user agent"), tr ("HTTP options"));
	userAgent.Modifiable_ = true;
	userAgent.Choosable_ = true;
	PropertyInfo_ ["UserAgent"] = userAgent;

	SettingsItemInfo interfaceUpdateTimeout = SettingsItemInfo (tr ("Interface update timeout"), tr ("Local options"));
	interfaceUpdateTimeout.IntRange_ = qMakePair (100, 2000);
	interfaceUpdateTimeout.SpinboxSuffix_ = tr (" ms");
	interfaceUpdateTimeout.SpinboxStep_ = 100;
	PropertyInfo_ ["InterfaceUpdateTimeout"] = interfaceUpdateTimeout;
}

void SettingsManager::ScheduleFlush ()
{
	if (!SaveChangesScheduled_)
	{
		SaveChangesScheduled_ = true;
		QTimer::singleShot (100, this, SLOT (flush ()));
	}
}

void SettingsManager::flush ()
{
	SaveChangesScheduled_ = false;
	WriteSettings ();
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

QString SettingsManager::GetDownloadDir () const
{
	return DownloadDir_;
}

void SettingsManager::SetDownloadDir (QString dir)
{
	DownloadDir_ = dir;
	ScheduleFlush ();
}

int SettingsManager::GetConnectTimeout () const
{
	return ConnectTimeout_;
}

void SettingsManager::SetConnectTimeout (int msecs)
{
	ConnectTimeout_ = msecs;
	ScheduleFlush ();
}

int SettingsManager::GetDefaultTimeout () const
{
	return DefaultTimeout_;
}

void SettingsManager::SetDefaultTimeout (int msecs)
{
	DefaultTimeout_ = msecs;
	ScheduleFlush ();
}

int SettingsManager::GetStopTimeout () const
{
	return StopTimeout_;
}

void SettingsManager::SetStopTimeout (int msecs)
{
	StopTimeout_ = msecs;
	ScheduleFlush ();
}

bool SettingsManager::GetProxyEnabled () const
{
	return ProxyEnabled_;
}

void SettingsManager::SetProxyEnabled (bool pe)
{
	ProxyEnabled_ = pe;
	ScheduleFlush ();
}

const QString& SettingsManager::GetProxyAddress () const
{
	return ProxyAddress_;
}

void SettingsManager::SetProxyAddress (const QString& address)
{
	ProxyAddress_ = address;
	ScheduleFlush ();
}

int SettingsManager::GetProxyPort () const
{
	return ProxyPort_;
}

void SettingsManager::SetProxyPort (int port)
{
	ProxyPort_ = port;
	ScheduleFlush ();
}

const QString& SettingsManager::GetResourceLogin () const
{
	return Login_;
}

void SettingsManager::SetResourceLogin (const QString& login)
{
	Login_ = login;
	ScheduleFlush ();
}

const QString& SettingsManager::GetResourcePassword () const
{
	return Password_;
}

void SettingsManager::SetResourcePassword (const QString& password)
{
	Password_ = password;
	ScheduleFlush ();
}

int SettingsManager::GetCacheSize () const
{
	return CacheSize_;
}

void SettingsManager::SetCacheSize (int cs)
{
	CacheSize_ = cs;
	ScheduleFlush ();
}

bool SettingsManager::GetAutostartChildren () const
{
	return AutostartChildren_;
}

void SettingsManager::SetAutostartChildren (bool au)
{
	AutostartChildren_ = au;
	ScheduleFlush ();
}

PairedStringList SettingsManager::GetUserAgent () const
{
	return UserAgent_;
}

void SettingsManager::SetUserAgent (const PairedStringList& val)
{
	qDebug () << Q_FUNC_INFO;
	UserAgent_ = val;
	ScheduleFlush ();
}

int SettingsManager::GetMaxConcurrentPerServer () const
{
	return MaxConcurrentPerServer_;
}

void SettingsManager::SetMaxConcurrentPerServer (int value)
{
	MaxConcurrentPerServer_ = value;
	ScheduleFlush ();
}

int SettingsManager::GetMaxTotalConcurrent () const
{
	return MaxTotalConcurrent_;
}

void SettingsManager::SetMaxTotalConcurrent (int value)
{
	MaxTotalConcurrent_ = value;
	ScheduleFlush ();
}

int SettingsManager::GetRetryTimeout () const
{
	return RetryTimeout_;
}

void SettingsManager::SetRetryTimeout (int value)
{
	RetryTimeout_ = value;
	ScheduleFlush ();
}

bool SettingsManager::GetAutoGetFileSize () const
{
	return AutoGetFileSize_;
}

void SettingsManager::SetAutoGetFileSize (bool value)
{
	AutoGetFileSize_ = value;
	ScheduleFlush ();
}

int SettingsManager::GetInterfaceUpdateTimeout () const
{
	return InterfaceUpdateTimeout_;
}

void SettingsManager::SetInterfaceUpdateTimeout (int value)
{
	InterfaceUpdateTimeout_ = value;
	ScheduleFlush ();
}

SettingsItemInfo SettingsManager::GetInfoFor (const QString& propertyname) const
{
	return PropertyInfo_ [propertyname];
}

