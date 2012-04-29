/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#pragma once

#include <QList>

class QObject;

namespace LeechCraft
{
namespace Blogique
{
	/** This is the base interface for plugins providing blogging platforms.
	 * Since these plugins are plugins for plugins, they
	 * should also implement IPlugin2 and return the
	 * "org.LeechCraft.Plugins.Blogique.Plugins.IBloggingPlatformPlugin"
	 * string, among others, from their IPlugin2::GetPluginClasses()
	 * method.
	 */
	class IBloggingPlatformPlugin
	{
	public:
		virtual ~IBloggingPlatformPlugin () {}

		/** @brief Returns the protocol plugin object as a QObject.
		 *
		 * @return The protocol plugin as a QObject.
		 */
		virtual QObject* GetObject () = 0;

		/** @brief Returns the blogging platforms list provided by this plugin.
		 * 
		 * Each object in this list must implement the IBloggingPlatform
		 * interface.
		 *
		 * @return The list of this plugin's blogging platforms.
		 *
		 * @sa IBloggingPlatform
		 */
		virtual QList<QObject*> GetBloggingPlatforms () const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Blogique::IBloggingPlatformPlugin,
		"org.Deviant.LeechCraft.Blogique.IBloggingPlatformPlugin/1.0");

