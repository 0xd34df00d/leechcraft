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

#ifndef PLUGINS_AZOTH_INTERFACES_IPROXYOBJECT_H
#define PLUGINS_AZOTH_INTERFACES_IPROXYOBJECT_H
#include <QString>
#include "azothcommon.h"

class QObject;
class QWebFrame;

namespace LeechCraft
{
namespace Util
{
	class ResourceLoader;
}

namespace Azoth
{
	class IProxyObject
	{
	public:
		virtual ~IProxyObject () {}

		enum PublicResourceLoader
		{
			PRLClientIcons,
			PRLStatusIcons,
			PRLSystemIcons
		};

		/** @brief Returns the Core's settings manager object.
		 *
		 * The returned object's properties could be queried to find out
		 * the settings and parameters of Core's settings.
		 *
		 * @return Core's settings manager object.
		 */
		virtual QObject* GetSettingsManager () = 0;

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
		 * @sa GetAccountPassword(), SetPassword(), IAccount
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

		/** @brief Retrieves password for the given account, asking user
		 * if needed.
		 *
		 * Returns password for the given account. If no password is
		 * stored, this function asks user to enter one and tries to
		 * store it after that. If the user refuses to enter a password,
		 * a null string would be returned.
		 *
		 * If there was no password and user entered a non-null string,
		 * this function would call SetPassword() by itself, so there is
		 * no need to call SetPassword() explicitly.
		 *
		 * This function may also ignore the already stored password if
		 * useStored is set to false. This is useful, for example, when
		 * a password previously returned by this function turned out to
		 * be wrong.
		 *
		 * The account object should implement the IAccount interface.
		 * Accounts are distinguished by their IDs.
		 *
		 * @param[in] account The account for which to retrieve the
		 * password. The object should implement IAccount.
		 * @param[in] useStored Whether returning already stored
		 * password is OK.
		 * @return The stored password, or user-entered password if
		 * there is no stored password or useStored is false, or null
		 * string if there is no stored password and user refused to
		 * enter one.
		 *
		 * @sa GetPassword()
		 */
		virtual QString GetAccountPassword (QObject *account, bool useStored = true) = 0;

		/** @brief Returns the name of the OS Azoth runs under.
		 *
		 * @return The name of the operating system.
		 */
		virtual QString GetOSName () = 0;

		/** @brief Queries whether autojoin is allowed.
		 *
		 * @return Whether autojoin is allowed.
		 */
		virtual bool IsAutojoinAllowed () = 0;

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

		/** @brief Returns all the accounts registered in Azoth.
		 *
		 * @return The list of objects implementing IAccount.
		 */
		virtual QList<QObject*> GetAllAccounts () const = 0;

		/** @brief Returns the entry object for the given entry ID.
		 *
		 * @param[in] entryID The entry ID.
		 * @param[in] accID The account ID to which this entry
		 * belongs.
		 *
		 * @return Entry object, or NULL if no such entry exists.
		 */
		virtual QObject* GetEntry (const QString& entryID, const QString& accID) const = 0;

		/** @brief Opens the chat with the given entry.
		 *
		 * This function allows to open a chat with the given entry
		 * identified by entryID for the given account identified by
		 * accId.
		 *
		 * @param[in] entryID The ID of the entry to open chat with.
		 * @param[in] accID The ID of the account where entryID belongs.
		 * @param[in] message Prepare this text in the message editor.
		 * @param[in] variant Select this variant, if available.
		 */
		virtual void OpenChat (const QString& entryID,
				const QString& accID,
				const QString& message = QString (),
				const QString& variant = QString ()) const = 0;

		/** @brief Generates the nickname colors for the given scheme.
		 *
		 * If the scheme is empty or equals "hash", then a random set of
		 * colors is generated based on current palette and settings.
		 * Otherwise, scheme is interpreted as space-separated list of
		 * colors, either named, like "green" or "cyan", or their RGB
		 * values in forms like "#FA12BB".
		 *
		 * @param[in] scheme The color scheme to use.
		 * @return The list of colors matching the given color scheme.
		 */
		virtual QList<QColor> GenerateColors (const QString& scheme) const = 0;

		/** @brief Returns the color for the given nick and color set.
		 *
		 * This function should be used to choose a color for the given
		 * nick. Internally, it calculates a hash from the nick and uses
		 * it to choose a corresponding color.
		 *
		 * @param[in] nick The nickname for which to choose the color.
		 * @param[in] colors The list of colors to choose from.
		 * @return The color name to use.
		 */
		virtual QString GetNickColor (const QString& nick, const QList<QColor>& colors) const = 0;

		/** @brief Formats the date for the given message.
		 *
		 * This function should be used to format the date when
		 * displaying the given message.
		 *
		 * @param[in] date The date to format.
		 * @param[in] message The message object implementing IMessage.
		 * @return The formatted date string.
		 */
		virtual QString FormatDate (QDateTime date, QObject *message) const = 0;

		/** @brief Formats the nickname for the given message and color.
		 *
		 * This function should be used to format the nick when
		 * displaying the given message. The color should be the one
		 * from GetNickColor().
		 *
		 * @param[in] nick The nickname to format.
		 * @param[in] message The message object implementing IMessage.
		 * @param[in] color The color of the nickname.
		 * @return The formatted nickname.
		 */
		virtual QString FormatNickname (QString nick, QObject *message, const QString& color) const = 0;

		/** @brief Formats the body for the given message.
		 *
		 * This function should be used to format the body of the given
		 * message.
		 *
		 * @param[in] body The body to format.
		 * @param[in] message The message object implementing IMessage.
		 * @return The formatted body.
		 */
		virtual QString FormatBody (QString body, QObject *message) const = 0;

		/** @brief Preprocesses the message before displaying it.
		 *
		 * This function should be called once on each message before
		 * displaying it.
		 *
		 * @param[in] message The message to preprocess.
		 */
		virtual void PreprocessMessage (QObject *message) = 0;

		virtual Util::ResourceLoader* GetResourceLoader (PublicResourceLoader loader) const = 0;

		virtual QIcon GetIconForState (State state) const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IProxyObject,
		"org.Deviant.LeechCraft.Plugins.Azoth.Plugins.IProxyObject/1.0");

#endif
