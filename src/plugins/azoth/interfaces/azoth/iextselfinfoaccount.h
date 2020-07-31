/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_IEXTSELFINFOACCOUNT_H
#define PLUGINS_AZOTH_INTERFACES_IEXTSELFINFOACCOUNT_H
#include <QMetaType>

namespace LC
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

Q_DECLARE_INTERFACE (LC::Azoth::IExtSelfInfoAccount,
		"org.Deviant.LeechCraft.Azoth.IExtSelfInfoAccount/1.0")

#endif
