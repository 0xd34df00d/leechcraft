/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QList>

namespace libtorrent
{
	class session;
	class torrent_handle;
}

namespace LC::BitTorrent
{
	class SessionHolder
	{
		libtorrent::session& Session_;
		QList<libtorrent::torrent_handle> Handles_;
	public:
		explicit SessionHolder (libtorrent::session&);

		SessionHolder (const SessionHolder&) = delete;
		SessionHolder (SessionHolder&&) = delete;

		SessionHolder& operator= (const SessionHolder&) = delete;
		SessionHolder& operator= (SessionHolder&&) = delete;

		libtorrent::session& GetSession () const;

		libtorrent::torrent_handle& operator[] (int);
		const libtorrent::torrent_handle& operator[] (int) const;

		void MoveUp (int);
		void MoveDown (int);
		void MoveToTop (int);
		void MoveToBottom (int);

		void AddHandle (const libtorrent::torrent_handle&);
		void RemoveHandleAt (int);
	};
}

