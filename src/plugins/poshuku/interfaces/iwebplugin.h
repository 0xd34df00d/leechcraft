/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_INTERFACES_IWEBPLUGIN_H
#define PLUGINS_POSHUKU_INTERFACES_IWEBPLUGIN_H
#include <QtPlugin>
#include <QWebPluginFactory>

namespace LeechCraft
{
	namespace Plugins
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
				 * inPlugins is true if the plugin is being queried inside
				 * the QWebPluginFactory::plugins(), false otherwise.
				 * Plugins can relate on this parameter for them to not
				 * be exposed to the DOM.
				 *
				 * @param[in] inPlugins If queried inside the
				 * QWebPluginFactory::plugins().
				 */
				virtual QWebPluginFactory::Plugin Plugin (bool inPlugins) const = 0;

				/** Askes the plugin to create its instance for the
				 * given mime, url, args and their params.
				 *
				 * See QWebPluginFactory::create() for more info.
				 */
				virtual QWidget* Create (const QString& mime,
						const QUrl& url,
						const QStringList& args,
						const QStringList& params) = 0;
			};
		};
	};
};

Q_DECLARE_INTERFACE (LeechCraft::Plugins::Poshuku::IWebPlugin,
		"org.Deviant.LeechCraft.Plugins.Poshuku.IWebPlugin/1.0");

#endif

