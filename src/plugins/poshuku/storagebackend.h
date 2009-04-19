#ifndef STORAGEBACKEND_H
#define STORAGEBACKEND_H
#include <boost/shared_ptr.hpp>
#include <QObject>
#include "historymodel.h"
#include "favoritesmodel.h"
#include "speeddialprovider.h"

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
		SBPostgres
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
	 * Removes all the history items that are older than days.
	 *
	 * @param[in] days Maximum age of an item.
	 */
	virtual void ClearOldHistory (int days) = 0;
	/** @brief Get all favorites items from the storage.
	 *
	 * Puts all the favorites items (FavoritesModel::FavoritesItem) from
	 * the storage backend into the passed container.
	 *
	 * @param[out] items. The container with items. They would be
	 * appended to the container.
	 */
	virtual void LoadFavorites (std::vector<FavoritesModel::FavoritesItem>& items) const = 0;
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
	/** @brief Get thumbnail of a page.
	 *
	 * Fills the passed SpeedDialProvider::Item with data for the page
	 * identified by the url in the passed item, or empty item if no
	 * such url exists.
	 *
	 * @param[out] item Speed dial item.
	 */
	virtual void GetThumbnail (SpeedDialProvider::Item& item) const = 0;
	/** @brief Set thumbnail of a page.
	 *
	 * Sets a SpeedDialProvider::Item with data filled for the page
	 * identified by the url in the passed item. If this url already
	 * exists, it is replaced with the new data.
	 *
	 * @param[in] item Speed dial item.
	 */
	virtual void SetThumbnail (const SpeedDialProvider::Item& item) = 0;
signals:
	void added (const HistoryItem&);
	void added (const FavoritesModel::FavoritesItem&);
	void updated (const FavoritesModel::FavoritesItem&);
	void removed (const FavoritesModel::FavoritesItem&);
};

#endif

