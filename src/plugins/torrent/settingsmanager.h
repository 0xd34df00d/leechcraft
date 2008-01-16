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
				   , LastSaveDirectory_;
	Guarded<IntRange> PortRange_;
	Guarded<bool> DHTEnabled_;
	Guarded<int> AutosaveInterval_
		, DownloadRateLimit_
		, UploadRateLimit_;
	Guarded<double> DesiredRating_;

	QMap<QString, QPair<QObject*, QString> > Property2Object_;
//	Guarded<libtorrent::entry> DHTState_;
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
	int GetAutosaveInterval () const;
	void SetAutosaveInterval (int);

	Q_PROPERTY (IntRange PortRange READ GetPortRange WRITE SetPortRange);
	Q_PROPERTY (bool DHTEnabled READ GetDHTEnabled WRITE SetDHTEnabled);
	Q_PROPERTY (int AutosaveInterval READ GetAutosaveInterval WRITE SetAutosaveInterval);
private:
	void FillMap ();
	void ScheduleSave ();
	void CallSlots (const QString&);
};

#endif

