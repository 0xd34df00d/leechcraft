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

#include "lastfmscrobble.h"
#include <QIcon>
#include <QByteArray>
#include <interfaces/core/icoreproxy.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/passutils.h>
#include "lastfmsubmitter.h"
#include "xmlsettingsmanager.h"
#include "pendingsimilarartists.h"
#include "albumartfetcher.h"

namespace LeechCraft
{
namespace Lastfmscrobble
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"lastfmscrobblesettings.xml");

		LFSubmitter_ = new LastFMSubmitter (this);
	}

	void Plugin::SecondInit ()
	{
		XmlSettingsManager::Instance ().RegisterObject ("lastfm.login",
				this, "handleSubmitterInit");
		handleSubmitterInit ();
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
		return tr ("Submits information about tracks you've listened to Last.FM.");
	}

	void Plugin::Release ()
	{
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/resources/images/lastfmscrobble.svg");
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	QString Plugin::GetServiceName () const
	{
		return "Last.FM";
	}

	void Plugin::NowPlaying (const Media::AudioInfo& info)
	{
		LFSubmitter_->NowPlaying (MediaMeta (info));
	}

	void Plugin::PlaybackStopped ()
	{
		LFSubmitter_->Clear ();
	}

	void Plugin::LoveCurrentTrack ()
	{
		LFSubmitter_->Love ();
	}

	void Plugin::RerequestRecommendations ()
	{
	}

	Media::IPendingSimilarArtists* Plugin::GetSimilarArtists (const QString& name, int num)
	{
		return new PendingSimilarArtists (name, num, this);
	}

	QString Plugin::GetAlbumArtProviderName () const
	{
		return GetServiceName ();
	}

	void Plugin::RequestAlbumArt (const Media::AlbumInfo& album) const
	{
		auto fetcher = new AlbumArtFetcher (album, Proxy_);
		connect (fetcher,
				SIGNAL (gotAlbumArt (Media::AlbumInfo, QList<QImage>)),
				this,
				SIGNAL (gotAlbumArt (Media::AlbumInfo, QList<QImage>)));
	}

	void Plugin::FeedPassword (bool authFailure)
	{
		const QString& login = XmlSettingsManager::Instance ()
				.property ("lastfm.login").toString ();
		LFSubmitter_->SetUsername (login);

		QString password;
		if (!login.isEmpty ())
		{
			const auto& text = tr ("Enter password for Last.fm account with login %1:")
						.arg (login);
			password = Util::GetPassword ("org.LeechCraft.Lastfmscrobble/" + login,
					text,
					this,
					!authFailure);
			if (password.isEmpty ())
				return;
		}

		LFSubmitter_->SetPassword (password);
		LFSubmitter_->Init (Proxy_->GetNetworkAccessManager ());
	}

	void Plugin::handleSubmitterInit ()
	{
		connect (LFSubmitter_,
				SIGNAL (authFailure ()),
				this,
				SLOT (handleAuthFailure ()),
				Qt::UniqueConnection);

		FeedPassword (false);
	}

	void Plugin::handleAuthFailure ()
	{
		FeedPassword (true);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_lastfmscrobble, LeechCraft::Lastfmscrobble::Plugin);
