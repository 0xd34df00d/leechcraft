/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <util/sll/either.h>
#include "chatfindbox.h"
#include "storage2.h"
#include "types.h"

namespace LC::Azoth::ChatHistory
{
	class StorageThread;

	class SearchHandler : public QObject
	{
		Q_OBJECT
	public:
		using EntryChangeGuard = std::function<Util::Either<EntryChanged, Util::Void> (qint64)>;
	private:
		StorageThread& StorageThread_;
		const EntryChangeGuard Guard_;

		QString PreviousSearchText_;
		std::optional<Storage2::Cursor> LastSearchCursor_;
	public:
		explicit SearchHandler (StorageThread&, EntryChangeGuard, QObject* = nullptr);

		struct NoResults {};
		using SearchResult = Util::Either<EntryChanged, std::variant<NoResults, Storage2::Cursor>>;

		Util::ContextTask<SearchResult> HandleSearch (Entry entry, QString text, ChatFindBox::FindFlags flags);

		bool IsFocusMessage (const Storage2::HistoryMessage& message) const;

		void Reset ();
	signals:
		void wrappedAround ();
	};
}
