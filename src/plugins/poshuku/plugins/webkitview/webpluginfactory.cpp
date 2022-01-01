/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "webpluginfactory.h"
#include <QWidget>
#include <interfaces/poshuku/iwebpluginprovider.h>

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	WebPluginFactory::WebPluginFactory (IPluginsManager *pm, QObject* parent)
	: QWebPluginFactory { parent }
	, PM_ { pm }
	{
		Reload ();
	}

	QObject* WebPluginFactory::create (const QString& mime,
			const QUrl& url,
			const QStringList& args, const QStringList& params) const
	{
		for (const auto plugin : MIME2Plugin_.value (mime))
			if (const auto result = plugin->Create (mime, url, args, params))
				return result;

		return nullptr;
	}

	QList<QWebPluginFactory::Plugin> WebPluginFactory::plugins () const
	{
		QList<Plugin> result;
		for (const auto plugin : Plugins_)
			if (const auto res = plugin->Plugin (true))
				result << *res;
		return result;
	}

	void WebPluginFactory::refreshPlugins ()
	{
		Reload ();
		QWebPluginFactory::refreshPlugins ();
	}

	void WebPluginFactory::Reload ()
	{
		Plugins_.clear ();
		MIME2Plugin_.clear ();

		for (const auto provider : PM_->GetAllCastableTo<IWebPluginProvider*> ())
			Plugins_ += provider->GetWebPlugins ();

		for (const auto plugin : Plugins_)
			if (const auto maybeInfo = plugin->Plugin (false))
				for (const auto& mime : maybeInfo->mimeTypes)
					MIME2Plugin_ [mime.name] << plugin;
	}
}
}
}
