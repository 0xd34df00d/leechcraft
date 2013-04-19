/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "playlistmanager.h"
#include <algorithm>
#include <QStandardItemModel>
#include <QTimer>
#include <QMimeData>
#include <boost/graph/graph_concepts.hpp>
#include "core.h"
#include "staticplaylistmanager.h"
#include "localcollection.h"
#include "mediainfo.h"

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		class PlaylistModel : public QStandardItemModel
		{
			PlaylistManager *Manager_;
		public:
			enum Roles
			{
				IncrementalFetch = Qt::UserRole + 1,
				PlaylistProvider
			};

			PlaylistModel (PlaylistManager *parent)
			: QStandardItemModel (parent)
			, Manager_ (parent)
			{
				setSupportedDragActions (Qt::CopyAction);
			}

			QStringList mimeTypes () const
			{
				return QStringList ("text/uri-list");
			}

			QMimeData* mimeData (const QModelIndexList& indexes) const
			{
				QMimeData *result = new QMimeData;

				QList<QUrl> urls;
				Q_FOREACH (const auto& idx, indexes)
				{
					const auto& sources = Manager_->GetSources (idx);
					std::transform (sources.begin (), sources.end (), std::back_inserter (urls),
							[] (decltype (sources.front ()) src) -> QUrl
							{
								switch (src.type ())
								{
								case Phonon::MediaSource::LocalFile:
									return QUrl::fromLocalFile (src.fileName ());
								case Phonon::MediaSource::Url:
									return src.url ();
								default:
									return QUrl ();
								}
							});
				}

				urls.removeAll (QUrl ());

				result->setUrls (urls);

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

		const std::vector<PlaylistTypes> types =
		{
			PlaylistTypes::Random50,
			PlaylistTypes::LovedTracks,
			PlaylistTypes::BannedTracks
		};
		const std::vector<QString> names =
		{
			tr ("50 random tracks"),
			tr ("Loved tracks"),
			tr ("Banned tracks")
		};

		for (size_t i = 0, size = types.size (); i < size; ++i)
		{
			auto item = new QStandardItem (names.at (i));
			item->setData (types.at (i), Roles::PlaylistType);
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
		root->setData (true, PlaylistModel::Roles::IncrementalFetch);
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

	QList<Phonon::MediaSource> PlaylistManager::GetSources (const QModelIndex& index) const
	{
		auto col = Core::Instance ().GetLocalCollection ();
		auto toSrcs = [col] (const QList<int>& ids) -> QList<Phonon::MediaSource>
		{
			const auto& paths = col->TrackList2PathList (ids);
			QList<Phonon::MediaSource> result;
			std::transform (paths.begin (), paths.end (), std::back_inserter (result),
					[] (const QString& path) { return Phonon::MediaSource (path); });
			return result;
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
		{
			QList<Phonon::MediaSource> result;
			const auto& urls = index.data (IPlaylistProvider::ItemRoles::SourceURLs).value<QList<QUrl>> ();
			std::transform (urls.begin (), urls.end (), std::back_inserter (result),
					[] (decltype (urls.front ()) path) { return Phonon::MediaSource (path); });
			return result;
		}
		}
	}

	boost::optional<MediaInfo> PlaylistManager::TryResolveMediaInfo (const QUrl& url) const
	{
		Q_FOREACH (auto provObj, PlaylistProviders_)
		{
			auto prov = qobject_cast<IPlaylistProvider*> (provObj);
			auto info = prov->GetURLInfo (url);
			if (!info)
				continue;

			return boost::make_optional (MediaInfo::FromAudioInfo (*info));
		}

		return boost::optional<MediaInfo> ();
	}

	void PlaylistManager::handleStaticPlaylistsChanged ()
	{
		while (StaticRoot_->rowCount ())
			StaticRoot_->removeRow (0);

		const auto& icon = Core::Instance ().GetProxy ()->GetIcon ("view-media-playlist");
		Q_FOREACH (const auto& name, Static_->EnumerateCustomPlaylists ())
		{
			auto item = new QStandardItem (icon, name);
			item->setData (PlaylistTypes::Static, Roles::PlaylistType);
			item->setEditable (false);
			StaticRoot_->appendRow (item);
		}
	}
}
}
