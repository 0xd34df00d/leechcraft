#include <QDynamicPropertyChangeEvent>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "xmlsettingsmanager.h"

#define PROP2CHAR(a) (a.toLatin1 ().constData ())

Q_GLOBAL_STATIC (XmlSettingsManager, XmlSettingsManagerInstance);

XmlSettingsManager::XmlSettingsManager ()
{
	BasicSettingsManager::Init ();
}

XmlSettingsManager* XmlSettingsManager::Instance ()
{
    return XmlSettingsManagerInstance ();
}

QSettings* XmlSettingsManager::BeginSettings ()
{
    QSettings *settings = new QSettings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
    settings->beginGroup ("Updater");
    settings->beginGroup ("mainsettings");
	return settings;
}

void XmlSettingsManager::EndSettings (QSettings* settings)
{
	settings->endGroup ();
	settings->endGroup ();
	delete settings;
}

