/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
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
class Q_DECL_EXPORT IPlugin2
{
public:
	/** @brief Returns the plugin classes of this second-level plugin.
	 *
	 * @note This function should be able to work correctly before
	 * IInfo::Init() is called on the plugin instance object.
	 *
	 * @return The plugin classes.
	 */
	virtual QSet<QByteArray> GetPluginClasses () const = 0;

	virtual ~IPlugin2 () {}
};

Q_DECLARE_INTERFACE (IPlugin2, "org.Deviant.LeechCraft.IPlugin2/1.0")

#endif

