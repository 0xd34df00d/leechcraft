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
#include "interfaces/monocle/ibackendplugin.h"
#include "interfaces/monocle/iredirectproxy.h"
#include "coreloadproxy.h"
#include "defaultbackendmanager.h"

namespace LC::Monocle
{
	DocumentLoader::DocumentLoader (QObject *parent)
	: QObject { parent }
	{
	}

	void DocumentLoader::AddPlugin (QObject *pluginObj)
	{
		auto plugin2 = qobject_cast<IPlugin2*> (pluginObj);
		const auto& classes = plugin2->GetPluginClasses ();
		if (classes.contains ("org.LeechCraft.Monocle.IBackendPlugin"))
			Backends_ << pluginObj;
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
		QObjectList redirectors;
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

	CoreLoadProxy* DocumentLoader::LoadDocument (const QString& path)
	{
		if (!QFile::exists (path))
			return nullptr;

		QObjectList loaders;
		QObjectList redirectors;
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
			return doc ? new CoreLoadProxy { *this, doc } : nullptr;
		}
		if (!loaders.isEmpty ())
		{
			if (const auto backend = BackendManager_.GetBackend (loaders))
			{
				const auto doc = qobject_cast<IBackendPlugin*> (backend)->LoadDocument (path);
				return doc ? new CoreLoadProxy { *this, doc } : nullptr;
			}
			return nullptr;
		}

		if (!redirectors.isEmpty ())
		{
			const auto backend = qobject_cast<IBackendPlugin*> (redirectors.first ());
			const auto redir = backend->GetRedirection (path);
			return redir ? new CoreLoadProxy { *this, redir } : nullptr;
		}

		return nullptr;
	}
}
