#include "xmlsettingsmanager.h"
#include <plugininterface/proxy.h>

using LeechCraft::Util::Proxy;
using namespace LeechCraft::Plugins::DBusManager;

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
	QSettings *settings = new QSettings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_DBusManager");
	return settings;
}

void XmlSettingsManager::EndSettings (QSettings *settings) const
{
}

