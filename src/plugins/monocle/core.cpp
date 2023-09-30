/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"
#include <algorithm>
#include <QFile>
#include <util/shortcuts/shortcutmanager.h>
#include <interfaces/iplugin2.h>
#include "interfaces/monocle/iredirectproxy.h"
#include "pixmapcachemanager.h"
#include "recentlyopenedmanager.h"
#include "defaultbackendmanager.h"
#include "docstatemanager.h"
#include "bookmarksmanager.h"
#include "coreloadproxy.h"

namespace LC
{
namespace Monocle
{
	Core::Core ()
	: CacheManager_ (new PixmapCacheManager (this))
	, ROManager_ (new RecentlyOpenedManager (this))
	, DefaultBackendManager_ (new DefaultBackendManager (this))
	, DocStateManager_ (new DocStateManager (this))
	, BookmarksManager_ (new BookmarksManager (this))
	{
		qRegisterMetaType<IDocument::Position> ("IDocument::Position");
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::SetProxy (ICoreProxy_ptr proxy, QObject *pluginObject)
	{
		Proxy_ = proxy;
		DefaultBackendManager_->LoadSettings ();

		ShortcutMgr_ = new Util::ShortcutManager { proxy, pluginObject };
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	void Core::AddPlugin (QObject *pluginObj)
	{
		auto plugin2 = qobject_cast<IPlugin2*> (pluginObj);
		const auto& classes = plugin2->GetPluginClasses ();
		if (classes.contains ("org.LeechCraft.Monocle.IBackendPlugin"))
			Backends_ << pluginObj;
	}

	bool Core::CanHandleMime (const QString& mime)
	{
		return std::any_of (Backends_.begin (), Backends_.end (),
				[&mime] (QObject *plugin)
				{
					return qobject_cast<IBackendPlugin*> (plugin)->GetSupportedMimes ().contains (mime);
				});
	}

	bool Core::CanLoadDocument (const QString& path)
	{
		decltype (Backends_) redirectors;
		for (auto backend : Backends_)
		{
			const auto ibp = qobject_cast<IBackendPlugin*> (backend);
			switch (ibp->CanLoadDocument (path))
			{
			case IBackendPlugin::LoadCheckResult::Can:
				return true;
			case IBackendPlugin::LoadCheckResult::Redirect:
				redirectors << backend;
				break;
			case IBackendPlugin::LoadCheckResult::Cannot:
				break;
			}
		}

		return std::any_of (redirectors.begin (), redirectors.end (),
				[&path, this] (QObject *redirectorObj)
				{
					const auto redirector = qobject_cast<IBackendPlugin*> (redirectorObj);
					const auto redirect = redirector->GetRedirection (path);
					return CanHandleMime (redirect->GetRedirectedMime ());
				});
	}

	CoreLoadProxy* Core::LoadDocument (const QString& path)
	{
		if (!QFile::exists (path))
			return nullptr;

		decltype (Backends_) loaders;
		decltype (Backends_) redirectors;
		for (auto backend : Backends_)
		{
			switch (qobject_cast<IBackendPlugin*> (backend)->CanLoadDocument (path))
			{
			case IBackendPlugin::LoadCheckResult::Can:
				loaders << backend;
				break;
			case IBackendPlugin::LoadCheckResult::Redirect:
				redirectors << backend;
				break;
			case IBackendPlugin::LoadCheckResult::Cannot:
				break;
			}
		}

		if (loaders.size () == 1)
		{
			const auto doc = qobject_cast<IBackendPlugin*> (loaders.at (0))->LoadDocument (path);
			return doc ? new CoreLoadProxy { doc } : nullptr;
		}
		else if (!loaders.isEmpty ())
		{
			if (const auto backend = DefaultBackendManager_->GetBackend (loaders))
			{
				const auto doc = qobject_cast<IBackendPlugin*> (backend)->LoadDocument (path);
				return doc ? new CoreLoadProxy { doc } : nullptr;
			}
			else
				return nullptr;
		}
		else if (!redirectors.isEmpty ())
		{
			const auto backend = qobject_cast<IBackendPlugin*> (redirectors.first ());
			const auto redir = backend->GetRedirection (path);
			return redir ? new CoreLoadProxy { redir } : nullptr;
		}
		else
			return nullptr;
	}

	PixmapCacheManager* Core::GetPixmapCacheManager () const
	{
		return CacheManager_;
	}

	RecentlyOpenedManager* Core::GetROManager () const
	{
		return ROManager_;
	}

	DefaultBackendManager* Core::GetDefaultBackendManager () const
	{
		return DefaultBackendManager_;
	}

	DocStateManager* Core::GetDocStateManager () const
	{
		return DocStateManager_;
	}

	BookmarksManager* Core::GetBookmarksManager () const
	{
		return BookmarksManager_;
	}

	Util::ShortcutManager* Core::GetShortcutManager () const
	{
		return ShortcutMgr_;
	}
}
}
