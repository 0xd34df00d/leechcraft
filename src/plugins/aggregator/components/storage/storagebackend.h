/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <variant>
#include <QObject>
#include <QSet>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "feed.h"

namespace LC::Aggregator
{
	class StorageBackend;
	using StorageBackend_ptr = std::shared_ptr<StorageBackend>;

	struct UnreadDelta
	{
		int delta;

		auto operator<=> (const UnreadDelta&) const = default;
	};

	struct UnreadTotal
	{
		qsizetype total;

		auto operator<=> (const UnreadTotal&) const = default;
	};

	struct UnreadChange : std::variant<UnreadDelta, UnreadTotal>
	{
		using variant::variant;

		auto operator<=> (const UnreadChange&) const = default;
	};

	/** @brief Abstract base class for storage backends.
	 *
	 * Specifies interface for all storage backends. Includes functions for
	 * appending, modifying and retrieving feeds, channels and items.
	 */
	class StorageBackend : public QObject
	{
		Q_OBJECT
	public:
		struct FeedNotFoundError {};
		struct ChannelNotFoundError {};
		struct ItemNotFoundError {};

		enum Type
		{
			SBSQLite,
			SBPostgres,
		};

		using QObject::QObject;

		static StorageBackend_ptr Create (const QString&, const QString& = QString ());
		static StorageBackend_ptr Create (Type, const QString& = QString ());

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
		 * @return All feed IDs.
		 */
		[[nodiscard]] virtual ids_t GetFeedsIDs () const = 0;

		/** @brief Returns the feed identified by its id.
		 *
		 * @param[in] id The ID of the feed to be returned.
		 * @return The full feed information.
		 */
		virtual Feed GetFeed (IDType_t id) const = 0;

		/** @brief Returns the ID of the feed with the given url.
		 *
		 * @param[in] url The URL of the feed to be found.
		 * @return The ID of the feed or an empty optional if there is no such feed.
		 */
		virtual std::optional<IDType_t> FindFeed (const QString& url) const = 0;

		/** @brief Returns feed's settings.
		 *
		 * Returns invalid (default-constructed) FeedSettings if no settings
		 * exist in the storage.
		 *
		 * @param[in] feed Feed's ID.
		 * @return FeedSettings for the feed.
		 */
		virtual std::optional<Feed::FeedSettings> GetFeedSettings (IDType_t feed) const = 0;

		/** @brief Sets feed's settings.
		 *
		 * Sets new feed settings replacing old ones if they exist.
		 *
		 * @param[in] settings New feed's settings.
		 */
		virtual void SetFeedSettings (const Feed::FeedSettings& settings) = 0;

		virtual std::optional<QStringList> GetFeedTags (IDType_t feed) const = 0;
		virtual void SetFeedTags (IDType_t feed, const QStringList& tags) = 0;

		virtual void SetFeedURL (IDType_t feed, const QString& url) = 0;

		/** @brief Get all the channels of a feed in the container.
		 *
		 * Returns short information about channels in the storage which
		 * have feedParent as their parent's feed ID.
		 *
		 * @param[out] shorts The container with short information about the
		 * channels to which retrieved info would be appended.
		 * @param[in] feedParent Parent feed's ID identifying the feed.
		 */
		virtual channels_shorts_t GetChannels (IDType_t feedParent) const = 0;

		/** @brief Returns full information about a channel.
		 *
		 * Returns full information about a channel identified
		 * by its ID. The channel's Items_ field isn't filled
		 * with child items though, the items should be
		 * requested by a separate call to GetItems().
		 *
		 * @param[in] channelId The ID of the channel.
		 * @return Full information about the requested channel.
		 */
		virtual Channel GetChannel (IDType_t channelId) const = 0;

		/** @brief Find channel with the given identifying information.
		 *
		 * @param[in] title The channel's title.
		 * @param[in] link The channel's link.
		 * @param[in] feedId ID of the parent feed.
		 * @return ID of the channel or an empty optional if no such channel exists.
		 */
		virtual std::optional<IDType_t> FindChannel (const QString& title,
				const QString& link, IDType_t feedId) const = 0;

		virtual std::optional<QImage> GetChannelPixmap (IDType_t channelId) const = 0;
		virtual void SetChannelPixmap (IDType_t channelId, const std::optional<QImage>& img) = 0;
		virtual void SetChannelFavicon (IDType_t channelId, const std::optional<QImage>& img) = 0;

		virtual void SetChannelTags (IDType_t channelId, const QStringList& tagIds) = 0;

		virtual void SetChannelDisplayTitle (IDType_t channelId, const QString& title) = 0;

		virtual void SetChannelTitle (IDType_t, const QString& title) = 0;
		virtual void SetChannelLink (IDType_t, const QString& link) = 0;

		/** @brief Trims the channel to remove old items.
		 *
		 * Emits channelDataUpdated() after that.
		 *
		 * @param[in] channelId The ID of the channel to trim.
		 * @param[in] days Max number of days.
		 * @param[in] number Max number of items.
		 */
		virtual void TrimChannel (IDType_t channelId, int days, int number) = 0;

		/** @brief Returns short information about items in a channel.
		 *
		 * Returns short information about items in the storage which are
		 * identified by their channel's ID.
		 *
		 * @param[out] items The container to which short information about
		 * the items would be appended.
		 * @param[in] channelId The ID of the channel.
		 */
		virtual items_shorts_t GetItems (IDType_t channelId) const = 0;

		enum class ReadStatus
		{
			All,
			Read,
			Unread,
		};

		virtual QSet<QString> GetItemsCategories (IDType_t channelId, ReadStatus readStatus = ReadStatus::All) const = 0;

		/** @brief Counts unread items number in a given channel.
		 *
		 * @param[in] id Channel's ID.
		 * @return Unread items count.
		 */
		virtual int GetUnreadItemsCount (IDType_t id) const = 0;

		/** @brief Returns the total items count in the \em channel.
		 *
		 * @param[in] channel Channel ID.
		 * @return Total items count.
		 */
		virtual int GetTotalItemsCount (IDType_t channel) const = 0;

		/** @brief Returns full information about an item.
		 *
		 * Returns full information about the item identified by
		 * its ID.
		 *
		 * @param[in] id The item's ID.
		 * @return Full information about the requested item.
		 */
		virtual std::optional<Item> GetItem (IDType_t id) const = 0;

		/** @brief Finds first item with the given title, link and parent
		 * channel.
		 *
		 * Returns its ID or an empty optional object if no such item is
		 * found.
		 *
		 * This function requires both \em title and \em link to match.
		 * Sometimes this may be too strict, so functions FindItemByLink()
		 * and FindItemByTitle() may be useful instead.
		 *
		 * @param[in] title Title of the item to be found.
		 * @param[in] link Link of the item to be found.
		 * @param[in] channel ID of the parent channel.
		 * @return ID of the found item or an empty optional object if no
		 * such item exists.
		 *
		 * @sa FindItemByLink()
		 * @sa FindItemByTitle()
		 */
		virtual std::optional<IDType_t> FindItem (const QString& title,
				const QString& link, IDType_t channel) const = 0;

		/** @brief Finds first item with the given title and parent
		 * channel.
		 *
		 * This function may be seen as a less strict one than FindItem().
		 *
		 * @param[in] title Title of the item to be found.
		 * @param[in] channel ID of the parent channel.
		 * @return ID of the found item or an empty optional object if no
		 * such item exists.
		 *
		 * @sa FindItem()
		 * @sa FindItemByLink()
		 */
		virtual std::optional<IDType_t> FindItemByTitle (const QString& title, IDType_t channel) const = 0;

		/** @brief Finds first item with the given link and parent
		 * channel.
		 *
		 * If the link is an empty string, this function returns an empty
		 * optional object.
		 *
		 * This function may be seen as a less strict one than FindItem().
		 *
		 * @param[in] link Link of the item to be found.
		 * @param[in] channel ID of the parent channel.
		 * @return ID of the found item or an empty optional object if no
		 * such item exists, or if \em link is empty.
		 *
		 * @sa FindItem()
		 * @sa FindItemByTitle()
		 */
		virtual std::optional<IDType_t> FindItemByLink (const QString& link, IDType_t channel) const = 0;

		/** @brief Puts a feed and all its child channels and items into the
		 * storage.
		 *
		 * If the specified feed already exists in the storage, this function
		 * should do nothing.
		 *
		 * @param[in] feed Pointer to the feed that should be added.
		 */
		virtual void AddFeed (const Feed& feed) = 0;

		/** @brief Adds a new channel to an already existing feed.
		 *
		 * Channels are identified by parent feed's URL.
		 *
		 * If the specified channel already exists in the storage, this
		 * function should do nothing.
		 *
		 * @param[in] channel Pointer to the channel that should be added.
		 */
		virtual void AddChannel (const Channel& channel) = 0;

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
		virtual void AddItem (const Item& item) = 0;

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
		virtual void UpdateItem (const Item& item) = 0;

		/** @brief Changes the read status of the \em item.
		 *
		 * @param[in] item The unique ID of the item.
		 * @param[in] unread Whether the item is unread.
		 */
		virtual void SetItemUnread (IDType_t channel, IDType_t item, bool unread) = 0;

		/** @brief Removes an already existing item.
		 *
		 * This function emits channelDataUpdated() and itemsRemoved()
		 * signals if it removes all the items successfully.
		 *
		 * If some of the specified items don't exist, this function
		 * merely ignores them..
		 *
		 * @param[in] ids IDs of the items that should be removed.
		 */
		virtual void RemoveItems (const QSet<IDType_t>& ids) = 0;

		/** @brief Removes an already existing channel.
		 *
		 * This function should remove the given channel, leaving other
		 * channels from the corresponding feed intact.
		 *
		 * If the specified channel doesn't exist, this function should
		 * do nothing.
		 *
		 * @param[in] id Channel ID.
		 */
		virtual void RemoveChannel (IDType_t id) = 0;

		/** @brief Removes an already existing feed.
		 *
		 * If the specified feed doesn't exist, this function should do
		 * nothing.
		 *
		 * @param[in] feedId The ID of the feed that should be removed.
		 */
		virtual void RemoveFeed (IDType_t feedId) = 0;

		/** @brief Update feeds storage section.
		 *
		 * This function is called if feeds storage section format version
		 * stored in application settings is lower than newer one.
		 *
		 * @param[in] oldV Old storage version.
		 *
		 * @return true if update successful, else false.
		 */
		virtual bool UpdateFeedsStorage (int oldV) = 0;

		/** @brief Update channels storage section.
		 *
		 * This function is called if channels storage section format version
		 * stored in application settings is lower than newer one.
		 *
		 * @param[in] oldV Old storage version.
		 *
		 * @return true if update successful, else false.
		 */
		virtual bool UpdateChannelsStorage (int oldV) = 0;

		/** @brief Update items storage section.
		 *
		 * This function is called if items storage section format version
		 * stored in application settings is lower than newer one.
		 *
		 * @param[in] oldV Old storage version.
		 *
		 * @return true if update successful, else false.
		 */
		virtual bool UpdateItemsStorage (int oldV) = 0;

		/** @brief Toggle channel state.
		 *
		 * Changes state of all the items matching the passed ID.
		 *
		 * @param[in] id Channel's ID.
		 * @param[in] state New state of the items.
		 */
		virtual void ToggleChannelUnread (IDType_t id, bool state) = 0;

		virtual QList<ITagsManager::tag_id> GetItemTags (IDType_t id) = 0;
		virtual void SetItemTags (IDType_t id, const QList<ITagsManager::tag_id>& tags) = 0;
		virtual QList<IDType_t> GetItemsForTag (const ITagsManager::tag_id& tag) = 0;

		/** @brief Searches for highest id of given type in the database
		 *
		 * @param[in] type of id to find
		 * @return highest channels id in the database or 0 if empty
		 */
		virtual IDType_t GetHighestID (const PoolType& type) const = 0;
	signals:
		void channelAdded (const Channel& channel) const;

		/** @brief Notifies about updated channel information.
		 *
		 * This signal is emitted when a channel is updated.
		 *
		 * @warning StorageBackendManager::channelUnreadCountUpdated() should
		 * be used instead as it collects the signal from all
		 * instantiated storage managers.
		 *
		 * @param[out] channel Pointer to the updated channel.
		 *
		 * @sa StorageBackendManager
		 */
		void channelUnreadCountUpdated (IDType_t channelId, const UnreadChange& unreadChange) const;

		void channelDataUpdated (const Channel&) const;

		void itemReadStatusUpdated (IDType_t channelId, IDType_t itemId, bool unread) const;

		/** @brief Notifies about updated item information.
		 *
		 * This signal is emitted when a single item is updated.
		 *
		 * @warning StorageBackendManager::itemDataUpdated() should
		 * be used instead as it collects the signal from all
		 * instantiated storage managers.
		 *
		 * @param[out] item Pointer to the updated item.
		 *
		 * @sa StorageBackendManager
		 */
		void itemDataUpdated (const Item& item) const;

		/** @brief Notifies that a number of items was removed.
		 *
		 * @warning StorageBackendManager::itemsRemoved() should
		 * be used instead as it collects the signal from all
		 * instantiated storage managers.
		 *
		 * @param[out] items The set of IDs of items that have been
		 * removed.
		 *
		 * @sa StorageBackendManager
		 */
		void itemsRemoved (const QSet<IDType_t>& items) const;

		/** @brief Should be emitted whenever a full item is loaded.
		 *
		 * @param[out] proxy Standard proxy object.
		 * @param[out] item The pointer to the already loaded item.
		 */
		void hookItemLoad (LC::IHookProxy_ptr proxy, Item *item) const;

		/** @brief Emitted whenever a new item is added.
		 *
		 * @param proxy Standard proxy object.
		 * @param item The item being added.
		 */
		void hookItemAdded (LC::IHookProxy_ptr proxy, const Item& item) const;

		void channelRemoved (IDType_t channelId);
		void feedRemoved (IDType_t feedId);
	};
}

Q_DECLARE_METATYPE (LC::Aggregator::UnreadChange)
