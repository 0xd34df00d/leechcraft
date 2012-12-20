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
#include <util/qml/colorthemeproxy.h>
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
	, NewArtistsModel_ (new SimilarModel (this))
	, TopArtistsModel_ (new SimilarModel (this))
	, NewTracksModel_ (new TracksModel (this))
	, TopTracksModel_ (new TracksModel (this))
	{
		Ui_.setupUi (this);

		Ui_.HypesView_->engine ()->addImageProvider ("sysIcons",
				new SysIconProvider (Core::Instance ().GetProxy ()));

		auto root = Ui_.HypesView_->rootContext ();
		root->setContextProperty ("newArtistsModel", NewArtistsModel_);
		root->setContextProperty ("newTracksModel", NewTracksModel_);
		root->setContextProperty ("topArtistsModel", TopArtistsModel_);
		root->setContextProperty ("topTracksModel", TopTracksModel_);
		root->setContextProperty ("artistsLabelText", tr ("Hyped artists"));
		root->setContextProperty ("tracksLabelText", tr ("Hyped tracks"));
		root->setContextProperty ("newsText", tr ("Show novelties"));
		root->setContextProperty ("topsText", tr ("Show tops"));
		root->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (Core::Instance ().GetProxy ()->GetColorThemeManager (), this));
		root->setContextProperty ("similarLabelPosition", "underArtist");
		Ui_.HypesView_->setSource (QUrl ("qrc:/lmp/resources/qml/HypesView.qml"));

		connect (Ui_.InfoProvider_,
				SIGNAL (activated (int)),
				this,
				SLOT (request ()));

		connect (Ui_.HypesView_->rootObject (),
				SIGNAL (linkActivated (QString)),
				this,
				SLOT (handleLink (QString)));
		connect (Ui_.HypesView_->rootObject (),
				SIGNAL (artistPreviewRequested (QString)),
				this,
				SIGNAL (artistPreviewRequested (QString)));
		connect (Ui_.HypesView_->rootObject (),
				SIGNAL (trackPreviewRequested (QString, QString)),
				this,
				SIGNAL (trackPreviewRequested (QString, QString)));
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
		NewArtistsModel_->clear ();
		TopArtistsModel_->clear ();
		NewTracksModel_->clear ();
		TopTracksModel_->clear ();

		const auto idx = Ui_.InfoProvider_->currentIndex ();
		if (idx < 0)
			return;

		for (auto prov : Providers_)
			disconnect (prov,
					0,
					this,
					0);

		auto provObj = Providers_.at (idx);
		auto prov = qobject_cast<Media::IHypesProvider*> (provObj);
		auto tryHype = [this, prov, provObj] (Media::IHypesProvider::HypeType type, const char *signal, const char *slot) -> void
		{
			if (!prov->SupportsHype (type))
				return;

			connect (provObj,
					signal,
					this,
					slot,
					Qt::UniqueConnection);
			prov->RequestHype (type);
		};
		tryHype (Media::IHypesProvider::HypeType::NewArtists,
				SIGNAL (gotHypedArtists (QList<Media::HypedArtistInfo>, Media::IHypesProvider::HypeType)),
				SLOT (handleArtists (QList<Media::HypedArtistInfo>, Media::IHypesProvider::HypeType)));
		tryHype (Media::IHypesProvider::HypeType::TopArtists,
				SIGNAL (gotHypedArtists (QList<Media::HypedArtistInfo>, Media::IHypesProvider::HypeType)),
				SLOT (handleArtists (QList<Media::HypedArtistInfo>, Media::IHypesProvider::HypeType)));
		tryHype (Media::IHypesProvider::HypeType::NewTracks,
				SIGNAL (gotHypedTracks (QList<Media::HypedTrackInfo>, Media::IHypesProvider::HypeType)),
				SLOT (handleTracks (QList<Media::HypedTrackInfo>, Media::IHypesProvider::HypeType)));
		tryHype (Media::IHypesProvider::HypeType::TopTracks,
				SIGNAL (gotHypedTracks (QList<Media::HypedTrackInfo>, Media::IHypesProvider::HypeType)),
				SLOT (handleTracks (QList<Media::HypedTrackInfo>, Media::IHypesProvider::HypeType)));

		XmlSettingsManager::Instance ()
				.setProperty ("LastUsedReleasesProvider", prov->GetServiceName ());
	}

	namespace
	{
		template<typename T>
		QStringList GetStats (const T& info)
		{
			QStringList stats;
			if (info.PercentageChange_)
				stats << HypesWidget::tr ("Growth: x%1", "better use unicode multiplication sign here instead of 'x'")
						.arg (info.PercentageChange_ / 100., 0, 'f', 2);
			if (info.Listeners_)
				stats << HypesWidget::tr ("%n listener(s)", 0, info.Listeners_);
			if (info.Playcount_)
				stats << HypesWidget::tr ("%n playback(s)", 0, info.Playcount_);
			return stats;
		}
	}

	void HypesWidget::handleArtists (const QList<Media::HypedArtistInfo>& infos, Media::IHypesProvider::HypeType type)
	{
		auto model = type == Media::IHypesProvider::HypeType::NewArtists ?
				NewArtistsModel_ :
				TopArtistsModel_;

		for (const auto& info : infos)
		{
			auto artist = info.Info_;

			if (artist.ShortDesc_.isEmpty ())
				artist.ShortDesc_ = tr ("%1 is not <em>that</em> mainstream to have a description.")
						.arg (artist.Name_);

			auto item = SimilarModel::ConstructItem (artist);
			item->setData (GetStats (info).join ("; "), SimilarModel::Role::Similarity);
			model->appendRow (item);
		}
	}

	void HypesWidget::handleTracks (const QList<Media::HypedTrackInfo>& infos, Media::IHypesProvider::HypeType type)
	{
		auto model = type == Media::IHypesProvider::HypeType::NewTracks ?
				NewTracksModel_ :
				TopTracksModel_;

		for (const auto& info : infos)
		{
			auto item = new QStandardItem;
			item->setData (info.TrackName_, TracksModel::Role::TrackName);
			item->setData (info.TrackPage_, TracksModel::Role::TrackURL);
			item->setData (info.ArtistName_, TracksModel::Role::ArtistName);
			item->setData (info.ArtistPage_, TracksModel::Role::ArtistURL);
			item->setData (info.Image_, TracksModel::Role::ThumbImageURL);
			item->setData (info.LargeImage_, TracksModel::Role::FullImageURL);
			item->setData (GetStats (info).join ("; "), TracksModel::Role::PercentageChange);

			model->appendRow (item);
		}
	}

	void HypesWidget::handleLink (const QString& link)
	{
		Core::Instance ().SendEntity (Util::MakeEntity (QUrl (link),
					QString (),
					FromUserInitiated | OnlyHandle));
	}
}
}
