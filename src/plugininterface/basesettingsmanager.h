#ifndef BASESETTINGSMANAGER_H
#define BASESETTINGSMANAGER_H
#include <QMap>
#include <QPair>
#include <QObject>
#include <QSettings>
#include <QDynamicPropertyChangeEvent>
#include <interfaces/interfaces.h>

#define PROP2CHAR(a) (a.toLatin1 ().constData ())

/*! @brief Base class for settings manager.
 *
 * Facilitates creation of settings managers due to providing some
 * frequently used features.
 */
class BaseSettingsManager : public QObject
{
    Q_OBJECT

    QMap<QByteArray, QPair<QObject*, QByteArray> > Properties2Object_;
    bool Initializing_;
public:
 /*! @brief Initalizes the settings manager.
  *
  * Loads all settings from the QSettings created by BeginSettings and
  * creates dynamic properties for them.
  *
  * @sa Release
  */
    void Init ()
    {
        QSettings *settings = BeginSettings ();
        QStringList properties = settings->childKeys ();
        Initializing_ = true;
        for (int i = 0; i < properties.size (); ++i)
            setProperty (PROP2CHAR (properties.at (i)), settings->value (properties.at (i), QVariant ()));
        Initializing_ = false;
        EndSettings (settings);
        delete settings;
        settings = 0;
    }

 /*! @brief Prepares the settings manager for deletion.
  *
  * Flushes all settigns to the QSettings created by BeginSettings
  * to prepare settings manager object for the deletion.
  *
  * @sa Init
  */
    void Release ()
    {
        QList<QByteArray> dProperties = dynamicPropertyNames ();
        QSettings *settings = BeginSettings ();
        for (int i = 0; i < dProperties.size (); ++i)
            settings->setValue (dProperties.at (i), property (dProperties.at (i).constData ()));
        EndSettings (settings);
    }

 /*! @brief Subscribes object to property changes.
  *
  * When a property changes, a specified object is called to notify
  * it that the property has changed.
  *
  * @param[in] propName The name of property object wants to
  * subscribe to.
  * @param[in] object The object instance that will get
  * notifications.
  * @param[in] funcName Name of the function that will be called.
  * Note that it should be known to the Qt's metaobject system, so
  * it should be a (public) slot.
  */
    void RegisterObject (const QByteArray& propName, QObject* object, const QByteArray& funcName)
    {
        Properties2Object_.insert (propName, qMakePair (object, funcName));
    }

 /*! @brief Gets a property with default value.
  *
  * This is a wrapper around standart QObject::property() function.
  * It checks whether specified property exists, and if so, it
  * returns its value, otherwise it creates this property, sets its
  * value to def and returns def.
  *
  * @param[in] propName Name of the property that should be checked
  * and returned.
  * @param[in] def Default value of the property.
  * @return Resulting value of the property.
  */
    QVariant Property (const QString& propName, const QVariant& def)
    {
        QVariant result = property (PROP2CHAR (propName));
        if (!result.isValid ())
        {
            result = def;
            setProperty (PROP2CHAR (propName), def);
        }

        return result;
    }
protected:
    virtual bool event (QEvent *e)
    {
        if (e->type () != QEvent::DynamicPropertyChange)
            return false;

        QDynamicPropertyChangeEvent *event = dynamic_cast<QDynamicPropertyChangeEvent*> (e);

        QByteArray name = event->propertyName ();
        QSettings *settings = BeginSettings ();
        settings->setValue (name, property (name));
        EndSettings (settings);
        delete settings;
        settings = 0;

        if (Properties2Object_.contains (name))
        {
            QPair<QObject*, QByteArray> object = Properties2Object_ [name];
            object.first->metaObject ()->invokeMethod (object.first, object.second);
        }

        event->accept ();
        return true;
    }

 /*! @brief Allocates and returns a QSettings object suitable for
  * use.
  *
  * @return The created QSettings object.
  * @sa EndSettings
  */
    virtual QSettings* BeginSettings () const = 0;
 /*! @brief Correctly closes the QSettings object.
  *
  * Closes the QSettings object previously created by
  * BeginSettings. It should NOT delete it, BaseSettignsManager's
  * code would do that.
  *
  * @param[in] settings The QSettings object.
  * @sa BeginSettings
  */
    virtual void EndSettings (QSettings *settings) const = 0;
};

#endif

