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

#ifndef PLUGINS_AZOTH_INTERFACES_IPROXYOBJECT_H
#define PLUGINS_AZOTH_INTERFACES_IPROXYOBJECT_H
#include <QString>
#include "azothcommon.h"

class QObject;
class QWebFrame;

namespace LeechCraft
{
namespace Azoth
{
	class IProxyObject
	{
	public:
		virtual ~IProxyObject () {}

		/** @brief Retrieves the password for the given account.
		 *
		 * Returns password for the given account, or a null string if
		 * no password is stored. The password should be previously set
		 * (stored) by the call to SetPassword().
		 *
		 * The account object should implement the IAccount interface.
		 * Accounts are distinguished by their IDs.
		 *
		 * @param[in] account The account for which to retrieve the
		 * password. The object should implement IAccount.
		 * @return The stored password or null string if no password is
		 * stored.
		 *
		 * @note This function may return null passwords even after
		 * corresponding calls to SetPassword() since the implementation
		 * uses secure storage plugins to store passwords, and if no
		 * such plugins are installed, no passwords are stored.
		 *
		 * @sa SetPassword(), IAccount
		 */
		virtual QString GetPassword (QObject *account) = 0;

		/** @brief Stores the password for the given account.
		 *
		 * The password set by this function overwrites any previously
		 * set ones. After this function is called for given account,
		 * the GetPassword() would return either the last stored
		 * password or null string if no password storage plugin is
		 * installed.
		 *
		 * The account object should implement the IAccount interface.
		 * Accounts are distinguished by their IDs.
		 *
		 * @param[in] password The password string to store. Null string
		 * may be used to overwrite/clear the saved password.
		 * @param[in] account The account for which the password should
		 * be stored. The object should implement IAccount.
		 *
		 * @sa GetPassword(), IAccount
		 */
		virtual void SetPassword (const QString& password, QObject *account) = 0;

		/** @brief Returns the name of the OS Azoth runs under.
		 *
		 * @return The name of the operating system.
		 */
		virtual QString GetOSName () = 0;

		/** @brief Returns a human-readable string for the given state.
		 *
		 * @return Human-readable string describing state.
		 */
		virtual QString StateToString (State state) const = 0;

		/** @brief Returns serialized name of the authorization status.
		 *
		 * @return Serialized name of the auth status.
		 *
		 * @sa AuthStatusFromString()
		 */
		virtual QString AuthStatusToString (AuthStatus status) const = 0;

		/** @brief Converts string representation to AuthStatus element.
		 *
		 * The string that's passed should be the one previously
		 * returned from AuthStatusToString().
		 *
		 * @param[in] str String previously returned from
		 * AuthStatusToString().
		 *
		 * @return AuthStatus element or ASNone if str is invalid.
		 *
		 * @sa AuthStatusToString()
		 */
		virtual AuthStatus AuthStatusFromString (const QString& str) const = 0;
		
		/** @brief Returns the account object for the given account ID.
		 * 
		 * If there is no such account, NULL is returned.
		 * 
		 * @param[in] accID The unique account ID.
		 * 
		 * @return Account object implementing IAccount, or NULL if no
		 * such account exists.
		 */
		virtual QObject* GetAccount (const QString& accID) const = 0;
		
		/** @brief Returns the entry object for the given entry ID.
		 * 
		 * @param[in] entryID The entry ID.
		 * @param[in] accID The account ID to which this entry
		 * belongs.
		 * 
		 * @return Entry object, or NULL if no such entry exists.
		 */
		virtual QObject* GetEntry (const QString& entryID, const QString& accID) const = 0;
		
		virtual QString GetSelectedChatTemplate () const = 0;
		
		virtual void AppendMessageByTemplate (QWebFrame*, QObject*, const QString&, bool, bool) const = 0;
		
		virtual QList<QColor> GenerateColors (const QString& scheme) const = 0;
		virtual QString GetNickColor (const QString& nick, const QList<QColor>& colors) const = 0;

		virtual QString FormatDate (QDateTime, QObject*) const = 0;
		virtual QString FormatNickname (QString, QObject*, const QString&) const = 0;
		virtual QString FormatBody (QString, QObject*) const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IProxyObject,
		"org.Deviant.LeechCraft.Plugins.Azoth.Plugins.IProxyObject/1.0");

#endif
