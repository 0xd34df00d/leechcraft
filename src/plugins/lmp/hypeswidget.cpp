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

#include "hypeswidget.h"
#include <QStandardItemModel>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QGraphicsObject>
#include <util/util.h>
#include <interfaces/media/ihypesprovider.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "util.h"
#include "xmlsettingsmanager.h"
#include "core.h"
#include "similarmodel.h"
#include "sysiconsprovider.h"

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		class TracksModel : public QStandardItemModel
		{
		public:
			enum Role
			{
				TrackName = Qt::UserRole + 1,
				TrackURL,
				ArtistName,
				ArtistURL,
				ThumbImageURL,
				FullImageURL,
				PercentageChange
			};

			TracksModel (QObject *parent)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> names;
				names [TrackName] = "trackName";
				names [TrackURL] = "trackURL";
				names [ArtistName] = "artistName";
				names [ArtistURL] = "artistURL";
				names [ThumbImageURL] = "thumbImageURL";
				names [FullImageURL] = "fullURL";
				names [PercentageChange] = "change";

				setRoleNames (names);
			}
		};
	}

	HypesWidget::HypesWidget (QWidget *parent)
	: QWidget (parent)
	, ArtistsModel_ (new SimilarModel (this))
	, TracksModel_ (new TracksModel (this))
	{
		Ui_.setupUi (this);

		Ui_.HypesView_->engine ()->addImageProvider ("sysIcons",
				new SysIconProvider (Core::Instance ().GetProxy ()));

		auto root = Ui_.HypesView_->rootContext ();
		root->setContextProperty ("artistsModel", ArtistsModel_);
		root->setContextProperty ("tracksModel", TracksModel_);
		root->setContextProperty ("artistsLabelText", tr ("Hyped artists"));
		root->setContextProperty ("tracksLabelText", tr ("Hyped tracks"));
		Ui_.HypesView_->setSource (QUrl ("qrc:/lmp/resources/qml/HypesView.qml"));

		connect (Ui_.InfoProvider_,
				SIGNAL (activated (int)),
				this,
				SLOT (request ()));

		connect (Ui_.HypesView_->rootObject (),
				SIGNAL (linkActivated (QString)),
				this,
				SLOT (handleLink (QString)));
	}

	void HypesWidget::InitializeProviders ()
	{
		const auto& lastProv = ShouldRememberProvs () ?
				XmlSettingsManager::Instance ()
					.Property ("LastUsedHypesProvider", QString ()).toString () :
				QString ();

		bool lastFound = false;

		Providers_ = Core::Instance ().GetProxy ()->GetPluginsManager ()->
				GetAllCastableRoots<Media::IHypesProvider*> ();
		for (auto provObj : Providers_)
		{
			auto prov = qobject_cast<Media::IHypesProvider*> (provObj);

			Ui_.InfoProvider_->addItem (prov->GetServiceName ());
			if (prov->GetServiceName () == lastProv)
			{
				const int idx = Providers_.size () - 1;
				Ui_.InfoProvider_->setCurrentIndex (idx);
				request ();
				lastFound = true;
			}
		}

		if (!lastFound)
			Ui_.InfoProvider_->setCurrentIndex (-1);
	}

	void HypesWidget::request ()
	{
		ArtistsModel_->clear ();
		TracksModel_->clear ();

		const auto idx = Ui_.InfoProvider_->currentIndex ();
		if (idx < 0)
			return;

		for (auto prov : Providers_)
			disconnect (dynamic_cast<QObject*> (prov),
					0,
					this,
					0);

		auto provObj = Providers_.at (idx);
		auto prov = qobject_cast<Media::IHypesProvider*> (provObj);
		if (prov->SupportsHype (Media::IHypesProvider::HypeType::NewArtist))
		{
			connect (provObj,
					SIGNAL (gotHypedArtists (QList<Media::HypedArtistInfo>, Media::IHypesProvider::HypeType)),
					this,
					SLOT (handleArtists (QList<Media::HypedArtistInfo>, Media::IHypesProvider::HypeType)));
			prov->RequestHype (Media::IHypesProvider::HypeType::NewArtist);
		}
		if (prov->SupportsHype (Media::IHypesProvider::HypeType::NewTrack))
		{
			connect (provObj,
					SIGNAL (gotHypedTracks(QList<Media::HypedTrackInfo>, Media::IHypesProvider::HypeType)),
					this,
					SLOT (handleTracks (QList<Media::HypedTrackInfo>, Media::IHypesProvider::HypeType)));
			prov->RequestHype (Media::IHypesProvider::HypeType::NewTrack);
		}

		XmlSettingsManager::Instance ()
				.setProperty ("LastUsedReleasesProvider", prov->GetServiceName ());
	}

	void HypesWidget::handleArtists (const QList<Media::HypedArtistInfo>& infos, Media::IHypesProvider::HypeType type)
	{
		if (type != Media::IHypesProvider::HypeType::NewArtist)
			return;

		for (const auto& info : infos)
		{
			auto artist = info.Info_;

			if (artist.ShortDesc_.isEmpty ())
				artist.ShortDesc_ = tr ("%1 is not <em>that</em> mainstream to have a description.")
						.arg (artist.Name_);

			auto item = SimilarModel::ConstructItem (artist);

			const auto& perc = tr ("Growth: x%1", "better use unicode multiplication sign here instead of 'x'")
					.arg (info.PercentageChange_ / 100.0, 0, 'f', 2);
			item->setData (perc, SimilarModel::Role::Similarity);

			ArtistsModel_->appendRow (item);
		}

		disconnect (sender (),
				SIGNAL (gotHypedArtists (QList<Media::HypedArtistInfo>)),
				this,
				SLOT (handleArtists (QList<Media::HypedArtistInfo>)));
	}

	void HypesWidget::handleTracks (const QList<Media::HypedTrackInfo>& infos, Media::IHypesProvider::HypeType type)
	{
		if (type != Media::IHypesProvider::HypeType::NewTrack)
			return;

		for (const auto& info : infos)
		{
			auto item = new QStandardItem;
			item->setData (info.TrackName_, TracksModel::Role::TrackName);
			item->setData (info.TrackPage_, TracksModel::Role::TrackURL);
			item->setData (info.ArtistName_, TracksModel::Role::ArtistName);
			item->setData (info.ArtistPage_, TracksModel::Role::ArtistURL);
			item->setData (info.Image_, TracksModel::Role::ThumbImageURL);
			item->setData (info.LargeImage_, TracksModel::Role::FullImageURL);

			const auto& perc = tr ("Growth: x%1", "better use unicode multiplication sign here instead of 'x'")
					.arg (info.PercentageChange_ / 100., 0, 'f', 2);
			item->setData (perc, TracksModel::Role::PercentageChange);

			TracksModel_->appendRow (item);
		}

		disconnect (sender (),
				SIGNAL (gotHypedTracks(QList<Media::HypedTrackInfo>)),
				this,
				SLOT (handleTracks (QList<Media::HypedTrackInfo>)));
	}

	void HypesWidget::handleLink (const QString& link)
	{
		Core::Instance ().SendEntity (Util::MakeEntity (QUrl (link),
					QString (),
					FromUserInitiated | OnlyHandle));
	}
}
}
