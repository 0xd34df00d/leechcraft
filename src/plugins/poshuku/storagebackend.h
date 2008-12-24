#ifndef STORAGEBACKEND_H
#define STORAGEBACKEND_H
#include <vector>
#include <QObject>
#include "historymodel.h"
#include "favoritesmodel.h"

/** @brief Abstract base class for storage backends.
 *
 * Specifies interface for all storage backends. Includes functions for
 * storing the history, favorites, saved passwords etc.
 */
class StorageBackend : public QObject
{
	Q_OBJECT
public:
	StorageBackend (QObject* = 0);
	virtual ~StorageBackend ();

	/** @brief Do post-initialization.
	 *
	 * This function is called by the Core after all the updates are
	 * checked and done, if required.
	 */
	virtual void Prepare () = 0;
	/** @brief Get all the history items from the storage.
	 *
	 * Puts all the history items (HistoryModel::HistoryItem) from the
	 * storage backend into the passed container.
	 *
	 * @param[out] items The container with items. They would be
	 * appended to the container.
	 */
	virtual void LoadHistory (std::vector<HistoryModel::HistoryItem>& items) const = 0;
	/** @brief Get unique history items from the storage.
	 *
	 * Puts unique history items (HistoryModel::HistoryItem) from the
	 * storage backend into the passed container. Items are differed by
	 * their title and URL, exact chosen date is undefined.
	 *
	 * @param[out] items The container with items. They would be
	 * appended to the container.
	 */
	virtual void LoadUniqueHistory (std::vector<HistoryModel::HistoryItem>& items) const = 0;
	/** @brief Add an item to history.
	 *
	 * Adds the passed item to the storage and emits the added() signal
	 * if the passed history item is added successfully.
	 *
	 * @param[in] item History item to add.
	 */
	virtual void AddToHistory (const HistoryModel::HistoryItem& item) = 0;
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
	/** @brief Get authentication information.
	 *
	 * Finds authentication information for the given realm and puts it
	 * into login and password. If no information is found, leaves these
	 * fields unchanged.
	 *
	 * @param[in] realm Realm of the interesting authentication resource.
	 * @param[out] login Stored login.
	 * @param[out] password Stored password.
	 */
	virtual void GetAuth (const QString& realm,
			QString& login, QString& password) const = 0;
	/** @brief Set authentication information.
	 *
	 * Sets authentication information for the given realm from passed
	 * parameters. Creates a new record if there is no older one,
	 * updates old record otherwise.
	 *
	 * @param[in] realm Realm of the authentication resource.
	 * @param[in] login New login.
	 * @param[in] password New password.
	 */
	virtual void SetAuth (const QString& realm,
			const QString& login, const QString& password) = 0;
signals:
	void added (const HistoryModel::HistoryItem&);
	void added (const FavoritesModel::FavoritesItem&);
	void updated (const FavoritesModel::FavoritesItem&);
	void removed (const FavoritesModel::FavoritesItem&);
};

#endif

