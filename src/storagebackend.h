/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef STORAGEBACKEND_H
#define STORAGEBACKEND_H
#include <QObject>

namespace LeechCraft
{
	/** @brief Abstract base class for storage backends.
	 *
	 * Specifies interface for all storage backends. Includes functions for
	 * storing the history, favorites, saved passwords etc.
	 */
	class StorageBackend : public QObject
	{
		Q_OBJECT
	public:
		StorageBackend (QObject* = 0);
		virtual ~StorageBackend ();

		/** @brief Do post-initialization.
		 *
		 * This function is called by the Core after all the updates are
		 * checked and done, if required.
		 */
		virtual void Prepare () = 0;
		/** @brief Get authentication information.
		 *
		 * Finds authentication information for the given realm and puts it
		 * into login and password. If no information is found, leaves these
		 * fields unchanged.
		 *
		 * @param[in] realm Realm of the interesting authentication resource.
		 * @param[out] login Stored login.
		 * @param[out] password Stored password.
		 */
		virtual void GetAuth (const QString& realm,
				QString& login, QString& password) const = 0;
		/** @brief Set authentication information.
		 *
		 * Sets authentication information for the given realm from passed
		 * parameters. Creates a new record if there is no older one,
		 * updates old record otherwise.
		 *
		 * @param[in] realm Realm of the authentication resource.
		 * @param[in] login New login.
		 * @param[in] password New password.
		 */
		virtual void SetAuth (const QString& realm,
				const QString& login, const QString& password) = 0;
	};
};

#endif

