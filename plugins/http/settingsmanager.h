#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H
#include <QMap>
#include <QPair>
#include <interfaces/interfaces.h>
#include <settingsdialog/settingsiteminfo.h>
#include <settingsdialog/datatypes.h>
#include <plugininterface/guarded.h>
#include "globals.h"

class Proxy;
class QMutex;

class SettingsManager : public QObject
					  , public ISettings
{
	Q_OBJECT;
	Q_INTERFACES (ISettings);

	SettingsManager ();
	static SettingsManager *Instance_;
	QMap<QString, SettingsItemInfo> PropertyInfo_;
	static QMutex *InstanceMutex_;
	bool SaveChangesScheduled_;
protected:
	Guarded<int> ConnectTimeout_;
	Guarded<int> DefaultTimeout_;
	Guarded<bool> ProxyEnabled_;
	Guarded<QString> ProxyAddress_;
	Guarded<int> ProxyPort_;
	Guarded<QString> Login_;
	Guarded<QString> Password_;
	Guarded<int> CacheSize_;
	Guarded<int> StopTimeout_;
	Guarded<QString> DownloadDir_;
	Guarded<bool> AutostartChildren_;
	Guarded<PairedStringList> UserAgent_;
private:
	void ReadSettings ();
	void WriteSettings ();
	void InitializeMap ();
	void ScheduleFlush ();
public slots:
	void flush ();
public:
	~SettingsManager ();
	static SettingsManager* Instance ();
	static void Release ();

	QString GetDownloadDir () const;
	void SetDownloadDir (QString);
	int GetConnectTimeout () const;
	void SetConnectTimeout (int);
	int GetDefaultTimeout () const;
	void SetDefaultTimeout (int);
	int GetStopTimeout () const;
	void SetStopTimeout (int);
	bool GetProxyEnabled () const;
	void SetProxyEnabled (bool);
	const QString& GetProxyAddress () const;
	void SetProxyAddress (const QString&);
	int GetProxyPort () const;
	void SetProxyPort (int);
	const QString& GetResourceLogin () const;
	void SetResourceLogin (const QString&);
	const QString& GetResourcePassword () const;
	void SetResourcePassword (const QString&);
	int GetCacheSize () const;
	void SetCacheSize (int);
	bool GetAutostartChildren () const;
	void SetAutostartChildren (bool);
	PairedStringList GetUserAgent () const;
	void SetUserAgent (const PairedStringList&);

	SettingsItemInfo GetInfoFor (const QString&) const;

	Q_PROPERTY (QString DownloadDir READ GetDownloadDir WRITE SetDownloadDir);
	Q_PROPERTY (int ConnectTimeout READ GetConnectTimeout WRITE SetConnectTimeout);
	Q_PROPERTY (int DefaultTimeout READ GetDefaultTimeout WRITE SetDefaultTimeout);
	Q_PROPERTY (int StopTimeout READ GetStopTimeout WRITE SetStopTimeout);
	Q_PROPERTY (bool ProxyEnabled READ GetProxyEnabled WRITE SetProxyEnabled);
	Q_PROPERTY (QString ProxyAddress READ GetProxyAddress WRITE SetProxyAddress);
	Q_PROPERTY (int ProxyPort READ GetProxyPort WRITE SetProxyPort);
	Q_PROPERTY (PairedStringList UserAgent READ GetUserAgent WRITE SetUserAgent);
	Q_PROPERTY (QString ResourceLogin READ GetResourceLogin WRITE SetResourceLogin);
	Q_PROPERTY (QString ResourcePassword READ GetResourcePassword WRITE SetResourcePassword);
	Q_PROPERTY (int CacheSize READ GetCacheSize WRITE SetCacheSize);
	Q_PROPERTY (bool AutostartChildren READ GetAutostartChildren WRITE SetAutostartChildren);
};

#endif

