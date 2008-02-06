#include <QSettings>
#include <QDynamicPropertyChangeEvent>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "xmlsettingsmanager.h"

namespace
{
   QSettings *httpBeginSettings ()
   {
      QSettings *settings = new QSettings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
      settings->beginGroup ("HTTP and FTP");
      settings->beginGroup ("Mainsettings");
      return settings;
   }

   void httpEndSettings (QSettings *settings)
   {
      settings->endGroup ();
      settings->endGroup ();
   }
};

#define PROP2CHAR(a) (a.toLatin1 ().constData ())

Q_GLOBAL_STATIC (XmlSettingsManager, XmlSettingsManagerInstance);

XmlSettingsManager::XmlSettingsManager ()
{
   BaseSettingsManager::Init ();
}

XmlSettingsManager* XmlSettingsManager::Instance ()
{
    return XmlSettingsManagerInstance ();
}

QSettings* XmlSettingsManager::BeginSettings () const
{
   return httpBeginSettings ();
}

void XmlSettingsManager::EndSettings (QSettings* settings) const
{
   httpEndSettings (settings);
}

