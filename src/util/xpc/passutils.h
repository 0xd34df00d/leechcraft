/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "xpcconfig.h"
#include <interfaces/core/icoreproxy.h>

class QString;
class QObject;

namespace LC::Util
{
	template<typename, typename>
	class EitherCont;

	/** @brief Returns password for the key, possibly asking the user.
	 *
	 * This function returns password for the given \em keyName.
	 * The password is typically stored in a storage plugin like SecMan.
	 *
	 * If the password isn't found in any password stores (or there are
	 * no password stores) or \em useStore is set to false, this function
	 * asks the user for the password instead using the \em dialogText
	 * for the dialog and returns the user input instead. If user has
	 * canceled the dialog it returns an null string. Otherwise, if the
	 * user has entered some text this function automatically stores the
	 * password under the \em keyName.
	 *
	 * @note Despite the name this function can be used to retrieve
	 * arbitrary string data saved via SavePassword().
	 *
	 * @param[in] keyName The name of the key uniquely identifying the
	 * password.
	 * @param[in] dialogText The text of the dialog to present to the
	 * user if the password isn't found or \em useStore is false.
	 * @param[in] proxy The core proxy used to communicate with other
	 * plugins.
	 * @param[in] useStore Whether stored password should be used at
	 * all. Calling code may explicitly set this to <code>false</code> if
	 * the saved password is known to be invalid, for instance.
	 * @return The password or a null string.
	 *
	 * @sa SavePassword()
	 */
	UTIL_XPC_API QString GetPassword (const QString& keyName,
			const QString& dialogText,
			const ICoreProxy_ptr& proxy,
			bool useStore = true);

	UTIL_XPC_API void GetPassword (const QString& keyName,
			const QString& dialogText,
			const ICoreProxy_ptr& proxy,
			const EitherCont<void (), void (QString)>& cont,
			QObject *depender = nullptr,
			bool useStore = true);

	/** @brief Saves the password to be retrieved later via GetPassword().
	 *
	 * This function stores the \em password under the given \em keyName.
	 * The password is typically stored in a storage plugin like SecMan.
	 *
	 * If there are no storage plugins, this function does nothing.
	 *
	 * @note Despite the name this function can be used to save arbitrary
	 * string data in secure storages like SecMan.
	 *
	 * @param[in] password The password string to save.
	 * @param[in] keyName The name of the key uniquely identifying the
	 * password.
	 * @param[in] proxy The core proxy used to communicate with other
	 * plugins.
	 *
	 * @sa GetPassword()
	 */
	UTIL_XPC_API void SavePassword (const QString& password,
			const QString& keyName,
			const ICoreProxy_ptr& proxy);
}
