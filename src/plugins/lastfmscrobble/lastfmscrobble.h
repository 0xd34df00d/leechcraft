/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
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

namespace LeechCraft
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
				Media::IEventsProvider)

		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;

		Authenticator *Auth_;
		LastFMSubmitter *LFSubmitter_;

		ICoreProxy_ptr Proxy_;

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

		QString GetServiceName () const;
		void NowPlaying (const Media::AudioInfo&);
		void PlaybackStopped ();
		void LoveCurrentTrack ();

		QString GetAlbumArtProviderName () const;
		void RequestAlbumArt (const Media::AlbumInfo& album) const;

		Media::IPendingSimilarArtists* GetSimilarArtists (const QString&, int);

		Media::IPendingSimilarArtists* RequestRecommended (int);

		Media::IRadioStation_ptr GetRadioStation (QStandardItem*, const QString&);
		QList<QStandardItem*> GetRadioListItems () const;

		void RequestRecentReleases (int, bool);

		Media::IPendingArtistBio* RequestArtistBio (const QString&);

		void UpdateRecommendedEvents ();
		void AttendEvent (qint64, Media::EventAttendType);
	signals:
		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);

		void gotAlbumArt (const Media::AlbumInfo&, const QList<QImage>&);

		void gotRecentReleases (const QList<Media::AlbumRelease>&);

		void gotRecommendedEvents (const Media::EventInfos_t&);
	};
}
}
