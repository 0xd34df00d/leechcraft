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

	Guarded<QString> LastTorrentDirectory_
				   , LastSaveDirectory_;

	Guarded<bool> SaveScheduled_;
	Guarded<IntRange> PortRange_;
//	Guarded<libtorrent::entry> DHTState_;
public:
	SettingsManager ();
	~SettingsManager ();
	void Release ();
	static SettingsManager* Instance ();
	SettingsItemInfo GetInfoFor (const QString&) const;
	void ScheduleSave ();
	void ReadSettings ();
private slots:
	void writeSettings ();
public:
	QString GetLastTorrentDirectory () const;
	void SetLastTorrentDirectory (const QString&);
	QString GetLastSaveDirectory () const;
	void SetLastSaveDirectory (const QString&);
//	const libtorrent::entry& GetDHTState () const;
//	void SetDHTState (const libtorrent::entry&);
	IntRange GetPortRange () const;
	void SetPortRange (const IntRange&);

	Q_PROPERTY (IntRange PortRange READ GetPortRange WRITE SetPortRange);
private:
	void FillMap ();
};

#endif

