#include <QDynamicPropertyChangeEvent>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "basicsettingsmanager.h"

#define PROP2CHAR(a) (a.toLatin1 ().constData ())

BasicSettingsManager::BasicSettingsManager ()
{
}

BasicSettingsManager::~BasicSettingsManager ()
{
}

void BasicSettingsManager::Init ()
{
    QSettings *settings = this->BeginSettings ();
    QStringList properties = settings->allKeys ();
    for (int i = 0; i < properties.size (); ++i)
        setProperty (PROP2CHAR (properties.at (i)), settings->value (properties.at (i), QVariant ()));
    this->EndSettings (settings);
}

void BasicSettingsManager::Release ()
{
    QList<QByteArray> dProperties = dynamicPropertyNames ();
    QSettings *settings = BeginSettings ();
    for (int i = 0; i < dProperties.size (); ++i)
        settings->setValue (dProperties.at (i), property (dProperties.at (i)));
    EndSettings (settings);
}

bool BasicSettingsManager::event (QEvent *e)
{
    if (e->type () != QEvent::DynamicPropertyChange)
        return false;

    QDynamicPropertyChangeEvent *event = dynamic_cast<QDynamicPropertyChangeEvent*> (e);

    QByteArray name = event->propertyName ();
    QSettings *settings = BeginSettings ();
    settings->setValue (name, property (name));
    EndSettings (settings);

    event->accept ();
    return true;
}

QSettings* BasicSettingsManager::BeginSettings ()
{}

void BasicSettingsManager::EndSettings (QSettings*)
{}

