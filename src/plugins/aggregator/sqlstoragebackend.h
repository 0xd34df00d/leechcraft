#ifndef SQLSTORAGEBACKEND_H
#define SQLSTORAGEBACKEND_H
#include <QSqlDatabase>
#include "storagebackend.h"

class SQLStorageBackend : public StorageBackend
{
	QSqlDatabase DB_;
public:
	SQLStorageBackend ();
	virtual ~SQLStorageBackend ();

	virtual void GetFeeds (feeds_container_t&) const;
	virtual void GetChannels (const Feed_ptr&,
			channels_container_t&) const;
	virtual void GetItems (const Channel_ptr&,
			items_container_t&) const;

	virtual void AddFeed (const Feed_ptr&);
	virtual void UpdateFeed (const Feed_ptr&);
	virtual void UpdateItem (const Item_ptr&);
private:
	void InitializeTables ();
	void DumpError (const QSqlError&) const;
};

#endif

