#ifndef SQLSTORAGEBACKEND_H
#define SQLSTORAGEBACKEND_H
#include "storagebackend.h"
#include <QSqlDatabase>
#include <QSqlQuery>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
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
						/** Binds:
						 * - items
						 */
						HistoryTruncater_,
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
						 * - url
						 *
						 * Returns:
						 * - form_index
						 * - name
						 * - type
						 * - value
						 */
						FormsGetter_,
						/** Binds:
						 * - url
						 * - form_index
						 * - name
						 * - type
						 * - value
						 */
						FormsSetter_,
						/** Binds:
						 * - url
						 */
						FormsClearer_,
						/** Binds:
						 * - url
						 */
						FormsIgnoreSetter_,
						/** Binds:
						 * - url
						 */
						FormsIgnoreGetter_,
						/** Binds:
						 * - url
						 */
						FormsIgnoreClearer_;
			public:
				SQLStorageBackend (Type);
				virtual ~SQLStorageBackend ();

				void Prepare ();

				virtual void LoadHistory (history_items_t&) const;
				virtual void LoadResemblingHistory (const QString&,
						history_items_t&) const;
				virtual void AddToHistory (const HistoryItem&);
				virtual void ClearOldHistory (int, int);
				virtual void LoadFavorites (std::vector<FavoritesModel::FavoritesItem>&) const;
				virtual void AddToFavorites (const FavoritesModel::FavoritesItem&);
				virtual void RemoveFromFavorites (const FavoritesModel::FavoritesItem&);
				virtual void UpdateFavorites (const FavoritesModel::FavoritesItem&);
				virtual void GetFormsData (const QString&, ElementsData_t&) const;
				virtual void SetFormsData (const QString&, const ElementsData_t&);
				virtual void SetFormsIgnored (const QString&, bool);
				virtual bool GetFormsIgnored (const QString&) const;
			private:
				void InitializeTables ();
				void CheckVersions ();
				QString GetSetting (const QString&) const;
				void SetSetting (const QString&, const QString&);
			};
		};
	};
};

#endif

