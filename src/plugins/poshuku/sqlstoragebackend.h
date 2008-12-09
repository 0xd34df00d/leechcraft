#ifndef SQLSTORAGEBACKEND_H
#define SQLSTORAGEBACKEND_H
#include "storagebackend.h"
#include <QSqlDatabase>
#include <QSqlQuery>

class SQLStorageBackend : public StorageBackend
{
	Q_OBJECT

	QSqlDatabase DB_;

			/** Returns:
			 * - title
			 * - date
			 * - url
			 */
	mutable QSqlQuery HistoryLoader_,
			/** Binds:
			 * - date
			 * - title
			 * - url
			 */
			HistoryAdder_,
			/** Returns:
			 * - title
			 * - url
			 * - tags
			 */
			FavoritesLoader_,
			/** Binds:
			 * - title
			 * - url
			 * - tags
			 */
			FavoritesAdder_,
			/** Binds:
			 * - title
			 * - url
			 * - tags
			 */
			FavoritesUpdater_,
			/** Binds:
			 * - url
			 */
			FavoritesRemover_,
			/** Binds:
			 * - realm
			 * Returns:
			 * - login
			 * - password
			 */
			AuthGetter_,
			/** Binds:
			 * - realm
			 * - login
			 * - password
			 */
			AuthInserter_,
			/** Binds:
			 * - realm
			 * - login
			 * - password
			 */
			AuthUpdater_;
public:
	SQLStorageBackend ();
	virtual ~SQLStorageBackend ();

	void Prepare ();

	virtual void LoadHistory (std::vector<HistoryModel::HistoryItem>&) const;
	virtual void AddToHistory (const HistoryModel::HistoryItem&);
	virtual void LoadFavorites (std::vector<FavoritesModel::FavoritesItem>&) const;
	virtual void AddToFavorites (const FavoritesModel::FavoritesItem&);
	virtual void RemoveFromFavorites (const FavoritesModel::FavoritesItem&);
	virtual void UpdateFavorites (const FavoritesModel::FavoritesItem&);
	virtual void GetAuth (const QString&, QString&, QString&) const;
	virtual void SetAuth (const QString&, const QString&, const QString&);
private:
	void InitializeTables ();
};

#endif

