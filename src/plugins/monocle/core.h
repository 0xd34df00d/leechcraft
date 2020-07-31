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
	class RecentlyOpenedManager;
	class PixmapCacheManager;
	class DefaultBackendManager;
	class DocStateManager;
	class BookmarksManager;
	class CoreLoadProxy;

	class Core : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		QList<QObject*> Backends_;

		PixmapCacheManager *CacheManager_;
		RecentlyOpenedManager *ROManager_;
		DefaultBackendManager *DefaultBackendManager_;
		DocStateManager *DocStateManager_;
		BookmarksManager *BookmarksManager_;

		Util::ShortcutManager *ShortcutMgr_;

		Core ();
	public:
		static Core& Instance ();

		void SetProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetProxy () const;

		void AddPlugin (QObject*);

		bool CanHandleMime (const QString&);
		bool CanLoadDocument (const QString&);
		CoreLoadProxy* LoadDocument (const QString&);

		PixmapCacheManager* GetPixmapCacheManager () const;
		RecentlyOpenedManager* GetROManager () const;
		DefaultBackendManager* GetDefaultBackendManager () const;
		DocStateManager* GetDocStateManager () const;
		BookmarksManager* GetBookmarksManager () const;

		Util::ShortcutManager* GetShortcutManager () const;
	};
}
}
