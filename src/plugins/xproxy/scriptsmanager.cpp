/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "scriptsmanager.h"
#include <QtDebug>
#include <interfaces/iscriptloader.h>
#include <interfaces/core/ipluginsmanager.h>
#include "urllistscript.h"

namespace LC
{
namespace XProxy
{
	ScriptsManager::ScriptsManager (const ICoreProxy_ptr& proxy)
	: Proxy_ { proxy }
	{
		const auto& loaders = Proxy_->GetPluginsManager ()->GetAllCastableTo<IScriptLoader*> ();
		for (const auto loader : loaders)
		{
			const auto instance = loader->CreateScriptLoaderInstance ("xproxy");
			instance->AddGlobalPrefix ();
			instance->AddLocalPrefix ();

			for (const auto& name : instance->EnumerateScripts ())
			{
				const auto& script = instance->LoadScript (name);
				Scripts_ << new UrlListScript { script };
			}
		}
	}

	QList<UrlListScript*> ScriptsManager::GetScripts () const
	{
		return Scripts_;
	}

	UrlListScript* ScriptsManager::GetScript (const QByteArray& id) const
	{
		for (const auto script : Scripts_)
			if (script->GetListId () == id)
				return script;

		return nullptr;
	}
}
}
