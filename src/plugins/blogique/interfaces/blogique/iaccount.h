/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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
#include <QVariant>
#include <QStringList>

class QWidget;
namespace LeechCraft
{
namespace Blogique
{
	/** @brief Interface representing a single account.
	 *
	 **/
	class IAccount
	{
	public:
		virtual ~IAccount () {}

		/** @brief Returns the account object as a QObject.
		 *
		 * @return Account object as QObject.
		 */
		virtual QObject* GetObject () = 0;

		/** @brief Returns the pointer to the parent blogging platform that this
		 * account belongs to.
		 *
		 * @return The parent blogging platforml of this account.
		 */
		virtual QObject* GetParentBloggingPlatform () const = 0;

		/** @brief Returns the human-readable name of this account.
		 *
		 * @return Human-readable name of this account.
		 *
		 * @sa RenameAccount()
		 */
		virtual QString GetAccountName () const = 0;

		/** @brief Returns the login of our user.
		 *
		 * @return Login name.
		 */
		virtual QString GetOurLogin () const = 0;

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
		 * The returned ID should be unique among all accounts and
		 * should not depend on the value of GetAccountName()
		 * (the human-readable name of the account).
		 *
		 * @return The unique and persistent account ID.
		 */
		virtual QByteArray GetAccountID () const = 0;

		/** @brief Requests the account to open its configuration dialog.
		 */
		virtual void OpenConfigurationDialog () = 0;

		/** @brief Returns validation state of account.
		 *
		 * If account not validated it can't be used for blogging.
		 *
		 * @return Validation state of the account.
		 */
		virtual bool IsValidated () const = 0;

		/** @brief Returns the pointer to account's profile widget.
		 *
		 * @return The account's profile widget.
		 */
		virtual QWidget* GetProfileWidget () = 0;

		/** @brief This signal should be emitted when account is renamed.
		 *
		 * This signal should be emitted even after an explicit call to
		 * RenameAccount().
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] newName The new name of this account.
		 */
		virtual void accountRenamed (const QString& newName) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Blogique::IAccount,
		"org.Deviant.LeechCraft.Blogique.IAccount/1.0");
