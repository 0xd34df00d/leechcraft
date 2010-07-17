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

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			namespace Plugins
			{
				class IProtocol;

				/** This is the base interface for plugins providing
				 * protocols.
				 */
				class IProtocolPlugin
				{
				public:
					virtual ~IProtocolPlugin () {}

					/** Returns the protocol plugin object as a QObject.
					 *
					 * @return The protocol plugin as a QObject.
					 */
					virtual QObject* GetObject () = 0;

					/** Returns the list of protocols supported by this
					 * plugin.
					 *
					 * @return The list of this plugin's protocols.
					 */
					virtual QList<IProtocol*> GetProtocols () const = 0;
				};
			}
		}
	}
}

Q_DECLARE_INTERFACE (LeechCraft::Plugins::Azoth::Plugins::IProtocolPlugin,
		"org.Deviant.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin/1.0");

#endif

