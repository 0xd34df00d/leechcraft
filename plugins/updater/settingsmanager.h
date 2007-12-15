#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H
#include <QString>
#include <interfaces/interfaces.h>
#include <settingsdialog/settingsiteminfo.h>
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
	Guarded<QStringList> Mirrors_;
	Guarded<bool> SaveDownloadedInHistory_;
private:
	void ReadSettings ();
	void WriteSettings ();
	void InitializeMap ();
	void ScheduleFlush ();
public slots:
	void flush ();
public:
	virtual ~SettingsManager ();
	static SettingsManager* Instance ();
	static void Release ();

	const QStringList& GetMirrors () const;
	void SetMirrors (const QStringList& mirrors);
	bool GetSaveDownloadedInHistory () const;
	void SetSaveDownloadedInHistory (bool);

	SettingsItemInfo GetInfoFor (const QString&) const;

	Q_PROPERTY (bool SaveDownloadedInHistory READ GetSaveDownloadedInHistory WRITE SetSaveDownloadedInHistory);
	Q_PROPERTY (QStringList Mirrors READ GetMirrors WRITE SetMirrors);
};

#endif

