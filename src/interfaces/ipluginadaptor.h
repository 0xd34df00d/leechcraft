/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef INTERFACES_IPLUGINADAPTOR_H
#define INTERFACES_IPLUGINADAPTOR_H
#include <QList>
#include <QObject>

/** @brief This class is used to provide plugins for LeechCraft from
 * additional sources.
 *
 * By default, LeechCraft core loads plugins only from the dynamic
 * shared objects (.so on *nix, .dll on Windows, .dylib on Mac OS X) it
 * finds in a set of platform-dependent predefined places. To provide
 * additional plugins for LeechCraft, one should implement this plugin
 * and return the set of plugin instances (analogous to the return value
 * of QPluginLoader::instance() function) from the GetPlugins()
 * function.
 *
 * The returned objects should behave as normal LeechCraft plugin
 * instances: each one should implement at least IInfo and whatever
 * other interfaces it decides to. Successful qobject_cast() to IInfo is
 * sufficient for the plugin object to be considered a valid LeechCraft
 * plugin.
 *
 * For example, the Qrosp plugin implements this interface in order to
 * provide script plugins for LeechCraft. It creates a wrapper object
 * for each script it finds. It also does some QMetaObject magic to
 * provide corresponding (and unknown in advance) signals/slots for each
 * script it wraps. Some trickery is also used to cast only to those
 * interfaces that the script claims to provide.
 *
 * Of course, each plugin adaptor should also implement at least IInfo.
 */
class Q_DECL_EXPORT IPluginAdaptor
{
public:
	virtual ~IPluginAdaptor () {}

	/** @brief Returns the list of plugins provided by this adaptor.
	 *
	 * Each object should be a valid LeechCraft plugin instance: it
	 * should be castable to IInfo at least.
	 *
	 * This function must be callable before IInfo::Init() is called on
	 * the plugin. In fact, GetPlugins() would be called on every
	 * IPluginAdaptor before the initialization takes place, hence this
	 * requirement: it should return the proper list of plugin instances
	 * before Init().
	 *
	 * @return The list of plugin instances provided by this adaptor.
	 */
	virtual QList<QObject*> GetPlugins () = 0;
};

Q_DECLARE_INTERFACE (IPluginAdaptor, "org.Deviant.LeechCraft.IPluginAdaptor/1.0")

#endif

