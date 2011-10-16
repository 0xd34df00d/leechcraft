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

#ifndef LASTFMSUBMITTER_H
#define LASTFMSUBMITTER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <vlc/vlc.h>
#include <lastfm.h>

namespace LeechCraft
{
namespace Laure
{
	class LastFMSubmitter : public QObject
	{
		Q_OBJECT
		lastfm::Audioscrobbler *Scrobbler_;
		QNetworkAccessManager *Manager_;
	public:
		LastFMSubmitter (QObject* parent = 0);
		virtual ~LastFMSubmitter ();
		
		bool IsConnected () const;
		void NowPlaying (libvlc_media_t *m);
	private slots:
		void status (int code);
		void getSessionKey (QNetworkReply *result);
	};
}
}

#endif // LASTFMSUBMITTER_H
