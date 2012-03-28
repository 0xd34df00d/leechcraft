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

#ifndef INTERFACES_IINFO_H
#define INTERFACES_IINFO_H
#include <memory>
#include <QString>
#include <QStringList>
#include <QtPlugin>
#include "structures.h"

class ICoreProxy;
typedef std::shared_ptr<ICoreProxy> ICoreProxy_ptr;

/** @brief Required interface for every plugin.
 *
 * This interface is a base for all plugins loadable by LeechCraft. If a
 * plugin doesn't provide this interface (qobject_cast<IInfo*> to it
 * fails), it would be considered as a malformed one and would be
 * unloaded.
 *
 * The initialization of plugins is split into two stages: Init() and
 * SecondInit().
 *
 * In the first one plugins ought to initialize their data structures,
 * allocate memory and perform other initializations that don't depend
 * depend on other plugins. After the first stage (after Init()) the
 * plugin should be in a usable state: no call shall fail because
 * required data hasn't been initialized.
 *
 * In the second stage, SecondInit(), plugins can fill the data that
 * depends on other plugins. For example, in SecondInit() the Tab++
 * plugin, which shows a fancy tab tree, queries the MainWindow about
 * existing tabs and about plugins that can open tabs and connects to
 * their respective signals.
 *
 * So, as the rule of thumb, if the action required to initialize your
 * plugin depends on others - move it to SecondInit(), leaving in Init()
 * only basic initialization/allocation stuff like allocation memory for
 * the objects.
 */
class IInfo
{
public:
	/** @brief Initializes the plugin.
	 *
	 * Init is called by the LeechCraft when it has finished
	 * initializing its core and is ready to initialize plugins.
	 * Generally, all initialization code should be placed into this
	 * method instead of plugin's instance object's constructor.
	 *
	 * After this call plugin should be in a usable state. That means
	 * that all data members should be initialized, callable from other
	 * plugins etc. That also means that in this function you can't rely
	 * on other plugins being initialized.
	 *
	 * @param[in] proxy The pointer to proxy to LeechCraft.
	 *
	 * @sa Release
	 * @sa SecondInit
	 */
	virtual void Init (ICoreProxy_ptr proxy) = 0;

	/** @brief Performs second stage of initialization.
	 *
	 * This function is called when all the plugins are initialized by
	 * IInfo::Init() and may now communicate with others with no fear of
	 * getting init-order bugs.
	 *
	 * @sa Init
	 */
	virtual void SecondInit () = 0;

	/** @brief Returns the unique ID of the plugin.
	 *
	 * The ID should never change, event between different versions of
	 * the plugin and between renames of the plugin. It should be unique
	 * among all other plugins, thus the Vendor.AppName form is
	 * suggested. For example, Poshuku Browser plugin would return an ID
	 * like "org.LeechCraft.Poshuku", and Poshuku CleanWeb, which is
	 * Poshuku plugin, would return "org.LeechCraft.Poshuku.CleanWeb".
	 *
	 * The ID is allowed to consist of upper- and lowercase latin
	 * letters, numbers, dots¸ plus and minus sign.
	 *
	 * @return Unique and persistent ID of the plugin.
	 */
	virtual QByteArray GetUniqueID () const = 0;

	/** @brief Returns the name of the plugin.
	 *
	 * This name is used only for the UI, internal communication is done
	 * through pointers to QObjects representing plugin instance
	 * objects.
	 *
	 * @note This function should be able to work before Init() is
	 * called.
	 *
	 * @sa GetInfo
	 * @sa GetIcon
	 */
	virtual QString GetName () const = 0;

	/** @brief Returns the information string about the plugin.
	 *
	 * Information string is only used to describe the plugin to the
	 * user.
	 *
	 * @note This function should be able to work before Init() is
	 * called.
	 *
	 * @sa GetName
	 * @sa GetInfo
	 */
	virtual QString GetInfo () const = 0;

	/** @brief Returns the list of provided features.
	 *
	 * The return value is used by LeechCraft to calculate the
	 * dependencies between plugins and link them together by passing
	 * object pointers to SetProvider().
	 *
	 * The default implementation returns an empty list.
	 *
	 * @note This function should be able to work before Init() is
	 * called.
	 *
	 * @return List of provided features.
	 *
	 * @sa Needs
	 * @sa Uses
	 * @sa SetProvider
	 */
	virtual QStringList Provides () const
	{
		return QStringList ();
	}

	/** @brief Returns the list of needed features.
	 *
	 * The return value is used by LeechCraft to calculate the
	 * dependencies between plugins and link them together by passing
	 * object pointers to SetProvider().
	 *
	 * If not all requirements are satisfied, LeechCraft would mark the
	 * plugin as unusable and would not make it active or use its
	 * features returned by Provides() in dependency calculations. So,
	 * the returned list should mention those features that plugin can't
	 * live without and would not work at all.
	 *
	 * The default implementation returns an empty list.
	 *
	 * @note This function should be able to work before Init() is
	 * called.
	 *
	 * @return List of needed features.
	 *
	 * @sa Provides
	 * @sa Uses
	 * @sa SetProvider
	 */
	virtual QStringList Needs () const
	{
		return QStringList ();
	}

	/** @brief Returns the list of used features.
	 *
	 * The return value is used by LeechCraft to calculate the
	 * dependencies between plugins and link them together by passing
	 * object pointers to SetProvider().
	 *
	 * If not all requirements are satisfied, LeechCraft would still
	 * consider the plugin as usable and functional, make it available
	 * to user and use the features returned by Provides() in dependency
	 * calculations. So, the return list should mention those features
	 * that plugin can use but which are not required to start it and do
	 * some basic work.
	 *
	 * @note This function should be able to work before Init() is
	 * called.
	 *
	 * @return List of used features.
	 *
	 * @sa Provides
	 * @sa Uses
	 * @sa SetProvider
	 */
	virtual QStringList Uses () const
	{
		return QStringList ();
	}

	/** @brief Sets the provider plugin for a given feature.
	 *
	 * This function is called by LeechCraft after dependency
	 * calculations to inform plugin about other plugins which provide
	 * the required features.
	 *
	 * @note This function should be able to work before Init() is
	 * called.
	 *
	 * @param[in] object Pointer to plugin instance of feature provider.
	 * @param[in] feature The feature which this object provides.
	 *
	 * @sa Provides
	 * @sa Needs
	 * @sa Uses
	 */
	virtual void SetProvider (QObject* object,
			const QString& feature)
	{
		Q_UNUSED (object);
		Q_UNUSED (feature);
	}

	/** @brief Destroys the plugin.
	 *
	 * This function is called to notify that the plugin would be
	 * unloaded soon - either the application is closing down or the
	 * plugin is unloaded for some reason. Plugin should free its
	 * resources and especially all the GUI stuff in this function
	 * instead of plugin instance's destructor.
	 *
	 * @sa Init
	 */
	virtual void Release () = 0;

	/** @brief Returns the plugin icon.
	 *
	 * The icon is used only in GUI stuff.
	 *
	 * @note This function should be able to work before Init() is
	 * called.
	 *
	 * @return Icon object.
	 *
	 * @sa GetName
	 * @sa GetInfo
	 */
	virtual QIcon GetIcon () const = 0;

	/** @brief Virtual destructor.
	 *
 * - gotEntity (const LeechCraft::Entity& entity);
 *   Notifies other plugins about a new entity.
 * - delegateEntity (const LeechCraft::Entity& entity, int *id, QObject **provider);
 *   Entity delegation request. If a suitable provider is found, the
 *   entity is delegated to it, id is set according to the task ID
 *   returned from the provider, and provider is set to point to the
 *   provider object.
	 */
	virtual ~IInfo () {}

	/** @brief This signal is emitted by plugin to query if the given
	 * entity could be handled.
	 *
	 * If there is at least one plugin that can handle the given entity,
	 * the could is set to true, otherwise it is set to false.
	 *
	 * @param[out] entity The entity to check if could be handled.
	 * @param[in] could The pointer to the variable that would contain
	 * the result of the check.
	 *
	 * @note This function is expected to be a signal in subclasses.
	 *
	 * @sa gotEntity(), delegateEntity()
	 */
	virtual void couldHandle (const LeechCraft::Entity& entity, bool *could)
	{
		Q_UNUSED (entity);
		Q_UNUSED (could);
	}

	/** @brief This signal is emitted by plugin to notify the Core and
	 * other plugins about an entity.
	 *
	 * In this case, the plugin doesn't care what would happen next to
	 * the entity after the announcement and whether it would be catched
	 * by any other plugin at all. This is the opposite to the semantics
	 * of delegateEntity().
	 *
	 * This signal is typically emitted, for example, when a plugin has
	 * just finished downloading something and wants to notify other
	 * plugins about newly created files.
	 *
	 * This signal is asynchronous: the handling happends after the
	 * control gets back to the event loop.
	 *
	 * @note This function is expected to be a signal in subclasses.
	 *
	 * @param[out] entity The entity.
	 */
	virtual void gotEntity (const LeechCraft::Entity& entity)
	{
		Q_UNUSED (entity);
	}

	/** @brief This signal is emitted by plugin to delegate the entity
	 * to an another plugin.
	 *
	 * In this case, the plugin actually cares whether the entity would
	 * be handled. This signal is typically used, for example, to
	 * delegate a download request.
	 *
	 * id and provider are used in download delegation requests. If
	 * these parameters are not NULL and the entity is handled, they are
	 * set to the task ID returned by the corresponding IDownload
	 * instance and the main plugin instance of the handling plugin,
	 * respectively. Thus, setting the id to a non-NULL value means that
	 * only downloading may occur as result but no handling.
	 *
	 * Nevertheless, if you need to enable entity handlers to handle
	 * your request as well, you may leave the id parameter as NULL and
	 * just set the provider to a non-NULL value.
	 *
	 * @note This function is expected to be a signal in subclasses.
	 *
	 * @param[out] entity The entity to delegate.
	 * @param[in] id The pointer to the variable that would contain the
	 * task ID of this delegate request, or NULL.
	 * @param[in] provider The pointer to the main plugin instance of
	 * the plugin that handles this delegate request, or NULL.
	 */
	virtual void delegateEntity (const LeechCraft::Entity& entity,
			int *id, QObject **provider)
	{
		Q_UNUSED (entity);
		Q_UNUSED (id);
		Q_UNUSED (provider);
	}
};

Q_DECLARE_INTERFACE (IInfo, "org.Deviant.LeechCraft.IInfo/1.0");

#define CURRENT_API_LEVEL 5

#define LC_EXPORT_PLUGIN(name,file) Q_EXPORT_PLUGIN2(name, file) \
	extern "C"\
	{\
		Q_DECL_EXPORT quint64 GetAPILevels ()\
		{\
			return CURRENT_API_LEVEL;\
		}\
	}

#endif
