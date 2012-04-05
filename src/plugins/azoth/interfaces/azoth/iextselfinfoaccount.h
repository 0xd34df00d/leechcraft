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

#ifndef PLUGINS_AZOTH_INTERFACES_IEXTSELFINFOACCOUNT_H
#define PLUGINS_AZOTH_INTERFACES_IEXTSELFINFOACCOUNT_H
#include <QMetaType>

namespace LeechCraft
{
namespace Azoth
{
	/** @brief Interface for accounts with extended self information.
	 *
	 * Implementing this interface allows other parts of Azoth to know
	 * such information as the avatar of this account, or which contact
	 * represents self-contact, if any.
	 *
	 * @sa IAccount
	 */
	class IExtSelfInfoAccount
	{
	public:
		virtual ~IExtSelfInfoAccount () {}

		/** @brief Returns the self-contact of this account.
		 *
		 * Self-contact represents the user of this account in the
		 * contact list. If there is no concept of self-contact in the
		 * given protocol, this function may return 0.
		 *
		 * Self-contact should also be returned from all the "standard"
		 * functions like IAccount::GetCLEntries() and such. Generally,
		 * it should be a normal ICLEntry-derived object.
		 *
		 * @return ICLEntry-derived object representing self contact.
		 */
		virtual QObject* GetSelfContact () const = 0;

		/** @brief Returns the avatar of this account.
		 *
		 * The returned avatar is typically used to represent the user
		 * of the account in contact list or in chat windows.
		 *
		 * @return The avatar of this account.
		 */
		virtual QImage GetSelfAvatar () const = 0;

		/** @brief Returns the icon of this account.
		 *
		 * The returned icon is used to distinguish this account from
		 * other accounts of the same protocol. For example, an XMPP
		 * account on GMail may choose to return a GMail-y icon.
		 *
		 * Returning a null icon means that general protocol icon will
		 * be used.
		 *
		 * @return The icon of this account.
		 */
		virtual QIcon GetAccountIcon () const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IExtSelfInfoAccount,
		"org.Deviant.LeechCraft.Azoth.IExtSelfInfoAccount/1.0");

#endif
