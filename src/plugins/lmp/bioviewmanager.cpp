/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bioviewmanager.h"
#include <numeric>
#include <QQuickWidget>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QStandardItemModel>
#include <util/util.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/themeimageprovider.h>
#include <util/sys/paths.h>
#include <util/models/rolenamesmixin.h>
#include <util/sll/slotclosure.h>
#include <util/sll/prelude.h>
#include <util/sll/visitor.h>
#include <util/threads/futures.h>
#include <interfaces/media/idiscographyprovider.h>
#include <interfaces/media/ialbumartprovider.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/iiconthememanager.h>
#include "biopropproxy.h"
#include "core.h"
#include "util.h"
#include "previewhandler.h"
#include "stdartistactionsmanager.h"
#include "localcollection.h"

namespace LC
{
namespace LMP
{
	namespace
	{
		class DiscoModel : public Util::RoleNamesMixin<QStandardItemModel>
		{
		public:
			enum Roles
			{
				AlbumName = Qt::UserRole + 1,
				AlbumYear,
				AlbumImage,
				AlbumTrackListTooltip
			};

			DiscoModel (QObject *parent)
			: RoleNamesMixin<QStandardItemModel> (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Roles::AlbumName] = "albumName";
				roleNames [Roles::AlbumYear] = "albumYear";
				roleNames [Roles::AlbumImage] = "albumImage";
				roleNames [Roles::AlbumTrackListTooltip] = "albumTrackListTooltip";
				setRoleNames (roleNames);
			}
		};

		const int AASize = 170;
	}

	BioViewManager::BioViewManager (const ICoreProxy_ptr& proxy,
			QQuickWidget *view, QObject *parent)
	: QObject (parent)
	, View_ (view)
	, BioPropProxy_ (new BioPropProxy (this))
	, DiscoModel_ (new DiscoModel (this))
	, Proxy_ (proxy)
	{
		View_->rootContext ()->setContextObject (BioPropProxy_);
		View_->rootContext ()->setContextProperty ("artistDiscoModel", DiscoModel_);
		View_->rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));
		View_->engine ()->addImageProvider ("ThemeIcons", new Util::ThemeImageProvider (proxy));

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, ""))
			View_->engine ()->addImportPath (cand);
	}

	void BioViewManager::InitWithSource ()
	{
		connect (View_->rootObject (),
				SIGNAL (albumPreviewRequested (int)),
				this,
				SLOT (handleAlbumPreviewRequested (int)));

		new StdArtistActionsManager (Proxy_, View_, this);
	}

	void BioViewManager::Request (Media::IArtistBioFetcher *fetcher, const QString& artist, const QStringList& releases)
	{
		// Only clear the models if the artist is different to avoid flicker in the (most common) case
		// of the artist being the same.
		if (CurrentArtist_ != artist)
		{
			DiscoModel_->clear ();
			BioPropProxy_->SetBio ({});

			CurrentArtist_ = artist;
		}

		Util::Sequence (this, fetcher->RequestArtistBio (CurrentArtist_)) >>
				Util::Visitor
				{
					[this] (const QString&) { BioPropProxy_->SetBio ({}); },
					[this] (const Media::ArtistBio& bio)
					{
						BioPropProxy_->SetBio (bio);
						emit gotArtistImage (bio.BasicInfo_.Name_, bio.BasicInfo_.LargeImage_);
					}
				};

		auto pm = Core::Instance ().GetProxy ()->GetPluginsManager ();
		for (auto prov : pm->GetAllCastableTo<Media::IDiscographyProvider*> ())
			Util::Sequence (this, prov->GetDiscography (CurrentArtist_, releases)) >>
					Util::Visitor
					{
						[artist] (const QString&) { qWarning () << Q_FUNC_INFO << "error for" << artist; },
						[this] (const auto& releases) { HandleDiscographyReady (releases); }
					};
	}

	QStandardItem* BioViewManager::FindAlbumItem (const QString& albumName) const
	{
		for (int i = 0, rc = DiscoModel_->rowCount (); i < rc; ++i)
		{
			auto item = DiscoModel_->item (i);
			const auto& itemData = item->data (DiscoModel::Roles::AlbumName);
			if (itemData.toString () == albumName)
				return item;
		}
		return 0;
	}

	bool BioViewManager::QueryReleaseImageLocal (const Media::AlbumInfo& info) const
	{
		const auto coll = Core::Instance ().GetLocalCollection ();
		const auto albumId = coll->FindAlbum (info.Artist_, info.Album_);
		if (albumId == -1)
			return false;

		const auto& album = coll->GetAlbum (albumId);
		if (!album)
			return false;

		const auto& path = album->CoverPath_;
		if (path.isEmpty () || !QFile::exists (path))
			return false;

		SetAlbumImage (info.Album_, QUrl::fromLocalFile (path));
		return true;
	}

	void BioViewManager::QueryReleaseImage (Media::IAlbumArtProvider *aaProv, const Media::AlbumInfo& info)
	{
		if (QueryReleaseImageLocal (info))
			return;

		Util::Sequence (this, aaProv->RequestAlbumArt (info)) >>
				Util::Visitor
				{
					[this, info] (const QList<QUrl>& urls)
					{
						if (info.Artist_ == CurrentArtist_ && !urls.isEmpty ())
							SetAlbumImage (info.Album_, urls.first ());
					},
					[] (const QString&) {}
				};
	}

	void BioViewManager::SetAlbumImage (const QString& album, const QUrl& img) const
	{
		auto item = FindAlbumItem (album);
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown item for"
					<< album;
			return;
		}

		item->setData (img, DiscoModel::Roles::AlbumImage);
	}

	void BioViewManager::HandleDiscographyReady (QList<Media::ReleaseInfo> releases)
	{
		const auto pm = Core::Instance ().GetProxy ()->GetPluginsManager ();
		const auto aaProvObj = pm->GetAllCastableRoots<Media::IAlbumArtProvider*> ().value (0);
		const auto aaProv = qobject_cast<Media::IAlbumArtProvider*> (aaProvObj);

		const auto& icon = Core::Instance ().GetProxy ()->GetIconThemeManager ()->
				GetIcon ("media-optical").pixmap (AASize * 2, AASize * 2);

		std::sort (releases.rbegin (), releases.rend (),
				Util::ComparingBy (&Media::ReleaseInfo::Year_));
		for (const auto& release : releases)
		{
			if (FindAlbumItem (release.Name_))
				continue;

			auto item = new QStandardItem;
			item->setData (release.Name_, DiscoModel::Roles::AlbumName);
			item->setData (QString::number (release.Year_), DiscoModel::Roles::AlbumYear);
			item->setData (Util::GetAsBase64Src (icon.toImage ()), DiscoModel::Roles::AlbumImage);

			item->setData (MakeTrackListTooltip (release.TrackInfos_),
					DiscoModel::Roles::AlbumTrackListTooltip);

			Album2Tracks_ << Util::Concat (release.TrackInfos_);

			DiscoModel_->appendRow (item);

			QueryReleaseImage (aaProv, { CurrentArtist_, release.Name_ });
		}
	}

	void BioViewManager::handleAlbumPreviewRequested (int index)
	{
		QList<QPair<QString, int>> tracks;
		for (const auto& track : Album2Tracks_.at (index))
			tracks.push_back ({ track.Name_, track.Length_ });

		const auto& album = DiscoModel_->item (index)->data (DiscoModel::Roles::AlbumName).toString ();

		auto ph = Core::Instance ().GetPreviewHandler ();
		ph->previewAlbum (CurrentArtist_, album, tracks);
	}
}
}
