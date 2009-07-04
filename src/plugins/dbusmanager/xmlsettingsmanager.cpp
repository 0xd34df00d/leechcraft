#include "xmlsettingsmanager.h"
#include <plugininterface/proxy.h>

using LeechCraft::Util::Proxy;
using namespace LeechCraft::Plugins::DBusManager;

XmlSettingsManager::XmlSettingsManager ()
{
	LeechCraft::Util::BaseSettingsManager::Init ();
}

XmlSettingsManager* XmlSettingsManager::Instance ()
{
	static XmlSettingsManager manager;
	return &manager;
}

QSettings* XmlSettingsManager::BeginSettings () const
{
	QSettings *settings = new QSettings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_DBusManager");
	return settings;
}

void XmlSettingsManager::EndSettings (QSettings *settings) const
{
}

