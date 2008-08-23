#include <QSettings>
#include <QDynamicPropertyChangeEvent>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "xmlsettingsmanager.h"

namespace Main
{
	XmlSettingsManager::XmlSettingsManager ()
	{
		BaseSettingsManager::Init ();
	}

	XmlSettingsManager* XmlSettingsManager::Instance ()
	{
		static XmlSettingsManager manager;
		return &manager;
	}

    QSettings* XmlSettingsManager::BeginSettings () const
    {
        QSettings *settings = new QSettings (Proxy::Instance ()->GetOrganizationName (),
				Proxy::Instance ()->GetApplicationName ());
        return settings;
    }

    void XmlSettingsManager::EndSettings (QSettings* settings) const
    {
    }
};

