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
	/** @brief The LastFMSubmitter class provides a simple interface for
	 * interacting with the last.fm scrobbling service.
	 * 
	 *  @author Minh Ngo <nlminhtl@gmail.com>
	 */
	class LastFMSubmitter : public QObject
	{
		Q_OBJECT
		
		boost::shared_ptr<lastfm::Audioscrobbler> Scrobbler_;
		QString Password_;
	public:
		/** @brief Constructs a new LastFMSubmitter with the given parent.
		 */
		LastFMSubmitter (QObject *parent = 0);
		
		/** @brief Initializes the submitter.
		 * 
		 * This method is called to generate last.fm session key and
		 * connect to the last.fm service.
		 * 
		 * Don't call it before setting up username and password.
		 * 
		 * @param[in] manager Network access manager.
		 * 
		 * @sa SetUsername()
		 * @sa SetPassword()
		 */
		void Init (QNetworkAccessManager *manager);
		
		/** @brief Sets last.fm username for connecting to the service.
		 * 
		 * @param[in] username Username.
		 * 
		 * @sa SetPassword()
		 */
		void SetUsername (const QString& username);
		
		/** @brief Sets last.fm password for connecting to the service.
		 * 
		 * @param[in] password Password.
		 */
		void SetPassword (const QString& password);
		
		/** @brief Returns connection state of the Scrobbler.
		 * 
		 * @return True if it is connected, false otherwise.
		 * 
		 * @sa SetUsername()
		 */
		bool IsConnected () const;
	public slots:
		
		/** @brief Send meta info about the current track to the last.fm service.
		 * 
		 * @param[in] info Media meta info.
		 */
		void sendTrack (const MediaMeta& info);
		
		/** @brief Submit the submission cache for the current user.
		 */
		void submit ();
	private slots:
		void getSessionKey ();
	signals:
		/** @brief Is emitted to show status in an appropriate manner.
		 * 
		 * See https://github.com/mxcl/liblastfm/blob/master/src/scrobble/Audioscrobbler.h
		 */
		void status (int code);
	};
}
}

#endif // PLUGINS_LAURE_LASTFMSUBMITTER_H
