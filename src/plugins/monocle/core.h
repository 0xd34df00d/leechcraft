/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/monocle/ibackendplugin.h>
#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace Util
{
class ShortcutManager;
}

namespace Monocle
{
	class PixmapCacheManager;
	class DocStateManager;

	class Core : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;

		PixmapCacheManager *CacheManager_;
		DocStateManager *DocStateManager_;

		Util::ShortcutManager *ShortcutMgr_;

		Core ();
	public:
		static Core& Instance ();

		void SetProxy (ICoreProxy_ptr, QObject*);

		PixmapCacheManager* GetPixmapCacheManager () const;
		DocStateManager* GetDocStateManager () const;

		Util::ShortcutManager* GetShortcutManager () const;
	};
}
}
