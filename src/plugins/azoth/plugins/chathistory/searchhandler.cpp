/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "searchhandler.h"
#include <util/threads/coro.h>

namespace LC::Azoth::ChatHistory
{
	SearchHandler::SearchHandler (StorageThread& storageThread, EntryChangeGuard guard, QObject *parent)
	: QObject { parent }
	, StorageThread_ { storageThread }
	, Guard_ { std::move (guard) }
	{
	}

	Util::ContextTask<SearchHandler::SearchResult> SearchHandler::HandleSearch (Entry entry, QString text, ChatFindBox::FindFlags flags)
	{
		co_await Util::AddContextObject { *this };

		if (PreviousSearchText_ != text)
		{
			LastSearchCursor_.reset ();
			PreviousSearchText_ = text;
		}

		using enum Storage2::SearchDirection;
		const auto cs = flags & ChatFindBox::FindCaseSensitively ? Qt::CaseSensitive : Qt::CaseInsensitive;
		const auto dir = flags & ChatFindBox::FindBackwards ? Backward : Forward;

		const auto wraparound = dir == Backward ?
				Storage2::Cursor::Max () :
				Storage2::Cursor::Min ();

		auto nextPos = co_await StorageThread_.Run (&Storage2::Search, entry, text, cs, dir, LastSearchCursor_.value_or (wraparound));
		if (!nextPos && flags & ChatFindBox::FindWrapsAround && LastSearchCursor_)
		{
			emit wrappedAround ();
			nextPos = co_await StorageThread_.Run (&Storage2::Search, entry, text, cs, dir, wraparound);
		}

		co_await Guard_ (entry.Id_);
		if (!nextPos)
			co_return NoResults {};

		LastSearchCursor_ = nextPos;
		co_return *nextPos;
	}

	bool SearchHandler::IsFocusMessage (const Storage2::HistoryMessage& message) const
	{
		return LastSearchCursor_ == Storage2::Cursor::FromMessage (message);
	}

	void SearchHandler::Reset ()
	{
		PreviousSearchText_.clear ();
		LastSearchCursor_.reset ();
	}
}
