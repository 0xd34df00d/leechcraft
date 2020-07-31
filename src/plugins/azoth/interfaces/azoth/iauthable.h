/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_IAUTHABLE_H
#define PLUGINS_AZOTH_INTERFACES_IAUTHABLE_H
#include <QString>
#include "azothcommon.h"

namespace LC
{
namespace Azoth
{
	/** @brief Represents an entry that supports authorizations.
	 */
	class IAuthable
	{
	public:
		virtual ~IAuthable () {}

		/** @brief Returns the AuthStatus between our user and this
		 * remote.
		 *
		 * @return Authorization status of this entry.
		 */
		virtual AuthStatus GetAuthStatus () const = 0;

		/** @brief Resends authorization to the entry.
		 *
		 * @param[in] reason Optional reason message, if applicable.
		 */
		virtual void ResendAuth (const QString& reason = QString ()) = 0;

		/** @brief Revokes authorization from the entry.
		 *
		 * @param[in] reason Optional reason message, if applicable.
		 */
		virtual void RevokeAuth (const QString& reason = QString ()) = 0;

		/** @brief Unsubscribes ourselves from the contact.
		 *
		 * @param[in] reason Optional reason message, if applicable.
		 */
		virtual void Unsubscribe (const QString& reason = QString ()) = 0;

		/** @brief Rerequest authorization.
		 *
		 * @param[in] reason Optional reason message, if applicable.
		 */
		virtual void RerequestAuth (const QString& reason = QString ()) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IAuthable,
		"org.Deviant.LeechCraft.Azoth.IAuthable/1.0")

#endif
