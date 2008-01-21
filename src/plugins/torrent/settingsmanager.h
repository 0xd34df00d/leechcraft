#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H
#include <QObject>
#include <QPair>
#include <entry.hpp>
#include <interfaces/interfaces.h>
#include <settingsdialog/settingsiteminfo.h>
#include <settingsdialog/datatypes.h>
#include <plugininterface/guarded.h>

class SettingsManager : public QObject
					  , public ISettings
{
	Q_OBJECT
	Q_INTERFACES (ISettings);

	QMap<QString, SettingsItemInfo> PropInfos_;
	Guarded<bool> SaveScheduled_;

	Guarded<QString> LastTorrentDirectory_
		, LastMakeTorrentDirectory_
		, LastAddDirectory_
		, LastSaveDirectory_
		, TrackerProxyAddress_
		, TrackerProxyLogin_
		, TrackerProxyPassword_
		, PeerProxyAddress_
		, PeerProxyLogin_
		, PeerProxyPassword_;
	Guarded<IntRange> PortRange_;
	Guarded<bool> DHTEnabled_
		, TrackerProxyEnabled_
		, PeerProxyEnabled_;
	Guarded<int> AutosaveInterval_
		, DownloadRateLimit_
		, UploadRateLimit_
		, MaxUploads_
		, MaxConnections_
		, TrackerProxyPort_
		, PeerProxyPort_;
	Guarded<double> DesiredRating_;

	QMap<QString, QPair<QObject*, QString> > Property2Object_;
public:
	SettingsManager ();
	~SettingsManager ();
	void Release ();
	static SettingsManager* Instance ();
	void RegisterObject (const QString&, QObject*, const QString&);
	SettingsItemInfo GetInfoFor (const QString&) const;
	void ReadSettings ();
private slots:
	void writeSettings ();
public:
	QString GetLastTorrentDirectory () const;
	void SetLastTorrentDirectory (const QString&);
	QString GetLastSaveDirectory () const;
	void SetLastSaveDirectory (const QString&);
	QString GetLastMakeTorrentDirectory () const;
	void SetLastMakeTorrentDirectory (const QString&);
	QString GetLastAddDirectory () const;
	void SetLastAddDirectory (const QString&);
	int GetUploadRateLimit () const;
	void SetUploadRateLimit (int);
	int GetDownloadRateLimit () const;
	void SetDownloadRateLimit (int);
	double GetDesiredRating () const;
	void SetDesiredRating (double);

//	const libtorrent::entry& GetDHTState () const;
//	void SetDHTState (const libtorrent::entry&);

	IntRange GetPortRange () const;
	void SetPortRange (const IntRange&);
	bool GetDHTEnabled () const;
	void SetDHTEnabled (bool);
	int GetMaxUploads () const;
	void SetMaxUploads (int);
	int GetMaxConnections () const;
	void SetMaxConnections (int);
	bool GetTrackerProxyEnabled () const;
	void SetTrackerProxyEnabled (bool);
	QString GetTrackerProxyAddress () const;
	void SetTrackerProxyAddress (QString);
	int GetTrackerProxyPort () const;
	void SetTrackerProxyPort (int);
	QString GetTrackerProxyLogin () const;
	void SetTrackerProxyLogin (QString);
	QString GetTrackerProxyPassword () const;
	void SetTrackerProxyPassword (QString);
	bool GetPeerProxyEnabled () const;
	void SetPeerProxyEnabled (bool);
	QString GetPeerProxyAddress () const;
	void SetPeerProxyAddress (QString);
	int GetPeerProxyPort () const;
	void SetPeerProxyPort (int);
	QString GetPeerProxyLogin () const;
	void SetPeerProxyLogin (QString);
	QString GetPeerProxyPassword () const;
	void SetPeerProxyPassword (QString);
	int GetAutosaveInterval () const;
	void SetAutosaveInterval (int);

	Q_PROPERTY (IntRange PortRange READ GetPortRange WRITE SetPortRange);
	Q_PROPERTY (bool DHTEnabled READ GetDHTEnabled WRITE SetDHTEnabled);
	Q_PROPERTY (int MaxUploads READ GetMaxUploads WRITE SetMaxUploads);
	Q_PROPERTY (int MaxConnections READ GetMaxConnections WRITE SetMaxConnections);
	Q_PROPERTY (bool TrackerProxyEnabled READ GetTrackerProxyEnabled WRITE SetTrackerProxyEnabled);
	Q_PROPERTY (QString TrackerProxyAddress READ GetTrackerProxyAddress WRITE SetTrackerProxyAddress);
	Q_PROPERTY (int TrackerProxyPort READ GetTrackerProxyPort WRITE SetTrackerProxyPort);
	Q_PROPERTY (QString TrackerProxyLogin READ GetTrackerProxyLogin WRITE SetTrackerProxyLogin);
	Q_PROPERTY (QString TrackerProxyPassword READ GetTrackerProxyPassword WRITE SetTrackerProxyPassword);
	Q_PROPERTY (bool PeerProxyEnabled READ GetPeerProxyEnabled WRITE SetPeerProxyEnabled);
	Q_PROPERTY (QString PeerProxyAddress READ GetPeerProxyAddress WRITE SetPeerProxyAddress);
	Q_PROPERTY (int PeerProxyPort READ GetPeerProxyPort WRITE SetPeerProxyPort);
	Q_PROPERTY (QString PeerProxyLogin READ GetPeerProxyLogin WRITE SetPeerProxyLogin);
	Q_PROPERTY (QString PeerProxyPassword READ GetPeerProxyPassword WRITE SetPeerProxyPassword);

	Q_PROPERTY (int AutosaveInterval READ GetAutosaveInterval WRITE SetAutosaveInterval);
private:
	void FillMap ();
	void ScheduleSave ();
	void CallSlots (const QString&);
};

#endif

