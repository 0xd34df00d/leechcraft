/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

class QObject;
class QString;
class QKeySequence;

template<typename T>
class QList;

/** @brief Proxy for requesting shortcuts from the shortcut manager in
 * the Core.
 *
 * The plugin can communicate with the shortcut manager via this proxy.
 *
 * @sa IHaveShortcuts::SetShortcutProxy().
 */
class Q_DECL_EXPORT IShortcutProxy
{
public:
	/** @brief Checks whether a given object has been registered already.
	 *
	 * @param[in] object The object to check.
	 * @return Returns whether the \em object has been already registered.
	 */
	virtual bool HasObject (QObject *object) const = 0;

	/** @brief Returns a QKeySequence for the given action.
	 *
	 * Returns a list of key sequences for the action with given id for
	 * the given object which is currently set in the shortcut manager.
	 * The id is the same as in return value of
	 * IHaveShortcuts::GetActionInfo().
	 *
	 * The object is used to distinguish between ids of different
	 * plugins. It can be said that object defines the context for the
	 * id.
	 *
	 * @param[in] object The object that should be checked.
	 * @param[in] id ID of the action.
	 * @return The key sequences for the passed action.
	 */
	virtual QList<QKeySequence> GetShortcuts (QObject *object, const QByteArray& id) = 0;

	virtual ~IShortcutProxy () { }
};

Q_DECLARE_INTERFACE (IShortcutProxy, "org.Deviant.LeechCraft.IShortcutProxy/1.0")
