#ifndef STORAGEBACKEND_H
#define STORAGEBACKEND_H
#include "feed.h"

class StorageBackend
{
public:
	virtual ~StorageBackend ();

	virtual void GetFeeds (feeds_container_t&) const = 0;
	virtual void GetChannels (const Feed_ptr&,
			channels_container_t&) const = 0;
	virtual void GetItems (const Channel_ptr&,
			items_container_t&) const = 0;

	virtual void AddFeed (const Feed_ptr&) = 0;
	virtual void UpdateFeed (const Feed_ptr&) = 0;
	virtual void UpdateItem (const Item_ptr&) = 0;
};

#endif

