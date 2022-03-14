/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "checkmodel.h"
#include <functional>
#include <QtDebug>
#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <interfaces/media/idiscographyprovider.h>
#include <interfaces/media/ialbumartprovider.h>
#include <interfaces/media/iartistbiofetcher.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/lmp/ilmpproxy.h>
#include <util/util.h>
#include <util/sll/slotclosure.h>
#include <util/sll/visitor.h>
#include <util/sll/either.h>
#include <util/threads/futures.h>

namespace LC::LMP::BrainSlugz
{
	namespace
	{
		const int AASize = 130;
		const int ArtistSize = 190;

		class ReleasesSubmodel : public Util::RoleNamesMixin<QStandardItemModel>
		{
		public:
			enum Role
			{
				ReleaseName = Qt::UserRole + 1,
				ReleaseYear,
				ReleaseArt
			};

			ReleasesSubmodel (QObject *parent)
			: RoleNamesMixin<QStandardItemModel> { parent }
			{
				QHash<int, QByteArray> roleNames;
				roleNames [ReleaseName] = "releaseName";
				roleNames [ReleaseYear] = "releaseYear";
				roleNames [ReleaseArt] = "releaseArt";
				setRoleNames (roleNames);
			}
		};

		QString GetIcon (const QString& name, int size)
		{
			return Util::GetAsBase64Src (GetProxyHolder ()->GetIconThemeManager ()->
						GetIcon (name).pixmap (size, size).toImage ());
		}
	}

	CheckModel::CheckModel (const Collection::Artists_t& artists, const ILMPProxy_ptr& lmpProxy, QObject *parent)
	: RoleNamesMixin<QStandardItemModel> { parent }
	, AllArtists_ { artists }
	, Proxy_ { lmpProxy }
	, DefaultAlbumIcon_ { GetIcon ("media-optical", AASize * 2) }
	, DefaultArtistIcon_ { GetIcon ("view-media-artist", ArtistSize * 2) }
	, AAProv_ { GetProxyHolder ()->GetPluginsManager ()->
				GetAllCastableTo<Media::IAlbumArtProvider*> ().value (0) }
	, BioProv_ { GetProxyHolder ()->GetPluginsManager ()->
				GetAllCastableTo<Media::IArtistBioFetcher*> ().value (0) }
	{
		QHash<int, QByteArray> roleNames;
		roleNames [Role::ArtistId] = "artistId";
		roleNames [Role::ArtistName] = "artistName";
		roleNames [Role::ScheduledToCheck] = "scheduled";
		roleNames [Role::IsChecked] = "isChecked";
		roleNames [Role::ArtistImage] = "artistImageUrl";
		roleNames [Role::Releases] = "releases";
		roleNames [Role::MissingCount] = "missingCount";
		roleNames [Role::PresentCount] = "presentCount";
		setRoleNames (roleNames);

		for (const auto& artist : artists)
		{
			if (artist.Name_.contains (" vs. ") ||
					artist.Name_.contains (" with ") ||
					artist.Albums_.isEmpty ())
				continue;

			auto item = new QStandardItem { artist.Name_ };
			item->setData (artist.ID_, Role::ArtistId);
			item->setData (artist.Name_, Role::ArtistName);
			item->setData (true, Role::ScheduledToCheck);
			item->setData (false, Role::IsChecked);
			item->setData (DefaultArtistIcon_, Role::ArtistImage);
			item->setData (0, Role::MissingCount);
			item->setData (artist.Albums_.size (), Role::PresentCount);

			const auto submodel = new ReleasesSubmodel { this };
			item->setData (QVariant::fromValue<QObject*> (submodel), Role::Releases);

			appendRow (item);

			Artist2Submodel_ [artist.ID_] = submodel;
			Artist2Item_ [artist.ID_] = item;

			Scheduled_ << artist.ID_;

			if (BioProv_)
				Util::Sequence (this, BioProv_->RequestArtistBio (artist.Name_, false)) >>
						Util::Visitor
						{
							[] (const QString&) {},
							[this, artist, item] (const Media::ArtistBio& bio)
							{
								if (Artist2Item_.contains (artist.ID_))
									item->setData (bio.BasicInfo_.LargeImage_, Role::ArtistImage);
							}
						};
		}
	}

	Collection::Artists_t CheckModel::GetSelectedArtists () const
	{
		Collection::Artists_t result = AllArtists_;
		result.erase (std::remove_if (result.begin (), result.end (),
					[this] (const Collection::Artist& artist)
					{
						return !Scheduled_.contains (artist.ID_);
					}),
				result.end ());
		return result;
	}

	void CheckModel::SetMissingReleases (const QList<Media::ReleaseInfo>& releases,
			const Collection::Artist& artist)
	{
		qDebug () << Q_FUNC_INFO << artist.Name_ << releases.size ();

		const auto item = Artist2Item_.value (artist.ID_);
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "no item for artist"
					<< artist.Name_;
			return;
		}

		const auto model = Artist2Submodel_.value (artist.ID_);
		for (const auto& release : releases)
		{
			auto item = new QStandardItem;
			item->setData (release.Name_, ReleasesSubmodel::ReleaseName);
			item->setData (release.Year_, ReleasesSubmodel::ReleaseYear);
			item->setData (DefaultAlbumIcon_, ReleasesSubmodel::ReleaseArt);
			model->appendRow (item);

			if (AAProv_)
				Util::Sequence (this, AAProv_->RequestAlbumArt ({ artist.Name_, release.Name_ })) >>
						Util::Visitor
						{
							[item] (const QList<QUrl>& urls)
							{
								if (!urls.isEmpty ())
									item->setData (urls.value (0), ReleasesSubmodel::ReleaseArt);
							},
							[] (const QString&) {}
						};
		}

		item->setData (releases.size (), Role::MissingCount);
		item->setData (true, Role::IsChecked);
	}

	void CheckModel::MarkNoNews (const Collection::Artist& artist)
	{
		SetMissingReleases ({}, artist);
	}

	void CheckModel::RemoveUnscheduled ()
	{
		for (const auto& artist : AllArtists_)
		{
			if (Scheduled_.contains (artist.ID_))
				continue;

			const auto item = Artist2Item_.take (artist.ID_);
			if (!item)
				continue;

			Artist2Submodel_.take (artist.ID_)->deleteLater ();

			removeRow (item->row ());
		}
	}

	void CheckModel::SelectAll ()
	{
		for (const auto& artist : AllArtists_)
			if (!Scheduled_.contains (artist.ID_))
				SetArtistScheduled (artist.ID_, true);
	}

	void CheckModel::SelectNone ()
	{
		for (const auto& artist : AllArtists_)
			if (Scheduled_.contains (artist.ID_))
				SetArtistScheduled (artist.ID_, false);
	}

	void CheckModel::SetArtistScheduled (int id, bool scheduled)
	{
		if (!Artist2Item_.contains (id))
			return;

		Artist2Item_ [id]->setData (scheduled, Role::ScheduledToCheck);

		if (scheduled)
			Scheduled_ << id;
		else
			Scheduled_.remove (id);
	}
}
