#ifndef DBLOCK_H
#define DBLOCK_H
#include "config.h"

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
			LEECHCRAFT_API DBLock (QSqlDatabase& database);
			/** @brief Destructor.
			 *
			 * Ends the transaction if the lock is in a correct state. If Good()
			 * was called, it commits the transaction, otherwise rolls back.
			 */
			LEECHCRAFT_API ~DBLock ();

			/** @brief Initializes the transaction.
			 *
			 * Tries to start the transaction. If this wasn't successful, the
			 * lock remains in a usable but not correct state.
			 *
			 * @throw std::runtime_error
			 */
			LEECHCRAFT_API void Init ();
			/** @brief Notifies the lock about successful higher-level
			 * operations.
			 *
			 * Calling this function makes the lock to commit the transaction
			 * upon destruction instead of rolling back.
			 */
			LEECHCRAFT_API void Good ();

			/** @brief Dumps the error to the qWarning() stream.
			 *
			 * @param[in] error The error class.
			 */
			LEECHCRAFT_API static void DumpError (const QSqlError& error);
			/** @brief Dumps the error to the qWarning() stream.
			 *
			 * @param[in] query The query that should be dumped.
			 */
			LEECHCRAFT_API static void DumpError (const QSqlQuery& query);
		};
	};
};

#endif

