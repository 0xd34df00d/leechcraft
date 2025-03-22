/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lastfmscrobble.h"
#include <QIcon>
#include <QStandardItemModel>
#include <QByteArray>
#include <QFuture>
#include <interfaces/core/icoreproxy.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/sll/unreachable.h>
#include "lastfmsubmitter.h"
#include "xmlsettingsmanager.h"
#include "pendingsimilarartists.h"
#include "albumartfetcher.h"
#include "authenticator.h"
#include "pendingrecommendedartists.h"
#include "lastfmradiostation.h"
#include "recentreleasesfetcher.h"
#include "pendingartistbio.h"
#include "receventsfetcher.h"
#include "eventsfetchaggregator.h"
#include "eventattendmarker.h"
#include "hypedartistsfetcher.h"
#include "hypedtracksfetcher.h"

namespace LC
{
namespace Lastfmscrobble
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
		XmlSettingsDialog_ = std::make_shared<Util::XmlSettingsDialog> ();
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"lastfmscrobblesettings.xml");

		Auth_ = new Authenticator (proxy->GetNetworkAccessManager (), proxy, this);

		LFSubmitter_ = new LastFMSubmitter (Proxy_->GetNetworkAccessManager (), this);

		connect (Auth_,
				SIGNAL (authenticated ()),
				LFSubmitter_,
				SLOT (handleAuthenticated ()));

		RadioRoot_ = new QStandardItem ("Last.FM");
		RadioRoot_->setEditable (false);
		RadioRoot_->setIcon (QIcon (":/resources/images/lastfm.png"));
		auto addPredefined = [this] (const QString& name, const QString& id, const QIcon& icon) -> QStandardItem*
		{
			auto item = new QStandardItem (name);
			item->setData (Media::RadioType::Predefined, Media::RadioItemRole::ItemType);
			item->setData (id, Media::RadioItemRole::RadioID);
			item->setEditable (false);
			item->setIcon (icon);
			RadioRoot_->appendRow (item);
			return item;
		};
		addPredefined (tr ("Library"), "library", QIcon (":/resources/images/personal.png"));
		addPredefined (tr ("Recommendations"), "recommendations", QIcon (":/resources/images/recs.png"));
		addPredefined (tr ("Loved"), "loved", QIcon (":/resources/images/loved.png"));
		addPredefined (tr ("Neighbourhood"), "neighbourhood", QIcon (":/resources/images/neighbours.png"));

		auto similarItem = addPredefined (tr ("Similar artists"),
				QString (), QIcon (":/resources/images/radio.png"));
		similarItem->setData (Media::RadioType::SimilarArtists, Media::RadioItemRole::ItemType);
		auto globalItem = addPredefined (tr ("Global tag"),
				QString (), QIcon (":/resources/images/tag.png"));
		globalItem->setData (Media::RadioType::GlobalTag, Media::RadioItemRole::ItemType);;

		RadioModel_ = new QStandardItemModel;
		RadioModel_->appendRow (RadioRoot_);
	}

	void Plugin::SecondInit ()
	{
		Auth_->Init ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Lastfmscrobble";
	}

	QString Plugin::GetName () const
	{
		return "Last.FM Scrobbler";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Client for the Last.FM social music service.");
	}

	void Plugin::Release ()
	{
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon ("lcicons:/resources/images/lastfmscrobble.svg");
		return icon;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	bool Plugin::SupportsFeature (Feature feature) const
	{
		switch (feature)
		{
		case Feature::Backdating:
			return true;
		}

		Util::Unreachable ();
	}

	QString Plugin::GetServiceName () const
	{
		return "Last.FM";
	}

	void Plugin::NowPlaying (const Media::AudioInfo& info)
	{
		LFSubmitter_->NowPlaying (info);
	}

	void Plugin::SendBackdated (const BackdatedTracks_t& tracks)
	{
		LFSubmitter_->SendBackdated (tracks);
	}

	void Plugin::PlaybackStopped ()
	{
		LFSubmitter_->Clear ();
	}

	void Plugin::LoveCurrentTrack ()
	{
		LFSubmitter_->Love ();
	}

	void Plugin::BanCurrentTrack ()
	{
		LFSubmitter_->Ban ();
	}

	QString Plugin::GetAlbumArtProviderName () const
	{
		return GetServiceName ();
	}

	QFuture<Media::IAlbumArtProvider::Result_t> Plugin::RequestAlbumArt (const Media::AlbumInfo& album) const
	{
		return (new AlbumArtFetcher (album, Proxy_))->GetFuture ();
	}

	QFuture<Media::SimilarityQueryResult_t> Plugin::GetSimilarArtists (const QString& name, int num)
	{
		return (new PendingSimilarArtists (name, num, Proxy_->GetNetworkAccessManager (), this))->GetFuture ();
	}

	QFuture<Media::SimilarityQueryResult_t> Plugin::RequestRecommended (int num)
	{
		return (new PendingRecommendedArtists (Auth_, Proxy_->GetNetworkAccessManager (), num, this))->GetFuture ();
	}

	Media::IRadioStation_ptr Plugin::GetRadioStation (const QModelIndex& item, const QString& name)
	{
		try
		{
			auto type = item.data (Media::RadioItemRole::ItemType).toInt ();
			const auto& param = type == Media::RadioType::Predefined ?
					item.data (Media::RadioItemRole::RadioID).toString () :
					name;

			auto nam = Proxy_->GetNetworkAccessManager ();
			return std::make_shared<LastFmRadioStation> (nam,
						static_cast<Media::RadioType> (type),
						param,
						item.data (Qt::DisplayRole).toString ());
		}
		catch (const LastFmRadioStation::UnsupportedType&)
		{
			return {};
		}
	}

	QList<QAbstractItemModel*> Plugin::GetRadioListItems () const
	{
		return { RadioModel_ };
	}

	void Plugin::RefreshItems (const QList<QModelIndex>&)
	{
	}

	QFuture<Media::IRecentReleases::Result_t> Plugin::RequestRecentReleases (int, bool withRecs)
	{
		auto nam = Proxy_->GetNetworkAccessManager ();
		return (new RecentReleasesFetcher (withRecs, nam, this))->GetFuture ();
	}

	QFuture<Media::IArtistBioFetcher::Result_t> Plugin::RequestArtistBio (const QString& artist, bool addImages)
	{
		return (new PendingArtistBio (artist, Proxy_->GetNetworkAccessManager (), addImages, this))->GetFuture ();
	}

	QFuture<Plugin::EventsQueryResult_t> Plugin::UpdateRecommendedEvents ()
	{
		auto nam = Proxy_->GetNetworkAccessManager ();

		auto aggregator = new EventsFetchAggregator (this);
		aggregator->AddFetcher (new RecEventsFetcher (Auth_, nam, RecEventsFetcher::Type::Recommended, this));
		aggregator->AddFetcher (new RecEventsFetcher (Auth_, nam, RecEventsFetcher::Type::Attending, this));
		return aggregator->GetFuture ();
	}

	void Plugin::AttendEvent (qint64 id, Media::EventAttendType type)
	{
		auto nam = Proxy_->GetNetworkAccessManager ();
		auto attendMarker = new EventAttendMarker (Auth_, nam, id, type, this);
		connect (attendMarker,
				SIGNAL (finished ()),
				this,
				SLOT (reloadRecommendedEvents ()));
	}

	bool Plugin::SupportsHype (HypeType type)
	{
		switch (type)
		{
		case HypeType::NewArtists:
		case HypeType::NewTracks:
		case HypeType::TopArtists:
		case HypeType::TopTracks:
			return true;
		}

		Util::Unreachable ();
	}

	QFuture<Plugin::HypeQueryResult_t> Plugin::RequestHype (HypeType type)
	{
		auto nam = Proxy_->GetNetworkAccessManager ();

		switch (type)
		{
		case HypeType::NewArtists:
		case HypeType::TopArtists:
			return (new HypedArtistsFetcher (nam, type, this))->GetFuture ();
		case HypeType::NewTracks:
		case HypeType::TopTracks:
			return (new HypedTracksFetcher (nam, type, this))->GetFuture ();
		}

		Util::Unreachable ();
	}

	void Plugin::reloadRecommendedEvents ()
	{
		UpdateRecommendedEvents ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_lastfmscrobble, LC::Lastfmscrobble::Plugin);
