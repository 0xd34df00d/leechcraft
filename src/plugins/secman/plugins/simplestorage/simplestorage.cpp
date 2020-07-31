/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "simplestorage.h"
#include <QSettings>
#include <QIcon>
#include <QCoreApplication>
#include <util/sll/prelude.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>

namespace LC
{
namespace SecMan
{
namespace SimpleStorage
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Storage_ = std::make_shared<QSettings> (QSettings::IniFormat,
				QSettings::UserScope,
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_SecMan_SimpleStorage");
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.SecMan.StoragePlugins.SimpleStorage";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "SimpleStorage";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Simple unencrypted storage plugin for SecMan");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.SecMan.StoragePlugins/1.0" };
	}

	IStoragePlugin::StorageTypes Plugin::GetStorageTypes () const
	{
		return STInsecure;
	}

	QList<QByteArray> Plugin::ListKeys (IStoragePlugin::StorageType)
	{
		auto keys = Storage_->allKeys ();
		qDebug () << Q_FUNC_INFO << keys;
		return Util::Map (keys, [] (auto&& str) { return str.toUtf8 (); });
	}

	void Plugin::Save (const QByteArray& key, const QVariant& value,
			IStoragePlugin::StorageType)
	{
		Storage_->setValue (key, value);
	}

	QVariant Plugin::Load (const QByteArray& key, IStoragePlugin::StorageType)
	{
		return Storage_->value (key);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_secman_simplestorage, LC::SecMan::SimpleStorage::Plugin);
