/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QTimer;

namespace libtorrent
{
	class session;
}

namespace LC::BitTorrent
{
	class SessionSettingsManager : public QObject
	{
		Q_OBJECT

		libtorrent::session * const Session_;
		QTimer * const ScrapeTimer_;
		QTimer * const SettingsSaveTimer_;
	public:
		explicit SessionSettingsManager (libtorrent::session*, QObject* = nullptr);

		enum class Preset
		{
			Default,
			MinMemoryUsage,
			HighPerfSeed
		};

		void SetPreset (Preset);

		void SetOverallDownloadRate (int);
		void SetOverallUploadRate (int);
		void SetMaxDownloadingTorrents (int);
		void SetMaxUploadingTorrents (int);
		int GetOverallDownloadRate () const;
		int GetOverallUploadRate () const;
		int GetMaxDownloadingTorrents () const;
		int GetMaxUploadingTorrents () const;
	private:
		void ManipulateSettings ();
		void TcpPortRangeChanged ();
		void SetProxySettings ();
		void SetGeneralSettings ();
		void SetDHTSettings ();
	signals:
		void scrapeRequested ();
		void saveSettingsRequested ();
	};
}
