#ifndef STORAGEBACKEND_H
#define STORAGEBACKEND_H
#include "feed.h"

class StorageBackend
{
public:
	virtual ~StorageBackend ();

	virtual void GetFeeds (feeds_container_t&) const = 0;
	virtual void GetChannels (Feed_ptr,
			channels_container_t&) const = 0;
	virtual void GetItems (Channel_ptr,
			items_container_t&) const = 0;

	virtual void AddFeed (Feed_ptr) = 0;
	virtual void UpdateFeed (Feed_ptr) = 0;
	virtual void UpdateItem (Item_ptr) = 0;
};

#endif

