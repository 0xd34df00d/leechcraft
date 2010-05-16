/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_AGGREGATOR_STORAGEBACKEND_H
#define PLUGINS_AGGREGATOR_STORAGEBACKEND_H
#include <QObject>
#include "feed.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			/** @brief Abstract base class for storage backends.
			 *
			 * Specifies interface for all storage backends. Includes functions for
			 * appending, modifying and retrieving feeds, channels and items.
			 */
			class StorageBackend : public QObject
			{
				Q_OBJECT
			public:
				struct ChannelNotFoundError {};
				struct ChannelGettingError {};
				struct ItemNotFoundError {};
				struct ItemGettingError {};
				struct FeedSettingsNotFoundError {};
				struct FeedGettingError {};
				struct FeedNotFoundError {};

				enum Type
				{
					SBSQLite,
					SBPostgres
				};
				StorageBackend (QObject* = 0);
				virtual ~StorageBackend ();
				static boost::shared_ptr<StorageBackend> Create (Type);

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

				/** @brief Returns all the feeds in the storage.
				 *
				 * Puts IDs of all the feeds in the storage into the passed
				 * container.
				 *
				 * @param[out] fids The container with feed IDs. The IDs would be
				 * appended to the container.
				 */
				virtual void GetFeedsIDs (ids_t& fids) const = 0;

				/** @brief Returns the feed identified by its id.
				 *
				 * @param[in] id The ID of the feed to be returned.
				 * @return The full feed information.
				 */
				virtual Feed_ptr GetFeed (const IDType_t& id) const = 0;

				/** @brief Returns the ID of the feed with the given url
				 * or -1 if it could not be found.
				 *
				 * @param[in] url The URL of the feed to be found.
				 * @return The ID of the feed or -1 if there is no such feed.
				 */
				virtual IDType_t FindFeed (const QString& url) const = 0;

				/** @brief Returns feed's settings.
				 *
				 * Returns invalid (default-constructed) FeedSettings if no settings
				 * exist in the storage.
				 *
				 * @param[in] feed Feed's ID.
				 * @return FeedSettings for the feed.
				 */
				virtual Feed::FeedSettings GetFeedSettings (const IDType_t& feed) const = 0;

				/** @brief Sets feed's settings.
				 *
				 * Sets new feed settings replacing old ones if they exist.
				 *
				 * @param[in] settings New feed's settings.
				 */
				virtual void SetFeedSettings (const Feed::FeedSettings& settings) = 0;

				/** @brief Get all the channels of a feed in the container.
				 *
				 * Returns short information about channels in the storage which
				 * have feedParent as their parent's feed ID.
				 *
				 * @param[out] shorts The container with short information about the
				 * channels to which retrieved info would be appended.
				 * @param[in] feedParent Parent feed's ID identifying the feed.
				 */
				virtual void GetChannels (channels_shorts_t& shorts,
						const IDType_t& feedParent) const = 0;

				/** @brief Returns full information about a channel.
				 *
				 * Returns full information about a channel identified
				 * by its ID. The channel's Items_ field isn't filled
				 * with child items though, the items should be
				 * requested by a separate call to GetItems().
				 *
				 * @param[in] channelId The ID of the channel.
				 * @param[in] feedParent Parent feed's ID identifying the feed.
				 * @return Full information about the requested channel.
				 */
				virtual Channel_ptr GetChannel (const IDType_t& channelId,
						const IDType_t& feedParent) const = 0;

				/** @brief Find channel with the given title, link and
				 * and given parent and returns its ID or -1 if it's not
				 * found.
				 *
				 * @param[in] title The channel's title.
				 * @param[in] link The channel's link.
				 * @param[in] feedId ID of the parent feed.
				 * @return ID of the found channel or -1 if it's not
				 * found.
				 */
				virtual IDType_t FindChannel (const QString& title,
						const QString& link, const IDType_t& feedId) const = 0;

				/** @brief Trims the channel to remove old items.
				 *
				 * Emits channelDataUpdated() after that.
				 *
				 * @param[in] channelId The ID of the channel to trim.
				 * @param[in] days Max number of days.
				 * @param[in] number Max number of items.
				 */
				virtual void TrimChannel (const IDType_t& channelId,
						int days, int number) = 0;

				/** @brief Returns short information about items in a channel.
				 *
				 * Returns short information about items in the storage which are
				 * identified by their channel's ID.
				 *
				 * @param[out] items The container to which short information about
				 * the items would be appended.
				 * @param[in] channelId The ID of the channel.
				 */
				virtual void GetItems (items_shorts_t& items,
						const IDType_t& channelId) const = 0;

				/** @brief Counts unread items number in a given channel.
				 *
				 * A possibly optimized version of getting items via
				 * GetItems() and counting unread items by hand.
				 *
				 * @param[in] id Channel's ID.
				 * @return Unread items count.
				 */
				virtual int GetUnreadItems (const IDType_t& id) const = 0;

				/** @brief Returns full information about an item.
				 *
				 * Returns full information about the item identified by
				 * its ID.
				 *
				 * @param[in] id The item's ID.
				 * @return Full information about the requested item.
				 */
				virtual Item_ptr GetItem (const IDType_t& id) const = 0;

				/** @brief Finds first item with the given title, link
				 * and parent channel and returns its ID or -1 if it's
				 * not found.
				 *
				 * @param[in] title Title of the item to be found.
				 * @param[in] link Link of the item to be found.
				 * @param[in] channel ID of the parent channel.
				 * @return ID of the found item or -1 if no such item
				 * exists.
				 */
				virtual IDType_t FindItem (const QString& title,
						const QString& link, const IDType_t& channel) const = 0;

				/** @brief Returns all items in the channel.
				 *
				 * Returns full information about all the items in the
				 * channel identified by channel's ID. The returned
				 * information is appended to the passed container.
				 *
				 * Usually you will use this only inside handleJobFinished().
				 *
				 * @param[out] items The container with items.
				 * @param[in] id The channel's ID.
				 */
				virtual void GetItems (items_container_t& items,
						const IDType_t& id) const = 0;

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
				virtual void AddChannel (Channel_ptr channel) = 0;

				/** @brief Adds a new item to an already existing channel.
				 *
				 * Items are compared by their ID.
				 *
				 * This function would emit channelDataUpdated() signal after it
				 * finishes.
				 *
				 * If the specified item already exists in the storage, this function
				 * should do nothing.
				 *
				 * @param[in] item Pointer to the item that should be added.
				 */
				virtual void AddItem (Item_ptr item) = 0;

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
				 */
				virtual void UpdateChannel (Channel_ptr channel) = 0;

				/** @brief Updates an already existing channel.
				 *
				 * This is an overloaded function provided for convenience.
				 *
				 * @param[in] channel Short information about channel.
				 */
				virtual void UpdateChannel (const ChannelShort& channel) = 0;

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
				 */
				virtual void UpdateItem (Item_ptr item) = 0;

				/** @brief Updates an already existing item.
				 *
				 * This is an overloaded function provided for convenience.
				 *
				 * @param[in] item Short new version of the item.
				 */
				virtual void UpdateItem (const ItemShort& item) = 0;

				/** @brief Removes an already existing item.
				 *
				 * This function would emit channelDataUpdated() signal after it
				 * finishes.
				 *
				 * If the specified item doesn't exist, this function should do
				 * nothing.
				 * 
				 * @param[in] id ID of the item that should be removed.
				 */
				virtual void RemoveItem (const IDType_t& id) = 0;

				/** @brief Removes an already existing feed.
				 *
				 * If the specified feed doesn't exist, this function should do
				 * nothing.
				 *
				 * @param[in] feed Pointer to the feed that should be removed.
				 */
				virtual void RemoveFeed (const IDType_t&) = 0;

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
				 * Changes state of all the items matching the passed ID.
				 *
				 * @param[in] id Channel's ID.
				 * @param[in] state New state of the items.
				 */
				virtual void ToggleChannelUnread (const IDType_t& id,
						bool state) = 0;
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
				 * @param[out] channel Pointer to the channel containing updated
				 * item.
				 */
				void itemDataUpdated (Item_ptr item, Channel_ptr channel) const;
			};
		};
	};
};

#endif

