#include "storagebackend.h"
#include "sqlstoragebackend.h"

StorageBackend::StorageBackend (QObject *parent)
: QObject (parent)
{
}

StorageBackend::~StorageBackend ()
{
}

boost::shared_ptr<StorageBackend> StorageBackend::Create (Type type)
{
	switch (type)
	{
		case SBSQLite:
		case SBPostgres:
			return boost::shared_ptr<StorageBackend> (new SQLStorageBackend (type));
	}
}

