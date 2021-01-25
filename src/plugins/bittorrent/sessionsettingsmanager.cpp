/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sessionsettingsmanager.h"
#include <QMessageBox>
#include <QMainWindow>
#include <QTimer>
#include <libtorrent/session.hpp>
#include <libtorrent/extensions/ut_metadata.hpp>
#include <libtorrent/extensions/ut_pex.hpp>
#include <libtorrent/extensions/smart_ban.hpp>
#include <util/sll/slotclosure.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/sll/util.h>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace BitTorrent
{
	SessionSettingsManager::SessionSettingsManager (libtorrent::session *session, const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject { parent }
	, Session_ { session }
	, Proxy_ { proxy }
	, ScrapeTimer_ { new QTimer { this } }
	, SettingsSaveTimer_ { new QTimer { this } }
	{
		setLoggingSettings ();
		tcpPortRangeChanged ();

		if (XmlSettingsManager::Instance ()->
				property ("EnablePEX").toBool ())
			Session_->add_extension (&libtorrent::create_ut_pex_plugin);
		if (XmlSettingsManager::Instance ()->
				property ("EnableUTMetadata").toBool ())
			Session_->add_extension (&libtorrent::create_ut_metadata_plugin);
		if (XmlSettingsManager::Instance ()->
				property ("EnableSmartBan").toBool ())
			Session_->add_extension (&libtorrent::create_smart_ban_plugin);

		maxUploadsChanged ();
		maxConnectionsChanged ();
		setProxySettings ();
		setGeneralSettings ();
		setScrapeInterval ();
		setDHTSettings ();

		setScrapeInterval ();
		autosaveIntervalChanged ();

		connect (ScrapeTimer_,
				SIGNAL (timeout ()),
				this,
				SIGNAL (scrapeRequested ()));
		connect (SettingsSaveTimer_,
				SIGNAL (timeout ()),
				this,
				SIGNAL (saveSettingsRequested ()));

		ManipulateSettings ();
	}

	void SessionSettingsManager::SetPreset (Preset sp)
	{
		switch (sp)
		{
		case Preset::MinMemoryUsage:
			// TODO file_checks_delay_per_block = 15
			// max_paused_peerlist_size = 50
			// recv_socket_buffer_size = 16 * 1024
			// send_socket_buffer_size = 16 * 1024
			// optimize_hashing_for_speed = false
			// coalesce_reads = false
			// coalesce_writes = false
			XmlSettingsManager::Instance ()->setProperty ("WholePiecesThreshold", 2);
			XmlSettingsManager::Instance ()->setProperty ("UseParoleMode", false);
			XmlSettingsManager::Instance ()->setProperty ("PrioritizePartialPieces", true);
			XmlSettingsManager::Instance ()->setProperty ("FilePoolSize", 4);
			XmlSettingsManager::Instance ()->setProperty ("AllowMultipleConnectionsPerIP", false);
			XmlSettingsManager::Instance ()->setProperty ("MaxFailcount", 2);
			XmlSettingsManager::Instance ()->setProperty ("InactivityTimeout", 120);
			XmlSettingsManager::Instance ()->setProperty ("MaxOutstandingDiskBytesPerConnection", 1);
			XmlSettingsManager::Instance ()->setProperty ("SendBufferWatermark", 9);
			XmlSettingsManager::Instance ()->setProperty ("CacheSize", 0);
			XmlSettingsManager::Instance ()->setProperty ("UseReadCache", false);
			XmlSettingsManager::Instance ()->setProperty ("CloseRedundantConnections", true);
			XmlSettingsManager::Instance ()->setProperty ("MaxPeerListSize", 500);
			XmlSettingsManager::Instance ()->setProperty ("PreferUDPTrackers", true);
			XmlSettingsManager::Instance ()->setProperty ("MaxRejects", 10);
			break;
		case Preset::HighPerfSeed:
			// TODO read_cache_line_size = 512
			// write_cache_line_size = 512
			// optimize_hashing_for_speed = true
			XmlSettingsManager::Instance ()->setProperty ("FilePoolSize", 500);
			XmlSettingsManager::Instance ()->setProperty ("AllowMultipleConnectionsPerIP", true);
			XmlSettingsManager::Instance ()->setProperty ("CacheSize", 512);
			XmlSettingsManager::Instance ()->setProperty ("UseReadCache", true);
			XmlSettingsManager::Instance ()->setProperty ("CacheExpiry", 60 * 60);
			XmlSettingsManager::Instance ()->setProperty ("CloseRedundantConnections", true);
			XmlSettingsManager::Instance ()->setProperty ("MaxRejects", 10);
			XmlSettingsManager::Instance ()->setProperty ("RequestTimeout", 10);
			XmlSettingsManager::Instance ()->setProperty ("PeerTimeout", 20);
			XmlSettingsManager::Instance ()->setProperty ("InactivityTimeout", 20);
			XmlSettingsManager::Instance ()->setProperty ("AutoUploadSlots", false);
			XmlSettingsManager::Instance ()->setProperty ("MaxFailcount", 1);
			break;
		default:
			break;
		}

		setGeneralSettings ();
	}

	namespace
	{
		using LTSettings_t = libtorrent::settings_pack;

#define LT_SET_INT_OPT(name, val) settings.set_int (libtorrent::settings_pack::name, val)

		void SetOverallDownloadRateImpl (LTSettings_t& settings, int val)
		{
			LT_SET_INT_OPT (download_rate_limit, val == 0 ? -1 : val * 1024);
		}

		void SetOverallUploadRateImpl (LTSettings_t& settings, int val)
		{
			LT_SET_INT_OPT (upload_rate_limit, val == 0 ? -1 : val * 1024);
		}

		void SetMaxDownloadingTorrentsImpl (LTSettings_t& settings, int val)
		{
			LT_SET_INT_OPT (active_downloads, val);
		}

		void SetMaxUploadingTorrentsImpl (LTSettings_t& settings, int val)
		{
			LT_SET_INT_OPT (active_seeds, val);
		}

#undef LT_SET_INT_OPT

		template<typename F>
		void WithSettings (libtorrent::session *session, F&& f)
		{
			auto settings = session->get_settings ();
			f (settings);
			session->apply_settings (settings);
		}

		template<typename F, typename V>
		void WithSettings (libtorrent::session *session, V&& val, F&& f)
		{
			WithSettings (session,
					[&] (auto& settings) { f (settings, val); });
		}
	}

	void SessionSettingsManager::SetOverallDownloadRate (int val)
	{
		WithSettings (Session_, val, SetOverallDownloadRateImpl);
		XmlSettingsManager::Instance ()->setProperty ("DownloadRateLimit", val);
	}

	void SessionSettingsManager::SetOverallUploadRate (int val)
	{
		WithSettings (Session_, val, SetOverallUploadRateImpl);
		XmlSettingsManager::Instance ()->setProperty ("UploadRateLimit", val);
	}

	void SessionSettingsManager::SetMaxDownloadingTorrents (int val)
	{
		WithSettings (Session_, val, SetMaxDownloadingTorrentsImpl);
		XmlSettingsManager::Instance ()->setProperty ("MaxDownloadingTorrents", val);
	}

	void SessionSettingsManager::SetMaxUploadingTorrents (int val)
	{
		WithSettings (Session_, val, SetMaxUploadingTorrentsImpl);
		XmlSettingsManager::Instance ()->setProperty ("MaxUploadingTorrents", val);
	}

	int SessionSettingsManager::GetOverallDownloadRate () const
	{
		return XmlSettingsManager::Instance ()->property ("DownloadRateLimit").toInt ();
	}

	int SessionSettingsManager::GetOverallUploadRate () const
	{
		return XmlSettingsManager::Instance ()->property ("UploadRateLimit").toInt ();
	}

	int SessionSettingsManager::GetMaxDownloadingTorrents () const
	{
		return XmlSettingsManager::Instance ()->Property ("MaxDownloadingTorrents", -1).toInt ();
	}

	int SessionSettingsManager::GetMaxUploadingTorrents () const
	{
		return XmlSettingsManager::Instance ()->Property ("MaxUploadingTorrents", -1).toInt ();
	}

	void SessionSettingsManager::ManipulateSettings ()
	{
		WithSettings (Session_,
				[] (auto& settings)
				{
					SetOverallDownloadRateImpl (settings,
							XmlSettingsManager::Instance ()->Property ("DownloadRateLimit", 5000).toInt ());
					SetOverallUploadRateImpl (settings,
							XmlSettingsManager::Instance ()->Property ("UploadRateLimit", 5000).toInt ());
					SetMaxDownloadingTorrentsImpl (settings,
							XmlSettingsManager::Instance ()->Property ("MaxDownloadingTorrents", -1).toInt ());
					SetMaxUploadingTorrentsImpl (settings,
							XmlSettingsManager::Instance ()->Property ("MaxUploadingTorrents", -1).toInt ());
				});

		XmlSettingsManager::Instance ()->RegisterObject ({ "TCPPortRange", "SSLPort", "EnableSSLPort" },
				this, "tcpPortRangeChanged");
		XmlSettingsManager::Instance ()->RegisterObject ("AutosaveInterval",
				this, "autosaveIntervalChanged");
		XmlSettingsManager::Instance ()->RegisterObject ("MaxUploads",
				this, "maxUploadsChanged");
		XmlSettingsManager::Instance ()->RegisterObject ("MaxConnections",
				this, "maxConnectionsChanged");

		const QList<QByteArray> proxySettings
		{
			"TrackerProxyEnabled",
			"TrackerProxyHost",
			"TrackerProxyPort",
			"TrackerProxyAuth",
			"PeerProxyEnabled",
			"PeerProxyHost",
			"PeerProxyPort",
			"PeerProxyAuth"
		};
		XmlSettingsManager::Instance ()->RegisterObject (proxySettings,
				this, "setProxySettings");

		const QList<QByteArray> generalSettings
		{
			"TrackerCompletionTimeout",
			"TrackerReceiveTimeout",
			"StopTrackerTimeout",
			"TrackerMaximumResponseLength",
			"PieceTimeout",
			"RequestQueueTime",
			"MaxAllowedInRequestQueue",
			"MaxOutRequestQueue",
			"WholePiecesThreshold",
			"PeerTimeout",
			"UrlSeedTimeout",
			"UrlSeedPipelineSize",
			"SeedingPieceQuota",
			"UrlSeedWaitRetry",
			"FilePoolSize",
			"AllowMultipleConnectionsPerIP",
			"MaxFailcount",
			"MinReconnectTime",
			"PeerConnectTimeout",
			"ConnectionSpeed",
			"SendRedundantHave",
			"InactivityTimeout",
			"UnchokeInterval",
			"OptimisticUnchokeMultiplier",
			"AnnounceIP",
			"NumWant",
			"InitialPickerThreshold",
			"AllowedFastSetSize",
			"MaxOutstandingDiskBytesPerConnection",
			"HandshakeTimeout",
			"UseDHTAsFallback",
			"FreeTorrentHashes",
			"SendBufferWatermark",
			"AutoUploadSlots",
			"UseParoleMode",
			"CacheSize",
			"CacheExpiry",
			"OutgoingPorts",
			"PeerTOS",
			"DontCountSlowTorrents",
			"AutoManageInterval",
			"ShareRatioLimit",
			"SeedTimeRatioLimit",
			"SeedTimeLimit",
			"CloseRedundantConnections",
			"AutoScrapeInterval",
			"AutoScrapeMinInterval",
			"MaxPeerListSize",
			"MinAnnounceInterval",
			"PrioritizePartialPieces",
			"AnnounceToAllTrackers",
			"PreferUDPTrackers",
			"StrictSuperSeeding"
		};
		XmlSettingsManager::Instance ()->RegisterObject (generalSettings,
				this, "setGeneralSettings");

		const QList<QByteArray> dhtSettings
		{
			"MaxPeersReply",
			"SearchBranching",
			"ServicePort",
			"MaxDHTFailcount",
			"DHTEnabled",
			"EnableLSD",
			"EnableUPNP",
			"EnableNATPMP"
		};
		XmlSettingsManager::Instance ()->RegisterObject (dhtSettings,
				this, "setDHTSettings");

		XmlSettingsManager::Instance ()->RegisterObject ({ "ScrapeInterval", "ScrapeEnabled" },
				this, "setScrapeInterval");

		const QList<QByteArray> loggingSettings
		{
			"PerformanceWarning",
			"NotificationError",
			"NotificationPeer",
			"NotificationPortMapping",
			"NotificationStorage",
			"NotificationTracker",
			"NotificationStatus",
			"NotificationIPBlock",
			"NotificationDHT"
		};
		XmlSettingsManager::Instance ()->RegisterObject (loggingSettings,
				this, "setLoggingSettings");

		XmlSettingsManager::Instance ()->RegisterObject ("NotificationStorage",
				this, "checkStorageSettings", Util::BaseSettingsManager::EventFlag::Select);
	}

	void SessionSettingsManager::setLoggingSettings ()
	{
		int mask = 0;

		if (XmlSettingsManager::Instance ()->property ("NotificationDHT").toBool ())
			mask |= libtorrent::alert::dht_notification;
		if (XmlSettingsManager::Instance ()->property ("PerformanceWarning").toBool ())
			mask |= libtorrent::alert::performance_warning;
		if (XmlSettingsManager::Instance ()->property ("NotificationError").toBool ())
			mask |= libtorrent::alert::error_notification;
		if (XmlSettingsManager::Instance ()->property ("NotificationPeer").toBool ())
			mask |= libtorrent::alert::peer_notification;
		if (XmlSettingsManager::Instance ()->property ("NotificationPortMapping").toBool ())
			mask |= libtorrent::alert::port_mapping_notification;
		if (XmlSettingsManager::Instance ()->property ("NotificationStorage").toBool ())
			mask |= libtorrent::alert::storage_notification;
		if (XmlSettingsManager::Instance ()->property ("NotificationTracker").toBool ())
			mask |= libtorrent::alert::tracker_notification;
		if (XmlSettingsManager::Instance ()->property ("NotificationStatus").toBool ())
			mask |= libtorrent::alert::status_notification;
		if (XmlSettingsManager::Instance ()->property ("NotificationIPBlock").toBool ())
			mask |= libtorrent::alert::ip_block_notification;

		auto settings = Session_->get_settings ();
		settings.set_int (libtorrent::settings_pack::alert_mask, mask);
		Session_->apply_settings (settings);
	}

	void SessionSettingsManager::tcpPortRangeChanged ()
	{
		const auto& ports = XmlSettingsManager::Instance ()->property ("TCPPortRange").toList ();

		auto settings = Session_->get_settings ();

		QStringList portsList;
		for (int port = ports.at (0).toInt (), endPort = ports.at (1).toInt (); port <= endPort; ++port)
			portsList << "0.0.0.0:" + QString::number (port);

		if (XmlSettingsManager::Instance ()->property ("EnableSSLPort").toBool ())
		{
			const auto sslPort = XmlSettingsManager::Instance ()->property ("SSLPort").toInt ();
			portsList << "0.0.0.0:" + QString::number (sslPort) + "s";
		}

		settings.set_str (libtorrent::settings_pack::listen_interfaces, portsList.join (",").toStdString ());

		Session_->apply_settings (settings);
	}

	void SessionSettingsManager::maxUploadsChanged ()
	{
		const int maxUps = XmlSettingsManager::Instance ()->property ("MaxUploads").toInt ();

		auto settings = Session_->get_settings ();
		settings.set_int (libtorrent::settings_pack::unchoke_slots_limit, maxUps);
		Session_->apply_settings (settings);
	}

	void SessionSettingsManager::maxConnectionsChanged ()
	{
		const int maxConn = XmlSettingsManager::Instance ()->property ("MaxConnections").toInt ();

		auto settings = Session_->get_settings ();
		settings.set_int (libtorrent::settings_pack::connections_limit, maxConn);
		Session_->apply_settings (settings);
	}

	void SessionSettingsManager::setProxySettings ()
	{
		const auto xsm = XmlSettingsManager::Instance ();

		const auto& hostname = xsm->property ("PeerProxyAddress").toString ().toStdString ();
		const auto port = xsm->property ("PeerProxyPort").toInt ();
		const auto& auth = xsm->property ("PeerProxyAuth").toString ().split ('@');
		const auto& username = auth.value (0).toStdString ();
		const auto& password = auth.value (1).toStdString ();
		const auto& pt = xsm->property ("PeerProxyType").toString ();

		auto settings = Session_->get_settings ();
		const auto settingsGuard = Util::MakeScopeGuard ([&settings, this] { Session_->apply_settings (settings); });

		if (!xsm->property ("PeerProxyEnabled").toBool ())
		{
			settings.set_int (libtorrent::settings_pack::proxy_type,
					libtorrent::settings_pack::proxy_type_t::none);
			return;
		}

		settings.set_str (libtorrent::settings_pack::proxy_hostname, hostname);
		settings.set_int (libtorrent::settings_pack::proxy_port, port);
		settings.set_str (libtorrent::settings_pack::proxy_username, username);
		settings.set_str (libtorrent::settings_pack::proxy_password, password);

		const auto proxyType = [&]
		{
			bool passworded = !password.empty ();
			if (pt == "http")
				return passworded ?
						libtorrent::settings_pack::proxy_type_t::http_pw :
						libtorrent::settings_pack::proxy_type_t::http;
			else if (pt == "socks4")
				return libtorrent::settings_pack::proxy_type_t::socks4;
			else if (pt == "socks5")
				return passworded ?
						libtorrent::settings_pack::proxy_type_t::socks5_pw :
						libtorrent::settings_pack::proxy_type_t::socks5;
			else
				return libtorrent::settings_pack::proxy_type_t::none;
		} ();
		settings.set_int (libtorrent::settings_pack::proxy_type, proxyType);
	}

#define LT_SET_BOOL_OPT(name, val) settings.set_bool (libtorrent::settings_pack::bool_types::name, \
			xsm->property (val).toBool ())
#define LT_SET_INT_OPT(name, val) settings.set_int (libtorrent::settings_pack::int_types::name, \
			xsm->property (val).toInt ())
#define LT_SET_PERCENT_OPT(name, val) settings.set_int (libtorrent::settings_pack::int_types::name, \
			std::round (xsm->property (val).toDouble () * 100))
#define LT_SET_INT_OPT2(name, val, mod) settings.set_int (libtorrent::settings_pack::int_types::name, \
			xsm->property (val).toInt () mod)
#define LT_SET_BARE_INT_OPT(name, val) settings.set_int (libtorrent::settings_pack::int_types::name, val)
#define LT_SET_BARE_STR_OPT(name, val) settings.set_str (libtorrent::settings_pack::string_types::name, val)

	void SessionSettingsManager::setGeneralSettings ()
	{
		const auto xsm = XmlSettingsManager::Instance ();

		auto settings = Session_->get_settings ();

		LT_SET_INT_OPT (tracker_completion_timeout, "TrackerCompletionTimeout");
		LT_SET_INT_OPT (tracker_receive_timeout, "TrackerReceiveTimeout");
		LT_SET_INT_OPT (stop_tracker_timeout, "StopTrackerTimeout");
		LT_SET_INT_OPT (piece_timeout, "PieceTimeout");
		LT_SET_INT_OPT (request_timeout, "RequestTimeout");
		LT_SET_INT_OPT (request_queue_time, "RequestQueueTime");
		LT_SET_INT_OPT (max_allowed_in_request_queue, "MaxAllowedInRequestQueue");
		LT_SET_INT_OPT (max_out_request_queue, "MaxOutRequestQueue");
		LT_SET_INT_OPT (whole_pieces_threshold, "WholePiecesThreshold");
		LT_SET_INT_OPT (peer_timeout, "PeerTimeout");
		LT_SET_INT_OPT (urlseed_timeout, "UrlSeedTimeout");
		LT_SET_INT_OPT (urlseed_pipeline_size, "UrlSeedPipelineSize");
		LT_SET_INT_OPT (urlseed_wait_retry, "UrlSeedWaitRetry");
		LT_SET_INT_OPT (file_pool_size, "FilePoolSize");
		LT_SET_INT_OPT (max_failcount, "MaxFailcount");
		LT_SET_INT_OPT (min_reconnect_time, "MinReconnectTime");
		LT_SET_INT_OPT (peer_connect_timeout, "PeerConnectTimeout");
		LT_SET_INT_OPT (connection_speed, "ConnectionSpeed");
		LT_SET_INT_OPT (inactivity_timeout, "InactivityTimeout");
		LT_SET_INT_OPT (unchoke_interval, "UnchokeInterval");
		LT_SET_INT_OPT (optimistic_unchoke_interval, "OptimisticUnchokeMultiplier");
		LT_SET_INT_OPT (num_want, "NumWant");
		LT_SET_INT_OPT (initial_picker_threshold, "InitialPickerThreshold");
		LT_SET_INT_OPT (allowed_fast_set_size, "AllowedFastSetSize");
		LT_SET_INT_OPT (handshake_timeout, "HandshakeTimeout");
		LT_SET_INT_OPT (cache_expiry, "CacheExpiry");
		LT_SET_INT_OPT (peer_tos, "PeerTOS");
		LT_SET_INT_OPT (auto_manage_interval, "AutoManageInterval");
		LT_SET_INT_OPT (auto_scrape_min_interval, "AutoScrapeMinInterval");
		LT_SET_INT_OPT (max_peerlist_size, "MaxPeerListSize");
		LT_SET_INT_OPT (min_announce_interval, "MinAnnounceInterval");
		LT_SET_INT_OPT (seeding_piece_quota, "SeedingPieceQuota");
		LT_SET_INT_OPT (auto_manage_startup, "AutoManageStartup");
		LT_SET_INT_OPT (max_rejects, "MaxRejects");

		LT_SET_PERCENT_OPT (share_ratio_limit, "ShareRatioLimit");
		LT_SET_PERCENT_OPT (seed_time_ratio_limit, "SeedTimeRatioLimit");
		LT_SET_PERCENT_OPT (peer_turnover, "PeerTurnover");

		LT_SET_INT_OPT2 (cache_size, "CacheSize", * (1048576 / 16384));
		LT_SET_INT_OPT2 (max_queued_disk_bytes, "MaxOutstandingDiskBytesPerConnection", * 1024);
		LT_SET_INT_OPT2 (send_buffer_watermark, "SendBufferWatermark", * 1024);
		LT_SET_INT_OPT2 (tracker_maximum_response_length, "TrackerMaximumResponseLength", * 1024);
		LT_SET_INT_OPT2 (seed_time_limit, "SeedTimeLimit", * 60);
		LT_SET_INT_OPT2 (auto_scrape_interval, "AutoScrapeInterval", * 60);

		LT_SET_BOOL_OPT (allow_multiple_connections_per_ip, "AllowMultipleConnectionsPerIP");
		LT_SET_BOOL_OPT (send_redundant_have, "SendRedundantHave");
		LT_SET_BOOL_OPT (use_dht_as_fallback, "UseDHTAsFallback");
		LT_SET_BOOL_OPT (use_parole_mode, "UseParoleMode");
		LT_SET_BOOL_OPT (use_read_cache, "UseReadCache");
		LT_SET_BOOL_OPT (auto_manage_prefer_seeds, "AutoManagePreferSeeds");
		LT_SET_BOOL_OPT (dont_count_slow_torrents, "DontCountSlowTorrents");
		LT_SET_BOOL_OPT (close_redundant_connections, "CloseRedundantConnections");
		LT_SET_BOOL_OPT (prioritize_partial_pieces, "PrioritizePartialPieces");
		LT_SET_BOOL_OPT (announce_to_all_trackers, "AnnounceToAllTrackers");
		LT_SET_BOOL_OPT (announce_to_all_tiers, "AnnounceToAllTiers");
		LT_SET_BOOL_OPT (prefer_udp_trackers, "PreferUDPTrackers");
		LT_SET_BOOL_OPT (strict_super_seeding, "StrictSuperSeeding");

		const auto& ports = XmlSettingsManager::Instance ()->property ("OutgoingPorts").toList ();
		if (ports.size () == 2)
		{
			LT_SET_BARE_INT_OPT (outgoing_port, ports.at (0).toInt ());
			LT_SET_BARE_INT_OPT (num_outgoing_ports, ports.at (1).toInt () - ports.at (0).toInt () + 1);
		}

		LT_SET_BARE_INT_OPT (active_limit, 16384);
		LT_SET_BARE_STR_OPT (announce_ip, xsm->property ("AnnounceIP").toString ().toStdString ());
		LT_SET_BARE_STR_OPT (user_agent, "LeechCraft BitTorrent/" + Proxy_->GetVersion ().toStdString ());

		Session_->apply_settings (settings);
	}

	void SessionSettingsManager::setDHTSettings ()
	{
		auto settings = Session_->get_settings ();

		auto setBool = [&settings] (int id, const char *name)
		{
			const auto val = XmlSettingsManager::Instance ()->property (name).toBool ();
			settings.set_bool (id, val);
			return val;
		};
		setBool (libtorrent::settings_pack::enable_lsd, "EnableLSD");
		setBool (libtorrent::settings_pack::enable_upnp, "EnableUPNP");
		setBool (libtorrent::settings_pack::enable_natpmp, "EnableNATPMP");

		if (setBool (libtorrent::settings_pack::enable_dht, "DHTEnabled"))
			settings.set_str (libtorrent::settings_pack::dht_bootstrap_nodes,
					"router.bittorrent.com:6881,"
					"router.utorrent.com:6881,"
					"dht.transmissionbt.com:6881,"
					"dht.aelitis.com:6881");

		libtorrent::dht_settings dhtSettings;

		dhtSettings.max_peers_reply = XmlSettingsManager::Instance ()->property ("MaxPeersReply").toInt ();
		dhtSettings.search_branching = XmlSettingsManager::Instance ()->property ("SearchBranching").toInt ();
		dhtSettings.service_port = XmlSettingsManager::Instance ()->property ("ServicePort").toInt ();
		dhtSettings.max_fail_count = XmlSettingsManager::Instance ()->property ("MaxDHTFailcount").toInt ();

		Session_->set_dht_settings (dhtSettings);
	}

	void SessionSettingsManager::checkStorageSettings (const QVariant& val)
	{
		if (val.toBool ())
			return;

		const auto rootWM = Proxy_->GetRootWindowsManager ();

		const auto box = new QMessageBox
		{
			QMessageBox::Question,
			"LeechCraft BitTorrent",
			tr ("Storage notifications are disabled. Live streaming "
				"definitely won't work without them, so if you are "
				"experiencing troubles, re-enable storage notifications "
				"in \"Notifications\" section of BitTorrent settings. "
				"Do you want to enable them now?"),
			QMessageBox::Yes | QMessageBox::No,
			rootWM->GetPreferredWindow ()
		};

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[box]
			{
				box->deleteLater ();
				if (box->standardButton (box->clickedButton ()) == QMessageBox::Yes)
					XmlSettingsManager::Instance ()->setProperty ("NotificationStorage", true);
			},
			box,
			SIGNAL (finished (int)),
			box
		};

		box->show ();
	}

	void SessionSettingsManager::setScrapeInterval ()
	{
		if (XmlSettingsManager::Instance ()->property ("ScrapeEnabled").toBool ())
		{
			ScrapeTimer_->stop ();
			ScrapeTimer_->start (XmlSettingsManager::Instance ()->
					property ("ScrapeInterval").toInt () * 1000);
		}
		else
			ScrapeTimer_->stop ();
	}

	void SessionSettingsManager::autosaveIntervalChanged ()
	{
		SettingsSaveTimer_->stop ();
		SettingsSaveTimer_->start (XmlSettingsManager::Instance ()->
				property ("AutosaveInterval").toInt () * 1000);
	}
}
}
