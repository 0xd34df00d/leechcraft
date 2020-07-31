/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/media/iaudioscrobbler.h>
#include <interfaces/media/icurrentsongkeeper.h>

namespace LC
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

		LC_PLUGIN_METADATA ("org.LeechCraft.Xtazy")

		Util::XmlSettingsDialog_ptr SettingsDialog_;
		QList<TuneSourceBase*> TuneSources_;

		LCSource *LCSource_;

		Media::AudioInfo Previous_;

		typedef QPair<QPointer<QObject>, QString> UploadNotifee_t;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		bool SupportsFeature (Feature) const;
		QString GetServiceName () const;
		void NowPlaying (const Media::AudioInfo& audio);
		void SendBackdated (const BackdatedTracks_t&);
		void PlaybackStopped ();
		void LoveCurrentTrack ();
		void BanCurrentTrack ();

		Media::AudioInfo GetCurrentSong () const;
	private slots:
		void publish (const Media::AudioInfo&, TuneSourceBase*);
	signals:
		void currentSongChanged (const Media::AudioInfo&);
	};
}
}
