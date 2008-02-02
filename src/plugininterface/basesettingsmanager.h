#ifndef BASESETTINGSMANAGER_H
#define BASESETTINGSMANAGER_H
#include <QMap>
#include <QPair>
#include <QObject>
#include <QSettings>
#include <QDynamicPropertyChangeEvent>
#include <interfaces/interfaces.h>

#define PROP2CHAR(a) (a.toLatin1 ().constData ())

class BaseSettingsManager : public QObject
{
	Q_OBJECT

	QMap<QByteArray, QPair<QObject*, QByteArray> > Properties2Object_;
	bool Initializing_;
public:
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

	void Release ()
	{
		QList<QByteArray> dProperties = dynamicPropertyNames ();
		QSettings *settings = BeginSettings ();
		for (int i = 0; i < dProperties.size (); ++i)
			settings->setValue (dProperties.at (i), property (dProperties.at (i).constData ()));
		EndSettings (settings);
//		delete settings;
//		settings = 0;
	}

	void RegisterObject (const QByteArray& propName, QObject* object, const QByteArray& funcName)
	{
		Properties2Object_.insert (propName, qMakePair (object, funcName));
	}

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

	virtual QSettings* BeginSettings () = 0;
	virtual void EndSettings (QSettings*) = 0;
};

#endif

