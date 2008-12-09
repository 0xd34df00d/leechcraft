#ifndef SQLSTORAGEBACKEND_H
#define SQLSTORAGEBACKEND_H
#include "storagebackend.h"
#include <QSqlDatabase>
#include <QSqlQuery>

class SQLStorageBackend : public StorageBackend
{
	Q_OBJECT

	QSqlDatabase DB_;

			/** Returns:
			 * - title
			 * - date
			 * - url
			 */
	mutable QSqlQuery HistoryLoader_,
			/** Binds:
			 * - date
			 * - title
			 * - url
			 */
			HistoryAdder_;
public:
	SQLStorageBackend ();
	virtual ~SQLStorageBackend ();

	void Prepare ();

	virtual void LoadHistory (std::vector<HistoryModel::HistoryItem>&) const;
	virtual void AddToHistory (const HistoryModel::HistoryItem&);
private:
	void InitializeTables ();
};

#endif

