/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSet>

namespace LC
{
namespace Launchy
{
	class FavoritesManager : public QObject
	{
		Q_OBJECT

		QSet<QString> Favorites_;
	public:
		explicit FavoritesManager (QObject* = nullptr);

		const QSet<QString>& GetFavorites () const;
		bool IsFavorite (const QString&) const;

		void AddFavorite (const QString&);
		void RemoveFavorite (const QString&);
	private:
		void Save () const;
		void Load ();
	signals:
		void favoriteAdded (const QString&);
		void favoriteRemoved (const QString&);
	};
}
}
