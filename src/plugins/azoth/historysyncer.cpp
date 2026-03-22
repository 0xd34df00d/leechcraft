/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "historysyncer.h"
#include <QtDebug>
#include <ranges>
#include <util/threads/coro.h>
#include <util/threads/coro/inparallel.h>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/ihistoryplugin.h"
#include "interfaces/azoth/ihaveserverhistory.h"
#include "core.h"

namespace LC
{
namespace Azoth
{
	HistorySyncer::HistorySyncer (QObject *parent)
	: QObject { parent }
	{
	}

	void HistorySyncer::AddStorage (IHistoryPlugin *storage)
	{
		Storages_ << storage;
	}

	namespace
	{
		bool IsOnline (State st)
		{
			switch (st)
			{
			case SOffline:
			case SError:
			case SConnecting:
			case SInvalid:
				return false;
			default:
				return true;
			}
		}
	}

	void HistorySyncer::AddAccount (IAccount *acc)
	{
		const auto ihsh = qobject_cast<IHaveServerHistory*> (acc->GetQObject ());
		if (!ihsh)
			return;

		connect (&acc->GetAccountEmitter (),
				&Emitters::Account::statusChanged,
				this,
				[this, acc] (const EntryStatus& status)
				{
					if (!IsOnline (status.State_))
						CurrentlyOnline_.remove (acc);
					else if (!CurrentlyOnline_.contains (acc))
					{
						CurrentlyOnline_ << acc;
						SyncAccount (acc);
					}
				});
	}

	Util::ContextTask<void> HistorySyncer::SyncAccount (IAccount *acc)
	{
		qDebug () << Q_FUNC_INFO << acc->GetAccountID ();

		if (Storages_.isEmpty ())
		{
			qDebug () << "no history storage plugins are loaded";
			co_return;
		}

		co_await Util::AddContextObject { *this };
		co_await Util::AddContextObject { *acc->GetQObject () };

		auto results = co_await Util::InParallel (Storages_, &IHistoryPlugin::RequestMaxTimestamp, *acc)
				| std::views::filter ([] (const auto& opt) { return opt.has_value (); })
				| std::views::transform ([] (const auto& opt) { return opt.value (); })
				;
		const auto minDate = std::ranges::fold_left_first (results, std::ranges::min);
		co_await RequestAccountFrom (acc, minDate);
	}

	Util::ContextTask<void> HistorySyncer::RequestAccountFrom (IAccount *acc, const std::optional<QDateTime>& from)
	{
		qDebug () << acc->GetAccountID () << from;
		co_await Util::AddContextObject { *this };

		const auto ihsh = qobject_cast<IHaveServerHistory*> (acc->GetQObject ());
		const auto history = co_await ihsh->FetchServerHistory (from);
		AppendItems (history);
	}

	void HistorySyncer::AppendItems (const QList<History::SomeEntryWithMessages>& entries)
	{
		for (const auto& entryWithMessages : entries)
			for (const auto storage : Storages_)
				storage->AddMessages (entryWithMessages);
	}
}
}
