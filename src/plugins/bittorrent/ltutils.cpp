/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ltutils.h"
#include <QString>
#include <QPair>
#include <QSettings>
#include <QCoreApplication>
#include <QUrl>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/address.hpp>
#include <libtorrent/ip_filter.hpp>
#include <libtorrent/load_torrent.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/session_status.hpp>
#include <libtorrent/version.hpp>
#include "fileinfo.h"
#include "ipfilterdialog.h"
#include "sessionstats.h"
#include "types.h"
#include "xmlsettingsmanager.h"

Q_DECLARE_METATYPE (const libtorrent::torrent_handle*)

namespace LC::BitTorrent
{
	void AddPeer (const libtorrent::torrent_handle& handle, const QString& ip, unsigned short port)
	{
		handle.connect_peer (libtorrent::tcp::endpoint {
				libtorrent::make_address (ip.toStdString ()),
				port
			});
	}

	void BanPeers (libtorrent::session& session, const QPair<QString, QString>& peers, bool block)
	{
		auto filter = session.get_ip_filter ();
		filter.add_rule (libtorrent::make_address (peers.first.toStdString ()),
				libtorrent::make_address (peers.second.toStdString ()),
				block ?
						libtorrent::ip_filter::blocked :
						0);
		session.set_ip_filter (filter);
	}

	namespace
	{
		template<typename Range>
		BanRange_t GetBanRange (const Range& range)
		{
			return
			{
				QString::fromStdString (range.first.to_string ()),
				QString::fromStdString (range.last.to_string ())
			};
		}

		bool IsBlocked (uint32_t flags)
		{
			return static_cast<bool> (flags & libtorrent::ip_filter::blocked);
		}
	}

	BanList_t GetFilter (const libtorrent::session& session)
	{
		const auto& [v4, v6] = session.get_ip_filter ().export_filter ();

		QList<QPair<BanRange_t, bool>> result;
		for (const auto& range : v4)
			result.push_back ({ GetBanRange (range), IsBlocked (range.flags) });
		for (const auto& range : v6)
			result.push_back ({ GetBanRange (range), IsBlocked (range.flags) });
		return result;
	}

	namespace Keys
	{
		static const auto Group = QStringLiteral ("Core");
		static const auto IPFilter = QStringLiteral ("IPFilter");
		static const auto First = QStringLiteral ("First");
		static const auto Last = QStringLiteral ("Last");
		static const auto Block = QStringLiteral ("Block");
	}

	void RestoreFilter (libtorrent::session& session)
	{
		using namespace Keys;

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Torrent");
		settings.beginGroup (Group);
		int filters = settings.beginReadArray (IPFilter);
		for (int i = 0; i < filters; ++i)
		{
			settings.setArrayIndex (i);
			BanRange_t range
			{
				settings.value (First).toString (),
				settings.value (Last).toString ()
			};
			bool block = settings.value (Block).toBool ();
			BanPeers (session, range, block);
		}
		settings.endArray ();
		settings.endGroup ();
	}

	void SaveFilter (const libtorrent::session& session)
	{
		using namespace Keys;

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Torrent");
		settings.beginGroup (Group);
		settings.beginWriteArray (IPFilter);
		settings.remove ({});
		int i = 0;
		for (const auto& [range, block] : GetFilter (session))
		{
			settings.setArrayIndex (i++);
			settings.setValue (First, range.first);
			settings.setValue (Last, range.second);
			settings.setValue (Block, block);
		}
		settings.endArray ();
		settings.endGroup ();
	}

	void ClearFilter (libtorrent::session& session)
	{
		session.set_ip_filter ({});
	}

	void RunIPFilterDialog (libtorrent::session& session)
	{
		IPFilterDialog dia { GetFilter (session) };
		if (dia.exec () != QDialog::Accepted)
			return;

		ClearFilter (session);
		for (const auto& pair : dia.GetFilter ())
			BanPeers (session, pair.first, pair.second);
		SaveFilter (session);
	}

	void SetDownloadLimit (const libtorrent::torrent_handle& handle, int limit)
	{
		if (!handle.is_valid ())
			return;

		handle.set_download_limit (limit >= 0 ? limit * 1024 : limit);
	}

	int GetDownloadLimit (const libtorrent::torrent_handle& handle)
	{
		if (!handle.is_valid ())
			return -1;

		const auto val = handle.download_limit ();
		return val >= 0 ? val / 1024 : val;
	}

	void SetUploadLimit (const libtorrent::torrent_handle& handle, int limit)
	{
		if (!handle.is_valid ())
			return;

		handle.set_upload_limit (limit >= 0 ? limit * 1024 : limit);
	}

	int GetUploadLimit (const libtorrent::torrent_handle& handle)
	{
		if (!handle.is_valid ())
			return -1;

		const auto val = handle.upload_limit ();
		return val >= 0 ? val / 1024 : val;
	}

	bool IsValidTorrent (const QByteArray& data)
	{
		try
		{
			libtorrent::load_torrent_buffer (libtorrent::span { data.constData (), data.size () });
			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}
	}

	libtorrent::torrent_handle GetTorrentHandle (const QModelIndex& index)
	{
		if (!index.isValid ())
			return {};
		return index.data (Roles::TorrentHandle).value<libtorrent::torrent_handle> ();
	}

	int GetFilesCount (const libtorrent::torrent_handle& handle)
	{
		return handle.torrent_file () ? handle.torrent_file ()->num_files () : 0;
	}

	const libtorrent::file_storage& GetOriginalFileLayout (const libtorrent::torrent_info& info)
	{
#if LIBTORRENT_VERSION_NUM >= 20100
		return info.layout ();
#else
		return info.files ();
#endif
	}

	/* TODO libtorrent 2.1:
	 * 0. register a temporary async alert handler
	 * 1. call handle.save_resume_data ()
	 * 2. take the add_torrent_params from the resulting save_resume_data_alert
	 *    and pass it to make_magnet_uri (), after clearing peers and dht_nodes
	 */
	std::string MakeMagnetUri (const libtorrent::torrent_handle& handle)
	{
		if (!handle.is_valid ())
			return {};

		libtorrent::add_torrent_params atp;
		atp.info_hashes = handle.info_hashes ();
		atp.name = handle.status (libtorrent::torrent_handle::query_name).name;
		for (const auto& tracker : handle.trackers ())
			atp.trackers.push_back (tracker.url);
		for (const auto& seed : handle.url_seeds ())
			atp.url_seeds.push_back (seed);
		return libtorrent::make_magnet_uri (atp);
	}

	SessionStats GetSessionStats (const libtorrent::session& session)
	{
		const auto& status = session.status ();
		return
		{
			{ status.download_rate, status.upload_rate },
			{ status.ip_overhead_download_rate, status.ip_overhead_upload_rate },
			{ status.dht_download_rate, status.dht_upload_rate },
			{ status.tracker_download_rate, status.tracker_upload_rate },

			{ status.total_download, status.total_upload },
			{ status.total_ip_overhead_download, status.total_ip_overhead_upload },
			{ status.total_dht_download, status.total_dht_upload },
			{ status.total_tracker_download, status.total_tracker_upload },
			{ status.total_payload_download, status.total_payload_upload },

			status.num_peers,
			status.dht_global_nodes,
			status.dht_nodes,
			status.dht_torrents,

			status.total_failed_bytes,
			status.total_redundant_bytes,
		};
	}

	QList<FileInfo> GetTorrentFiles (const libtorrent::torrent_handle& handle)
	{
		const auto& infoPtr = handle.torrent_file ();
		if (!infoPtr)
			return {};

		const auto& info = *infoPtr;

		std::vector<std::int64_t> prbytes;

		libtorrent::torrent_handle::file_progress_flags_t flags {};
		if (!XmlSettingsManager::Instance ().property ("AccurateFileProgress").toBool ())
			flags |= libtorrent::torrent_handle::piece_granularity;
		handle.file_progress (prbytes, flags);

		const auto& priorities = handle.get_file_priorities ();

		const auto& files = info.files ();

		QList<FileInfo> result;
		for (const auto i : files.file_range ())
		{
			const auto idx = static_cast<int> (i);
			FileInfo fi;
			fi.Path_ = files.file_path (i);
			fi.Size_ = files.file_size (i);
			fi.Priority_ = static_cast<std::uint8_t> (priorities [idx]);
			fi.Progress_ = fi.Size_ ?
					prbytes.at (idx) / static_cast<float> (fi.Size_) :
					1;
			result << fi;
		}

		return result;
	}

	QMap<QString, PerTrackerStats> GetPerTrackerStats (const libtorrent::session& session)
	{
		QMap<QString, PerTrackerStats> stats;
		for (const auto& handle : session.get_torrents ())
		{
			const auto& s = handle.status ({});
			auto domain = QUrl { s.current_tracker.c_str () }.host ();
			if (!domain.isEmpty ())
			{
				auto& domainStats = stats [domain];
				domainStats.DownloadRate_ += s.download_payload_rate;
				domainStats.UploadRate_ += s.upload_payload_rate;
			}
		}
		return stats;
	}
}
