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
#include <QHttpRequestHeader>
#include <QDomDocument>
#include <QtDebug>
#include <QCryptographicHash>
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
			
				QByteArray data = QByteArray ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
							"<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" "
							"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
							"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
							"xmlns:si=\"http://soapinterop.org/xsd\" "
							"xmlns:tns=\"urn:LyricWiki\" "
							"xmlns:soap=\"http://schemas.xmlsoap.org/wsdl/soap/\" "
							"xmlns:wsdl=\"http://schemas.xmlsoap.org/wsdl/\" "
							"xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" "
							"SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
							"<SOAP-ENV:Body><tns:getSong>"
							"<artist xsi:type=\"xsd:string\">")
					.append (asa.at (0))
					.append ("</artist><song xsi:type=\"xsd:string\">")
					.append (asa.at (1))
					.append ("</song></tns:getSong></SOAP-ENV:Body></SOAP-ENV:Envelope>");
			
				QHttpRequestHeader request ("POST", "/server.php");
				request.setValue ("TE", "deflate,gzip;q=0.3");
				request.setValue ("Connection", "close");
				request.setValue ("Host", "lyricwiki.org");
				request.setValue ("User-Agent", "DeadLyRicS/LeechCraft");
				request.setValue ("Content-Length", QString::number (data.size ()));
				request.setValue ("Content-Type", "text/xml; charset=utf-8");
				request.setValue ("SOAPAction", "\"urn:LyricWiki#getSong\"");
			
				QHttp *http = new QHttp (this);
				http->setHost ("www.lyricwiki.org");
				http->request (request, data);
			
				connect (http,
						SIGNAL (done (bool)),
						this,
						SLOT (handleFinished ()));
			
				http->setObjectName (hash);
				http->setProperty ("IDHash", hash);
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
			
				QDomDocument doc;
				doc.setContent (response, false);
				QDomNodeList lyrics = doc.elementsByTagName ("lyrics");
				QDomNodeList artist = doc.elementsByTagName ("artist");
				QDomNodeList song = doc.elementsByTagName ("song");
				QDomNodeList url = doc.elementsByTagName ("url");
				if (!lyrics.size () ||
						!artist.size () ||
						!song.size () ||
						!url.size ())
				{
					qWarning () << Q_FUNC_INFO << "Lyrics fetch error" << response;
					emit error (tr ("Lyrics fetch error"));
					return;
				}
			
				QString text = lyrics.at (0).toElement ().text ();
				if (text == "Not found")
				{
					qWarning () << Q_FUNC_INFO << "No lyrics found";
					emit error (tr ("No lyrics found"));
					return;
				}
				QByteArray hash = http->property ("IDHash").toByteArray ();
			
				Lyrics result =
				{
					artist.at (0).toElement ().text (),
					"",
					song.at (0).toElement ().text (),
					text,
					url.at (0).toElement ().text ()
				};
			
				LyricsCache::Instance ().SetLyrics (hash, result);
			
				emit textFetched (result, hash);
			}
		};
	};
};

