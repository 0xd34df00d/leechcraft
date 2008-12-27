#include <QSettings>
#include <QDynamicPropertyChangeEvent>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	XmlSettingsManager::XmlSettingsManager ()
	{
		Util::BaseSettingsManager::Init ();
	}

	XmlSettingsManager::~XmlSettingsManager ()
	{
	}

	XmlSettingsManager* XmlSettingsManager::Instance ()
	{
		static XmlSettingsManager manager;
		return &manager;
	}

    QSettings* XmlSettingsManager::BeginSettings () const
    {
        QSettings *settings = new QSettings (Util::Proxy::Instance ()->GetOrganizationName (),
				Util::Proxy::Instance ()->GetApplicationName ());
        return settings;
    }

    void XmlSettingsManager::EndSettings (QSettings*) const
    {
    }
};

