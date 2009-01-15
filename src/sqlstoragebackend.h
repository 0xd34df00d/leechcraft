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

