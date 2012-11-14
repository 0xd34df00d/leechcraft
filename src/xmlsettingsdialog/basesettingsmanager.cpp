/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

namespace LeechCraft
{
namespace Util
{
	BaseSettingsManager::BaseSettingsManager (bool readAllKeys, QObject *parent)
	: QObject (parent)
	, ReadAllKeys_ (readAllKeys)
	{
	}

	void BaseSettingsManager::Init ()
	{
		auto settings = GetSettings ();
		const auto& properties = ReadAllKeys_ ?
				settings->allKeys () :
				settings->childKeys ();
		for (const auto& prop : properties)
			setProperty (PROP2CHAR (prop), settings->value (prop));
	}

	void BaseSettingsManager::Release ()
	{
		auto settings = GetSettings ();

		for (const auto& dProp : dynamicPropertyNames ())
			settings->setValue (QString::fromUtf8 (dProp), property (dProp.constData ()));
	}

	void BaseSettingsManager::RegisterObject (const QByteArray& propName,
			QObject *object, const QByteArray& funcName, EventFlags flags)
	{
		if (flags & EventFlag::Apply)
			ApplyProps_.insertMulti (propName, qMakePair (QPointer<QObject> (object), funcName));
		if (flags & EventFlag::Select)
			SelectProps_.insertMulti (propName, qMakePair (QPointer<QObject> (object), funcName));

		connect (object,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (cleanupObjects ()),
				Qt::UniqueConnection);
	}

	void BaseSettingsManager::RegisterObject (const QList<QByteArray>& propNames,
			QObject* object, const QByteArray& funcName, EventFlags flags)
	{
		for (auto i = propNames.begin (), end = propNames.end (); i != end; ++i)
			RegisterObject (*i, object, funcName, flags);
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
		GetSettings ()->setValue (path, val);
	}

	QVariant BaseSettingsManager::GetRawValue (const QString& path, const QVariant& def) const
	{
		return GetSettings ()->value (path, def);
	}

	void BaseSettingsManager::OptionSelected (const QByteArray& prop, const QVariant& val)
	{
		if (!SelectProps_.contains (prop))
			return;

		const auto& objects = SelectProps_.values (prop);
		Q_FOREACH (const ObjectElement_t& object, objects)
		{
			if (!object.first)
				continue;

			if (!QMetaObject::invokeMethod (object.first,
						object.second,
						Q_ARG (QVariant, val)))
				qWarning () << Q_FUNC_INFO
					<< "could not find method in the metaobject"
					<< prop
					<< object.first
					<< object.second;
		}
	}

	bool BaseSettingsManager::event (QEvent *e)
	{
		if (e->type () != QEvent::DynamicPropertyChange)
			return false;

		auto event = dynamic_cast<QDynamicPropertyChangeEvent*> (e);

		const QByteArray& name = event->propertyName ();
		const auto& propName = QString::fromUtf8 (name);
		const auto& propValue = property (name);
		GetSettings ()->setValue (propName, propValue);
		PropertyChanged (propName, propValue);

		if (ApplyProps_.contains (name))
		{
			const auto& objects = ApplyProps_.values (name);
			Q_FOREACH (const auto& object, objects)
			{
				if (!object.first)
					continue;

				if (!QMetaObject::invokeMethod (object.first, object.second))
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

	void BaseSettingsManager::PropertyChanged (const QString&, const QVariant&)
	{
	}

	Settings_ptr BaseSettingsManager::GetSettings() const
	{
		return Settings_ptr (BeginSettings (),
				[this] (QSettings *settings)
				{
					EndSettings (settings);
					delete settings;
				});
	}

	void BaseSettingsManager::cleanupObjects ()
	{
		auto senderObj = sender ();
		auto cleanupMap = [senderObj] (Properties2Object_t& map) -> void
		{
			for (const auto& key : map.keys ())
			{
				decltype (map.values (key)) vals2remove;
				for (const auto& val : map.values (key))
					if (!val.first || val.first == senderObj)
						vals2remove << val;

				for (const auto& val : vals2remove)
					map.remove (key, val);
			}
		};

		cleanupMap (ApplyProps_);
		cleanupMap (SelectProps_);
	}
}
}
