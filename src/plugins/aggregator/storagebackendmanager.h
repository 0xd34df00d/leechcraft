/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <util/sll/eitherfwd.h>
#include "storagebackend.h"

namespace LC
{
namespace Aggregator
{
	class StorageBackendManager : public QObject
	{
		Q_OBJECT

		StorageBackend_ptr PrimaryStorageBackend_;

		StorageBackendManager () = default;
	public:
		StorageBackendManager (const StorageBackendManager&) = delete;
		StorageBackendManager& operator= (const StorageBackendManager&) = delete;

		static StorageBackendManager& Instance ();

		void Release ();

		struct StorageCreationError
		{
			QString Message_;
		};

		using StorageCreationResult_t = Util::Either<StorageCreationError, StorageBackend_ptr>;
		StorageCreationResult_t CreatePrimaryStorage ();

		bool IsPrimaryStorageCreated () const;

		StorageBackend_ptr MakeStorageBackendForThread () const;

		void Register (const StorageBackend_ptr&);
	signals:
		void channelAdded (const Channel& channel) const;

		/** @brief Notifies about updated channel information.
		 *
		 * This signal is emitted whenever a channel is updated by any of
		 * the instantiated StorageBackend objects.
		 *
		 * @param[out] channel Pointer to the updated channel.
		 */
		void channelUnreadCountUpdated (IDType_t channelId, const UnreadChange& unreadCount) const;

		void channelDataUpdated (const Channel&) const;

		void itemReadStatusUpdated (IDType_t channelId, IDType_t itemId, bool unread) const;

		/** @brief Notifies about updated item information.
		 *
		 * This signal is emitted whenever an item is updated by any of
		 * the instantiated StorageBackend objects.
		 *
		 * @param[out] item Pointer to the updated item.
		 */
		void itemDataUpdated (const Item& item) const;

		/** @brief Notifies that a number of items was removed.
		 *
		 * This signal is emitted whenever items are removed is updated by
		 * any of the instantiated StorageBackend objects.
		 *
		 * @param[out] items The set of IDs of items that have been
		 * removed.
		 */
		void itemsRemoved (const QSet<IDType_t>&) const;

		void channelRemoved (IDType_t) const;
		void feedRemoved (IDType_t) const;

		void storageCreated ();

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
	};
}
}
