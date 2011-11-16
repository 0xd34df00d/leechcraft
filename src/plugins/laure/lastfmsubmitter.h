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
#include "vlcwrapper.h"

namespace lastfm
{
	class Audioscrobbler;
};

namespace LeechCraft
{
namespace Laure
{
	/** @brief The LastFMSubmitter class provides a simple interface for interacting with the last.fm scrobbling service.
	 *  @author Minh Ngo <nlminhtl@gmail.com>
	 */
	class LastFMSubmitter : public QObject
	{
		Q_OBJECT
		
		boost::shared_ptr<lastfm::Audioscrobbler> Scrobbler_;
	public:
		/** @brief Constructs a new LastFMSubmitter with the given
		 * ICoreProxy_ptr and parent.
		 * 
		 * @sa ICoreProxy_ptr
		 */
		LastFMSubmitter (ICoreProxy_ptr proxy, QObject *parent = 0);
		
		/** @brief Returns connection state of the Scrobbler.
		 * 
		 * @return true if it's connected, false otherwise.
		 */
		bool IsConnected () const;
	public slots:
		
		/** @brief Send meta info about the current track to the last.fm service.
		 * 
		 * @sa MediaMeta
		 */
		void sendTrack (const MediaMeta&);
		
		/** @brief Submit the submission cache for the current user.
		 */
		void submit ();
	private slots:
		void status (int);
		void getSessionKey ();
	};
}
}

#endif // PLUGINS_LAURE_LASTFMSUBMITTER_H
