/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sessionholder.h"
#include <libtorrent/torrent_handle.hpp>

namespace LC::BitTorrent
{
	SessionHolder::SessionHolder (libtorrent::session& session)
	: Session_ { session }
	, InvalidHandle_ { std::make_unique<libtorrent::torrent_handle> () }
	{
	}

	SessionHolder::~SessionHolder () = default;

	libtorrent::session& SessionHolder::GetSession () const
	{
		return Session_;
	}

	libtorrent::torrent_handle& SessionHolder::operator[] (int idx)
	{
		if (idx < 0 || idx >= Handles_.size ())
			return *InvalidHandle_;
		return Handles_ [idx];
	}

	const libtorrent::torrent_handle& SessionHolder::operator[] (int idx) const
	{
		if (idx < 0 || idx >= Handles_.size ())
			return *InvalidHandle_;
		return Handles_.at (idx);
	}

	void SessionHolder::MoveUp (int idx)
	{
		using std::swap;
		swap ((*this) [idx], (*this) [idx - 1]);
	}

	void SessionHolder::MoveDown (int idx)
	{
		using std::swap;
		swap ((*this) [idx], (*this) [idx - 1]);
	}

	void SessionHolder::MoveToTop (int idx)
	{
		auto handle = Handles_.takeAt (idx);
		Handles_.push_front (std::move (handle));
	}

	void SessionHolder::MoveToBottom (int idx)
	{
		auto handle = Handles_.takeAt (idx);
		Handles_.push_back (std::move (handle));
	}

	void SessionHolder::AddHandle (const libtorrent::torrent_handle& handle)
	{
		Handles_.push_back (handle);
	}

	void SessionHolder::RemoveHandleAt (int pos)
	{
		Handles_.removeAt (pos);
	}
}
