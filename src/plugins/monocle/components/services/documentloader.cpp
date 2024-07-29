/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "documentloader.h"
#include <algorithm>
#include <QFile>
#include <interfaces/iplugin2.h>
#include <util/threads/coro.h>
#include <util/threads/coro/context.h>
#include "interfaces/monocle/ibackendplugin.h"
#include "interfaces/monocle/iredirectorplugin.h"
#include "defaultbackendmanager.h"

namespace LC::Monocle
{
	void DocumentLoader::AddPlugin (QObject *pluginObj)
	{
		auto plugin2 = qobject_cast<IPlugin2*> (pluginObj);
		const auto& classes = plugin2->GetPluginClasses ();
		if (classes.contains (IBackendPlugin::PluginClass))
			Backends_ << pluginObj;
		if (classes.contains (IRedirectorPlugin::PluginClass))
			Redirectors_ << pluginObj;
	}

	DefaultBackendManager& DocumentLoader::GetDefaultBackendManager ()
	{
		return BackendManager_;
	}

	bool DocumentLoader::CanHandleMime (const QString& mime) const
	{
		return std::any_of (Backends_.begin (), Backends_.end (),
				[&mime] (QObject *plugin)
				{
					return qobject_cast<IBackendPlugin*> (plugin)->GetSupportedMimes ().contains (mime);
				});
	}

	bool DocumentLoader::CanLoadDocument (const QString& path) const
	{
		for (auto backend : Backends_)
		{
			const auto ibp = qobject_cast<IBackendPlugin*> (backend);
			if (ibp->CanLoadDocument (path))
				return true;
		}

		return std::any_of (Redirectors_.begin (), Redirectors_.end (),
				[&path, this] (QObject *redirectorObj)
				{
					const auto redirector = qobject_cast<IRedirectorPlugin*> (redirectorObj);
					const auto redirectedMime = redirector->GetRedirectionMime (path);
					return !redirectedMime.isEmpty () && CanHandleMime (redirectedMime);
				});
	}

	Util::ContextTask<IDocument_ptr> DocumentLoader::LoadDocument (QString path)
	{
		if (!QFile::exists (path))
			co_return {};

		QObjectList loaders;
		for (auto backend : Backends_)
			if (qobject_cast<IBackendPlugin*> (backend)->CanLoadDocument (path))
				loaders << backend;

		QObjectList redirectors;
		for (auto redirector : Redirectors_)
			if (qobject_cast<IRedirectorPlugin*> (redirector)->CanRedirectDocument (path))
				redirectors << redirector;

		if (loaders.size () == 1)
			co_return qobject_cast<IBackendPlugin*> (loaders.at (0))->LoadDocument (path);

		if (!loaders.isEmpty ())
		{
			if (const auto backend = BackendManager_.GetBackend (loaders))
				co_return qobject_cast<IBackendPlugin*> (backend)->LoadDocument (path);
			co_return {};
		}

		co_await Util::AddContextObject { *this };

		for (const auto redirector : redirectors)
		{
			const auto backend = qobject_cast<IRedirectorPlugin*> (redirector);
			const auto& redirection = co_await backend->GetRedirection (path);
			if (!redirection)
				continue;

			const auto& target = redirection->TargetPath_;

			auto cleanupConverted = [target]
			{
				qDebug () << "removing" << target;
				QFile::remove (target);
			};

			const auto& doc = co_await LoadDocument (target);
			if (!doc || !doc->IsValid ())
			{
				qWarning () << "unable to load document after conversion";
				cleanupConverted ();
				continue;
			}

			QObject::connect (doc->GetQObject (),
					&QObject::destroyed,
					cleanupConverted);
			co_return doc;
		}

		co_return {};
	}
}
