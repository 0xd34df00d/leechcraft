/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "basesettingsmanager.h"
#include <QtDebug>

using namespace LeechCraft::Util;

BaseSettingsManager::BaseSettingsManager (bool readAllKeys, QObject *parent)
: QObject (parent)
, Settings_ (0)
, ReadAllKeys_ (readAllKeys)
{
}

void BaseSettingsManager::Init ()
{
	Settings_ = BeginSettings ();
	QStringList properties = ReadAllKeys_ ?
			Settings_->allKeys () :
			Settings_->childKeys ();
	Initializing_ = true;
	for (int i = 0; i < properties.size (); ++i)
		setProperty (PROP2CHAR (properties.at (i)),
				Settings_->value (properties.at (i)));
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
	Properties2Object_.insertMulti (propName,
			qMakePair (QPointer<QObject> (object), funcName));
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

void BaseSettingsManager::SetRawValue (const QString& path, const QVariant& val)
{
	Settings_->setValue (path, val);
}

QVariant BaseSettingsManager::GetRawValue (const QString& path, const QVariant& def) const
{
	return Settings_->value (path, def);
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
		QList<QPair<QPointer<QObject>, QByteArray> > objects = Properties2Object_.values (name);
		QPair<QPointer<QObject>, QByteArray> object;
		Q_FOREACH (object, objects)
		{
			if (!object.first)
				continue;

			if (!object.first->metaObject ()->invokeMethod (object.first, object.second))
				qWarning () << Q_FUNC_INFO
					<< "could not find method in the metaobject"
					<< name
					<< object.first
					<< object.second;
		}
	}

	event->accept ();
	return true;
}

