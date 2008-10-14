#ifndef SQLSTORAGEBACKEND_H
#define SQLSTORAGEBACKEND_H
#include <QSqlDatabase>
#include <QSqlQuery>
#include "storagebackend.h"

class SQLStorageBackend : public StorageBackend
{
	QSqlDatabase DB_;
	QSqlQuery FeedFinderByURL_,
			  ChannelFinder_,
			  ItemFinder_,
			  InsertFeed_,
			  InsertChannel_,
			  InsertItem_,
			  UpdateChannel_,
			  UpdateItem_,
			  RemoveFeed_,
			  RemoveChannel_,
			  RemoveItem_;
public:
	SQLStorageBackend ();
	virtual ~SQLStorageBackend ();
	
	virtual void Prepare ();

	virtual void GetFeeds (feeds_container_t&) const;

	virtual void AddFeed (Feed_ptr);
	virtual void UpdateChannel (Channel_ptr, const QString&);
	virtual void UpdateItem (Item_ptr, const QString&);
	virtual void AddChannel (Channel_ptr, const QString&);
	virtual void AddItem (Item_ptr, const QString&);
	virtual void RemoveItem (Item_ptr,
			const QString&);
	virtual void RemoveFeed (Feed_ptr);
    virtual bool UpdateFeedsStorage (int, int);
    virtual bool UpdateChannelsStorage (int, int);
    virtual bool UpdateItemsStorage (int, int);
private:
	void GetChannels (Feed_ptr) const;
	void GetItems (Channel_ptr) const;
	bool InitializeTables ();
	void DumpError (const QSqlError&) const;
	void DumpError (const QSqlQuery&) const;
	QByteArray SerializePixmap (const QPixmap&) const;
	QPixmap UnserializePixmap (const QByteArray&) const;
    bool RollItemsStorage (int);
};

#endif

