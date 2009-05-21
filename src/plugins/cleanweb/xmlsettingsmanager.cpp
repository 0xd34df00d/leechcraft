#include <plugininterface/proxy.h>
#include "xmlsettingsmanager.h"

using LeechCraft::Util::Proxy;
using namespace LeechCraft::Plugins::CleanWeb;

XmlSettingsManager::XmlSettingsManager ()
{
	LeechCraft::Util::BaseSettingsManager::Init ();
}

XmlSettingsManager& XmlSettingsManager::Instance ()
{
	static XmlSettingsManager xsm;
	return xsm;
}

QSettings* XmlSettingsManager::BeginSettings () const
{
	return new QSettings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_CleanWeb");
}

void XmlSettingsManager::EndSettings (QSettings*) const
{
}

