/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkine
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

#include <QMetaType>

class QObject;
class QIcon;

namespace LeechCraft
{
namespace Blogique
{
	class IBloggingPlatformPlugin;

	/** @brief Represents a protocol.
	 *
	 * IBloggingPlatform class represents a single blogging platform with its own set of
	 * accounts.
	 *
	 * When user decides to add a new account within this protocol, the
	 * GetAccountRegistrationWidgets() function is called to get the
	 * list of widgets that should be filled out by the user by the
	 * exact values of his account, and if the user accepts the
	 * registration, RegisterAccount() would be called with those
	 * widgets.
	 *
	 */
	class IBloggingPlatform
	{
	public:
		virtual ~IBloggingPlatform () {}

		/** @brief Returns the protocol object as a QObject.
		 *
		 * @return Blogging platform object as QObject.
		 */
		virtual QObject* GetObject () = 0;

		/** @brief Returns the accounts within this blogging platform.
		 *
		 * Each object in the returned list must implement IAccount.
		 *
		 * @return The list of accounts of this blogging platform.
		 */
		virtual QList<QObject*> GetRegisteredAccounts () = 0;

		/** @brief Returns the pointer to the parent blogging platform plugin
		 * that this blogging platform belongs to.
		 *
		 * @return The parent blogging platform plugin of this blogging platform, which
		 * must implement IBloggingPlatformPlugin.
		 */
		virtual QObject* GetParentProtocolPlugin () const = 0;

		/** @brief Returns the human-readable name of this blogging platform,
		 * like "LiveJournal" or "Blogger".
		 *
		 * @return Human-readable name of the blogging platform.
		 */
		virtual QString GetProtocolName () const = 0;

		/** @brief Returns the icon of this blogging platform.
		 *
		 * @return The icon of the blogging platform.
		 */
		virtual QIcon GetBloggingPlatformIcon () const = 0;

		/** @brief Returns the protocol ID, which must be unique among
		 * all the protocols.
		 *
		 * @return The unique ID of this protocol.
		 */
		virtual QByteArray GetBloggingPlatformID () const = 0;

		/** @brief Returns the widgets used for account addition.
		 *
		 * The widgets from the returned list are shown in the account
		 * addition wizard in the same order they appear in the returned
		 * list.
		 *
		 * If the user accepts registering the account, the
		 * RegisterAccount () method would be called with the same set of
		 * widgets in the same order as returned from this function.
		 *
		 * The ownership is transferred to the caller.
		 *
		 * @param[in] options The options selected by the user to
		 * perform the account addition.
		 *
		 * @return The widgets to be shown and filled by the user.
		 *
		 * @sa RegisterAccount()
		 */
		virtual QList<QWidget*> GetAccountRegistrationWidgets () = 0;

		/** @brief Adds an account with the given name and widgets.
		 *
		 * The list of widgets is the same (and in the same order) as
		 * returned from the GetAccountRegistrationWidgets() function.
		 * If this method is called, the widgets were shown to the user,
		 * and the user has accepted the account registration, so the
		 * widgets would be filled by the user to contain the required
		 * values. One could use qobject_cast to cast each widget in the
		 * list to the exact widget type and then use the values filled
		 * by the user.
		 *
		 * If the account is added successfully, the accountAdded()
		 * signal should be emitted, otherwise nothing should be done.
		 *
		 * @note Since ownership of the widgets is transferred to the
		 * caller, one shouldn't delete widgets in this method, and one
		 * shouldn't store them to use after this function has returned.
		 *
		 * @param[in] name The name of the account to be registered.
		 * @param[in] widgets The list of widgets returned from the
		 * GetAccountRegistrationWidgets() and filled by the user.
		 *
		 * @sa GetAccountRegistrationWidgets()
		 */
		virtual void RegisterAccount (const QString& name, const QList<QWidget*>& widgets) = 0;

		/** @brief Removes the given account.
		 *
		 * This function shouldn't ask anything from the user, just
		 * remove the account.
		 *
		 * If the account is not registered, this function should do
		 * nothing.
		 *
		 * After the removal, the accountRemoved() signal is expected to
		 * be emitted.
		 *
		 * @param[in] account The account to remove.
		 */
		virtual void RemoveAccount (QObject *account) = 0;

		/** @brief Notifies about new account.
		 *
		 * This signal should be emitted whenever a new account appears
		 * in this blogging platform.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] account The newly added account, which must
		 * implement IAccount.
		 */
		virtual void accountAdded (QObject *account) = 0;

		/** @brief Notifies about an account having been removed.
		 *
		 * This signal should be emitted whenever an already registered
		 * account is removed, for example, as the result of the call to
		 * RemoveAccount().
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] account The just removed account, which must
		 * implement IAccount.
		 */
		virtual void accountRemoved (QObject *account) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Blogique::IBloggingPlatform,
		"org.Deviant.LeechCraft.Blogique.IBloggingPlatform/1.0");
