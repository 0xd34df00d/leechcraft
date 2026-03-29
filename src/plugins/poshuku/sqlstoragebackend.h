/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSqlDatabase>
#include <util/sll/util.h>
#include <util/db/oral/oralfwd.h>
#include "interfaces/poshuku/poshukutypes.h"
#include "interfaces/poshuku/istoragebackend.h"
#include "favoritesmodel.h"

namespace LC
{
namespace Poshuku
{
	class SQLStorageBackend
		: public QObject
		, public IStorageBackend
	{
		Q_OBJECT
		Q_INTERFACES (LC::Poshuku::IStorageBackend)

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
		SQLStorageBackend ();
		~SQLStorageBackend ();

		void LoadHistory (history_items_t&) const override;
		history_items_t LoadResemblingHistory (const QString&) const;
		void AddToHistory (const HistoryItem&);
		void ClearOldHistory (int, int);
		void LoadFavorites (FavoritesModel::items_t&) const;
		void AddToFavorites (const FavoritesModel::FavoritesItem&);
		void RemoveFromFavorites (const FavoritesModel::FavoritesItem&);
		void UpdateFavorites (const FavoritesModel::FavoritesItem&);
		void SetFormsIgnored (const QString&, bool);
		bool GetFormsIgnored (const QString&) const;
	signals:
		void added (const HistoryItem&);
		void added (const FavoritesModel::FavoritesItem&);
		void updated (const FavoritesModel::FavoritesItem&);
		void removed (const FavoritesModel::FavoritesItem&);
	};
}
}
