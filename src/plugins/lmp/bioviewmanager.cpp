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

#include "bioviewmanager.h"
#include <numeric>
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QGraphicsObject>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QStandardItemModel>
#include <util/util.h>
#include <util/qml/colorthemeproxy.h>
#include <interfaces/media/idiscographyprovider.h>
#include <interfaces/media/ialbumartprovider.h>
#include <interfaces/core/ipluginsmanager.h>
#include "biopropproxy.h"
#include "core.h"
#include "util.h"
#include "previewhandler.h"

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		class DiscoModel : public QStandardItemModel
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
			: QStandardItemModel (parent)
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

	BioViewManager::BioViewManager (QDeclarativeView *view, QObject *parent)
	: QObject (parent)
	, View_ (view)
	, BioPropProxy_ (new BioPropProxy (this))
	, DiscoModel_ (new DiscoModel (this))
	{
		View_->rootContext ()->setContextObject (BioPropProxy_);
		View_->rootContext ()->setContextProperty ("artistDiscoModel", DiscoModel_);
		View_->rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (Core::Instance ().GetProxy ()->GetColorThemeManager (), this));
	}

	void BioViewManager::InitWithSource ()
	{
		connect (View_->rootObject (),
				SIGNAL (linkActivated (QString)),
				this,
				SLOT (handleLink (QString)));
		connect (View_->rootObject (),
				SIGNAL (albumPreviewRequested (int)),
				this,
				SLOT (handleAlbumPreviewRequested (int)));
	}

	void BioViewManager::Request (Media::IArtistBioFetcher *fetcher, const QString& artist)
	{
		DiscoModel_->clear ();

		CurrentArtist_ = artist;

		auto pending = fetcher->RequestArtistBio (CurrentArtist_);
		connect (pending->GetObject (),
				SIGNAL (ready ()),
				this,
				SLOT (handleBioReady ()));

		auto pm = Core::Instance ().GetProxy ()->GetPluginsManager ();
		for (auto prov : pm->GetAllCastableTo<Media::IDiscographyProvider*> ())
		{
			auto fetcher = prov->GetDiscography (CurrentArtist_);
			connect (fetcher->GetObject (),
					SIGNAL (ready ()),
					this,
					SLOT (handleDiscographyReady ()));
		}
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

	void BioViewManager::SetAlbumImage (const QString& album, const QImage& img)
	{
		auto item = FindAlbumItem (album);
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown item for"
					<< album;
			return;
		}

		item->setData (Util::GetAsBase64Src (img), DiscoModel::Roles::AlbumImage);
	}

	void BioViewManager::handleBioReady ()
	{
		auto pending = qobject_cast<Media::IPendingArtistBio*> (sender ());
		const auto& bio = pending->GetArtistBio ();
		BioPropProxy_->SetBio (bio);

		emit gotArtistImage (bio.BasicInfo_.Name_, bio.BasicInfo_.LargeImage_);
	}

	void BioViewManager::handleDiscographyReady ()
	{
		auto pm = Core::Instance ().GetProxy ()->GetPluginsManager ();
		auto aaProvObj = pm->GetAllCastableRoots<Media::IAlbumArtProvider*> ().value (0);
		auto aaProv = qobject_cast<Media::IAlbumArtProvider*> (aaProvObj);
		if (aaProvObj)
			connect (aaProvObj,
					SIGNAL (gotAlbumArt (Media::AlbumInfo, QList<QImage>)),
					this,
					SLOT (handleAlbumArt (Media::AlbumInfo, QList<QImage>)),
					Qt::UniqueConnection);

		auto fetcher = qobject_cast<Media::IPendingDisco*> (sender ());
		const auto& icon = Core::Instance ().GetProxy ()->GetIcon ("media-optical").pixmap (AASize * 2, AASize * 2);
		for (const auto& release : fetcher->GetReleases ())
		{
			if (FindAlbumItem (release.Name_))
				continue;

			auto item = new QStandardItem;
			item->setData (release.Name_, DiscoModel::Roles::AlbumName);
			item->setData (QString::number (release.Year_), DiscoModel::Roles::AlbumYear);
			item->setData (Util::GetAsBase64Src (icon.toImage ()), DiscoModel::Roles::AlbumImage);

			item->setData (MakeTrackListTooltip (release.TrackInfos_),
					DiscoModel::Roles::AlbumTrackListTooltip);

			auto tracks = std::accumulate (release.TrackInfos_.begin (), release.TrackInfos_.end (),
					decltype (release.TrackInfos_.value (0)) ());
			Album2Tracks_ << tracks;

			DiscoModel_->appendRow (item);

			aaProv->RequestAlbumArt ({ CurrentArtist_, release.Name_ });
		}
	}

	namespace
	{
		struct ScaleResult
		{
			QImage Image_;
			QString Album_;
		};
	}

	void BioViewManager::handleAlbumArt (const Media::AlbumInfo& info, const QList<QImage>& images)
	{
		if (info.Artist_ != CurrentArtist_ || images.isEmpty ())
			return;

		auto img = images.first ();
		if (img.width () <= AASize)
		{
			SetAlbumImage (info.Album_, img);
			return;
		}

		auto watcher = new QFutureWatcher<ScaleResult> ();
		connect (watcher,
				SIGNAL (finished ()),
				this,
				SLOT (handleImageScaled ()));

		watcher->setFuture (QtConcurrent::run ([img, info] () -> ScaleResult
				{ return { img.scaled (AASize, AASize, Qt::KeepAspectRatio, Qt::SmoothTransformation), info.Album_ }; }));
	}

	void BioViewManager::handleImageScaled ()
	{
		auto watcher = dynamic_cast<QFutureWatcher<ScaleResult>*> (sender ());
		watcher->deleteLater ();

		const auto& result = watcher->result ();
		SetAlbumImage (result.Album_, result.Image_);
	}

	void BioViewManager::handleAlbumPreviewRequested (int index)
	{
		qDebug () << Q_FUNC_INFO;

		QList<QPair<QString, int>> tracks;
		for (const auto& track : Album2Tracks_.at (index))
			tracks.push_back ({ track.Name_, track.Length_ });

		const auto& album = DiscoModel_->item (index)->data (DiscoModel::Roles::AlbumName).toString ();

		auto ph = Core::Instance ().GetPreviewHandler ();
		ph->previewAlbum (CurrentArtist_, album, tracks);
	}

	void BioViewManager::handleLink (const QString& link)
	{
		Core::Instance ().SendEntity (Util::MakeEntity (QUrl (link),
					QString (),
					FromUserInitiated | OnlyHandle));
	}
}
}
