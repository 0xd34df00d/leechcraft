/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#ifndef UTIL_PASSUTILS_H
#define UTIL_PASSUTILS_H
#include "utilconfig.h"

class QString;
class QObject;

namespace LeechCraft
{
namespace Util
{
	/** @brief Returns password for the key, possibly asking the user.
	 *
	 * This function returns password for the given \em keyName and using
	 * the given \em emitter object. The password is typically stored in
	 * a storage plugin like SecMan.
	 *
	 * If the password isn't found in any password stores (or there are
	 * no password stores) or \em useStore is set to false, this function
	 * asks the user for the password instead using the \em dialogText
	 * for the dialog and returns the user input instead. If user has
	 * canceled the dialog it returns an null string. Otherwise, if the
	 * user has entered some text this function automatically stores the
	 * password under the \em keyName.
	 *
	 * The emitter object is used to send the corresponding internal
	 * Entity objects around, so it should have its gotEntity() and
	 * delegateEntity() signals both relayed to the corresponding plugin
	 * instance object.
	 *
	 * @note Despite the name this function can be used to retrieve
	 * arbitrary string data saved via SavePassword().
	 *
	 * @param[in] keyName The name of the key uniquely identifying the
	 * password.
	 * @param[in] dialogText The text of the dialog to present to the
	 * user if the password isn't found or \em useStore is false.
	 * @param[in] emitter The object used to emit the gotEntity() and
	 * delegateEntity() signals.
	 * @param[in] useStore Whether stored password should be used at
	 * all. Calling code may explicitly set this to <code>false</code> if
	 * the saved password is known to be invalid, for instance.
	 * @return The password or a null string.
	 *
	 * @sa SavePassword()
	 */
	UTIL_API QString GetPassword (const QString& keyName,
			const QString& dialogText,
			QObject *emitter,
			bool useStore = true);

	/** @brief Saves the password to be retrieved later via GetPassword().
	 *
	 * This function stores the \em password under the given \em keyName
	 * and using the given \em emitter object. The password is typically
	 * stored in a storage plugin like SecMan.
	 *
	 * If there are no storage plugins, this function does nothing.
	 *
	 * The emitter object is used to send the corresponding internal
	 * Entity objects around, so it should have its gotEntity() and
	 * delegateEntity() signals both relayed to the corresponding plugin
	 * instance object.
	 *
	 * @note Despite the name this function can be used to save arbitrary
	 * string data in secure storages like SecMan.
	 *
	 * @param[in] password The password string to save.
	 * @param[in] keyName The name of the key uniquely identifying the
	 * password.
	 * @param[in] emitter The object used to emit the gotEntity() and
	 * delegateEntity() signals.
	 *
	 * @sa GetPassword()
	 */
	UTIL_API void SavePassword (const QString& password,
			const QString& keyName,
			QObject *emitter);
}
}

#endif
