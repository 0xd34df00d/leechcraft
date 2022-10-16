/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "basesettingsmanager.h"
#include <QtDebug>
#include <QTimer>
#include <util/sll/visitor.h>
#include "settingsthreadmanager.h"

#define PROP2CHAR(a) (a.toUtf8 ().constData ())

namespace LC::Util
{
	BaseSettingsManager::BaseSettingsManager (QObject *parent)
	: QObject { parent }
	{
	}

	BaseSettingsManager::BaseSettingsManager (bool readAllKeys, QObject *parent)
	: QObject { parent }
	, ReadAllKeys_ { readAllKeys }
	{
	}

	void BaseSettingsManager::Init ()
	{
		IsInitializing_ = true;

		auto settings = GetSettings ();
		const auto& properties = ReadAllKeys_ ?
				settings->allKeys () :
				settings->childKeys ();
		for (const auto& prop : properties)
			setProperty (PROP2CHAR (prop), settings->value (prop));

		IsInitializing_ = false;
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
		RegisterObjectImpl (propName, object, funcName, flags);
	}

	void BaseSettingsManager::RegisterObject (const QByteArray& propName,
			QObject *object, const std::function<void () >& func, EventFlags flags)
	{
		RegisterObject (propName, object, [func] (const QVariant&) { func (); }, flags);
	}

	void BaseSettingsManager::RegisterObject (const QByteArray& propName,
			QObject *object, const VariantHandler_f& func, EventFlags flags)
	{
		RegisterObjectImpl (propName, object, func, flags);
	}

	void BaseSettingsManager::RegisterObject (const QList<QByteArray>& propNames,
			QObject* object, const QByteArray& funcName, EventFlags flags)
	{
		for (const auto& prop : propNames)
			RegisterObject (prop, object, funcName, flags);
	}

	void BaseSettingsManager::RegisterObject (const QList<QByteArray>& propNames,
			QObject *object, const std::function<void ()>& func, EventFlags flags)
	{
		const std::function<void (QVariant)> wrapper = [func] (const QVariant&) { func (); };
		for (const auto& prop : propNames)
			RegisterObject (prop, object, wrapper, flags);
	}

	QVariant BaseSettingsManager::Property (std::string_view propName, const QVariant& def)
	{
		auto result = property (propName.data ());
		if (!result.isValid ())
		{
			result = def;
			setProperty (propName.data (), def);
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

	void BaseSettingsManager::ShowSettingsPage (const QString& optionName)
	{
		emit showPageRequested (this, optionName);
	}

	void BaseSettingsManager::OptionSelected (const QByteArray& prop, const QVariant& val)
	{
		for (const auto& object : SelectProps_.value (prop))
		{
			if (!object.first)
				continue;

			Visit (object.second,
					[&val] (const VariantHandler_f& func) { func (val); },
					[&] (const QByteArray& methodName)
					{
						if (!QMetaObject::invokeMethod (object.first,
								methodName,
								Q_ARG (QVariant, val)))
							qWarning () << Q_FUNC_INFO
									<< "could not find method in the metaobject"
									<< prop
									<< object.first
									<< methodName;
					});
		}
	}

	std::shared_ptr<void> BaseSettingsManager::EnterInitMode ()
	{
		if (IsInitializing_)
			return {};

		IsInitializing_ = true;
		return std::shared_ptr<void> (nullptr, [this] (void*) { IsInitializing_ = false; });
	}

	namespace
	{
		template<typename H>
		void PerformApply (const QByteArray& propName,
				const QVariant& propValue, QObject *propContext, const H& handler)
		{
			Visit (handler,
					[&propValue] (const BaseSettingsManager::VariantHandler_f& func) { func (propValue); },
					[&] (const QByteArray& methodName)
					{
						if (!QMetaObject::invokeMethod (propContext, methodName))
							qWarning () << Q_FUNC_INFO
								<< "could not find method in the metaobject"
								<< propName
								<< propContext
								<< methodName;
					});
		}
	}

	bool BaseSettingsManager::event (QEvent *e)
	{
		if (e->type () != QEvent::DynamicPropertyChange)
			return false;

		auto event = static_cast<QDynamicPropertyChangeEvent*> (e);

		const QByteArray& name = event->propertyName ();
		const auto& nameStr = QString::fromUtf8 (name);
		const auto& propValue = property (name);

		if (!IsInitializing_)
			SettingsThreadManager::Instance ().Add (this, nameStr, propValue);

		PropertyChanged (nameStr, propValue);

		for (const auto& object : ApplyProps_.value (name))
		{
			if (!object.first)
				continue;

			PerformApply (name, propValue, object.first, object.second);
		}

		event->accept ();
		return true;
	}

	void BaseSettingsManager::PropertyChanged (const QString&, const QVariant&)
	{
	}

	Settings_ptr BaseSettingsManager::GetSettings () const
	{
		return Settings_ptr (BeginSettings (),
				[this] (QSettings *settings)
				{
					EndSettings (settings);
					delete settings;
				});
	}

	void BaseSettingsManager::RegisterObjectImpl (const QByteArray& propName,
			QObject *object, const PropHandler_t& handler, EventFlags flags)
	{
		if (flags & EventFlag::Apply)
			ApplyProps_ [propName].append ({ object, handler });
		if (flags & EventFlag::Select)
			SelectProps_ [propName].append ({ object, handler });

		if (flags & EventFlag::ImmediateUpdate)
			PerformApply (propName, property (propName), object, handler);

		connect (object,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (scheduleCleanup ()),
				Qt::UniqueConnection);
	}

	void BaseSettingsManager::scheduleCleanup ()
	{
		if (CleanupScheduled_)
			return;

		CleanupScheduled_ = true;
		QTimer::singleShot (100,
				this,
				SLOT (cleanupObjects ()));
	}

	void BaseSettingsManager::cleanupObjects ()
	{
		CleanupScheduled_= false;

		auto cleanupMap = [] (Properties2Object_t& map)
		{
			for (auto it = map.begin (); it != map.end (); )
			{
				auto& subscribers = it.value ();
				for (auto lit = subscribers.begin (); lit != subscribers.end (); )
				{
					if (!lit->first)
						lit = subscribers.erase (lit);
					else
						++lit;
				}

				if (subscribers.isEmpty ())
					it = map.erase (it);
				else
					++it;
			}
		};

		cleanupMap (ApplyProps_);
		cleanupMap (SelectProps_);
	}
}
