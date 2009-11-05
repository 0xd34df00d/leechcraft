/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "lyricwikisearcher.h"
#include <QHttp>
#include <QtDebug>
#include <QCryptographicHash>
#include <QUrl>
#include "core.h"
#include "lyricscache.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DeadLyrics
		{
			LyricWikiSearcher::LyricWikiSearcher ()
			{
				setObjectName ("lyricwiki");
			}
			
			void LyricWikiSearcher::Start (const QStringList& asa, QByteArray& hash)
			{
				hash = QCryptographicHash::hash (asa.join ("").toUtf8 (),
						QCryptographicHash::Sha1);
				try
				{
					Lyrics result = LyricsCache::Instance ().GetLyrics (hash);
					emit textFetched (result, hash);
					return;
				}
				catch (...)
				{
				}
			
				QHttp *http = new QHttp ("www.lyricsplugin.com", 80, this);
				QUrl url ("http://www.lyricsplugin.com/winamp03/plugin/");
				url.addQueryItem ("artist", asa.at (0));
				url.addQueryItem ("title", asa.at (1));
				http->get (url.toEncoded ());
			
				connect (http,
						SIGNAL (done (bool)),
						this,
						SLOT (handleFinished ()));
			
				http->setObjectName (hash);
				http->setProperty ("IDHash", hash);
				http->setProperty ("Artist", asa.at (0));
				http->setProperty ("Title", asa.at (1));
				http->setProperty ("URL", url);
			}
			
			void LyricWikiSearcher::Stop (const QByteArray& hash)
			{
				qDeleteAll (findChildren<QHttp*> (hash));
			}
			
			void LyricWikiSearcher::handleFinished ()
			{
				QHttp *http = qobject_cast<QHttp*> (sender ());
				QByteArray response = http->readAll ();
				http->deleteLater ();
			
				QByteArray hash = http->property ("IDHash").toByteArray ();
			
				Lyrics result =
				{
					http->property ("Artist").toString (),
					"",
					http->property ("Title").toString (),
					QString::fromUtf8 (response),
					http->property ("URL").value<QUrl> ().toString ()
				};
			
				LyricsCache::Instance ().SetLyrics (hash, result);
			
				emit textFetched (result, hash);
			}
		};
	};
};

