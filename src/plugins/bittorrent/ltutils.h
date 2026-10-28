/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <libtorrent/fwd.hpp>
#include <libtorrent/units.hpp>

class QByteArray;
class QModelIndex;
class QString;

template<typename, typename>
class QMap;

template<typename>
class QList;

namespace LC::BitTorrent
{
	void AddPeer (const libtorrent::torrent_handle& handle, const QString& ip, unsigned short port);

	using BanRange_t = std::pair<QString, QString>;
	using BanList_t = QList<std::pair<BanRange_t, bool>>;

	void BanPeers (libtorrent::session&, const BanRange_t&, bool block = true);
	BanList_t GetFilter (const libtorrent::session&);
	void RestoreFilter (libtorrent::session&);
	void SaveFilter (const libtorrent::session&);
	void ClearFilter (libtorrent::session&);

	void RunIPFilterDialog (libtorrent::session&);

	// in kbps, -1 for unlimilted
	void SetDownloadLimit (const libtorrent::torrent_handle&, int);
	int GetDownloadLimit (const libtorrent::torrent_handle&);
	void SetUploadLimit (const libtorrent::torrent_handle&, int);
	int GetUploadLimit (const libtorrent::torrent_handle&);

	bool IsValidTorrent (const QByteArray&);

	libtorrent::torrent_handle GetTorrentHandle (const QModelIndex&);

	int GetFilesCount (const libtorrent::torrent_handle&);

	const libtorrent::file_storage& GetOriginalFileLayout (const libtorrent::torrent_info&);

	std::string GetCurrentFilePath (const libtorrent::torrent_handle&, const libtorrent::torrent_info&, libtorrent::file_index_t);

	std::string MakeMagnetUri (const libtorrent::torrent_handle&);

	QString GetTorrentName (const libtorrent::torrent_alert&);

	struct FileInfo;
	QList<FileInfo> GetTorrentFiles (const libtorrent::torrent_handle&);

	struct PerTrackerStats
	{
		int64_t DownloadRate_ = 0;
		int64_t UploadRate_ = 0;
	};
	QMap<QString, PerTrackerStats> GetPerTrackerStats (const libtorrent::session&);
}
