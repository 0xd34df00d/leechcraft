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

		Storage_->AddToCollection (infos);
		PresentPaths_ += paths;
	}
}
}
