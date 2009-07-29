#include "basesettingsmanager.h"
#include <QtDebug>

using namespace LeechCraft::Util;

void BaseSettingsManager::Init ()
{
	Settings_ = BeginSettings ();
	QStringList properties = Settings_->childKeys ();
	Initializing_ = true;
	for (int i = 0; i < properties.size (); ++i)
		setProperty (PROP2CHAR (properties.at (i)), Settings_->value (properties.at (i), QVariant ()));
	Initializing_ = false;
}

void BaseSettingsManager::Release ()
{
	if (!Settings_)
	{
		qWarning () << Q_FUNC_INFO << "already released";
		return;
	}

	QList<QByteArray> dProperties = dynamicPropertyNames ();
	for (int i = 0; i < dProperties.size (); ++i)
		Settings_->setValue (dProperties.at (i),
				property (dProperties.at (i).constData ()));
	EndSettings (Settings_);
	delete Settings_;
	Settings_ = 0;
}

void BaseSettingsManager::RegisterObject (const QByteArray& propName,
		QObject* object, const QByteArray& funcName)
{
	Properties2Object_.insertMulti (propName, qMakePair (object, funcName));
}

void BaseSettingsManager::RegisterObject (const QList<QByteArray>& propNames,
		QObject* object, const QByteArray& funcName)
{
	for (QList<QByteArray>::const_iterator i = propNames.begin (),
			end = propNames.end (); i != end; ++i)
		RegisterObject (*i, object, funcName);
}

QVariant BaseSettingsManager::Property (const QString& propName, const QVariant& def)
{
	QVariant result = property (PROP2CHAR (propName));
	if (!result.isValid ())
	{
		result = def;
		setProperty (PROP2CHAR (propName), def);
	}

	return result;
}

bool BaseSettingsManager::event (QEvent *e)
{
	if (e->type () != QEvent::DynamicPropertyChange)
		return false;

	QDynamicPropertyChangeEvent *event = dynamic_cast<QDynamicPropertyChangeEvent*> (e);

	QByteArray name = event->propertyName ();
	Settings_->setValue (name, property (name));

	if (Properties2Object_.contains (name))
	{
		QList<QPair<QObject*, QByteArray> > objects = Properties2Object_.values (name);
		QPair<QObject*, QByteArray> object;
		Q_FOREACH (object, objects)
			if (!object.first->metaObject ()->invokeMethod (object.first, object.second))
				qWarning () << Q_FUNC_INFO
					<< "could not find method in the metaobject"
					<< name
					<< object.first
					<< object.second;
	}

	event->accept ();
	return true;
}

