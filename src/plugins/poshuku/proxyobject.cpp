/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "proxyobject.h"
#include <interfaces/core/icoreproxy.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "browserwidget.h"
#include "linkopenmodifier.h"

namespace LC
{
namespace Poshuku
{
	QObject* ProxyObject::GetHistoryModel () const
	{
		return Core::Instance ().GetHistoryModel ();
	}

	QObject* ProxyObject::GetFavoritesModel () const
	{
		return Core::Instance ().GetFavoritesModel ();
	}

	QObject* ProxyObject::OpenInNewTab (const QUrl& url, bool inverted) const
	{
		bool raise = XmlSettingsManager::Instance ()->
				property ("BackgroundNewTabs").toBool ();
		if (inverted)
			raise = !raise;
		return Core::Instance ().NewURL (url, raise);
	}

	IStorageBackend_ptr ProxyObject::CreateStorageBackend ()
	{
		return StorageBackend::Create ();
	}

	QString ProxyObject::GetUserAgent (const QUrl& url) const
	{
		return Core::Instance ().GetUserAgent (url);
	}

	QVariant ProxyObject::GetPoshukuConfigValue (const QByteArray& name) const
	{
		return XmlSettingsManager::Instance ()->property (name);
	}

	ILinkOpenModifier_ptr ProxyObject::GetLinkOpenModifier () const
	{
		return std::make_shared<LinkOpenModifier> ();
	}

	void ProxyObject::RegisterHookable (QObject *obj) const
	{
		Core::Instance ().GetPluginManager ()->RegisterHookable (obj);
	}
}
}
