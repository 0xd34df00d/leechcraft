/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QStringList>
#include <QObjectList>

class ILoadProgressReporter;
using ILoadProgressReporter_ptr = std::shared_ptr<ILoadProgressReporter>;

class QIcon;

/** @brief This interface is used to represent LeechCraft's global
 * plugin manager.
 *
 * Through is interface you can get information about currently
 * installed and running plugins, communicate with them at low-level
 * (for example, by getting a pointer to the corresponding QObject* and
 * casting it to any interface you want). It is also possible to inject
 * new plugins into LeechCraft via this interface and try to release
 * already running ones.
 *
 * This object also has the following signals:
 * - pluginInjected(QObject*), which is emitted after a successful
 *   plugin injection.
 */
class Q_DECL_EXPORT IPluginsManager
{
public:
	virtual ~IPluginsManager () {}

	/** @brief Returns list of pointers to all present plugins.
	 *
	 * @return The list of pointers to all present plugins.
	 */
	virtual QObjectList GetAllPlugins () const = 0;

	/** @brief Filters the given list of plugins and returns only those
	 * that can be casted to the given template type.
	 *
	 * Please note that you will almost always call it as Filter<T*> —
	 * note that you check against the pointer type.
	 *
	 * @param[in] source The list of plugins to filter.
	 * @return The list of plugins from source that can be casted ty type T.
	 */
	template<typename T>
	QObjectList Filter (const QObjectList& source) const
	{
		QObjectList result;
		for (const auto sp : source)
			if (qobject_cast<T> (sp))
				result << sp;
		return result;
	}

	/** @brief This is the same as Filter<T> (GetAllPlugins()).
	 *
	 * This function takes all available plugins from GetAllPlugins()
	 * and returns only those that can be casted to T via passing the
	 * result of GetAllPlugins() to Filter<T>().
	 *
	 * @return The list of pointers to plugin instances that are
	 * castable to type T.
	 */
	template<typename T>
	QObjectList GetAllCastableRoots () const
	{
		return Filter<T> (GetAllPlugins ());
	}

	/** @brief Similar to GetAlLCastableRoots() and provided for
	 * convenience.
	 *
	 * This function is almost the same as GetAllCastableRoots(), except
	 * it returns the list of pointers to the corresponding interface.
	 * Generally, it takes the result of GetAllCastableRoots() and
	 * converts each pointer from the list returned by
	 * GetAllCastableRoots() to the requested interface.
	 *
	 * @return The list of pointers to the requested interface.
	 */
	template<typename T>
	QList<T> GetAllCastableTo () const
	{
		QList<T> result;
		for (const auto root : GetAllCastableRoots<T> ())
			result << qobject_cast<T> (root);
		return result;
	}

	/** @brief Returns plugin identified by its id.
	 *
	 * If there is no such plugin with the given id, this function
	 * returns a null pointer.
	 *
	 * @param[in] id The ID of the plugin.
	 * @return The plugin instance or null if no such plugin exists.
	 */
	virtual QObject* GetPluginByID (const QByteArray& id) const = 0;

	/** @brief Returns the library path from which plugin instance
	 * \em object has been loaded.
	 *
	 * If the path could not be determined for some reason (for example,
	 * if the plugin was injected or provided by a plugin adaptor, or if
	 * the given object is not a plugin instance) this function returns
	 * an empty string.
	 *
	 * Please note that you cannot get library path for an arbitrary
	 * object. Only objects that are plugin instances ("root" objects in
	 * a plugin) are supported.
	 *
	 * @param[in] object The object for which to get the library path.
	 * @return The path corresponding to the plugin represented by the
	 * plugin instance \em object.
	 */
	virtual QString GetPluginLibraryPath (const QObject* object) const = 0;

	/** @brief Injects the given plugin object.
	 *
	 * The object's semantics are the same as the semantics of the
	 * "root" plugin object (returned by QPluginLoader::instance()).
	 * Thus, object should be castable at least to IInfo interface in
	 * order to be successfully loaded.
	 *
	 * @param[in] object The pointer to the "root" plugin object.
	 *
	 * @sa ReleasePlugin()
	 */
	virtual void InjectPlugin (QObject *object) = 0;

	/** @brief Releases and removes the given plugin object.
	 *
	 * This function tries to release and remove the given object plugin
	 * from LeechCraft. Because of that, object has to be injected
	 * earlier with InjectPlugin().
	 *
	 * Usage of this function is discouraged as it may lead to
	 * instabilities, because other plugins may have established
	 * connections and kept pointers to the object.
	 *
	 * @param[in] object The object previously injected with
	 * InjectPlugin().
	 *
	 * @sa InjectPlugin()
	 */
	virtual void ReleasePlugin (QObject *object) = 0;

	/** @brief Returns the pointer to plugin manager as a QObject.
	 *
	 * You can connect to signals of the plugin manager with the use of
	 * this function, for example.
	 *
	 * @return The plugin manager as a QObject.
	 */
	virtual QObject* GetQObject () = 0;

	/** @brief Opens the settings page for the given plugin object.
	 *
	 * If the plugin doesn't implement IHaveSettings, this function
	 * does nothing.
	 *
	 * @param[in] plugin The plugin for which to open the settings page.
	 */
	virtual void OpenSettings (QObject *plugin) = 0;

	/** @brief Creates an object for reporting progress of a long-running
	 * load-time operation.
	 *
	 * This method should be called by any plugin doing some long-running
	 * operation during LeechCraft load (like a DB migration) because of
	 * usability reasons.
	 *
	 * @param[in] thisPlugin The pointer to the instance object of the
	 * plugin that runs the long-running operation (typically the plugin
	 * where the code invoking this function belongs).
	 * @return The object used to track progress of the long-running
	 * operation. The returnd object may be used for progress reports of
	 * multiple operations at once.
	 *
	 * @sa ILoadProgressReporter
	 */
	virtual ILoadProgressReporter_ptr CreateLoadProgressReporter (QObject *thisPlugin) = 0;

	virtual QIcon GetPluginIcon (QObject*) = 0;
};

Q_DECLARE_INTERFACE (IPluginsManager, "org.Deviant.LeechCraft.IPluginsManager/1.0")
