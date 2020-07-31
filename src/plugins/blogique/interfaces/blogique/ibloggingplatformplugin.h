/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QList>

class QObject;

namespace LC
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
		virtual QObject* GetQObject () = 0;

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

Q_DECLARE_INTERFACE (LC::Blogique::IBloggingPlatformPlugin,
		"org.Deviant.LeechCraft.Blogique.IBloggingPlatformPlugin/1.0")

