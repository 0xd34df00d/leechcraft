/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "lastfmsubmitter.h"
#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <lastfm/Track>
#include <lastfm.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Laure
{
	namespace
	{
		QString AuthToken (const QString& username, const QString& password)
		{
			const QString& passHash = QCryptographicHash::hash (password.toAscii (),
					QCryptographicHash::Md5).toHex ();
			return QCryptographicHash::hash ((username + passHash).toAscii (),
					QCryptographicHash::Md5).toHex ();
		}
		
		QString ApiSig (const QString& api_key, const QString& authToken,
				const QString& method, const QString& username,
				const QString& secret)
		{
			const QString& str = QString ("api_key%1authToken%2method%3username%4%5")
					.arg (api_key)
					.arg (authToken)
					.arg (method)
					.arg (username)
					.arg (secret);
			return QCryptographicHash::hash (str.toAscii (),
					QCryptographicHash::Md5).toHex ();
		}
	};

	bool LastFMSubmitter::IsConnected () const
	{
		return Scrobbler_ != NULL;
	}

	LastFMSubmitter::LastFMSubmitter (ICoreProxy_ptr proxy, QObject* parent)
	: QObject (parent)
	{
		lastfm::ws::ApiKey = "be076efd1c241366f27fde6fd024e567";
		lastfm::ws::SharedSecret = "8352aead3be59ab319cd4e578d374843";
		lastfm::ws::Username = XmlSettingsManager::Instance ()
				.property ("lastfm.login").toString ();
				
		Manager_ = proxy.get ()->GetNetworkAccessManager ();
			
		connect (Manager_,
				SIGNAL ( finished (QNetworkReply*)),
				this,
				SLOT (getSessionKey (QNetworkReply*)));
		
		const QString& password = XmlSettingsManager::Instance ()
				.property ("lastfm.password").toString ();
		const QString& authToken = AuthToken (lastfm::ws::Username,
				password);
		
		const QString& api_sig = ApiSig (lastfm::ws::ApiKey, authToken,
				"auth.getMobileSession", lastfm::ws::Username,
				lastfm::ws::SharedSecret);
		const QString& scrobblingSite =  XmlSettingsManager::Instance ()
				.property ("lastfm.url").toString ();
		const QString& url = QString ("%1?method=%2&username=%3&authToken=%4&api_key=%5&api_sig=%6")
				.arg (scrobblingSite)
				.arg ("auth.getMobileSession")
				.arg (lastfm::ws::Username)
				.arg (authToken)
				.arg (lastfm::ws::ApiKey)
				.arg (api_sig);

		Manager_->get (QNetworkRequest (QUrl (url)));
	}
		
	void LastFMSubmitter::getSessionKey (QNetworkReply *result)
	{
		QDomDocument doc;
		doc.setContent (QString::fromUtf8 (result->readAll ()));
		QDomNodeList domList = doc.documentElement ()
				.elementsByTagName ("key");
		if (domList.size () > 0)
		{
			lastfm::ws::SessionKey = doc.documentElement ()
					.elementsByTagName ("key").at (0).toElement ()
					.text ();
			
			Scrobbler_.reset (new lastfm::Audioscrobbler ("tst"));
		
			connect (Scrobbler_.get (),
					SIGNAL (status (int)),
					this,
					SLOT (status (int)));
		}
	}
		
	void LastFMSubmitter::status (int code)
	{
		// code
	}
		
	void LastFMSubmitter::NowPlaying (libvlc_media_t *m)
	{
		if (!m || !IsConnected ())
			return;
		
		lastfm::Track track;
		lastfm::MutableTrack mutableTrack (track);
		libvlc_media_parse (m);
		mutableTrack.setTitle (libvlc_media_get_meta (m, libvlc_meta_Title));
		mutableTrack.setAlbum (libvlc_media_get_meta (m, libvlc_meta_Album));
		mutableTrack.setArtist (libvlc_media_get_meta (m, libvlc_meta_Artist));

		Scrobbler_->nowPlaying (track);
	}
}
}