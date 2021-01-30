/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>

class QAbstractItemModel;
class QString;

template<typename, typename>
class QPair;

template<typename>
class QList;

namespace libtorrent
{
	class torrent_handle;
	class session;
}

namespace LC::BitTorrent
{
	void AddPeer (const libtorrent::torrent_handle& handle, const QString& ip, unsigned short port);

	using BanRange_t = QPair<QString, QString>;
	using BanList_t = QList<QPair<BanRange_t, bool>>;

	void BanPeers (libtorrent::session&, const BanRange_t&, bool block = true);
	BanList_t GetFilter (const libtorrent::session&);
	void RestoreFilter (libtorrent::session&);
	void SaveFilter (const libtorrent::session&);
	void ClearFilter (libtorrent::session&);

	void RunIPFilterDialog (libtorrent::session&);

	// in kbps, -1 for unlimilted
	void SetDownloadLimit (libtorrent::torrent_handle&, int);
	int GetDownloadLimit (const libtorrent::torrent_handle&);
	void SetUploadLimit (libtorrent::torrent_handle&, int);
	int GetUploadLimit (const libtorrent::torrent_handle&);

	std::unique_ptr<QAbstractItemModel> MakeWebSeedsModel (const libtorrent::torrent_handle&);
}
