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
#include <QCoreApplication>
#include <util/sll/visitor.h>
#include "settingsthreadmanager.h"

namespace LC::Util
{
	BaseSettingsManager::BaseSettingsManager (QObject *parent)
	: BaseSettingsManager ({}, false, parent)
	{
	}

	BaseSettingsManager::BaseSettingsManager (bool readAllKeys, QObject *parent)
	: BaseSettingsManager ({}, readAllKeys, parent)
	{
	}

	BaseSettingsManager::BaseSettingsManager (QString settingsFileSuffix, bool readAllKeys, QObject *parent)
	: QObject { parent }
	, ReadAllKeys_ { readAllKeys }
	, SettingsFileSuffix_ { std::move (settingsFileSuffix) }
	{
	}

	void BaseSettingsManager::Init ()
	{
		IsInitializing_ = true;

		auto settings = MakeSettings ();
		const auto& properties = ReadAllKeys_ ?
				settings->allKeys () :
				settings->childKeys ();
		for (const auto& prop : properties)
			setProperty (prop.toUtf8 ().constData (), settings->value (prop));

		IsInitializing_ = false;
	}

	void BaseSettingsManager::Release ()
	{
		auto settings = MakeSettings ();

		for (const auto& dProp : dynamicPropertyNames ())
			settings->setValue (QString::fromUtf8 (dProp), property (dProp.constData ()));
	}

	void BaseSettingsManager::RegisterObject (const QByteArray& propName,
			QObject *object, const QByteArray& funcName, EventFlags flags)
	{
		RegisterObjectImpl (propName, object, funcName, flags);
	}

	void BaseSettingsManager::RegisterObject (const QByteArray& propName,
			QObject *object, const std::function<void ()>& func, EventFlags flags)
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

	QVariant BaseSettingsManager::Property (const char *propName, const QVariant& def)
	{
		auto result = property (propName);
		if (!result.isValid ())
		{
			result = def;
			setProperty (propName, def);
		}

		return result;
	}

	void BaseSettingsManager::SetRawValue (const QString& path, const QVariant& val)
	{
		MakeSettings ()->setValue (path, val);
	}

	QVariant BaseSettingsManager::GetRawValue (const QString& path, const QVariant& def) const
	{
		return MakeSettings ()->value (path, def);
	}

	void BaseSettingsManager::ShowSettingsPage (const QString& optionName)
	{
		emit showPageRequested (this, optionName);
	}

	void BaseSettingsManager::OptionSelected (const QByteArray& prop, const QVariant& val)
	{
		for (const auto& [object, handler] : SelectProps_.value (prop))
		{
			if (!object)
				continue;

			Visit (handler,
					[&val] (const VariantHandler_f& func) { func (val); },
					[&] (const QByteArray& methodName)
					{
						if (!QMetaObject::invokeMethod (object,
								methodName,
								Q_ARG (QVariant, val)))
							qWarning () << Q_FUNC_INFO
									<< "could not find method in the metaobject"
									<< prop
									<< object
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

		for (const auto& [object, handler] : ApplyProps_.value (name))
			if (object)
				PerformApply (name, propValue, object, handler);

		event->accept ();
		return true;
	}

	auto BaseSettingsManager::MakeSettings () const -> QSettings_ptr
	{
		return std::make_shared<QSettings> (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + '_' + SettingsFileSuffix_);
	}

	void BaseSettingsManager::PropertyChanged (const QString&, const QVariant&)
	{
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
				subscribers.removeIf ([] (const auto& elem) { return !elem.Obj_; });

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
