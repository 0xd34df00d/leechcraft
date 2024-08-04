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

namespace LC
{
namespace Monocle
{
	Core::Core ()
	: CacheManager_ (new PixmapCacheManager (this))
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

	Util::ShortcutManager* Core::GetShortcutManager () const
	{
		return ShortcutMgr_;
	}
}
}
