/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "favoritesmanager.h"
#include <QCoreApplication>
#include <QSettings>
#include <QStringList>
#include <util/sll/containerconversions.h>

namespace LC
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
				QCoreApplication::applicationName () + "_Launchy");
		settings.beginGroup ("Favorites");
		settings.setValue ("IDs", QStringList (Favorites_.values ()));
		settings.endGroup ();
	}

	void FavoritesManager::Load ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Launchy");
		settings.beginGroup ("Favorites");
		Favorites_ = Util::AsSet (settings.value ("IDs").toStringList ());
		settings.endGroup ();
	}
}
}
