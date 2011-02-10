/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_INTERFACES_IPROTOCOLPLUGIN_H
#define PLUGINS_AZOTH_INTERFACES_IPROTOCOLPLUGIN_H
#include <QList>

class QObject;

namespace LeechCraft
{
namespace Azoth
{
	/** This is the base interface for plugins providing messaging
	 * protocols. Since these plugins are plugins for plugins, they
	 * should also implement IPlugin2 and return the
	 * "org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin"
	 * string, among others, from their IPlugin2::GetPluginClasses()
	 * method.
	 */
	class IProtocolPlugin
	{
	public:
		virtual ~IProtocolPlugin () {}

		/** @brief Returns the protocol plugin object as a QObject.
		 *
		 * @return The protocol plugin as a QObject.
		 */
		virtual QObject* GetObject () = 0;

		/** @brief Returns the protocols list provided by this plugin.
		 *
		 * Each object in this list should implement the IProtocol
		 * interface.
		 *
		 * @return The list of this plugin's protocols.
		 *
		 * @sa IProtocol
		 */
		virtual QList<QObject*> GetProtocols () const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IProtocolPlugin,
		"org.Deviant.LeechCraft.Azoth.IProtocolPlugin/1.0");

#endif

