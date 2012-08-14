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
#include <QStandardItemModel>
#include <QByteArray>
#include <interfaces/core/icoreproxy.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include "lastfmsubmitter.h"
#include "xmlsettingsmanager.h"
#include "pendingsimilarartists.h"
#include "albumartfetcher.h"
#include "authenticator.h"
#include "pendingrecommendedartists.h"
#include "radiostation.h"
#include "recentreleasesfetcher.h"
#include "pendingartistbio.h"

namespace LeechCraft
{
namespace Lastfmscrobble
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("lastfmscrobble");

		Proxy_ = proxy;
		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"lastfmscrobblesettings.xml");

		Auth_ = new Authenticator (proxy->GetNetworkAccessManager (), this);
		connect (Auth_,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
		connect (Auth_,
				SIGNAL (delegateEntity (LeechCraft::Entity, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (LeechCraft::Entity, int*, QObject**)));

		LFSubmitter_ = new LastFMSubmitter (this);
		LFSubmitter_->Init (Proxy_->GetNetworkAccessManager ());

		connect (Auth_,
				SIGNAL (authenticated ()),
				LFSubmitter_,
				SLOT (handleAuthenticated ()));

		RadioRoot_ = new QStandardItem ("Last.FM");
		RadioRoot_->setEditable (false);
		auto addPredefined = [this] (const QString& name, const QString& id)
		{
			auto item = new QStandardItem (name);
			item->setData (Media::RadioType::Predefined, Media::RadioItemRole::ItemType);
			item->setData (id, Media::RadioItemRole::RadioID);
			item->setEditable (false);
			RadioRoot_->appendRow (item);
			return item;
		};
		addPredefined (tr ("Library"), "library");
		addPredefined (tr ("Recommendations"), "recommendations");
		addPredefined (tr ("Loved"), "loved");
		addPredefined (tr ("Neighbourhood"), "neighbourhood");

		auto similarItem = addPredefined (tr ("Similar artists"), QString ());
		similarItem->setData (Media::RadioType::SimilarArtists, Media::RadioItemRole::ItemType);
		auto globalItem = addPredefined (tr ("Global tag"), QString ());
		globalItem->setData (Media::RadioType::GlobalTag, Media::RadioItemRole::ItemType);;
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
		return tr ("Submits information about tracks you've listened to Last.FM.");
	}

	void Plugin::Release ()
	{
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon (":/resources/images/lastfmscrobble.svg");
		return icon;
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

	Media::IPendingSimilarArtists* Plugin::GetSimilarArtists (const QString& name, int num)
	{
		return new PendingSimilarArtists (name, num, Proxy_->GetNetworkAccessManager (), this);
	}

	Media::IPendingSimilarArtists* Plugin::RequestRecommended (int num)
	{
		return new PendingRecommendedArtists (Auth_,
				Proxy_->GetNetworkAccessManager (), num, this);
	}

	Media::IRadioStation_ptr Plugin::GetRadioStation (QStandardItem *item, const QString& name)
	{
		try
		{
			auto type = item->data (Media::RadioItemRole::ItemType).toInt ();
			const auto& param = type == Media::RadioType::Predefined ?
					item->data (Media::RadioItemRole::RadioID).toString () :
					name;

			auto nam = Proxy_->GetNetworkAccessManager ();
			return Media::IRadioStation_ptr (new RadioStation (nam,
						static_cast<Media::RadioType> (type),
						param,
						item->text ()));
		}
		catch (const RadioStation::UnsupportedType&)
		{
			return Media::IRadioStation_ptr ();
		}
	}

	QList<QStandardItem*> Plugin::GetRadioListItems () const
	{
		return QList<QStandardItem*> () << RadioRoot_;
	}

	void Plugin::RequestRecentReleases (int num, bool withRecs)
	{
		auto nam = Proxy_->GetNetworkAccessManager ();
		connect (new RecentReleasesFetcher (withRecs, num, nam, this),
				SIGNAL (gotRecentReleases (QList<Media::AlbumRelease>)),
				this,
				SIGNAL (gotRecentReleases (QList<Media::AlbumRelease>)));
	}

	Media::IPendingArtistBio* Plugin::RequestArtistBio (const QString& artist)
	{
		return new PendingArtistBio (artist, Proxy_->GetNetworkAccessManager (), this);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_lastfmscrobble, LeechCraft::Lastfmscrobble::Plugin);
