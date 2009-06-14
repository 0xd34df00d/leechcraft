#include "storagebackend.h"
#include "sqlstoragebackend.h"

using namespace LeechCraft::Plugins::Aggregator;

StorageBackend::StorageBackend (QObject *parent)
: QObject (parent)
{
}

StorageBackend::~StorageBackend ()
{
}

boost::shared_ptr<StorageBackend> StorageBackend::Create (Type type)
{
	boost::shared_ptr<StorageBackend> result;
	switch (type)
	{
		case SBSQLite:
		case SBPostgres:
			result.reset (new SQLStorageBackend (type));
	}
	return result;
}


