#include <QDynamicPropertyChangeEvent>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "xmlsettingsmanager.h"

#define PROP2CHAR(a) (a.toLatin1 ().constData ())

Q_GLOBAL_STATIC (XmlSettingsManager, XmlSettingsManagerInstance);

XmlSettingsManager::XmlSettingsManager ()
{
    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
    settings.beginGroup ("Updater");
    settings.beginGroup ("mainsettings");
	QStringList properties = settings.allKeys ();
	qDebug () << properties;
	for (int i = 0; i < properties.size (); ++i)
		setProperty (PROP2CHAR (properties.at (i)), settings.value (properties.at (i)));
	settings.endGroup ();
	settings.endGroup ();
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

bool XmlSettingsManager::event (QEvent *e)
{
	if (e->type () != QEvent::DynamicPropertyChange)
		return false;

	QDynamicPropertyChangeEvent *event = dynamic_cast<QDynamicPropertyChangeEvent*> (e);

	QByteArray name = event->propertyName ();
    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
    settings.beginGroup ("Updater");
    settings.beginGroup ("mainsettings");
	settings.setValue (name, property (name));
	settings.endGroup ();
	settings.endGroup ();

	event->accept ();
	return true;
}

