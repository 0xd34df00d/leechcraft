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

#ifndef PLUGINS_LAURE_LASTFMSUBMITTER_H
#define PLUGINS_LAURE_LASTFMSUBMITTER_H
#include <QObject>
#include <boost/shared_ptr.hpp>
#include <vlc/vlc.h>
#include <interfaces/core/icoreproxy.h>

namespace lastfm
{
	class Audioscrobbler;
};

class QNetworkAccessManager;
class QNetworkReply;

namespace LeechCraft
{
namespace Laure
{
	class LastFMSubmitter : public QObject
	{
		Q_OBJECT
		
		boost::shared_ptr<lastfm::Audioscrobbler> Scrobbler_;
		QNetworkAccessManager *Manager_;
	public:
		LastFMSubmitter (ICoreProxy_ptr proxy, QObject *parent = 0);
		
		bool IsConnected () const;
		void NowPlaying (libvlc_media_t*);
	private slots:
		void status (int);
		void getSessionKey (QNetworkReply*);
	};
}
}

#endif // PLUGINS_LAURE_LASTFMSUBMITTER_H
