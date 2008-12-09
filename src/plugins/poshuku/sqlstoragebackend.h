#ifndef SQLSTORAGEBACKEND_H
#define SQLSTORAGEBACKEND_H
#include "storagebackend.h"
#include <QSqlDatabase>

class SQLStorageBackend : public StorageBackend
{
	Q_OBJECT

	QSqlDatabase DB_;
public:
	SQLStorageBackend ();
	virtual ~SQLStorageBackend ();
private:
	void InitializeTables ();
};

#endif

