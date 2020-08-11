/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "interfaces/lmp/iplaylistprovider.h"
#include "engine/audiosource.h"
#include "nativeplaylist.h"

class QAbstractItemModel;
class QStandardItemModel;
class QStandardItem;
class QModelIndex;

namespace LC
{
namespace LMP
{
	struct MediaInfo;
	class StaticPlaylistManager;

	class PlaylistManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel *Model_;
		QStandardItem *StaticRoot_;

		StaticPlaylistManager *Static_;

		enum PlaylistTypes
		{
			Other,
			Static,
			Random50,
			LovedTracks,
			BannedTracks
		};
		enum Roles
		{
			PlaylistType = IPlaylistProvider::ItemRoles::Max + 1
		};

		QObjectList PlaylistProviders_;
	public:
		PlaylistManager (QObject* = 0);

		QAbstractItemModel* GetPlaylistsModel () const;
		StaticPlaylistManager* GetStaticManager () const;

		void AddProvider (QObject*);

		bool CanDeletePlaylist (const QModelIndex&) const;
		void DeletePlaylist (const QModelIndex&);

		NativePlaylist_t GetSources (const QModelIndex&) const;

		std::optional<MediaInfo> TryResolveMediaInfo (const QUrl&) const;
	private slots:
		void handleStaticPlaylistsChanged ();
	};
}
}
