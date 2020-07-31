/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"
#include <interfaces/iplugin2.h>
#include <interfaces/secman/istorageplugin.h>

namespace LC
{
namespace SecMan
{
	Core::Core ()
	{
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	QSet<QByteArray> Core::GetExpectedPluginClasses () const
	{
		return QSet<QByteArray> () << "org.LeechCraft.SecMan.StoragePlugins/1.0";
	}

	void Core::AddPlugin (QObject *plugin)
	{
		IPlugin2 *ip2 = qobject_cast<IPlugin2*> (plugin);
		if (!ip2)
		{
			qWarning () << Q_FUNC_INFO
					<< "passed object is not a IPlugin2"
					<< plugin;
			return;
		}

		QSet<QByteArray> classes = ip2->GetPluginClasses ();
		if (classes.contains ("org.LeechCraft.SecMan.StoragePlugins/1.0"))
			AddStoragePlugin (plugin);
	}

	QObjectList Core::GetStoragePlugins () const
	{
		return { GetStoragePlugin () };
	}

	void Core::AddStoragePlugin (QObject *plugin)
	{
		if (!qobject_cast<IStoragePlugin*> (plugin))
		{
			qWarning () << Q_FUNC_INFO
					<< "passed object is not a IStoragePlugin"
					<< plugin;
			return;
		}
		StoragePlugins_ << plugin;
	}

	void Core::Store (const QByteArray& key, const QVariant& value)
	{
		const auto storage = GetStoragePlugin ();
		if (!storage)
		{
			qWarning () << Q_FUNC_INFO
					<< "null storage";
			return;
		}

		qobject_cast<IStoragePlugin*> (storage)->Save (key, value, IStoragePlugin::STSecure);
	}

	QVariant Core::Load (const QByteArray& key)
	{
		const auto storage = GetStoragePlugin ();
		if (!storage)
		{
			qWarning () << Q_FUNC_INFO
					<< "null storage";
			return {};
		}

		for (const auto storage : StoragePlugins_)
		{
			const auto& loaded = qobject_cast<IStoragePlugin*> (storage)->Load (key, IStoragePlugin::STSecure);
			if (!loaded.isNull ())
				return loaded;
		}

		return {};
	}

	QObject* Core::GetStoragePlugin () const
	{
		return StoragePlugins_.value (0);
	}
}
}
