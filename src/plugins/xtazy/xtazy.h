/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2013  Georg Rudoy
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

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/media/iaudioscrobbler.h>
#include <interfaces/media/icurrentsongkeeper.h>

namespace LeechCraft
{
namespace Xtazy
{
	class TuneSourceBase;
	class LCSource;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveSettings
				 , public Media::IAudioScrobbler
				 , public Media::ICurrentSongKeeper
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings Media::IAudioScrobbler Media::ICurrentSongKeeper)

		Util::XmlSettingsDialog_ptr SettingsDialog_;
		QList<TuneSourceBase*> TuneSources_;

		LCSource *LCSource_;

		Media::AudioInfo Previous_;

		typedef QPair<QPointer<QObject>, QString> UploadNotifee_t;
		QMap<QString, QList<UploadNotifee_t>> PendingUploads_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QString GetServiceName () const;
		void NowPlaying (const Media::AudioInfo& audio);
		void PlaybackStopped ();
		void LoveCurrentTrack ();
		void BanCurrentTrack ();

		Media::AudioInfo GetCurrentSong () const;
	private slots:
		void publish (const Media::AudioInfo&);
	signals:
		void currentSongChanged (const Media::AudioInfo&);
	};
}
}
