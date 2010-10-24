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

#ifndef INTERFACES_IPLUGINREADY_H
#define INTERFACES_IPLUGINREADY_H
#include <QtPlugin>
#include <QSet>

/** @brief Base class for plugins accepting second-level plugins.
 *
 * A plugin for LeechCraft could be actually a plugin for another
 * plugin. Then, to simplify the process, if a plugin could handle such
 * second-level plugins (if it's a host for them), it's better to
 * implement this interface. LeechCraft would the automatically manage
 * the dependencies, perform correct initialization order and feed the
 * matching first-level plugins with second-level ones.
 *
 * Plugins of different levels are matched with each other by their
 * classes, which is returned by IPlugin2::GetPluginClasses() and by
 * IPluginReady::GetExpectedPluginClasses().
 */
class IPluginReady
{
public:
	virtual ~IPluginReady () {}

	/** @brief Returns the expected classes of the plugins for this
	 * plugin.
	 *
	 * Returns the expected second level plugins' classes expected by this
	 * first-level plugin.
	 *
	 * @note This function should be able to work before IInfo::Init() is
	 * called.
	 *
	 * @return The expected plugin class entity.
	 */
	virtual QSet<QByteArray> GetExpectedPluginClasses () const = 0;

	/** @brief Adds second-level plugin to this one.
	 *
	 * @note This function should be able to work before IInfo::Init() is
	 * called. The second-level plugin also comes uninitialized.
	 *
	 * @param[in] plugin The pointer to the plugin instance.
	 */
	virtual void AddPlugin (QObject *plugin) = 0;
};

Q_DECLARE_INTERFACE (IPluginReady, "org.Deviant.LeechCraft.IPluginReady/1.0");

#endif

