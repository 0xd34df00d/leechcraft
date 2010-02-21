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

#ifndef INTERFACES_IPLUGIN2_H
#define INTERFACES_IPLUGIN2_H
#include <QtPlugin>
#include <QByteArray>
#include <QSet>

/** @brief Base class for second-level plugins.
 *
 * A plugin for LeechCraft could be actually a plugin for another
 * plugin. Then, to simplify the process, it's better to implement this
 * interface. LeechCraft would the automatically manage the
 * dependencies, perform correct initialization order and feed the
 * matching first-level plugins with second-level ones.
 *
 * Plugins of different levels are matched with each other by their
 * class, which is returned by IPlugin2::GetPluginClasses() and by
 * IPluginReady::GetExpectedPluginClasses().
 */
class IPlugin2
{
public:
	/** @brief Returns the plugin classes of this second-level plugin.
	 *
	 * @note This function should be able to work before IInfo::Init()
	 * is called.
	 *
	 * @return The plugin classes.
	 */
	virtual QSet<QByteArray> GetPluginClasses () const = 0;

	virtual ~IPlugin2 () {}
};

Q_DECLARE_INTERFACE (IPlugin2, "org.Deviant.LeechCraft.IPlugin2/1.0");

#endif

