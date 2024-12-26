/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <optional>
#include <QFuture>
#include <QImage>
#include <interfaces/structures.h>
#include <interfaces/an/constants.h>
#include <interfaces/an/ianemitter.h>
#include <interfaces/core/icoreproxy.h>
#include "xpcconfig.h"

class IEntityManager;
class QStandardItem;

namespace LC::Util
{
	/** @brief Creates an Advanced Notifications-enabled notify entity.
	 *
	 * Returns an entity with the given \em header, \em text and a bunch
	 * of other parameters that can be handled by Advanced Notifications.
	 *
	 * The returned entity will also be handled by a visual notifications
	 * plugin if AN is not present. To avoid this set the MIME type of
	 * the result to <em>x-leechcraft/notification+advanced</em>.
	 *
	 * Refer to the <a href="http://leechcraft.org/development-advanced-notifications">hand-written documentation</a>
	 * for more information about using Advanced Notifications.
	 *
	 * @param[in] header The header of the notification. This field will
	 * also be used if AN is not present.
	 * @param[in] text The text of the notification. This field will also
	 * be used if AN is not present.
	 * @param[in] priority The priority of this notification.
	 * @param[in] senderID The ID of the plugin sending this notification.
	 * @param[in] cat The category of this notification (one of Cat
	 * constants in interfaces/an/constants.h).
	 * @param[in] type The type of this notification (one of Type
	 * constants in interfaces/an/constants.h).
	 * @param[in] id The ID of this notification, used to group
	 * consecutive notifications about similar events like incoming
	 * message from the same IM contact.
	 * @param[in] visualPath The list of names for a menu-like structure
	 * wishing to show this notification.
	 * @param[in] delta The change of count of events with this id, or 0
	 * to use count.
	 * @param[in] count The total count of events with this id, used if
	 * delta is 0.
	 * @param[in] fullText The full text of this notification. If null,
	 * the text parameter is used.
	 * @param[in] extendedText The extended text of this notification. If
	 * null, the text parameter is used.
	 *
	 * @sa MakeANCancel()
	 * @sa MakeANRule()
	 */
	UTIL_XPC_API Entity MakeAN (const QString& header, const QString& text, Priority priority,
			const QString& senderID, const QString& cat, const QString& type,
			const QString& id, const QStringList& visualPath,
			int delta = 1, int count = 0,
			const QString& fullText = QString (), const QString& extendedText = QString ());

	/** @brief Creates an Entity defining an Advanced Notifications rule.
	 *
	 * Returns an entity describing a notifications rule triggering under
	 * various conditions, defined by the parameters of this function.
	 *
	 * @param[in] title The human-readable title of the rule.
	 * @param[in] senderID The plugin ID of the sender (must not be
	 * empty).
	 * @param[in] category The category of the event (must not be empty).
	 * @param[in] types The types of events in the given \em category. If
	 * this list is empty, every event type matches.
	 * @param[in] flags The flags describing the notification behavior for
	 * for this rule
	 * @param[in] openConfiguration Whether the configuration widget for
	 * the just created rule should be opened automatically.
	 * @param[in] fields The list of pairs of a field ID (as in
	 * LC::AN::FieldData::ID_) and corresponding field value as ANFieldValue.
	 * @return The Entity object describing a notifications rule for the
	 * passed parameters.
	 *
	 * @sa MakeAN()
	 */
	UTIL_XPC_API Entity MakeANRule (const QString& title,
			const QString& senderID,
			const QString& category,
			const QStringList& types,
			AN::NotifyFlags flags = AN::NotifyNone,
			bool openConfiguration = false,
			const QList<QPair<QString, AN::FieldValue>>& fields = {});

	/** @brief Returns the data filter plugins that can handle \em data.
	 *
	 * This function queries all available data filters plugins if they
	 * can handle \em data and returns the list of object instances that
	 * can handle it. The object instances are guaranteed to implement
	 * the IDataFilter interface as well as IEntityHandler interface.
	 *
	 * The \em manager is used to obtain the list of plugins. It can be
	 * obtained from the ICoreProxy_ptr object that is passed to your
	 * plugin's <em>Init()</em> function.
	 *
	 * @param[in] data The data to query.
	 * @param[in] manager The manager used to get the plugins.
	 * @return The list of data filters that can handle \em data.
	 */
	UTIL_XPC_API QList<QObject*> GetDataFilters (const QVariant& data, IEntityManager *manager);

	/** @brief An utility function to make a Entity.
	 *
	 * Creates a Entity that wraps the given entity from
	 * given location with parameterrs identified by tp and given
	 * mime type (which is null by default).
	 *
	 * This function is provided for convenience and is equivalent
	 * to manually filling the Entity.
	 *
	 * @param[in] entity The Entity_ field of the Entity.
	 * @param[in] location The Location_ field of the Entity.
	 * @param[in] tp The Params_ field of the Entity.
	 * @param[in] mime The Mime_ field of the Entity.
	 * @return The resulting Entity.
	 *
	 * @sa Entity, MakeNotification()
	 */
	UTIL_XPC_API Entity MakeEntity (const QVariant& entity,
			const QString& location,
			LC::TaskParameters tp,
			const QString& mime = QString ());

	/** @brief An utility function to make a Entity with
	 * notification.
	 *
	 * Creates a Entity that holds information about
	 * user-visible notification. These notifications have
	 * "x-leechcraft/notification" MIME.
	 *
	 * You can further customize the returned Entity to suit
	 * your exact needs. See the documentation for Entity
	 * about such entities.
	 *
	 * @param[in] header The header of the notification.
	 * @param[in] text The text of the notification.
	 * @param[in] priority The priority level of the notification.
	 * @return The Entity containing the corresponding
	 * notification.
	 *
	 * @sa Entity, MakeEntity()
	 */
	UTIL_XPC_API Entity MakeNotification (const QString& header,
			const QString& text,
			Priority priority);

	/** @brief Makes an event for canceling another Advanced
	 * Notifications event.
	 *
	 * Creates an Entity that cancels a previously generated
	 * Advanced Notifications event. The returned entity can be
	 * then emitted to notify plugins that the given event has been
	 * canceled.
	 *
	 * @param[in] event The event to cancel.
	 * @return The Entity canceling the given event.
	 */
	UTIL_XPC_API Entity MakeANCancel (const Entity& event);

	/** @brief Makes an event for canceling another Advanced
	 * Notifications event.
	 *
	 * Creates an Entity that cancels a previously generated
	 * Advanced Notifications event. The returned entity can be
	 * then emitted to notify plugins that the given event has been
	 * canceled.
	 *
	 * This function doesn't take a previously created entity as the
	 * other overload does. Instead, it plainly creates the required
	 * entity from the given senderId and eventId. They should match
	 * those of the event in question.
	 *
	 * @param[in] senderId The ID of the sender of the event that is
	 * to be canceled.
	 * @param[in] eventId The ID of the event that is to be canceled.
	 * @return The Entity canceling the given event.
	 */
	UTIL_XPC_API Entity MakeANCancel (const QString& senderId, const QString& eventId);

	/** @brief Returns persistent data stored under given \em key.
	 *
	 * The persistent data itself is stored in plugins implementing the
	 * IPersistentStoragePlugin interface. This function uses the passed
	 * \em proxy to get the list of those.
	 *
	 * If no data is found under the given \em key, a null QVariant is
	 * returned.
	 *
	 * @param[in] key The key to look for.
	 * @param[in] proxy The ICoreProxy object for getting the list of
	 * persistent storage plugins.
	 * @return The data stored under the given \em key as a QVariant,
	 * or a null QVariant of no data is found (and, particularly, if no
	 * storage plugins are available).
	 *
	 * @sa IPersistentStoragePlugin
	 */
	UTIL_XPC_API QVariant GetPersistentData (const QByteArray& key,
			const ICoreProxy_ptr& proxy);

	/** @brief Sets the progress values on the given \em row.
	 *
	 * This function first retrieves the QStandardItem object at the
	 * position defined by JobHolderColumn::JobProgress in the passed
	 * \em row and then sets its text to \em text and updates the
	 * ProcessStateInfo structure under the JobHolderRole::ProcessState
	 * role to have the given amount of \em done and \em total.
	 *
	 * @param[in] row The row to set data on.
	 * @param[in] done The amount of work done.
	 * @param[in] total The total amount of work.
	 * @param[in] text The text that the progress-related item should
	 * have in the \em row.
	 *
	 * @sa ProcessStateInfo
	 */
	UTIL_XPC_API void SetJobHolderProgress (const QList<QStandardItem*>& row,
			qint64 done, qint64 total, const QString& text);

	/** @brief Sets the \em done and \em total progress values on the
	 * given \em item.
	 *
	 * This function updates the ProcessStateInfo structure stored under
	 * the JobHolderRole::ProcessState role in the given \em item.
	 *
	 * @param[in] item The item to set data on.
	 * @param[in] done The amount of work done.
	 * @param[in] total The total amount of work.
	 *
	 * @sa ProcessStateInfo
	 */
	UTIL_XPC_API void SetJobHolderProgress (QStandardItem *item, qint64 done, qint64 total);

	UTIL_XPC_API void InitJobHolderRow (const QList<QStandardItem*>& row);

	using LazyNotificationPixmap_t = std::function<std::optional<QFuture<QImage>> ()>;
}

Q_DECLARE_METATYPE (LC::Util::LazyNotificationPixmap_t)
