/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_INTERFACES_IACCOUNT_H
#define PLUGINS_AZOTH_INTERFACES_IACCOUNT_H
#include <QFlags>
#include <QMetaType>
#include <QStringList>
#include <interfaces/azothcommon.h>
#include "iclentry.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
namespace Plugins
{
	class IProtocol;
	class ICLEntry;

	/** Represents an account — an entity within IProtocol owning some
	 * ICLEntry objects.
	 */
	class IAccount
	{
	public:
		virtual ~IAccount () {}

		/** Represents the features that may be supported by an acoount.
		 */
		enum AccountFeature
		{
			/** This account can be renamed, so calls to RenameAccount()
			 * would not be senseless.
			 */
			FRenamable = 0x01,

			/** This account supports Extended Away statuses.
			 */
			FSupportsXA = 0x02,

			/** This account has configuration dialog.
			 */
			FHasConfigurationDialog = 0x04,

			/** This account can add accounts to CL while being offline.
			 */
			FCanAddContactsInOffline = 0x08
		};

		Q_DECLARE_FLAGS (AccountFeatures, AccountFeature);

		/** @brief Returns the account object as a QObject.
		 *
		 * @return Account object as QObject.
		 */
		virtual QObject* GetObject () = 0;

		/** @brief Returns the pointer to the parent protocol that this
		 * account belongs to.
		 *
		 * @return The parent protocol of this account.
		 */
		virtual QObject* GetParentProtocol () const = 0;

		/** @brief Returns the OR-ed combination of features of this
		 * account.
		 *
		 * @return The features of this account.
		 */
		virtual AccountFeatures GetAccountFeatures () const = 0;

		/** @brief Returns the list of contact list entries of this
		 * account.
		 *
		 * Typically this would be the list of contacts added to the
		 * account plus any entries that represent multiuser chatrooms,
		 * their participants and such, if applicable.
		 *
		 * @return The list of contact list entries of this account.
		 */
		virtual QList<QObject*> GetCLEntries () = 0;

		/** @brief Returns the human-readable name of this account.
		 *
		 * @return Human-readable name of this account.
		 *
		 * @sa RenameAccount()
		 */
		virtual QString GetAccountName () const = 0;

		/** @brief Returns the nickname of our user.
		 *
		 * @return Nickname of our user.
		 */
		virtual QString GetOurNick () const = 0;

		/** @brief Sets the human-readable name of this account to the
		 * new name.
		 *
		 * @param[in] name The new name of the account.
		 *
		 * @sa GetAccountName()
		 */
		virtual void RenameAccount (const QString& name) = 0;

		/** @brief Returns the ID of this account.
		 *
		 * The returned ID should be unique among all accounts of this
		 * protocol and should not depend on the value of
		 * GetAccountName() (the human-readable name of the account).
		 *
		 * @return The unique and persistent account ID.
		 */
		virtual QByteArray GetAccountID () const = 0;

		/** @brief Request message w/ info/vcard information for the given address.
		 *
		 * The address should be in format compatible with the result of
		 * ICLEntry::GetHumanReadableID() function whenever possible.
		 *
		 * @param[in] address Address or entry ID to query.
		 */
		virtual void QueryInfo (const QString& address) = 0;

		/** @brief Requests the account to open its configuration dialog.
		 */
		virtual void OpenConfigurationDialog () = 0;

		/** @brief Returns the current status of this account.
		 *
		 * @return Current status of this account.
		 */
		virtual EntryStatus GetState () const = 0;

		/** @brief Sets the status of this account.
		 *
		 * If the account was offline, it is expected to connect at this
		 * point automatically.
		 *
		 * @param[in] status The new status of this account.
		 */
		virtual void ChangeState (const EntryStatus& status) = 0;

		/** @brief Synchronizes changes made to this account locally
		 * with any remote, if applicable.
		 */
		virtual void Synchronize () = 0;

		/** @brief Grants authorization to the given entry.
		 *
		 * entry is expected to be deleted in this function.
		 *
		 * @param[in] entry Entry object implementing ICLEntry.
		 */
		virtual void Authorize (QObject *entry) = 0;

		/** @brief Denies authorization for the given entry.
		 *
		 * entry is expected to be deleted in this function.
		 *
		 * @param[in] entry Entry object implementing ICLEntry.
		 */
		virtual void DenyAuth (QObject *entry) = 0;

		/** @brief Requests authorization from the given entry.
		 *
		 * entry is a human-readable identifier of the remote, as
		 * returned by ICLEntry::GetHumanReadableID().
		 *
		 * If applicable and msg is a non-empty string, the msg should
		 * be sent to the entry as the message sent along with the
		 * request.
		 *
		 * name should be the name under which the entry is added to the
		 * contact list, if applicable. If name is an empty string, a
		 * sane default should be used, like the human-readable ID —
		 * entry.
		 *
		 * groups is the list of groups that this entry is added to. If
		 * groups is an emptry list, the behavior should be defined by
		 * the plugin - the entry should either be added to some kind of
		 * default group or have no assigned groups at all. If protocol
		 * supports only one group per entry, the first one in the list
		 * should be used.
		 *
		 * @param[in] entry Human-readable ID of the remote.
		 * @param[in] msg Optional request string to display to remote,
		 * if applicable.
		 */
		virtual void RequestAuth (const QString& entry,
				const QString& msg = QString (),
				const QString& name = QString (),
				const QStringList& groups = QStringList ()) = 0;

		/** @brief This signal should be emitted when new contact list
		 * items appear in this account.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] items The list of newly appeared items.
		 */
		virtual void gotCLItems (const QList<QObject*>& items) = 0;

		/** @brief This signal should be emitted after any contact list
		 * items are removed.
		 *
		 * The reason for removal doesn't matter. It could be a
		 * groupchat participant that exited or changed nickname, or
		 * some other stuff.
		 *
		 * @note This functions is expected to be a signal.
		 *
		 * @param[out] items The list of removed items.
		 */
		virtual void removedCLItems (const QList<QObject*>& items) = 0;

		/** @brief This signal should be emitted when a new groupchat
		 * has been joined.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] groupchat Pointer to the ICLEntry representing
		 * the groupchat.
		 */
		virtual void joinedGroupchat (QObject *groupchat) = 0;

		/** @brief This signal should be emitted when another user
		 * requests authorization from this account.
		 *
		 * When a remote user requests authorization (or subscription in
		 * terms of XMPP, for example) from this account, this signal
		 * should be emitted. The entry is expected to represent the
		 * remote that requested the authorization and it must implement
		 * ICLEntry.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] entry The object representing the requesting
		 * entry, must be an ICLEntry.
		 * @param[out] message Optional request message, if applicable.
		 */
		virtual void authorizationRequested (QObject *entry,
				const QString& message) = 0;

		/** @brief This signal should be emitted when state of this
		 * account changes for whatever reason.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] state New state of this account.
		 */
		virtual void statusChanged (const EntryStatus&) = 0;
	};

	Q_DECLARE_OPERATORS_FOR_FLAGS (IAccount::AccountFeatures);
}
}
}
}

Q_DECLARE_METATYPE (LeechCraft::Plugins::Azoth::Plugins::IAccount*);

Q_DECLARE_INTERFACE (LeechCraft::Plugins::Azoth::Plugins::IAccount,
		"org.Deviant.LeechCraft.Plugins.Azoth.Plugins.IAccount/1.0");


#endif
