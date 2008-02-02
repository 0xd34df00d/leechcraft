#include <QSettings>
#include <QDynamicPropertyChangeEvent>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "xmlsettingsmanager.h"

namespace
{
	QSettings *torrentBeginSettings ()
	{
		QSettings *settings = new QSettings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
		settings->beginGroup ("Torrent");
		return settings;
	}

	void torrentEndSettings (QSettings *settings)
	{
		settings->endGroup ();
	}
};

#define PROP2CHAR(a) (a.toLatin1 ().constData ())

Q_GLOBAL_STATIC (XmlSettingsManager, XmlSettingsManagerInstance);

XmlSettingsManager::XmlSettingsManager ()
{
	BaseSettingsManager::Init ();
}

XmlSettingsManager* XmlSettingsManager::Instance ()
{
    return XmlSettingsManagerInstance ();
}

QSettings* XmlSettingsManager::BeginSettings ()
{
	return torrentBeginSettings ();
}

void XmlSettingsManager::EndSettings (QSettings* settings)
{
	return torrentEndSettings (settings);
}

