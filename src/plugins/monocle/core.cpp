/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"
#include <util/shortcuts/shortcutmanager.h>
#include "components/services/pixmapcachemanager.h"
#include "components/services/recentlyopenedmanager.h"
#include "components/services/docstatemanager.h"

namespace LC
{
namespace Monocle
{
	Core::Core ()
	: CacheManager_ (new PixmapCacheManager (this))
	, ROManager_ (new RecentlyOpenedManager (this))
	, DocStateManager_ (new DocStateManager (this))
	{
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::SetProxy (ICoreProxy_ptr proxy, QObject *pluginObject)
	{
		Proxy_ = proxy;
		ShortcutMgr_ = new Util::ShortcutManager { proxy, pluginObject };
	}

	PixmapCacheManager* Core::GetPixmapCacheManager () const
	{
		return CacheManager_;
	}

	RecentlyOpenedManager* Core::GetROManager () const
	{
		return ROManager_;
	}

	DocStateManager* Core::GetDocStateManager () const
	{
		return DocStateManager_;
	}

	Util::ShortcutManager* Core::GetShortcutManager () const
	{
		return ShortcutMgr_;
	}
}
}
