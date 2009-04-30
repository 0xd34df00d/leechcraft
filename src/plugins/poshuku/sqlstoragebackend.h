#ifndef SQLSTORAGEBACKEND_H
#define SQLSTORAGEBACKEND_H
#include "storagebackend.h"
#include <QSqlDatabase>
#include <QSqlQuery>

class SQLStorageBackend : public StorageBackend
{
	Q_OBJECT

	Type Type_;
	QSqlDatabase DB_;

			/** Returns:
			 * - title
			 * - date
			 * - url
			 */
	mutable QSqlQuery HistoryLoader_,
			/** Binds:
			 * - titlebase
			 * - urlbase
			 *
			 * Returns:
			 * - title
			 * - url
			 */
			HistoryRatedLoader_,
			/** Binds:
			 * - date
			 * - title
			 * - url
			 */
			HistoryAdder_,
			/** Binds:
			 * - age
			 */
			HistoryEraser_,
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
			FavoritesRemover_;
public:
	SQLStorageBackend (Type);
	virtual ~SQLStorageBackend ();

	void Prepare ();

	virtual void LoadHistory (history_items_t&) const;
	virtual void LoadResemblingHistory (const QString&,
			history_items_t&) const;
	virtual void AddToHistory (const HistoryItem&);
	virtual void ClearOldHistory (int);
	virtual void LoadFavorites (std::vector<FavoritesModel::FavoritesItem>&) const;
	virtual void AddToFavorites (const FavoritesModel::FavoritesItem&);
	virtual void RemoveFromFavorites (const FavoritesModel::FavoritesItem&);
	virtual void UpdateFavorites (const FavoritesModel::FavoritesItem&);
private:
	void InitializeTables ();
	void CheckVersions ();
	QString GetSetting (const QString&) const;
	void SetSetting (const QString&, const QString&);
};

#endif

