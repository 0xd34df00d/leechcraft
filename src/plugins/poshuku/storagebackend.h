/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_STORAGEBACKEND_H
#define PLUGINS_POSHUKU_STORAGEBACKEND_H
#include <memory>
#include <QObject>
#include "interfaces/poshuku/poshukutypes.h"
#include "interfaces/poshuku/istoragebackend.h"
#include "favoritesmodel.h"
#include "pageformsdata.h"

namespace LC
{
namespace Poshuku
{
	/** @brief Abstract base class for storage backends.
		*
		* Specifies interface for all storage backends. Includes functions for
		* storing the history, favorites, saved passwords etc.
		*/
	class StorageBackend : public QObject
						 , public IStorageBackend
	{
		Q_OBJECT
		Q_INTERFACES (LC::Poshuku::IStorageBackend)
	public:
		enum Type
		{
			SBSQLite,
			SBPostgres
		};

		using QObject::QObject;

		static std::shared_ptr<StorageBackend> Create (Type);
		static std::shared_ptr<StorageBackend> Create ();

		/** @brief Get resembling history items from the storage.
			*
			* Puts resembling history items (HistoryItem) from the
			* storage backend into the passed container. An item is considered
			* resembling if its title or URL contains base string. The
			* resembling items container should be sorted by date in descending
			* order.
			*
			* @param[in] base The base string.
			* @return The similar history items.
			*/
		virtual history_items_t LoadResemblingHistory (const QString& base) const = 0;

		/** @brief Add an item to history.
			*
			* Adds the passed item to the storage and emits the added() signal
			* if the passed history item is added successfully.
			*
			* @param[in] item History item to add.
			*/
		virtual void AddToHistory (const HistoryItem& item) = 0;

		/** @brief Clears old history items.
			*
			* Removes all the history items that are older than days. Also
			* removes items that are overlimit.
			*
			* @param[in] days Maximum age of an item.
			* @param[in] items How much items should be kept at most.
			*/
		virtual void ClearOldHistory (int days, int items) = 0;

		/** @brief Get all favorites items from the storage.
			*
			* Puts all the favorites items (FavoritesModel::FavoritesItem) from
			* the storage backend into the passed container.
			*
			* @param[out] items The container with items. They would be
			* appended to the container.
			*/
		virtual void LoadFavorites (FavoritesModel::items_t& items) const = 0;

		/** @brief Add an item to the favorites list.
			*
			* Adds the passed item to the storage and emits the added() signal
			* if the passed favorites item is added successfully.
			*
			* @param[in] item Favorites item to add.
			*/
		virtual void AddToFavorites (const FavoritesModel::FavoritesItem& item) = 0;

		/** @brief Remove an item from the favorites list.
			*
			* Removes the passed item from the storage and emits the removed()
			* signal if the passed favorites item is removed successfully.
			*
			* @param[in] item Favorites item to remove.
			*/
		virtual void RemoveFromFavorites (const FavoritesModel::FavoritesItem& item) = 0;

		/** @brief Update an item in the favorites list.
			*
			* Finds matching record in the database by URL and updates its
			* title and tags according to passed item's ones.
			*
			* @param[in] item Favorites item to update.
			*/
		virtual void UpdateFavorites (const FavoritesModel::FavoritesItem& item) = 0;

		/** @brief Sets the URL to be ignored by password manager.
			*
			* @param[in] url The url of the page that should be ignored.
			* @param[in] ignored Whether it should be ignored or unignored.
			*/
		virtual void SetFormsIgnored (const QString& url, bool ignored) = 0;

		/** @brief Returns the ignore state of the page.
			*
			* @param[in] url The url of the page that is queried.
			*
			* @return Whether the page is ignored or not.
			*/
		virtual bool GetFormsIgnored (const QString& url) const = 0;
	signals:
		void added (const HistoryItem&);
		void added (const FavoritesModel::FavoritesItem&);
		void updated (const FavoritesModel::FavoritesItem&);
		void removed (const FavoritesModel::FavoritesItem&);
	};
}
}

#endif
