#include "xmlsettingsmanager.h"
#include <plugininterface/proxy.h>

using LeechCraft::Util::Proxy;

namespace
{
    QSettings *torrentBeginSettings ()
    {
        QSettings *settings = new QSettings (Proxy::Instance ()->GetOrganizationName (),
				Proxy::Instance ()->GetApplicationName () + "_Poshuku_FUA");
        return settings;
    }

    void torrentEndSettings (QSettings *settings)
    {
    }
};

#define PROP2CHAR(a) (a.toLatin1 ().constData ())

Q_GLOBAL_STATIC (XmlSettingsManager, XmlSettingsManagerInstance);

XmlSettingsManager::XmlSettingsManager ()
{
	LeechCraft::Util::BaseSettingsManager::Init ();
}

XmlSettingsManager* XmlSettingsManager::Instance ()
{
    return XmlSettingsManagerInstance ();
}

QSettings* XmlSettingsManager::BeginSettings () const
{
    return torrentBeginSettings ();
}

void XmlSettingsManager::EndSettings (QSettings* settings) const
{
    return torrentEndSettings (settings);
}

