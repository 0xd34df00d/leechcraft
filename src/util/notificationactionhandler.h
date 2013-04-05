/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#pragma once

#include <functional>
#include <QObject>
#include <interfaces/structures.h>
#include "utilconfig.h"

namespace LeechCraft
{
namespace Util
{
	/** @brief Aids in adding actions to notifications.
	 *
	 * This class is used to easily add actions to notification entities
	 * created via MakeNotification() or MakeAN() functions. The usage is
	 * quite simple and best illustrated by an example:
	 * \code
	 * Entity e = Util::MakeNotification ("Header", "Title", PInfo_);
	 * auto nah = new Util::NotificationActionHandler (e);
	 * nah->AddFunction ("Open file", [fileURL] () { QDesktopServices::openUrl (fileURL); });
	 * \endcode
	 *
	 * The entity this handler is created upon takes ownership of the
	 * object, so when the last instance of the entity is destroyed this
	 * object is destroyed itself. The object should \em never be
	 * explicitly deleted.
	 *
	 * Some actions don't make sense if some other object is destroyed
	 * after emitting the entity but before the user has reacted to the
	 * event. For example, a user in an IRC channel writes to us a
	 * message then leaves. In this case the "Reply" action will be
	 * invalid and useless.
	 *
	 * To avoid this NotificationActionHandler supports dependent objects
	 * — objects whose destruction will render its actions useless.
	 * Dependent objects are added via the AddDependentObject() method
	 * and their lifetime is tracked automatically.
	 *
	 * @note Only one handler can be created on an entity. If multiple
	 * handlers are created, the last one is used.
	 *
	 * @sa MakeNotification(), MakeAN().
	 */
	class NotificationActionHandler : public QObject
	{
		Q_OBJECT

		Entity& Entity_;
	public:
		/** @brief Type of functions used as actions in the handler.
		 */
		typedef std::function<void ()> Callback_t;
	private:
		QList<QPair<QString, Callback_t>> ActionName2Callback_;

		QList<QPointer<QObject>> DependentObjects_;
	public:
		/** @brief Creates the handler on the given \em entity.
		 *
		 * The entity takes the ownership of the handler.
		 *
		 * @param[in] entity The entity to add actions to.
		 */
		UTIL_API NotificationActionHandler (Entity& entity, QObject* = 0);

		/** @brief Adds an action with the given name.
		 *
		 * This function adds an action to this entity. It will be shown
		 * under the given \em name to the user, and when the user
		 * selects it, \em action will be invoked.
		 *
		 * @param[in] name The human-readable name of the action.
		 * @param[in] action The functor to invoke when the user selects
		 * the action.
		 */
		UTIL_API void AddFunction (const QString& name, Callback_t action);

		/** @brief Adds an object as a dependent object.
		 *
		 * @param[in] object The object actions in this handler depend upon.
		 */
		UTIL_API void AddDependentObject (QObject *object);
	public slots:
		void notificationActionTriggered (int);
	};
}
}
