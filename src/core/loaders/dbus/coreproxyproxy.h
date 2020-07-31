/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QDBusInterface>
#include <interfaces/core/icoreproxy.h>

class QDBusObjectPath;

namespace LC
{
namespace DBus
{
	class CoreProxyProxy : public QObject
						 , public ICoreProxy
	{
		mutable QDBusInterface Proxy_;
	public:
		CoreProxyProxy (const QString& service, const QDBusObjectPath& path);

		QNetworkAccessManager* GetNetworkAccessManager () const;
		IShortcutProxy* GetShortcutProxy () const;
		QModelIndex MapToSource (const QModelIndex&) const;
		Util::BaseSettingsManager* GetSettingsManager () const;
		IIconThemeManager* GetIconThemeManager () const;
		IColorThemeManager* GetColorThemeManager () const;
		IRootWindowsManager* GetRootWindowsManager () const;
		ITagsManager* GetTagsManager () const;
		QStringList GetSearchCategories () const;
		int GetID ();
		void FreeID (int);
		IPluginsManager* GetPluginsManager () const;
		IEntityManager* GetEntityManager () const;
		QString GetVersion() const;
		void RegisterSkinnable (QAction*);
		bool IsShuttingDown();
	};
}
}
