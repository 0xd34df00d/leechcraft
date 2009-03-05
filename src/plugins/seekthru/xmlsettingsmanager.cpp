#include "xmlsettingsmanager.h"
#include <plugininterface/proxy.h>

using namespace LeechCraft::Util;

XmlSettingsManager::XmlSettingsManager ()
{
	BaseSettingsManager::Init ();
}

XmlSettingsManager::~XmlSettingsManager ()
{
}

XmlSettingsManager& XmlSettingsManager::Instance ()
{
	static XmlSettingsManager xsm;
	return xsm;
}

QSettings* XmlSettingsManager::BeginSettings () const
{
	QSettings *settings = new QSettings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_SeekThru");
	return settings;
}

void XmlSettingsManager::EndSettings (QSettings*) const
{
}

