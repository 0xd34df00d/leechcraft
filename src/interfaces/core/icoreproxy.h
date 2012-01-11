/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef INTERFACES_CORE_ICOREPROXY_H
#define INTERFACES_CORE_ICOREPROXY_H
#include <memory>
#include <QtNetwork/QNetworkAccessManager>
#include <QTabBar>

class IShortcutProxy;
class IMWProxy;
class ITagsManager;
class IPluginsManager;
class ICoreTabWidget;
class QTreeView;
class QModelIndex;
class QIcon;
class QMainWindow;
class QAbstractItemModel;
class QTabWidget;

namespace LeechCraft
{
	namespace Util
	{
		class BaseSettingsManager;
	}
}

/** @brief Proxy class for the communication with LeechCraft.
 *
 * Allows one to talk with LeechCraft, requesting and getting various
 * services.
 */
class ICoreProxy
{
public:
	virtual ~ICoreProxy () {}

	/** @brief Returns application-wide network access manager.
	 *
	 * If your plugin wants to work well with other internet-related
	 * ones and wants to integrate with application-wide cookie database
	 * and network cache, it should use the returned
	 * QNetworkAccessManager.
	 *
	 * @return Application-wide QNetworkAccessManager.
	 */
	virtual QNetworkAccessManager* GetNetworkAccessManager () const = 0;

	/** @brief Returns the shortcut proxy used to communicate with the
	 * shortcut manager.
	 *
	 * @sa IShortcutProxy
	 */
	virtual IShortcutProxy* GetShortcutProxy () const = 0;

	/** @brief Returns the main window proxy.
	 *
	 * @sa IMWProxy
	 */
	virtual IMWProxy* GetMWProxy () const = 0;

	/** @brief Maps the given index up to the plugin's through the
	 * hierarchy of LeechCraft's models
	 */
	virtual QModelIndex MapToSource (const QModelIndex&) const = 0;

	/** @brief Returns the LeechCraft's settings manager.
	 *
	 * In the returned settings manager you can use any property name
	 * you want if it starts from "PluginsStorage". To avoid name
	 * collisions from different plugins it's strongly encouraged to
	 * also use the plugin name in the property. So the property name
	 * would look like "PluginsStorage/PluginName/YourProperty".
	 */
	virtual LeechCraft::Util::BaseSettingsManager* GetSettingsManager () const = 0;

	/** Returns the current theme's icon for the given on and off
	 * states. Similar to the mapping files.
	 *
	 * @param[in] on The name of the icon in the "on" state.
	 * @param[in] off The name of the icon in the "off" state, if any.
	 * @return The QIcon object created from image files which could be
	 * obtained via GetIconPath().
	 *
	 * @sa GetIconPath
	 */
	virtual QIcon GetIcon (const QString& on, const QString& off = QString ()) const = 0;

	/** @brief Updates the icons of the given actions according to current iconset.
	 */
	virtual void UpdateIconset (const QList<QAction*>& actions) const = 0;

	/** Returns main LeechCraft's window.
	 */
	virtual QMainWindow* GetMainWindow () const = 0;

	/** Returns the main tab widget.
	 */
	virtual ICoreTabWidget* GetTabWidget () const = 0;

	/** Returns the application-wide tags manager.
	 */
	virtual ITagsManager* GetTagsManager () const = 0;

	/** Returns the list of all possible search categories from the
	 * finders installed.
	 */
	virtual QStringList GetSearchCategories () const = 0;

	/** @brief Returns an ID for a delegated task from the pool.
	 *
	 * Use this in your downloader plugin when generating an ID for a
	 * newly added task. This way you can avoid ID clashes with other
	 * downloaders.
	 *
	 * @return The ID of the task.
	 *
	 * @sa FreeID()
	 */
	virtual int GetID () = 0;

	/** @brief Marks an ID previously returned by GetID as unused.
	 *
	 * Returns the id to the global ID pool. Use this in your downloader
	 * plugins after your download finishes.
	 *
	 * @param[in] id An ID previously obtained by GetID().
	 *
	 * @sa GetID()
	 */
	virtual void FreeID (int id) = 0;

	/** @brief Returns the application's plugin manager.
	 */
	virtual IPluginsManager* GetPluginsManager () const = 0;

	/** @brief Returns the version of LeechCraft core and base system.
	 */
	virtual QString GetVersion () const = 0;

	/** @brief Returns the pointer to itself as QObject*.
	 *
	 * Just to avoid nasty reinterpret_casts.
	 */
	virtual QObject* GetSelf () = 0;

	/** @brief Registers the given action as having skinnable icons.
	 *
	 * Registers the given action so that it automatically gets its icon
	 * updated whenever the current iconset changes.
	 *
	 * @param[in] action The action to register.
	 */
	virtual void RegisterSkinnable (QAction *action) = 0;

	virtual bool IsShuttingDown () = 0;
};

typedef std::shared_ptr<ICoreProxy> ICoreProxy_ptr;

Q_DECLARE_INTERFACE (ICoreProxy, "org.Deviant.LeechCraft.ICoreProxy/1.0");

#endif
