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
#include <QByteArray>
#include <QFuture>
#include <interfaces/core/icoreproxy.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/sll/unreachable.h>
#include <util/threads/coro/task.h>
#include "lastfmsubmitter.h"
#include "xmlsettingsmanager.h"
#include "pendingsimilarartists.h"
#include "albumartfetcher.h"
#include "authenticator.h"
#include "pendingrecommendedartists.h"
#include "pendingartistbio.h"
#include "topartistsfetcher.h"
#include "toptracksfetcher.h"

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

	Util::Channel_ptr<Media::IAlbumArtProvider::AlbumArtResponse> Plugin::RequestAlbumArt (const Media::AlbumInfo& album) const
	{
		return FetchAlbumArt (album);
	}

	QFuture<Media::SimilarityQueryResult_t> Plugin::GetSimilarArtists (const QString& name, int num)
	{
		return (new PendingSimilarArtists (name, num, Proxy_->GetNetworkAccessManager (), this))->GetFuture ();
	}

	QFuture<Media::SimilarityQueryResult_t> Plugin::RequestRecommended (int num)
	{
		return (new PendingRecommendedArtists (Auth_, Proxy_->GetNetworkAccessManager (), num, this))->GetFuture ();
	}

	QFuture<Media::IArtistBioFetcher::Result_t> Plugin::RequestArtistBio (const QString& artist, bool addImages)
	{
		return (new PendingArtistBio (artist, Proxy_->GetNetworkAccessManager (), addImages, this))->GetFuture ();
	}

	QFuture<Plugin::TopArtistsResult_t> Plugin::RequestTopArtists ()
	{
		return (new TopArtistsFetcher (Proxy_->GetNetworkAccessManager (), this))->GetFuture ();
	}

	QFuture<Plugin::TopTracksResult_t> Plugin::RequestTopTracks ()
	{
		return (new TopTracksFetcher (Proxy_->GetNetworkAccessManager (), this))->GetFuture ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_lastfmscrobble, LC::Lastfmscrobble::Plugin);
