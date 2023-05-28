/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "storagebackend.h"
#include <QSqlDatabase>
#include <util/sll/util.h>
#include <util/db/oral/oralfwd.h>

namespace LC
{
namespace Poshuku
{
	class SQLStorageBackend : public StorageBackend
	{
		const Type Type_;
		QSqlDatabase DB_;
		const Util::DefaultScopeGuard DBGuard_;
	public:
		struct History;
		struct Favorites;
		struct FormsNever;
	private:
		Util::oral::ObjectInfo_ptr<History> History_;
		Util::oral::ObjectInfo_ptr<Favorites> Favorites_;
		Util::oral::ObjectInfo_ptr<FormsNever> FormsNever_;
	public:
		SQLStorageBackend (Type);

		void LoadHistory (history_items_t&) const override;
		history_items_t LoadResemblingHistory (const QString&) const override;
		void AddToHistory (const HistoryItem&) override;
		void ClearOldHistory (int, int) override;
		void LoadFavorites (FavoritesModel::items_t&) const override;
		void AddToFavorites (const FavoritesModel::FavoritesItem&) override;
		void RemoveFromFavorites (const FavoritesModel::FavoritesItem&) override;
		void UpdateFavorites (const FavoritesModel::FavoritesItem&) override;
		void SetFormsIgnored (const QString&, bool) override;
		bool GetFormsIgnored (const QString&) const override;
	};
}
}
