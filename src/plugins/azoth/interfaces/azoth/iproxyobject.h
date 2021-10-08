/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QString>
#include "azothcommon.h"
#include "ihaveavatars.h"

class QObject;

template<typename>
class QFuture;

namespace LC
{
namespace Util
{
	class ResourceLoader;
}

namespace Azoth
{
	class IFormatterProxyObject
	{
	public:
		virtual ~IFormatterProxyObject () {}

		/** @brief Generates the nickname colors for the given scheme.
		 *
		 * If the scheme is empty or equals "hash", then a random set of
		 * colors is generated based on \em bg and settings.
		 *
		 * Otherwise, scheme is interpreted as space-separated list of
		 * colors, either named, like "green" or "cyan", or their RGB
		 * values in forms like "#FA12BB".
		 *
		 * @param[in] scheme The color scheme to use.
		 * @param[in] bg The background color to generate colors for.
		 * Pass an invalid color to use application palette's background.
		 * @return The list of colors matching the given color scheme.
		 */
		virtual QList<QColor> GenerateColors (const QString& scheme, QColor bg) const = 0;

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
		 * This function should be used to format the date of a
		 * \em message when displaying it.
		 *
		 * @param[in] date The timestamp of the message to format.
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

		virtual QString EscapeBody (QString body, IMessage::EscapePolicy) const = 0;

		/** @brief Formats the body for the given message.
		 *
		 * This function should be used to format the body of the given
		 * message.
		 *
		 * This function also accepts the list of colors used for
		 * nick coloring in the current chat window, since the
		 * \em body of the \em message may also be colored if it contains
		 * other participants' nicks.
		 *
		 * @param[in] body The body to format.
		 * @param[in] message The message object implementing IMessage.
		 * @param[in] coloring The set of colors used for nick coloring.
		 * @return The formatted body.
		 */
		virtual QString FormatBody (QString body, QObject *message, const QList<QColor>& coloring) const = 0;

		/** @brief Preprocesses the message before displaying it.
		 *
		 * This function should be called once on each message before
		 * displaying it.
		 *
		 * @param[in] message The message to preprocess.
		 */
		virtual void PreprocessMessage (QObject *message) = 0;

		virtual void FormatLinks (QString& body) = 0;

		virtual QStringList FindLinks (const QString&) = 0;
	};

	class IAvatarsManager
	{
	protected:
		virtual ~IAvatarsManager () {}
	public:
		virtual QFuture<QImage> GetAvatar (QObject *entryObj, IHaveAvatars::Size size) = 0;

		virtual QFuture<std::optional<QByteArray>> GetStoredAvatarData (const QString& entryId, IHaveAvatars::Size size) = 0;
	};

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
		virtual QByteArray AuthStatusToString (AuthStatus status) const = 0;

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
		virtual AuthStatus AuthStatusFromString (const QByteArray& str) const = 0;

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
		 * The \em accID may be empty, in which case all available
		 * accounts are queried.
		 *
		 * @param[in] entryID The entry ID.
		 * @param[in] accID The account ID to which this entry
		 * belongs.
		 *
		 * @return Entry object, or NULL if no such entry exists.
		 */
		virtual QObject* GetEntry (const QString& entryID, const QString& accID = {}) const = 0;

		/** @brief Opens the chat with the given entry.
		 *
		 * This function allows one to open a chat with the given entry
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

		virtual QWidget* FindOpenedChat (const QString& entryID, const QByteArray& accID) const = 0;

		virtual Util::ResourceLoader* GetResourceLoader (PublicResourceLoader loader) const = 0;

		virtual QIcon GetIconForState (State state) const = 0;

		virtual QObject* CreateCoreMessage (const QString& body,
				const QDateTime& date,
				IMessage::Type type,
				IMessage::Direction dir,
				QObject *other,
				QObject *parent = nullptr) = 0;

		virtual QString ToPlainBody (QString body) = 0;

		virtual bool IsMessageRead (QObject *msgObj) = 0;

		virtual void MarkMessagesAsRead (QObject *entryObject) = 0;

		/** @brief Formats the \em datetime according to current locale.
		 *
		 * This method returns the \em datetime formatted according to
		 * the current locale. The timezones are respected.
		 *
		 * The primary difference from <code>QLocale {}.toString (datetime)</code>
		 * is that Boost.Locale backend is used if available to avoid
		 * nasty timezone-related problems.
		 *
		 * @param[in] datetime The timestamp to format.
		 * @return The formatted \em datetime as string.
		 */
		virtual QString PrettyPrintDateTime (const QDateTime& datetime) = 0;

		/** @brief Tries to find a cusotm status under the given \em name.
		 *
		 * @param[in] name The name of the custom status.
		 * @return A custom status if it's found, or an empty
		 * std::optional object otherwise.
		 *
		 * @sa GetCustomStatusNames()
		 */
		virtual std::optional<CustomStatus> FindCustomStatus (const QString& name) const = 0;

		/** @brief Returns the names of all available custom statuses.
		 *
		 * The returned names can be passed to FindCustomStatus() to
		 * obtain the full CustomStatus structure.
		 *
		 * @return The names of custom statuses, suitable for
		 * FindCustomStatus().
		 *
		 * @sa FindCustomStatus()
		 */
		virtual QStringList GetCustomStatusNames () const = 0;

		virtual void RedrawItem (QObject*) const = 0;

		virtual QObject* GetFirstUnreadMessage (QObject *entryObj) const = 0;

		virtual QImage GetDefaultAvatar (int size = -1) const = 0;

		virtual IFormatterProxyObject& GetFormatterProxy () = 0;

		virtual IAvatarsManager* GetAvatarsManager () = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IProxyObject,
		"org.LeechCraft.Azoth.IProxyObject/1.0")
