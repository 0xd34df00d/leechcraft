/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

				/** Represents an account — an entity within IProtocol
				 * owning some ICLEntry objects.
				 */
				class IAccount
				{
				public:
					virtual ~IAccount () {}

					/** Represents the features that may be supproted by
					 * an acoount.
					 */
					enum AccountFeature
					{
						FRenamable				= 0x01, //!< FRenamable This account can be renamed, so calls to RenameAccount() would not be senseless.
						FSupportsXA				= 0x02, //!< FSupportsXA This account supports Extended Away statuses.
						FHasConfigurationDialog	= 0x04  //!< FHasConfigurationDialog This account has configuration dialog.
					};

					Q_DECLARE_FLAGS (AccountFeatures, AccountFeature);

					enum State
					{
						SOffline,
						SOnline,
						SChat,
						SAway,
						SDND,
						SXA,
						SUnavailable,
						SProbe,
						SError,
						SInvalid
					};

					/** Returns the account object as a QObject.
					 *
					 * @return Account object as QObject.
					 */
					virtual QObject* GetObject () = 0;

					/** Returns the pointer to the parent protocol
					 * that this account belongs to.
					 *
					 * @return The parent protocol of this account.
					 */
					virtual IProtocol* GetParentProtocol () const = 0;

					/** Returns the OR-ed combination of features of
					 * this account.
					 *
					 * @return The features of this account.
					 */
					virtual AccountFeatures GetAccountFeatures () const = 0;

					/** @brief Returns the list of contact list entries
					 * of this account.
					 *
					 * Typically this would a list of contacts added to
					 * the account.
					 *
					 * @return The list of contact list entries of this
					 * account.
					 */
					virtual QList<ICLEntry*> GetCLEntries () = 0;

					/** @brief Returns the human-readable name of this
					 * account.
					 *
					 * @return Human-readable name of this account.
					 *
					 * @sa RenameAccount()
					 */
					virtual QString GetAccountName () const = 0;

					/** @brief Sets the human-readable name of this
					 * account to the new name.
					 *
					 * @param[in] name The new name of the account.
					 *
					 * @sa GetAccountName()
					 */
					virtual void RenameAccount (const QString& name) = 0;

					/** @brief Returns the ID of this account.
					 *
					 * The returned ID should be unique among all
					 * accounts and should not depend on the value of
					 * GetAccountName() (the human-readable name of the
					 * account).
					 *
					 * @return The unique and persistent account ID.
					 */
					virtual QByteArray GetAccountID () const = 0;

					/** @brief Requests the account to open its
					 * configuration dialog.
					 */
					virtual void OpenConfigurationDialog () = 0;

					/** @brief Sets the state of this account.
					 *
					 * If the account was offline, it is expected to
					 * connect at this point automatically.
					 *
					 * @param[in] state The new state of the account.
					 * @param[in] msg The status message, if it makes
					 * sense.
					 */
					virtual void ChangeState (State state,
							const QString& msg = QString ()) = 0;

					/** Synchronizes changes between this account and
					 * the server, if applicable.
					 */
					virtual void Synchronize () = 0;

					/** @brief This signal is emitted when a new group
					 * chat has been joined.
					 *
					 * @note This function is expected to be a signal.
					 *
					 * @param[out] groupchat Pointer to the contact list
					 * entry representing the groupchat.
					 */
					virtual void joinedGroupchat (QObject *groupchat) = 0;
				};

				Q_DECLARE_OPERATORS_FOR_FLAGS (IAccount::AccountFeatures);
			}
		}
	}
}

Q_DECLARE_INTERFACE (LeechCraft::Plugins::Azoth::Plugins::IAccount,
		"org.Deviant.LeechCraft.Plugins.Azoth.Plugins.IAccount/1.0");


#endif
