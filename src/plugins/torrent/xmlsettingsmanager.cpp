#include <QSettings>
#include <QDynamicPropertyChangeEvent>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "xmlsettingsmanager.h"

#define PROP2CHAR(a) (a.toLatin1 ().constData ())

Q_GLOBAL_STATIC (XmlSettingsManager, XmlSettingsManagerInstance);

XmlSettingsManager::XmlSettingsManager ()
{
    QSettings *settings = BeginSettings ();
    QStringList properties = settings->childKeys ();
    Initializing_ = true;
    for (int i = 0; i < properties.size (); ++i)
        setProperty (PROP2CHAR (properties.at (i)), settings->value (properties.at (i), QVariant ()));
    Initializing_ = false;
    EndSettings (settings);
}

XmlSettingsManager* XmlSettingsManager::Instance ()
{
    return XmlSettingsManagerInstance ();
}

void XmlSettingsManager::Release ()
{
    QList<QByteArray> dProperties = dynamicPropertyNames ();
    QSettings *settings = BeginSettings ();
    for (int i = 0; i < dProperties.size (); ++i)
        settings->setValue (dProperties.at (i), property (dProperties.at (i).constData ()));
    EndSettings (settings);
}

void XmlSettingsManager::RegisterObject (const QByteArray& propName, QObject* object, const QByteArray& funcName)
{
    Properties2Object_.insert (propName, qMakePair (object, funcName));
}

QVariant XmlSettingsManager::Property (const QString& propName, const QVariant& def)
{
    QVariant result = property (PROP2CHAR (propName));
    if (!result.isValid ())
    {
        result = def;
        setProperty (PROP2CHAR (propName), def);
    }

    return result;
}

bool XmlSettingsManager::event (QEvent *e)
{
    if (e->type () != QEvent::DynamicPropertyChange)
        return false;

    QDynamicPropertyChangeEvent *event = dynamic_cast<QDynamicPropertyChangeEvent*> (e);

    QByteArray name = event->propertyName ();
    QSettings *settings = BeginSettings ();
    settings->setValue (name, property (name));
    EndSettings (settings);

    if (Properties2Object_.contains (name))
    {
        QPair<QObject*, QByteArray> object = Properties2Object_ [name];
        object.first->metaObject ()->invokeMethod (object.first, object.second);
    }

    event->accept ();
    return true;
}


QSettings* XmlSettingsManager::BeginSettings ()
{
    QSettings *settings = new QSettings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
    settings->beginGroup ("Torrent");
    return settings;
}

void XmlSettingsManager::EndSettings (QSettings* settings)
{
    settings->endGroup ();
//    delete settings;
//    settings = 0;
}

