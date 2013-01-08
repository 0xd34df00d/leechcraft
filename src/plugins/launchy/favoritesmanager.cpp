/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "favoritesmanager.h"
#include <QCoreApplication>
#include <QSettings>
#include <QStringList>

namespace LeechCraft
{
namespace Launchy
{
	FavoritesManager::FavoritesManager (QObject *parent)
	: QObject (parent)
	{
		Load ();
	}

	const QSet<QString>& FavoritesManager::GetFavorites () const
	{
		return Favorites_;
	}

	bool FavoritesManager::IsFavorite (const QString& id) const
	{
		return Favorites_.contains (id);
	}

	void FavoritesManager::AddFavorite (const QString& id)
	{
		if (Favorites_.contains (id))
			return;

		Favorites_ << id;
		Save ();
		emit favoriteAdded (id);
	}

	void FavoritesManager::RemoveFavorite (const QString& id)
	{
		if (Favorites_.remove (id))
		{
			emit favoriteRemoved (id);
			Save ();
		}
	}

	void FavoritesManager::Save () const
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_LMP");
		settings.beginGroup ("Favorites");
		settings.setValue ("IDs", QStringList (Favorites_.toList ()));
		settings.endGroup ();
	}

	void FavoritesManager::Load ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_LMP");
		settings.beginGroup ("Favorites");
		Favorites_ = QSet<QString>::fromList (settings.value ("IDs").toStringList ());
		settings.endGroup ();
	}
}
}
