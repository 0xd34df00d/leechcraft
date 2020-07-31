/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "coreproxywrapper.h"
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/iiconthememanager.h>
#include "shortcutproxywrapper.h"
#include "pluginsmanagerwrapper.h"
#include "tagsmanagerwrapper.h"

namespace LC
{
namespace Qrosp
{
	CoreProxyWrapper::CoreProxyWrapper (ICoreProxy_ptr proxy)
	: Proxy_ (proxy)
	{
	}

	QNetworkAccessManager* CoreProxyWrapper::GetNetworkAccessManager () const
	{
		return Proxy_->GetNetworkAccessManager ();
	}

	QObject* CoreProxyWrapper::GetShortcutProxy () const
	{
		return new ShortcutProxyWrapper (Proxy_->GetShortcutProxy ());
	}

	QModelIndex CoreProxyWrapper::MapToSource (const QModelIndex& index) const
	{
		return Proxy_->MapToSource (index);
	}

	QIcon CoreProxyWrapper::GetIcon (const QString& on, const QString& off) const
	{
		return Proxy_->GetIconThemeManager ()->GetIcon (on, off);
	}

	QMainWindow* CoreProxyWrapper::GetMainWindow () const
	{
		return Proxy_->GetRootWindowsManager ()->GetMainWindow (0);
	}

	ICoreTabWidget* CoreProxyWrapper::GetTabWidget () const
	{
		return Proxy_->GetRootWindowsManager ()->GetTabWidget (0);
	}

	QObject* CoreProxyWrapper::GetTagsManager () const
	{
		return new TagsManagerWrapper (Proxy_->GetTagsManager ());
	}

	QStringList CoreProxyWrapper::GetSearchCategories () const
	{
		return Proxy_->GetSearchCategories ();
	}

	QObject* CoreProxyWrapper::GetPluginsManager () const
	{
		return new PluginsManagerWrapper (Proxy_->GetPluginsManager ());
	}
}
}
