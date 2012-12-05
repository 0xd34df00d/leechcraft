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
#include <QDateTime>

class QWidget;

namespace LeechCraft
{
namespace Blogique
{
	struct Event
	{
		QString Target_;
		QString Subject_;
		QString Content_;
		QDateTime Date_;
		QStringList Tags_;
		QVariantMap PostOptions_;
		QVariantMap CustomData_;
		qlonglong EntryDBId_;
		qlonglong EntryId_;

		Event ()
		: EntryDBId_ (-1)
		, EntryId_ (-1)
		{
		}

		bool IsEmpty () const
		{
			return Content_.isEmpty ();
		};
	};

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

		/** @brief Returns the pointer to account's profile.
		 *
		 * @return The account's profile.
		 */
		virtual QObject* GetProfile () = 0;

		/** @brief Fetch last entries from blog.
		 *
		 * @param[in] count Amount of entries to fetch.
		 */
		virtual void GetLastEntries (int count) = 0;

		/** @brief Remove entry from blog.
		 *
		 * @param[in] event Removing event.
		 */
		virtual void RemoveEntry (const Event& event) = 0;

		/** @brief Submit post to blog.
		 *
		 * @param[in] event Posting event.
		 */
		virtual void submit (const Event& event) = 0;

		/** @brief Request updating profile data.
		 *
		 */
		virtual void updateProfile () = 0;

		virtual void backup () = 0;

	protected:
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

		/** @brief This signal should be emitted when entry
		 * successfully posted to blog.
		 *
		 * @note This function is expected to be a signal.
		 */
		virtual void entryPosted () = 0;

		//TODO
		virtual void entryRemoved (int itemId) = 0;

		//TODO
		virtual void entryUpdated (int ItemId) = 0;

		/** @brief This signal should be emitted when account want to backup
		 * some amount of entries.
		 *
		 * @note This function is expected to be a signal.
		 */
		virtual void gotEvents2Backup (const QList<Event>& events) = 0;

		/** @brief This signal should be emitted all entries for backup were downloaded.
		 *
		 * @note This function is expected to be a signal.
		 */
		virtual void gettingEvents2BackupFinished () = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Blogique::IAccount,
		"org.Deviant.LeechCraft.Blogique.IAccount/1.0");
