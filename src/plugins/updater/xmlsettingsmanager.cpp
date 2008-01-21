#include "xmlsettingsmanager.h"

Q_GLOBAL_STATIC (XmlSettingsManager, XmlSettingsManagerInstance);

XmlSettingsManager::XmlSettingsManager ()
{
}

XmlSettingsManager::~XmlSettingsManager ()
{
}

void XmlSettingsManager::Release ()
{
}

XmlSettingsManager* XmlSettingsManager::Instance ()
{
    return XmlSettingsManagerInstance ();
}
