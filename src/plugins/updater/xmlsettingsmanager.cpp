#include <QDynamicPropertyChangeEvent>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "xmlsettingsmanager.h"

namespace
{
    QSettings *updaterBeginSettings ()
    {
        QSettings *settings = new QSettings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
        settings->beginGroup ("Updater");
        settings->beginGroup ("mainsettings");
        return settings;
    }
    void updaterEndSettings (QSettings *settings)
    {
        settings->endGroup ();
        settings->endGroup ();
    }
};

Q_GLOBAL_STATIC (XmlSettingsManager, XmlSettingsManagerInstance);

XmlSettingsManager::XmlSettingsManager ()
{
    BaseSettingsManager::Init ();
}

XmlSettingsManager* XmlSettingsManager::Instance ()
{
    return XmlSettingsManagerInstance ();
}

QSettings* XmlSettingsManager::BeginSettings () const
{
    return updaterBeginSettings ();
}

void XmlSettingsManager::EndSettings (QSettings* settings) const
{
    updaterEndSettings (settings);
}

