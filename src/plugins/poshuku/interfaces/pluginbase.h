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

#ifndef PLUGINBASE_H
#define PLUGINBASE_H
#include <stdexcept>
#include <QObject>
#include <qwebpage.h>
#include <qwebframe.h>
#include "iwebplugin.h"

class QNetworkRequest;
class QWebView;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class IProxyObject;

			/** @brief Base class for all the plugins.
			 *
			 * Provides some kind of interface for communication with
			 * plugins.
			 *
			 * Functions either don't return anything, or return a boolean
			 * value (true means "stop processing", false means "continue"),
			 * or return some custom value. In the later case returning
			 * something means "stop processing and use this value instead
			 * of default", throwing any exception means "continue".
			 */
			class PluginBase
			{
			public:
				/** @brief Initializes the plugin.
				 *
				 * Initializes the plugin with the given proxy object.
				 * Through the proxy object plugin can access and manipulate
				 * LeechCraft::Poshuku's internals.
				 *
				 * @param[in] proxy Pointer to the proxy object.
				 */
				virtual void Init (IProxyObject *proxy) = 0;

				virtual ~PluginBase ()
				{
				}
			};

			typedef PluginBase *PluginBase_ptr;
		};
	};
};

Q_DECLARE_INTERFACE (LeechCraft::Plugins::Poshuku::PluginBase,
		"org.Deviant.LeechCraft.Plugins.Poshuku.PluginBase/1.0");

#endif

