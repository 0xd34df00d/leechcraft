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

namespace LC
{
namespace Scroblibre
{
	class AccountsManager;
	class AuthManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveSettings
				 , public Media::IAudioScrobbler
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings Media::IAudioScrobbler)

		LC_PLUGIN_METADATA ("org.LeechCraft.Scroblibre")

		Util::XmlSettingsDialog_ptr XSD_;
		AccountsManager *AccMgr_;
		AuthManager *AuthMgr_;
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
		void NowPlaying (const Media::AudioInfo&);
		void SendBackdated (const BackdatedTracks_t&);
		void PlaybackStopped ();
		void LoveCurrentTrack ();
		void BanCurrentTrack ();
	};
}
}
