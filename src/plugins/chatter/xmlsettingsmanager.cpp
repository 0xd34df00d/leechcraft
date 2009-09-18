#include "xmlsettingsmanager.h"
#include <QCoreApplication>

using namespace LeechCraft::Plugins::Chatter;

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
	QSettings *settings = new QSettings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Chatter");
	return settings;
}

void XmlSettingsManager::EndSettings (QSettings *settings) const
{
}

