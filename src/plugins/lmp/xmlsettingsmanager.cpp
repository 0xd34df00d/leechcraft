#include <plugininterface/proxy.h>
#include "xmlsettingsmanager.h"

using LeechCraft::Util::Proxy;
using namespace LeechCraft::Plugins::LMP;

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
			Proxy::Instance ()->GetApplicationName () + "_LMP");
	return settings;
}

void XmlSettingsManager::EndSettings (QSettings*) const
{
}

