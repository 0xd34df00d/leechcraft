/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QtPlugin>
#include <qwebpluginfactory.h>

namespace LC
{
namespace Poshuku
{
	/** Base class for plugins that want to be available from the
	 * Poshuku's Web Plugin Factory which is a subclass of
	 * QWebPluginFactory.
	 */
	class IWebPlugin
	{
	public:
		virtual ~IWebPlugin () {}

		/** Queries the plugin for the information.
		 *
		 * inPlugins is true if the plugin is being queried inside the
		 * QWebPluginFactory::plugins(), false otherwise. Plugins can
		 * relate on this parameter for them to not be exposed to the
		 * DOM.
		 *
		 * @param[in] inPlugins If queried inside the
		 * QWebPluginFactory::plugins().
		 */
		virtual std::optional<QWebPluginFactory::Plugin> Plugin (bool inPlugins) const = 0;

		/** Askes the plugin to create its instance for the given mime,
		 * url, args and their params.
		 *
		 * See QWebPluginFactory::create() for more info.
		 */
		virtual QWidget* Create (const QString& mime,
				const QUrl& url,
				const QStringList& args,
				const QStringList& params) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Poshuku::IWebPlugin,
		"org.LeechCraft.Poshuku.IWebPlugin/1.0")
