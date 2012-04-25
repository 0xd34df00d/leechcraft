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

#pragma once

#include <interfaces/iinfo.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/media/iaudioscrobbler.h>

namespace LeechCraft
{
namespace Lastfmscrobble
{
	class LastFMSubmitter;

	class Plugin : public QObject
				, public IInfo
				, public IEntityHandler
				, public IHaveSettings
				, public Media::IAudioScrobbler
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IEntityHandler
				IHaveSettings
				Media::IAudioScrobbler)

		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;
		LastFMSubmitter *LFSubmitter_;
		ICoreProxy_ptr Proxy_;
	public:
		void Init (ICoreProxy_ptr proxy);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		void Release ();
		QIcon GetIcon () const;

		EntityTestHandleResult CouldHandle (const Entity& entity) const;
		void Handle (Entity entity);

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QString GetServiceName () const;
		void NowPlaying (const Media::AudioInfo&);
		void PlaybackStopped ();
	private slots:
		void handleSubmitterInit ();
	signals:
		void gotEntity (const Entity&);
		void delegateEntity (const Entity&, int*, QObject**);
	};
}
}
