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

#ifndef PLUGINS_AZOTH_INTERFACES_IPROTOCOL_H
#define PLUGINS_AZOTH_INTERFACES_IPROTOCOL_H
#include <QFlags>
#include <QMetaType>

class QObject;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			namespace Plugins
			{
				class IAccount;
				class IProtocolPlugin;

				/** @brief Represents a protocol.
				 *
				 * IProtocol class represents a single protocol with its
				 * own set of accounts.
				 *
				 * Implementations of this interface are expected to
				 * have the following signals:
				 * - accountAdded(QObject*), which is emitted when a new
				 *   account has been added. The parameter is expected
				 *   to be a pointer to the newly added IAccount
				 *   instance.
				 */
				class IProtocol
				{
				public:
					virtual ~IProtocol () {}

					enum ProtocolFeature
					{
						/** Multiuser chats are possible in this proto.
						 */
						PFSupportsMUCs,
						/** One could join MUCs as he wishes.
						 */
						PFMUCsJoinable
					};

					Q_DECLARE_FLAGS (ProtocolFeatures, ProtocolFeature);

					/** Returns the protocol object as a QObject.
					 *
					 * @return Protocol object as QObject.
					 */
					virtual QObject* GetObject () = 0;

					/** Returns the list of features supported by this
					 * protocol.
					 */
					virtual ProtocolFeatures GetFeatures () const = 0;

					/** Returns the accounts registered within this
					 * protocol.
					 *
					 * @return The list of accoutns of this protocol.
					 */
					virtual QList<QObject*> GetRegisteredAccounts () = 0;

					/** Returns the pointer to the parent protocol
					 * plugin that this protocol belongs to.
					 *
					 * @return The parent protocol plugin of this
					 * protocol.
					 */
					virtual QObject* GetParentProtocolPlugin () const = 0;

					/** Returns the human-readable name of this
					 * protocol, like "Jabber" or "ICQ".
					 *
					 * @return Human-readable name of the protocol.
					 */
					virtual QString GetProtocolName () const = 0;

					/** Returns the protocol ID, which must be unique
					 * among all the protocols.
					 *
					 * @return The unique ID of this protocol.
					 */
					virtual QByteArray GetProtocolID () const = 0;

					/** @brief Notifies the protocol that a new account
					 * should be registered.
					 *
					 * Protocol plugin is expected to ask the user for
					 * account details, register the account and emit
					 * the accountAdded(QObject*) signal.
					 */
					virtual void InitiateAccountRegistration () = 0;

					/** @brief Notifies the plugin that a MUC join
					 * dialog should be opened.
					 */
					virtual void InitiateMUCJoin () = 0;

					/** @brief Removes the given account.
					 *
					 * This function shouldn't ask anything from the
					 * user, just remove the account.
					 *
					 * If the account is not registered, this function
					 * should do nothing.
					 */
					virtual void RemoveAccount (QObject *account) = 0;

					virtual void accountAdded (QObject *account) = 0;
					virtual void accountRemoved (QObject *account) = 0;
				};

				Q_DECLARE_OPERATORS_FOR_FLAGS (IProtocol::ProtocolFeatures);
			}
		}
	}
}

Q_DECLARE_METATYPE (LeechCraft::Plugins::Azoth::Plugins::IProtocol*);
Q_DECLARE_INTERFACE (LeechCraft::Plugins::Azoth::Plugins::IProtocol,
		"org.Deviant.LeechCraft.Plugins.Azoth.Plugins.IProtocol/1.0");

#endif
