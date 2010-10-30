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

#ifndef PLUGINS_POSHUKU_SQLSTORAGEBACKEND_MYSQL_H
#define PLUGINS_POSHUKU_SQLSTORAGEBACKEND_MYSQL_H
#include "storagebackend.h"
#include <QSqlDatabase>
#include <QSqlQuery>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class SQLStorageBackendMysql : public StorageBackend
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
				SQLStorageBackendMysql (Type);
				virtual ~SQLStorageBackendMysql ();

				void Prepare ();

				virtual void LoadHistory (history_items_t&) const;
				virtual void LoadResemblingHistory (const QString&,
						history_items_t&) const;
				virtual void AddToHistory (const HistoryItem&);
				virtual void ClearOldHistory (int, int);
				virtual void LoadFavorites (FavoritesModel::items_t&) const;
				virtual void AddToFavorites (const FavoritesModel::FavoritesItem&);
				virtual void RemoveFromFavorites (const FavoritesModel::FavoritesItem&);
				virtual void UpdateFavorites (const FavoritesModel::FavoritesItem&);
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

