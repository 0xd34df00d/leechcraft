#ifndef SQLSTORAGEBACKEND_H
#define SQLSTORAGEBACKEND_H
#include <QSqlDatabase>
#include <QSqlQuery>
#include "storagebackend.h"

class SQLStorageBackend : public StorageBackend
{
	Q_OBJECT

	QSqlDatabase DB_;
					  /** Returns:
					   * - last_update
					   *
					   * Binds:
					   * - url
					   */
	mutable QSqlQuery FeedFinderByURL_,
					  /** Returns:
					   * - title
					   * - url
					   * - tags
					   * - last_build
					   * - favicon
					   *
					   * Binds:
					   * - parent_feed_url
					   */
					  ChannelsShortSelector_,
					  /** Returns:
					   * - url
					   * - description
					   * - last_build
					   * - tags
					   * - language
					   * - author
					   * - pixmap_url
					   * - pixmap
					   * - favicon
					   *
					   * Binds:
					   * - parent_feed_url
					   * - title
					   */
					  ChannelsFullSelector_,
					  /** Returns:
					   * - number of unread items
					   *
					   * Binds:
					   * - parents_hash
					   */
					  UnreadItemsCounter_,
					  /** Returns:
					   * - title
					   * - url
					   * - category
					   * - pub_date
					   * - unread
					   *
					   * Binds:
					   * - parents_hash
					   */
					  ItemsShortSelector_,
					  /** Returns:
					   * - title
					   * - url
					   * - description
					   * - author
					   * - category
					   * - guid
					   * - pub_date
					   * - unread
					   * - num_comments
					   * - comments_url
					   * - comments_page_url
					   *
					   * Binds:
					   * - parents_hash
					   * - title
					   * - url
					   */
					  ItemsFullSelector_,
					  /** Returns:
					   * - description
					   *
					   * Binds:
					   * - parent_feed_url
					   * - title
					   * - url
					   */
					  ChannelFinder_,
					  /** Returns:
					   * - title
					   *
					   * Binds:
					   * - parents_hash
					   * - title
					   * - url
					   */
					  ItemFinder_,
					  /** Binds:
					   * - url
					   * - last_update
					   */
					  InsertFeed_,
					  /** Binds:
					   * - parent_feed_url
					   * - url
					   * - title
					   * - description
					   * - last_build
					   * - tags
					   * - language
					   * - author
					   * - pixmap_url
					   * - pixmap
					   * - favicon
					   */
					  InsertChannel_,
					  /** Binds:
					   * - parents_hash
					   * - title
					   * - url
					   * - description
					   * - author
					   * - category
					   * - guid
					   * - pub_date
					   * - unread
					   * - num_comments
					   * - comments_url
					   * - comments_page_url
					   */
					  InsertItem_,
					  /** Binds:
					   * - title
					   * - url
					   * - tags
					   * - last_build
					   * - parent_feed_url
					   */
					  UpdateShortChannel_,
					  /** Binds:
					   * - description
					   * - last_build
					   * - tags
					   * - language
					   * - author
					   * - pixmap_url
					   * - pixmap
					   * - favicon
					   * - parent_feed_url
					   * - url
					   * - title
					   */
					  UpdateChannel_,
					  /** Binds:
					   * - unread
					   * - parents_hash
					   * - title
					   * - url
					   */
					  UpdateShortItem_,
					  /** Binds:
					   * - description
					   * - author
					   * - category
					   * - pub_date
					   * - unread
					   * - num_comments
					   * - comments_url
					   * - comments_page_url
					   * - parents_hash
					   * - title
					   * - url
					   * - guid
					   */
					  UpdateItem_,
					  /** Binds:
					   * - unread
					   * - parents_hash
					   */
					  ToggleChannelUnread_,
					  /** Binds:
					   * - url
					   */
					  RemoveFeed_,
					  /** Binds:
					   * - parent_feed_url
					   */
					  RemoveChannel_,
					  /** Binds:
					   * - parents_hash
					   * - title
					   * - url
					   * - guid
					   */
					  RemoveItem_,
					  /** Binds:
					   * - url
					   * - type
					   * - length
					   * - lang
					   * - item_parents_hash
					   * - item_title
					   * - item_url
					   */
					  WriteEnclosure_,
					  /** Binds:
					   * - item_parents_hash
					   * - item_title
					   * - item_url
					   */
					  RemoveEnclosures_,
					  /** Returns:
					   * - url
					   * - type
					   * - length
					   * - lang
					   *
					   * Binds:
					   * - item_parents_hash
					   * - item_title
					   * - item_url
					   */
					  GetEnclosures_;
public:
	SQLStorageBackend ();
	virtual ~SQLStorageBackend ();
	
	virtual void Prepare ();

	virtual void GetFeedsURLs (feeds_urls_t&) const;
	virtual void GetChannels (channels_shorts_t&, const QString&) const;
	virtual Channel_ptr GetChannel (const QString&,
			const QString&) const;
	virtual void GetItems (items_shorts_t&, const QString&) const;
	virtual int GetUnreadItems (const QString&, const QString&) const;
	virtual Item_ptr GetItem (const QString&, const QString&,
			const QString&) const;

	virtual void AddFeed (Feed_ptr);
	virtual void UpdateChannel (Channel_ptr, const QString&);
	virtual void UpdateChannel (const ChannelShort&,
			const QString&);
	virtual void UpdateItem (Item_ptr,
			const QString&, const QString&);
	virtual void UpdateItem (const ItemShort&,
			const QString&, const QString&);
	virtual void AddChannel (Channel_ptr, const QString&);
	virtual void AddItem (Item_ptr, const QString&, const QString&);
	virtual void RemoveItem (Item_ptr,
			const QString&,
			const QString&,
			const QString&);
	virtual void RemoveFeed (const QString&);
    virtual bool UpdateFeedsStorage (int, int);
    virtual bool UpdateChannelsStorage (int, int);
    virtual bool UpdateItemsStorage (int, int);
	virtual void ToggleChannelUnread (const QString&, const QString&, bool);
private:
	bool InitializeTables ();
	QByteArray SerializePixmap (const QPixmap&) const;
	QPixmap UnserializePixmap (const QByteArray&) const;
    bool RollItemsStorage (int);
};

#endif

