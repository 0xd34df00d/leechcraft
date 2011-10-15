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
#include <QtXml/QDomDocument>

namespace LeechCraft
{
	namespace Potorchu
	{
		const char *api_key = "be076efd1c241366f27fde6fd024e567";
		
		LastFMSubmitter::LastFMSubmitter (QObject* parent)
		: QObject (parent)
		{
			Manager_ = NULL;
		}
		
		void LastFMSubmitter::NowPlaying (libvlc_media_t *m)
		{
			const QUrl& apiUrl = QUrl (QString ("http://ws.audioscrobbler.com/2.0/?method=auth.getToken&api_key=")
					+ api_key);
	
			Manager_ = new QNetworkAccessManager (this);
			connect (Manager_,	
					SIGNAL (finished (QNetworkReply*)),
					this,
					SLOT (getToken (QNetworkReply*)));
			Manager_->get (QNetworkRequest (apiUrl));
		}
		
		void LastFMSubmitter::getToken (QNetworkReply *reply)
		{
			QDomDocument doc;
			doc.setContent (QString::fromUtf8 (reply->readAll ()));
			QDomElement root = doc.documentElement ();
			QDomNode node = root.firstChild ().firstChildElement ();
			qDebug () << node.nodeValue ();
			
		}
	}
}