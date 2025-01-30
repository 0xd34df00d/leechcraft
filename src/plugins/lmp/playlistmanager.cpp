/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "playlistmanager.h"
#include <algorithm>
#include <QStandardItemModel>
#include <QTimer>
#include <QMimeData>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/sll/functional.h>
#include <util/sll/prelude.h>
#include <util/gui/util.h>
#include <util/models/dndactionsmixin.h>
#include "util/lmp/util.h"
#include "core.h"
#include "staticplaylistmanager.h"
#include "localcollection.h"
#include "mediainfo.h"

namespace LC
{
namespace LMP
{
	namespace
	{
		class PlaylistModel : public Util::DndActionsMixin<QStandardItemModel>
		{
			PlaylistManager *Manager_;
		public:
			enum Roles
			{
				PlaylistProvider = Qt::UserRole + 1
			};

			PlaylistModel (PlaylistManager *parent)
			: DndActionsMixin<QStandardItemModel> (parent)
			, Manager_ (parent)
			{
				setSupportedDragActions (Qt::CopyAction);
			}

			QStringList mimeTypes () const override
			{
				return
				{
					"text/uri-list",
					"x-leechcraft-lmp/media-info-list"
				};
			}

			QMimeData* mimeData (const QModelIndexList& indexes) const override
			{
				QMimeData *result = new QMimeData;

				QList<QUrl> urls;
				QList<MediaInfo> infos;

				for (const auto& idx : indexes)
					for (const auto& item : Manager_->GetSources (idx))
					{
						const auto& url = item.first.ToUrl ();
						if (!url.isValid ())
							continue;

						urls << url;
						infos << item.second.value_or (MediaInfo {});
					}

				result->setUrls (urls);
				Util::Save2MimeData (result, "x-leechcraft-lmp/media-info-list", infos);

				return result;
			}
		};
	}

	PlaylistManager::PlaylistManager (QObject *parent)
	: QObject (parent)
	, Model_ (new PlaylistModel (this))
	, StaticRoot_ (new QStandardItem (tr ("Static playlists")))
	, Static_ (new StaticPlaylistManager (this))
	{
		StaticRoot_->setEditable (false);
		Model_->appendRow (StaticRoot_);

		connect (Static_,
				SIGNAL (customPlaylistsChanged ()),
				this,
				SLOT (handleStaticPlaylistsChanged ()));
		QTimer::singleShot (100,
				this,
				SLOT (handleStaticPlaylistsChanged ()));

		auto dynamicRoot = new QStandardItem (tr ("Dynamic playlists"));
		dynamicRoot->setEditable (false);
		Model_->appendRow (dynamicRoot);

		const std::initializer_list<QPair<PlaylistTypes, QString>> typesNames
		{
			{ PlaylistTypes::Random50, tr ("50 random tracks") },
			{ PlaylistTypes::LovedTracks, tr ("Loved tracks") },
			{ PlaylistTypes::BannedTracks, tr ("Banned tracks") },
		};

		for (const auto& [type, name] : typesNames)
		{
			auto item = new QStandardItem { name };
			item->setData (type, Roles::PlaylistType);
			item->setEditable (false);
			dynamicRoot->appendRow (item);
		}
	}

	QAbstractItemModel* PlaylistManager::GetPlaylistsModel () const
	{
		return Model_;
	}

	StaticPlaylistManager* PlaylistManager::GetStaticManager () const
	{
		return Static_;
	}

	void PlaylistManager::AddProvider (QObject *provObj)
	{
		auto prov = qobject_cast<IPlaylistProvider*> (provObj);
		if (!prov)
			return;

		PlaylistProviders_ << provObj;

		auto root = prov->GetPlaylistsRoot ();
		root->setData (QVariant::fromValue (provObj), PlaylistModel::Roles::PlaylistProvider);
		Model_->appendRow (root);
	}

	bool PlaylistManager::CanDeletePlaylist (const QModelIndex& index) const
	{
		return index.data (Roles::PlaylistType).toInt () == PlaylistTypes::Static;
	}

	void PlaylistManager::DeletePlaylist (const QModelIndex& index)
	{
		if (index.data (Roles::PlaylistType).toInt () == PlaylistTypes::Static)
			Static_->DeleteCustomPlaylist (index.data ().toString ());
	}

	NativePlaylist_t PlaylistManager::GetSources (const QModelIndex& index) const
	{
		auto col = Core::Instance ().GetLocalCollection ();
		auto toSrcs = [] (const auto& paths)
		{
			return Util::Map (paths, [] (const auto& path) { return NativePlaylistItem_t { path, {} }; });
		};

		switch (index.data (Roles::PlaylistType).toInt ())
		{
		case PlaylistTypes::Static:
			return Static_->GetCustomPlaylist (index.data ().toString ());
		case PlaylistTypes::Random50:
			return toSrcs (col->GetDynamicPlaylist (LocalCollection::DynamicPlaylist::Random50));
		case PlaylistTypes::LovedTracks:
			return toSrcs (col->GetDynamicPlaylist (LocalCollection::DynamicPlaylist::LovedTracks));
		case PlaylistTypes::BannedTracks:
			return toSrcs (col->GetDynamicPlaylist (LocalCollection::DynamicPlaylist::BannedTracks));
		default:
			return toSrcs (index.data (IPlaylistProvider::ItemRoles::SourceURLs).value<QList<QUrl>> ());
		}
	}

	std::optional<MediaInfo> PlaylistManager::TryResolveMediaInfo (const QUrl& url) const
	{
		for (auto provObj : PlaylistProviders_)
			if (const auto info = qobject_cast<IPlaylistProvider*> (provObj)->GetURLInfo (url))
				return MediaInfo::FromAudioInfo (*info);

		return {};
	}

	void PlaylistManager::handleStaticPlaylistsChanged ()
	{
		while (StaticRoot_->rowCount ())
			StaticRoot_->removeRow (0);

		const auto& icon = GetProxyHolder ()->GetIconThemeManager ()->GetIcon ("view-media-playlist");
		for (const auto& name : Static_->EnumerateCustomPlaylists ())
		{
			auto item = new QStandardItem (icon, name);
			item->setData (PlaylistTypes::Static, Roles::PlaylistType);
			item->setEditable (false);
			StaticRoot_->appendRow (item);
		}
	}
}
}
