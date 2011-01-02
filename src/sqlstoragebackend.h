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

#ifndef SQLSTORAGEBACKEND_H
#define SQLSTORAGEBACKEND_H
#include "storagebackend.h"
#include <QSqlDatabase>
#include <QSqlQuery>

namespace LeechCraft
{
	class SQLStorageBackend : public StorageBackend
	{
		Q_OBJECT

		QSqlDatabase DB_;

				/** Binds:
				 * - realm
				 * Returns:
				 * - login
				 * - password
				 */
		mutable QSqlQuery AuthGetter_,
				/** Binds:
				 * - realm
				 * - login
				 * - password
				 */
				AuthInserter_,
				/** Binds:
				 * - realm
				 * - login
				 * - password
				 */
				AuthUpdater_;
	public:
		SQLStorageBackend ();
		virtual ~SQLStorageBackend ();

		void Prepare ();

		virtual void GetAuth (const QString&, QString&, QString&) const;
		virtual void SetAuth (const QString&, const QString&, const QString&);
	private:
		void InitializeTables ();
	};
};

#endif

