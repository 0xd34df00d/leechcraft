/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMutex>
#include <QString>
#include <QSet>
#include "dbconfig.h"

class QSqlError;
class QSqlQuery;
class QSqlDatabase;

namespace LC
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
	 *
	 * @ingroup DbUtil
	 */
	class DBLock
	{
		QSqlDatabase& Database_;

		bool Good_ = false;
		bool Initialized_ = false;

		static QMutex LockedMutex_;
		static QSet<QString> LockedBases_;
	public:
		DBLock (const DBLock&) = delete;
		DBLock& operator= (const DBLock&) = delete;

		DBLock (DBLock&&) = default;

		/** @brief Constructor.
		 *
		 * Constructs the lock and prepares it to work with the database.
		 * Creating the lock doesn't begin the transaction. Lock is in
		 * usable state after that.
		 *
		 * @param[in] database Non-const reference to the database to be
		 * guarded.
		 */
		UTIL_DB_API DBLock (QSqlDatabase& database);

		/** @brief Destructor.
		 *
		 * Ends the transaction if the lock is in a correct state. If Good()
		 * was called, it commits the transaction, otherwise rolls back.
		 */
		UTIL_DB_API ~DBLock ();

		/** @brief Initializes the transaction.
		 *
		 * Tries to start the transaction. If this wasn't successful, the
		 * lock remains in a usable but not correct state.
		 *
		 * @throw std::runtime_error
		 */
		UTIL_DB_API void Init ();

		/** @brief Notifies the lock about successful higher-level
		 * operations.
		 *
		 * Calling this function makes the lock to commit the transaction
		 * upon destruction instead of rolling back.
		 */
		UTIL_DB_API void Good ();

		/** @brief Dumps the error to the qWarning() stream.
		 *
		 * @param[in] error The error class.
		 */
		UTIL_DB_API static void DumpError (const QSqlError& error);

		/** @brief Dumps the error to the qWarning() stream.
		 *
		 * @param[in] query The query that should be dumped.
		 */
		UTIL_DB_API static void DumpError (const QSqlQuery& query);

		/** @brief Tries to execute the given query.
		 *
		 * If query execution is successful, this function just
		 * returns. Otherwise it calls DumpError() to write the error
		 * to the logs and throws an exception.
		 *
		 * @param[in] query The query that should be dumped.
		 */
		UTIL_DB_API static void Execute (QSqlQuery& query);
	};
};
};
