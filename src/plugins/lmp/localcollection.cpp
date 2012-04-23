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

#include "localcollection.h"
#include <QStandardItemModel>
#include <QtDebug>
#include "localcollectionstorage.h"
#include "core.h"
#include "util.h"
#include "localfileresolver.h"
#include "player.h"

namespace LeechCraft
{
namespace LMP
{
	LocalCollection::LocalCollection (QObject *parent)
	: QObject (parent)
	, Storage_ (new LocalCollectionStorage (this))
	, CollectionModel_ (new QStandardItemModel (this))
	{
		Artists_ = Storage_->Load ();
		Q_FOREACH (const auto& artist, Artists_)
			Q_FOREACH (auto album, artist.Albums_)
				Q_FOREACH (const auto& track, album->Tracks_)
					PresentPaths_ << track.FilePath_;

		AppendToModel (Artists_);
	}

	QAbstractItemModel* LocalCollection::GetCollectionModel () const
	{
		return CollectionModel_;
	}

	void LocalCollection::Scan (const QString& path)
	{
		auto resolver = Core::Instance ().GetLocalFileResolver ();
		auto paths = QSet<QString>::fromList (RecIterate (path));
		paths.subtract (PresentPaths_);

		QList<MediaInfo> infos;
		std::transform (paths.begin (), paths.end (), std::back_inserter (infos),
				[resolver] (const QString& path) { return resolver->ResolveInfo (path); });

		auto newArts = Storage_->AddToCollection (infos);
		PresentPaths_ += paths;

		AppendToModel (newArts);
	}

	namespace
	{
		template<typename T, typename U, typename Parent>
		QStandardItem* GetItem (T& c, U idx, const QString& name, Parent parent)
		{
			if (c.contains (idx))
				return c [idx];

			auto item = new QStandardItem (name);
			parent->appendRow (item);
			c [idx] = item;
			return item;
		}
	}

	void LocalCollection::AppendToModel (const Collection::Artists_t& artists)
	{
		Q_FOREACH (const auto& artist, artists)
		{
			auto artistItem = GetItem (Artist2Item_, artist.ID_, artist.Name_, CollectionModel_);
			Q_FOREACH (auto album, artist.Albums_)
			{
				auto albumItem = GetItem (Album2Item_,
						album->ID_,
						QString ("%1 - %2")
							.arg (album->Year_)
							.arg (album->Name_),
						artistItem);

				Q_FOREACH (const auto& track, album->Tracks_)
				{
					const QString& name = QString ("%1 - %2")
							.arg (track.Number_)
							.arg (track.Name_);
					auto item = new QStandardItem (name);
					albumItem->appendRow (item);
				}
			}
		}
	}
}
}
