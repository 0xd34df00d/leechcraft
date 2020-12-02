/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include "shortcutsconfig.h"
#include "interfaces/ihaveshortcuts.h"
#include "interfaces/core/icoreproxy.h"
#include "interfaces/structures.h"

class QAction;
class QShortcut;
class IShortcutProxy;

namespace LC
{
struct Entity;

namespace Util
{
	/** @brief Aids in providing configurable shortcuts.
	 *
	 * This class serves as a "collector" for different QActions and
	 * QShortcuts. One typically instantiates an object of this class
	 * as a per-plugin global object (via a singleton, for example),
	 * registers all required actions via the RegisterShortcut(),
	 * RegisterAction() and RegisterActionInfo() functions and relays
	 * calls to the IHaveShortcuts::SetShortcut() and
	 * IHaveShortcuts::GetActionInfo() functions to this class.
	 *
	 * Though one can register actions at arbitrary points of time,
	 * only those "kinds" of actions registered during the IInfo::Init()
	 * will be visible to the LeechCraft core. Actions added later will
	 * still have customized shortcuts (if any), but only if another
	 * action with the same ID has been added during IInfo::Init().
	 *
	 * ShortcutManager also supports global shortcuts via the
	 * RegisterGlobalShortcut() and AnnounceGlobalShorcuts() methods.
	 *
	 * See the documentation for IHaveShortcuts for more information
	 * about actions and their IDs.
	 *
	 * @sa IHaveShortcuts
	 *
	 * @ingroup ShortcutsUtil
	 */
	class UTIL_SHORTCUTS_API ShortcutManager : public QObject
	{
		ICoreProxy_ptr CoreProxy_;
		QObject *ContextObj_ = nullptr;

		QHash<QString, QList<QAction*>> Actions_;

		QHash<QString, QList<QShortcut*>> Shortcuts_;
		QHash<QShortcut*, QList<QShortcut*>> Shortcut2Subs_;

		QHash<QString, Entity> Globals_;

		QMap<QString, ActionInfo> ActionInfo_;
	public:
		/** @brief Creates the shortcut manager.
		 *
		 * @param[in] proxy The proxy object passed to IInfo::Init() of
		 * your plugin.
		 * @param[in] parent The parent object of this object.
		 */
		ShortcutManager (ICoreProxy_ptr proxy, QObject *parent = nullptr);

		/** @brief Sets the plugin instance object of this manager.
		 *
		 * The plugin instance object serves as a kind of "context" for
		 * the shortcut manager.
		 *
		 * @param[in] pluginObj The plugin instance object.
		 */
		void SetObject (QObject *pluginObj);

		/** @brief Registers the given QAction by the given id.
		 *
		 * This function registers the given action at the given id and
		 * updates it if necessary. The ActionInfo structure is created
		 * automatically, and \em ActionIcon property of the action is
		 * used to fetch its icon.
		 *
		 * @param[in] id The ID of action to register.
		 * @param[in] action The action to register.
		 *
		 * @sa RegisterShortcut(), RegisterActionInfo()
		 */
		void RegisterAction (const QString& id, QAction *action);

		using IDPair_t = QPair<QString, QAction*>;

		void RegisterActions (const std::initializer_list<IDPair_t>& actions);

		/** @brief Registers the given QShortcut with the given id.
		 *
		 * @param[in] id The ID of QShortcut to register.
		 * @param[in] info The additional ActionInfo about this shortcut.
		 * @param[in] shortcut The QShortcut to register.
		 *
		 * @sa RegisterAction(), RegisterActionInfo()
		 */
		void RegisterShortcut (const QString& id,
				const ActionInfo& info, QShortcut *shortcut);

		/** @brief Registers the given action info with the given id.
		 *
		 * This function can be used to register an action info with the
		 * given ID before any actions or shortcuts with this ID are
		 * really created. This function can be used, for example, to
		 * register shortcuts that will be available during some time
		 * after IInfo::Init(), like a reload action in a web page (as
		 * there are no web pages during plugin initialization).
		 *
		 * @param[in] id The ID of an action or QShortcut to register.
		 * @param[in] info The ActionInfo about this shortcut.
		 *
		 * @sa RegisterAction(), RegisterShortcut()
		 */
		void RegisterActionInfo (const QString& id, const ActionInfo& info);

		/** @brief Registers the given global shortcut with the given id.
		 *
		 * Registered global shortcuts need to be announced during
		 * SecondInit() of your plugin by calling the
		 * AnnounceGlobalShorcuts() method.
		 *
		 * @param[in] id The ID of the global shortcut to register.
		 * @param[in] target The object whose \em method will be invoked
		 * on shortcut activation.
		 * @param[in] method The method of the \em object which will be
		 * invoked on shortcut activation.
		 * @param[in] info The ActionInfo about this global shortcut.
		 *
		 * @sa AnnounceGlobalShorcuts()
		 */
		void RegisterGlobalShortcut (const QString& id,
				QObject *target, const QByteArray& method,
				const ActionInfo& info);

		/** @brief Announces the global shortcuts.
		 *
		 * This function announces global shortcuts registered via
		 * RegisterGlobalShortcut() method. Because global shortcuts are
		 * handled by a special plugin like GActs, this function needs to
		 * be called in IInfo::SecondInit() of your plugin.
		 *
		 * @sa RegisterGlobalShortcut()
		 */
		void AnnounceGlobalShorcuts ();

		/** @brief Sets the key sequence for the given action.
		 *
		 * This function updates all the registered actions with the
		 * given ID. It is intended to be called only from
		 * IHaveShortcuts::SetShortcut(), user code should hardly ever
		 * need to call it elsewhere.
		 *
		 * @param[in] id The ID of the action to update.
		 * @param[in] sequences The list of sequences to for the action.
		 */
		void SetShortcut (const QString& id, const QKeySequences_t& sequences);

		/** @brief Returns the map with information about actions.
		 *
		 * The return result is suitable to be returned from
		 * IHaveShortcuts::GetActionInfo().
		 *
		 * @return Action info map.
		 */
		QMap<QString, ActionInfo> GetActionInfo () const;

		/** @brief Utility function equivalent to RegisterAction().
		 *
		 * This function is equivalent to calling
		 * <code>RegisterAction (pair.first, pair.second);</code>.
		 *
		 * @param[in] pair The pair of action ID and the action itself.
		 * @return The shortcut manager object.
		 */
		ShortcutManager& operator<< (const QPair<QString, QAction*>& pair);
	private:
		bool HasActionInfo (const QString&) const;
	};
}
}
