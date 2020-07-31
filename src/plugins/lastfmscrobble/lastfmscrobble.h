/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/media/iaudioscrobbler.h>
#include <interfaces/media/ialbumartprovider.h>
#include <interfaces/media/isimilarartists.h>
#include <interfaces/media/irecommendedartists.h>
#include <interfaces/media/iradiostationprovider.h>
#include <interfaces/media/irecentreleases.h>
#include <interfaces/media/iartistbiofetcher.h>
#include <interfaces/media/ieventsprovider.h>
#include <interfaces/media/ihypesprovider.h>

class QStandardItem;
class QStandardItemModel;

namespace LC
{
namespace Lastfmscrobble
{
	class Authenticator;
	class LastFMSubmitter;

	class Plugin : public QObject
				, public IInfo
				, public IHaveSettings
				, public Media::IAudioScrobbler
				, public Media::IAlbumArtProvider
				, public Media::ISimilarArtists
				, public Media::IRecommendedArtists
				, public Media::IRadioStationProvider
				, public Media::IRecentReleases
				, public Media::IArtistBioFetcher
				, public Media::IEventsProvider
				, public Media::IHypesProvider
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IHaveSettings
				Media::IAudioScrobbler
				Media::IAlbumArtProvider
				Media::ISimilarArtists
				Media::IRecommendedArtists
				Media::IRadioStationProvider
				Media::IRecentReleases
				Media::IArtistBioFetcher
				Media::IEventsProvider
				Media::IHypesProvider)

		LC_PLUGIN_METADATA ("org.LeechCraft.LastFMScrobble")

		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;

		Authenticator *Auth_;
		LastFMSubmitter *LFSubmitter_;

		ICoreProxy_ptr Proxy_;

		QStandardItemModel *RadioModel_;
		QStandardItem *RadioRoot_;
	public:
		void Init (ICoreProxy_ptr proxy);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		void Release ();
		QIcon GetIcon () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		bool SupportsFeature (Feature) const;
		QString GetServiceName () const;
		void NowPlaying (const Media::AudioInfo&);
		void SendBackdated (const BackdatedTracks_t&);
		void PlaybackStopped ();
		void LoveCurrentTrack ();
		void BanCurrentTrack ();

		QString GetAlbumArtProviderName () const;
		QFuture<Media::IAlbumArtProvider::Result_t> RequestAlbumArt (const Media::AlbumInfo& album) const;

		QFuture<Media::SimilarityQueryResult_t> GetSimilarArtists (const QString&, int);

		QFuture<Media::SimilarityQueryResult_t> RequestRecommended (int);

		Media::IRadioStation_ptr GetRadioStation (const QModelIndex&, const QString&);
		QList<QAbstractItemModel*> GetRadioListItems () const;
		void RefreshItems (const QList<QModelIndex>&);

		QFuture<IRecentReleases::Result_t> RequestRecentReleases (int, bool);

		QFuture<IArtistBioFetcher::Result_t> RequestArtistBio (const QString&, bool);

		QFuture<EventsQueryResult_t> UpdateRecommendedEvents ();
		void AttendEvent (qint64, Media::EventAttendType);

		bool SupportsHype (HypeType);
		QFuture<HypeQueryResult_t> RequestHype (HypeType);
	private slots:
		void reloadRecommendedEvents ();
	};
}
}
