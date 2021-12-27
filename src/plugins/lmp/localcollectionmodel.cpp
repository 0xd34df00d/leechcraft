/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "localcollectionmodel.h"
#include <numeric>
#include <QUrl>
#include <QMimeData>
#include <interfaces/core/iiconthememanager.h>
#include <util/sll/prelude.h>
#include <util/sll/unreachable.h>
#include "core.h"
#include "localcollectionstorage.h"
#include "util.h"

namespace LC::LMP
{
	LocalCollectionModel::LocalCollectionModel (LocalCollectionStorage *storage, QObject *parent)
	: DndActionsMixin<QStandardItemModel> { parent }
	, Storage_ { storage }
	{
		setSupportedDragActions (Qt::CopyAction);
	}

	QStringList LocalCollectionModel::mimeTypes () const
	{
		return { "text/uri-list" };
	}

	namespace
	{
		template<typename T, LocalCollectionModel::Role Role>
		QList<T> CollectTrackValues (const QModelIndex& index, const QAbstractItemModel *model)
		{
			const auto type = index.data (LocalCollectionModel::Role::Node).toInt ();
			if (type == LocalCollectionModel::NodeType::Track)
				return { index.data (Role).value<T> () };

			QList<T> result;
			for (int i = 0; i < model->rowCount (index); ++i)
				result += CollectTrackValues<T, Role> (model->index (i, 0, index), model);
			return result;
		}

		QStringList CollectPaths (const QModelIndex& index, const QAbstractItemModel *model)
		{
			return CollectTrackValues<QString, LocalCollectionModel::Role::TrackPath> (index, model);
		}
	}

	QMimeData* LocalCollectionModel::mimeData (const QModelIndexList& indexes) const
	{
		QList<QUrl> urls;
		for (const auto& index : indexes)
			urls += Util::Map (CollectPaths (index, this), &QUrl::fromLocalFile);

		if (urls.isEmpty ())
			return nullptr;

		auto result = new QMimeData;
		result->setUrls (urls);
		return result;
	}

	namespace
	{
		struct RefreshTooltipState
		{
			Collection::TrackStats LastStats_;
			QString VisibleName_;
		};

		QString GetVisibleName (int type, QStandardItem *item)
		{
			switch (type)
			{
			case LocalCollectionModel::NodeType::Track:
				return item->data (LocalCollectionModel::Role::TrackTitle).toString ();
			case LocalCollectionModel::NodeType::Album:
				return item->data (LocalCollectionModel::Role::AlbumName).toString ();
			case LocalCollectionModel::NodeType::Artist:
				return item->data (LocalCollectionModel::Role::ArtistName).toString ();
			}

			Util::Unreachable ();
		}

		RefreshTooltipState RefreshTooltip (QStandardItem *item, LocalCollectionStorage *storage)
		{
			const auto type = item->data (LocalCollectionModel::Role::Node).toInt ();
			if (type == LocalCollectionModel::NodeType::Track)
			{
				const auto trackId = item->data (LocalCollectionModel::Role::TrackID).toInt ();
				const auto& stats = storage->GetTrackStats (trackId);

				if (stats)
				{
					const auto& last = LocalCollectionModel::tr ("Last playback: %1")
							.arg (FormatDateTime (stats.LastPlay_));
					const auto& total = LocalCollectionModel::tr ("Played %n time(s) since %1", nullptr, stats.Playcount_)
							.arg (FormatDateTime (stats.Added_));
					item->setToolTip (last + "\n" + total);
				}
				else
					item->setToolTip (LocalCollectionModel::tr ("Never has been played"));

				return { stats, GetVisibleName (type, item) };
			}

			RefreshTooltipState latest;
			for (int i = 0; i < item->rowCount (); ++i)
				latest = std::max (RefreshTooltip (item->child (i), storage), latest,
						Util::ComparingBy ([] (const auto& state) { return state.LastStats_.LastPlay_; }));
			if (!latest.LastStats_)
			{
				item->setToolTip (LocalCollectionModel::tr ("Never has been played"));
				return {};
			}

			const auto& lastStr = LocalCollectionModel::tr ("Last playback: %1 (%2)")
					.arg (FormatDateTime (latest.LastStats_.LastPlay_))
					.arg ("<em>" + latest.VisibleName_ + "</em>");
			item->setToolTip (lastStr);

			return { latest.LastStats_, GetVisibleName (type, item) };
		}
	}

	QVariant LocalCollectionModel::data (const QModelIndex& index, int role) const
	{
		if (role == Qt::ToolTipRole)
			RefreshTooltip (itemFromIndex (index), Storage_);

		return QStandardItemModel::data (index, role);
	}

	QList<QUrl> LocalCollectionModel::ToSourceUrls (const QList<QModelIndex>& indexes) const
	{
		const auto& paths = std::accumulate (indexes.begin (), indexes.end (), QStringList {},
				[this] (const QStringList& paths, const QModelIndex& item)
					{ return paths + CollectPaths (item, this); });

		QList<QUrl> result;
		result.reserve (paths.size ());
		for (const auto& path : paths)
			result << QUrl::fromLocalFile (path);
		return result;
	}

	namespace
	{
		template<typename T, typename Init, typename Parent, typename Idx>
		QStandardItem* GetItem (T& c, Init f, Parent parent, Idx idx)
		{
			auto& item = c [idx];
			if (item)
				return item;

			item = new QStandardItem;
			item->setEditable (false);
			f (item);
			parent->appendRow (item);
			return item;
		}

		template<typename T, typename Init, typename Parent, typename Idx, typename... Idxs>
		QStandardItem* GetItem (T& c, Init f, Parent parent, Idx idx, Idxs... idxs)
		{
			return GetItem (c [idx], f, parent, idxs...);
		}
	}

	void LocalCollectionModel::AddArtists (const Collection::Artists_t& artists)
	{
		for (const auto& artist : artists)
		{
			auto artistItem = GetItem (Artist2Item_,
					[this, &artist] (QStandardItem *item)
					{
						item->setIcon (ArtistIcon_);
						item->setText (artist.Name_);
						item->setData (artist.Name_, Role::ArtistName);
						item->setData (NodeType::Artist, Role::Node);
					},
					this,
					artist.ID_);

			for (const auto& album : artist.Albums_)
			{
				auto albumItem = GetItem (Album2Item_,
						[album, artist] (QStandardItem *item)
						{
							item->setText (QString::fromUtf8 ("%1 — %2")
									.arg (album->Year_)
									.arg (album->Name_));
							item->setData (album->ID_, Role::AlbumID);
							item->setData (album->Year_, Role::AlbumYear);
							item->setData (album->Name_, Role::AlbumName);
							item->setData (artist.Name_, Role::ArtistName);
							item->setData (NodeType::Album, Role::Node);
							if (!album->CoverPath_.isEmpty ())
								item->setData (album->CoverPath_, Role::AlbumArt);
						},
						artistItem,
						album->ID_,
						artist.ID_);

				for (const auto& track : album->Tracks_)
				{
					const QString& name = QString::fromUtf8 ("%1 — %2")
							.arg (track.Number_)
							.arg (track.Name_);
					auto item = new QStandardItem (name);
					item->setEditable (false);
					item->setData (album->Year_, Role::AlbumYear);
					item->setData (album->Name_, Role::AlbumName);
					item->setData (artist.Name_, Role::ArtistName);
					item->setData (track.ID_, Role::TrackID);
					item->setData (track.Number_, Role::TrackNumber);
					item->setData (track.Name_, Role::TrackTitle);
					item->setData (track.FilePath_, Role::TrackPath);
					item->setData (track.Genres_, Role::TrackGenres);
					item->setData (track.Length_, Role::TrackLength);
					item->setData (NodeType::Track, Role::Node);
					albumItem->appendRow (item);

					Track2Item_ [track.ID_] = item;
				}
			}
		}
	}

	void LocalCollectionModel::Clear ()
	{
		clear ();

		Artist2Item_.clear ();
		Album2Item_.clear ();
		Track2Item_.clear ();
	}

	void LocalCollectionModel::IgnoreTrack (int id)
	{
		auto item = Track2Item_.value (id);
		item->setData (true, IsTrackIgnored);
	}

	void LocalCollectionModel::RemoveTrack (int id)
	{
		auto item = Track2Item_.take (id);
		item->parent ()->removeRow (item->row ());
	}

	void LocalCollectionModel::RemoveAlbum (int id)
	{
		for (const auto item : Album2Item_.take (id))
			item->parent ()->removeRow (item->row ());
	}

	QVariant LocalCollectionModel::GetTrackData (int trackId, LocalCollectionModel::Role role) const
	{
		const auto item = Track2Item_.value (trackId);
		return item ? item->data (role) : QVariant ();
	}

	void LocalCollectionModel::RemoveArtist (int id)
	{
		removeRow (Artist2Item_.take (id)->row ());
	}

	void LocalCollectionModel::SetAlbumArt (int id, const QString& path)
	{
		for (const auto item : Album2Item_.value (id))
			item->setData (path, Role::AlbumArt);
	}

	void LocalCollectionModel::UpdatePlayStats (int trackId)
	{
		auto item = Track2Item_ [trackId];

		while (item)
		{
			item->setToolTip ({});
			item = item->parent ();
		}
	}
}
