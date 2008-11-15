#ifndef STORAGEBACKEND_H
#define STORAGEBACKEND_H
#include <QObject>
#include "feed.h"

/** @brief Abstract base class for storage backends.
 *
 * Specifies interface for all storage backends. Includes functions for
 * appending, modifying and retrieving feeds, channels and items.
 */
class StorageBackend : public QObject
{
	Q_OBJECT
public:
	virtual ~StorageBackend ();

	/** @brief Do post-initialization.
	 *
	 * This function is called by the Core after all the updates are
	 * checked and done, if required.
	 * 
	 * @sa UpdateFeedsStorage
	 * @sa UpdateChannelsStorage
	 * @sa UpdateItemsStorage
	 */
	virtual void Prepare () = 0;

	/** @brief Get all the feeds in the storage.
	 *
	 * Puts URLs of all the feeds in the storage into the passed
	 * container.
	 *
	 * @param[out] furls The container with feed URLs. The URLs would be
	 * appended to the container.
	 */
	virtual void GetFeedsURLs (feeds_urls_t& furls) const = 0;
	/** @brief Get all the channels of a feed in the container.
	 *
	 * Returns short information about channels in the storage which
	 * have feedParent as their parent's feed URL.
	 *
	 * @param[out] shorts The container with short information about the
	 * channels to which retrieved info would be appended.
	 * @param[in] feedParent Parent feed's URL identifying the channel.
	 */
	virtual void GetChannels (channels_shorts_t& shorts,
			const QString& feedParent) const = 0;
	/** @brief Returns full information about a channel.
	 *
	 * Returns full information about a channel identified by its title
	 * and parent feed's URL. The channel's Items_ field isn't filled
	 * with child items though, the items should be requested by a
	 * separate call to GetItems().
	 *
	 * @param[in] title The title of the required channel.
	 * @param[in] feedparent The URL of parent feed.
	 * @return Full information about the requested channel.
	 */
	virtual Channel_ptr GetChannel (const QString& title,
			const QString& feedParent) const = 0;
	/** @brief Returns short information about items in a channel.
	 *
	 * Returns short information about items in the storage which are
	 * identified by their hash.
	 *
	 * @param[out] items The container to which short information about
	 * the items would be appended.
	 * @param[in] hash The unique identifier of the items set.
	 */
	virtual void GetItems (items_shorts_t& items,
			const QString& hash) const = 0;
	/** @brief Returns full information about an item.
	 *
	 * Returns full information about the item identified by its title,
	 * link it points to and a hash.
	 *
	 * @param[in] title The item's title.
	 * @param[in] link The item's link.
	 * @param[in] hash The item's hash.
	 * @return Full information about the requested item.
	 */
	virtual Item_ptr GetItem (const QString& title,
			const QString& link,
			const QString& hash) const = 0;

	/** @brief Puts a feed and all its child channels and items into the
	 * storage.
	 *
	 * If the specified feed already exists in the storage, this function
	 * should do nothing.
	 *
	 * @param[in] feed Pointer to the feed that should be added.
	 */
	virtual void AddFeed (Feed_ptr feed) = 0;
	/** @brief Adds a new channel to an already existing feed.
	 *
	 * Channels are identified by parent feed's URL.
	 *
	 * If the specified channel already exists in the storage, this
	 * function should do nothing.
	 *
	 * @param[in] channel Pointer to the channel that should be added.
	 * @param[in] feedURL Parent feed's URL.
	 */
	virtual void AddChannel (Channel_ptr channel,
			const QString& feedURL) = 0;
	/** @brief Adds a new item to an already existing channel.
	 *
	 * Items are differentiated by a hash consisting of parent channel's
	 * and feed's data and some own data. Storage backend shouldn't rely
	 * on how exactly the hash is built.
	 *
	 * This function would emit channelDataUpdated() signal after it
	 * finishes.
	 *
	 * If the specified item already exists in the storage, this function
	 * should do nothing.
	 *
	 * @param[in] channel Pointer to the item that should be added.
	 * @param[in] hash The string identifying the item.
	 */
	virtual void AddItem (Item_ptr item, const QString& hash) = 0;
	/** @brief Updates an already existing channel.
	 *
	 * If the specified channel doesn't exist in the storage, it should
	 * be inserted to the storage, so this function should behave like
	 * AddChannel() in this case.
	 *
	 * This function would emit channelDataUpdated() signal after it
	 * finishes.
	 *
	 * @param[in] channel Pointer to the new version of the channel
	 * thath should be updated.
	 * @param[in] parent The parent feed's URL.
	 */
	virtual void UpdateChannel (Channel_ptr channel,
			const QString& parent) = 0;
	/** @brief Updates an already existing channel.
	 *
	 * This is an overloaded function provided for convenience.
	 *
	 * @param[in] channel Short information about channel.
	 * @param[in] parent The parent feed's URL.
	 */
	virtual void UpdateChannel (const ChannelShort& channel,
			const QString& parent) = 0;
	/** @brief Updates an already existing item.
	 *
	 * If the specified item doesn't exist in the storage, it should be
	 * inserted to the storage, so this function should behave like
	 * AddItem() in this case.
	 *
	 * This function would emit itemDataUpdated() signal after it
	 * finishes.
	 *
	 * @param[in] item Pointer to the new version of the item that
	 * should be updated.
	 * @param[in] hash Hash identifying the item.
	 */
	virtual void UpdateItem (Item_ptr item,
			const QString& hash) = 0;
	/** @brief Updates an already existing item.
	 *
	 * This is an overloaded function provided for convenience.
	 *
	 * @param[in] item Short new version of the item.
	 * @param[in] hash Hash identifying the item.
	 */
	virtual void UpdateItem (const ItemShort& item,
			const QString& hash) = 0;

	/** @brief Removes an already existing item.
	 *
	 * This function would emit channelDataUpdated() signal after it
	 * finishes.
	 *
	 * If the specified item doesn't exist, this function should do
	 * nothing.
	 * 
	 * @param[in] item Pointer to the item that should be removed.
	 * @param[in] hash Hash identifying the item.
	 * @param[in] parentTitle Title of the parent channel.
	 * @param[in] feedURL Parent channel's parent feed's URL.
	 */
	virtual void RemoveItem (Item_ptr item,
			const QString& hash,
			const QString& parentTitle,
			const QString& feedURL) = 0;
	/** @brief Removes an already existing feed.
	 *
	 * If the specified feed doesn't exist, this function should do
	 * nothing.
	 *
	 * @param[in] feed Pointer to the feed that should be removed.
	 */
	virtual void RemoveFeed (const QString&) = 0;

	/** @brief Update feeds storage section.
	 *
	 * This function is called if feeds storage section format version
	 * stored in application settings is lower than newer one.
	 * 
	 * @param[in] oldV Old storage version.
	 * @param[in] newV New storage version.
	 *
	 * @return true if update successful, else false.
	 */
    virtual bool UpdateFeedsStorage (int oldV, int newV) = 0;
	/** @brief Update channels storage section.
	 *
	 * This function is called if channels storage section format version
	 * stored in application settings is lower than newer one.
	 * 
	 * @param[in] oldV Old storage version.
	 * @param[in] newV New storage version.
	 *
	 * @return true if update successful, else false.
	 */
    virtual bool UpdateChannelsStorage (int oldV, int newV) = 0;
	/** @brief Update items storage section.
	 *
	 * This function is called if items storage section format version
	 * stored in application settings is lower than newer one.
	 * 
	 * @param[in] oldV Old storage version.
	 * @param[in] newV New storage version.
	 *
	 * @return true if update successful, else false.
	 */
    virtual bool UpdateItemsStorage (int oldV, int newV) = 0;
	/** @brief Toggle channel state.
	 *
	 * Changes state of all the items matching the passed hash.
	 *
	 * @param[in] purl Parent feed's URL.
	 * @param[in] title Parent channel's title.
	 * @param[in] state New state of the items.
	 */
	virtual void ToggleChannelUnread (const QString& purl,
			const QString& title,
			bool state) = 0;
	/** @brief Count unread items number.
	 *
	 * Counts how much unread items are there in the storage.
	 *
	 * @return Number of unread items.
	 */
	virtual int GetUnreadItemsNumber () const = 0;
signals:
	/** @brief Notifies about updated channel information.
	 *
	 * This signal is emitted when a channel is updated.
	 *
	 * @param[out] channel Pointer to the updated channel.
	 */
	void channelDataUpdated (Channel_ptr channel) const;
	/** @brief Notifies about updated item information.
	 *
	 * This signal is emitted when a single item is updated.
	 *
	 * @param[out] item Pointer to the updated item.
	 */
	void itemDataUpdated (Item_ptr item) const;
};

#endif

