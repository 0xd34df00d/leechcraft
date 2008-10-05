#ifndef STORAGEBACKEND_H
#define STORAGEBACKEND_H
#include "feed.h"

class StorageBackend
{
public:
	virtual ~StorageBackend ();

	virtual void GetFeeds (feeds_container_t&) const = 0;

	virtual void AddFeed (Feed_ptr) = 0;
	virtual void AddChannel (Channel_ptr, const QString&) = 0;
	virtual void AddItem (Item_ptr, const QString&) = 0;
	virtual void UpdateChannel (Channel_ptr,
			const QString&) = 0;
	virtual void UpdateItem (Item_ptr,
			const QString&) = 0;

	virtual void RemoveItem (Item_ptr,
			const QString&) = 0;
	virtual void RemoveFeed (Feed_ptr) = 0;
};

#endif

