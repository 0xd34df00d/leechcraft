#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H
#include <QObject>
#include <interfaces/interfaces.h>
#include <settingsdialog/settingsiteminfo.h>
#include <settingsdialog/datatypes.h>
#include <plugininterface/guarded.h>

class SettingsManager : public QObject
					  , public ISettings
{
	Q_OBJECT
	Q_INTERFACES (ISettings);

	Guarded<QString> LastTorrentDirectory_
				   , LastSaveDirectory_;

	Guarded<bool> SaveScheduled_;
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
};

#endif

