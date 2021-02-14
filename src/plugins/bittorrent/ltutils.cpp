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
#include <libtorrent/address.hpp>
#include <libtorrent/ip_filter.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/session_status.hpp>
#include "ipfilterdialog.h"
#include "sessionstats.h"
#include "types.h"

Q_DECLARE_METATYPE (const libtorrent::torrent_handle*)

namespace LC::BitTorrent
{
	void AddPeer (const libtorrent::torrent_handle& handle, const QString& ip, unsigned short port)
	{
		handle.connect_peer (libtorrent::tcp::endpoint {
				libtorrent::address::from_string (ip.toStdString ()),
				port
			});
	}

	void BanPeers (libtorrent::session& session, const QPair<QString, QString>& peers, bool block)
	{
		auto filter = session.get_ip_filter ();
		filter.add_rule (libtorrent::address::from_string (peers.first.toStdString ()),
				libtorrent::address::from_string (peers.second.toStdString ()),
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
	}

	BanList_t GetFilter (const libtorrent::session& session)
	{
		const auto& [v4, v6] = session.get_ip_filter ().export_filter ();

		QList<QPair<BanRange_t, bool>> result;
		for (const auto& range : v4)
			result.push_back ({ GetBanRange (range), range.flags });
		for (const auto& range : v6)
			result.push_back ({ GetBanRange (range), range.flags });
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
		libtorrent::error_code ec;
		libtorrent::torrent_info result { data.constData (), data.size (), ec };
		return !ec;
	}

	const libtorrent::torrent_handle& GetTorrentHandle (const QModelIndex& index)
	{
		static const libtorrent::torrent_handle guard;
		if (!index.isValid ())
			return guard;

		return *index.data (Roles::TorrentHandle).value<const libtorrent::torrent_handle*> ();
	}

	int GetFilesCount (const libtorrent::torrent_handle& handle)
	{
		return handle.torrent_file () ? handle.torrent_file ()->num_files () : 0;
	}

	SessionStats GetSessionStats (const libtorrent::session& session)
	{
		const auto& status = session.status ();

		libtorrent::cache_status cacheStatus;
		session.get_cache_info (&cacheStatus, {}, libtorrent::session::disk_cache_no_pieces);

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

			cacheStatus.blocks_written,
			cacheStatus.writes,
			cacheStatus.blocks_read,
			cacheStatus.blocks_read_hit,
			cacheStatus.cache_size,
			cacheStatus.read_cache_size,
		};
	}
}
