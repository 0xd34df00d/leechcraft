/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "coreproxyproxy.h"
#include <QModelIndex>
#include <QDBusReply>

namespace LC
{
namespace DBus
{
	CoreProxyProxy::CoreProxyProxy (const QString& service, const QDBusObjectPath& path)
	: Proxy_ { service, path.path () }
	{
	}

	QNetworkAccessManager* CoreProxyProxy::GetNetworkAccessManager () const
	{
		return nullptr;
	}

	IShortcutProxy* CoreProxyProxy::GetShortcutProxy () const
	{
		return nullptr;
	}

	QModelIndex CoreProxyProxy::MapToSource (const QModelIndex&) const
	{
		return {};
	}

	Util::BaseSettingsManager* CoreProxyProxy::GetSettingsManager () const
	{
		return nullptr;
	}

	IIconThemeManager* CoreProxyProxy::GetIconThemeManager () const
	{
		return nullptr;
	}

	IColorThemeManager* CoreProxyProxy::GetColorThemeManager () const
	{
		return nullptr;
	}

	IRootWindowsManager* CoreProxyProxy::GetRootWindowsManager () const
	{
		return nullptr;
	}

	ITagsManager* CoreProxyProxy::GetTagsManager () const
	{
		return nullptr;
	}

	QStringList CoreProxyProxy::GetSearchCategories () const
	{
		QDBusReply<QStringList> reply { Proxy_.call ("GetSearchCategories") };
		return reply.value ();
	}

	int CoreProxyProxy::GetID ()
	{
		QDBusReply<int> reply { Proxy_.call ("GetID") };
		return reply.value ();
	}

	void CoreProxyProxy::FreeID (int id)
	{
		Proxy_.call ("FreeID", id);
	}

	IPluginsManager* CoreProxyProxy::GetPluginsManager () const
	{
		return nullptr;
	}

	IEntityManager* CoreProxyProxy::GetEntityManager () const
	{
		return nullptr;
	}

	QString CoreProxyProxy::GetVersion () const
	{
		QDBusReply<QString> reply { Proxy_.call ("GetVersion") };
		return reply.value ();
	}

	void CoreProxyProxy::RegisterSkinnable (QAction*)
	{
	}

	bool CoreProxyProxy::IsShuttingDown ()
	{
		QDBusReply<bool> reply { Proxy_.call ("IsShuttingDown") };
		return reply.value ();
	}
}
}
