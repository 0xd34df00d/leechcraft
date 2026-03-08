/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/azothcommon.h>
#include "../azothutilconfig.h"

namespace LC::Azoth::Emitters
{
	class AZOTH_UTIL_API Account : public QObject
	{
		Q_OBJECT
	public:
		using QObject::QObject;
	signals:
		/** @brief This signal should be emitted when account is renamed.
		 *
		 * This signal should be emitted even after an explicit call to
		 * RenameAccount().
		 *
		 * @param[out] newName The new name of this account.
		 */
		void accountRenamed (const QString& newName);

		/** @brief This signal should be emitted when new contact list
		 * items appear in this account.
		 *
		 * @param[out] items The list of newly appeared items.
		 */
		void gotCLItems (const QList<QObject*>& items);

		/** @brief This signal should be emitted after any contact list
		 * items are removed.
		 *
		 * The reason for removal doesn't matter. It could be a
		 * groupchat participant that exited or changed nickname, or
		 * some other stuff.
		 *
		 * @param[out] items The list of removed items.
		 */
		void removedCLItems (const QList<QObject*>& items);

		/** @brief This signal should be emitted when another user
		 * requests authorization from this account.
		 *
		 * When a remote user requests authorization (or subscription in
		 * terms of XMPP, for example) from this account, this signal
		 * should be emitted. The entry is expected to represent the
		 * remote that requested the authorization and it must implement
		 * ICLEntry.
		 *
		 * @param[out] entry The object representing the requesting
		 * entry, must be an ICLEntry.
		 * @param[out] message Optional request message, if applicable.
		 *
		 * @sa Authorize(), DenyAuth(), RequestAuth(),
		 * itemSubscribed(), itemUnsubscribed(),
		 * itemCancelledSubscription(), itemGrantedSubscription()
		 */
		void authorizationRequested (QObject *entry, const QString& message);

		/** @brief This signal should be emitted when an already added
		 * entry has just subscribed to us.
		 *
		 * If the item didn't previously exist, the proper gotCLItems()
		 * signal should be emitted before this one, of course.
		 *
		 * @param[out] entry The object representing the just subscribed
		 * entry in the contact list, must be an ICLEntry.
		 * @param[out] message An optional reason message.
		 *
		 * @sa Authorize(), DenyAuth(), RequestAuth(),
		 * authorizationRequested(), itemUnsubscribed(),
		 * itemCancelledSubscription(), itemGrantedSubscription()
		 */
		void itemSubscribed (QObject *entry, const QString& message);

		/** @brief This signal should be emitted when an already added
		 * entry has just unsubscribed from us.
		 *
		 * If the item didn't exist in the roster, another overload of
		 * itemUnsubscribed() should be used, which takes the entry's ID
		 * as first parameter.
		 *
		 * @param[out] entry The object representing the unsubscribed
		 * entry in the contact list, must be an ICLEntry.
		 * @param[out] message An optional reason message.
		 *
		 * @sa Authorize(), DenyAuth(), RequestAuth(),
		 * authorizationRequested(), itemSubscribed(),
		 * itemCancelledSubscription(), itemGrantedSubscription()
		 */
		void itemUnsubscribed (QObject *entry, const QString& message);

		/** @brief This signal should be emitted when a non-roster item
		 * has just unsubscribed from us.
		 *
		 * @param[out] entryID The ID of just unsubscribed entry.
		 * @param[out] message An optional reason message.
		 *
		 * @sa Authorize(), DenyAuth(), RequestAuth(),
		 * authorizationRequested(), itemSubscribed(),
		 * itemCancelledSubscription(), itemGrantedSubscription()
		 */
		void itemUnsubscribed (const QString& entryID, const QString& message);

		/** @brief This signal should be emitted when a roster item
		 * cancels (or denies) our subscription.
		 *
		 * @param[out] entry The object representing the entry that
		 * granted the subscription, must be an ICLEntry.
		 * @param[out] message Optional reason message.
		 *
		 * @sa RequestAuth(), authorizationRequested(), itemSubscribed(),
		 * itemGrantedSubscription()
		 */
		void itemCancelledSubscription (QObject *entry, const QString& message);

		/** @brief This signal should be emitted when a roster item
		 * grants us subscription.
		 *
		 * @param[out] entry The object representing the entry that
		 * granted the subscription, must be an ICLEntry.
		 * @param[out] message Optional reason message.
		 *
		 * @sa RequestAuth(), authorizationRequested(), itemSubscribed(),
		 * itemCancelledSubscription()
		 */
		void itemGrantedSubscription (QObject *entry, const QString& message);

		/** @brief This signal should be emitted when status of this
		 * account changes for whatever reason.
		 *
		 * @param[out] status New status of this account.
		 */
		void statusChanged (const EntryStatus& status);

		/** @brief This signal should be emitted whenever a MUC
		 * invitation has been received.
		 *
		 * The ident parameter contains the map with the identifying
		 * data suitable for the IMUCJoinWidget of this account. Refer
		 * to IMUCJoinWidget documentation for more information.
		 *
		 * @param[out] ident MUC identifying data for IMUCJoinWidget.
		 * @param[out] inviter The inviter's source ID or nickname.
		 * @param[out] reason An optional reason string.
		 */
		void mucInvitationReceived (const QVariantMap& ident, const QString& inviter, const QString& reason);
	};
}
