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

#ifndef PLUGINS_POSHUKU_STORAGEBACKEND_H
#define PLUGINS_POSHUKU_STORAGEBACKEND_H
#include <boost/shared_ptr.hpp>
#include <QObject>
#include "historymodel.h"
#include "favoritesmodel.h"
#include "pageformsdata.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			/** @brief Abstract base class for storage backends.
			 *
			 * Specifies interface for all storage backends. Includes functions for
			 * storing the history, favorites, saved passwords etc.
			 */
			class StorageBackend : public QObject
			{
				Q_OBJECT
			public:
				enum Type
				{
					SBSQLite,
					SBPostgres,
                    SBMysql
				};

				StorageBackend (QObject* = 0);
				virtual ~StorageBackend ();
				static boost::shared_ptr<StorageBackend> Create (Type);

				/** @brief Do post-initialization.
				 *
				 * This function is called by the Core after all the updates are
				 * checked and done, if required.
				 */
				virtual void Prepare () = 0;

				/** @brief Get all the history items from the storage.
				 *
				 * Puts all the history items (HistoryItem) from the
				 * storage backend into the passed container.
				 *
				 * @param[out] items The container with items. They would be
				 * appended to the container.
				 */
				virtual void LoadHistory (history_items_t& items) const = 0;

				/** @brief Get resembling history items from the storage.
				 *
				 * Puts resembling history items (HistoryItem) from the
				 * storage backend into the passed container. An item is considered
				 * resembling if its title or URL contains base string. The
				 * resembling items container should be sorted by date in descending
				 * order.
				 *
				 * @param[in] base The base string .
				 * @param[out] items The container with items. They would be
				 * appended to the container.
				 */
				virtual void LoadResemblingHistory (const QString& base,
						history_items_t& items) const = 0;

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
				 * @param[out] items. The container with items. They would be
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
		};
	};
};

#endif

