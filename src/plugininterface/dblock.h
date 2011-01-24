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

#ifndef PLUGININTERFACE_DBLOCK_H
#define PLUGININTERFACE_DBLOCK_H
#include "piconfig.h"

class QSqlError;
class QSqlQuery;
class QSqlDatabase;

namespace LeechCraft
{
	namespace Util
	{
		/** @brief Provides database transaction lock.
		 *
		 * To use the lock, create an instance of it passing a non-const
		 * reference to the QSqlDatabase to be guarded. To initialize and start
		 * the locking mechanism, call Init(). To notify DBLock that everything
		 * is good and the database shouldn't be rolled back, call Good().
		 * Transaction would be either commited or rolled back in class'
		 * destructor.
		 *
		 * The state could be unusable, usable or correct. Unusable means that
		 * the class itself isn't usable, usable means that the class is usable,
		 * but the transaction state isn't necesseraly in a correct state, and
		 * correct means that the lock class is usable and the transaction state
		 * is correct.
		 */
		class DBLock
		{
			QSqlDatabase &Database_;

			bool Good_;
			bool Initialized_;

			DBLock (const DBLock&);
			DBLock& operator= (const DBLock&);
		public:
			/** @brief Constructor.
			 *
			 * Constructs the lock and prepares it to work with the database.
			 * Creating the lock doesn't begin the transaction. Lock is in
			 * usable state after that.
			 *
			 * @param[in] database Non-const reference to the database to be
			 * guarded.
			 */
			PLUGININTERFACE_API DBLock (QSqlDatabase& database);
			/** @brief Destructor.
			 *
			 * Ends the transaction if the lock is in a correct state. If Good()
			 * was called, it commits the transaction, otherwise rolls back.
			 */
			PLUGININTERFACE_API ~DBLock ();

			/** @brief Initializes the transaction.
			 *
			 * Tries to start the transaction. If this wasn't successful, the
			 * lock remains in a usable but not correct state.
			 *
			 * @throw std::runtime_error
			 */
			PLUGININTERFACE_API void Init ();
			/** @brief Notifies the lock about successful higher-level
			 * operations.
			 *
			 * Calling this function makes the lock to commit the transaction
			 * upon destruction instead of rolling back.
			 */
			PLUGININTERFACE_API void Good ();

			/** @brief Dumps the error to the qWarning() stream.
			 *
			 * @param[in] error The error class.
			 */
			PLUGININTERFACE_API static void DumpError (const QSqlError& error);
			/** @brief Dumps the error to the qWarning() stream.
			 *
			 * @param[in] query The query that should be dumped.
			 */
			PLUGININTERFACE_API static void DumpError (const QSqlQuery& query);
		};
	};
};

#endif

