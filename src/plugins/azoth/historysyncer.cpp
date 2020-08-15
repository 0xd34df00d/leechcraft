/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "historysyncer.h"
#include <QFutureSynchronizer>
#include <QtConcurrentRun>
#include <QtDebug>
#include <util/sll/slotclosure.h>
#include <util/sll/either.h>
#include <util/sll/prelude.h>
#include <util/sll/visitor.h>
#include <util/sll/qtutil.h>
#include <util/threads/futures.h>
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

		const auto accObj = acc->GetQObject ();
		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this, acc]
			{
				if (!IsOnline (acc->GetState ().State_))
					CurrentlyOnline_.remove (acc);
				else if (!CurrentlyOnline_.contains (acc))
				{
					CurrentlyOnline_ << acc;
					StartAccountSync (acc);
				}
			},
			accObj,
			SIGNAL (statusChanged (EntryStatus)),
			accObj
		};
	}

	void HistorySyncer::StartAccountSync (IAccount *acc)
	{
		qDebug () << Q_FUNC_INFO
				<< acc->GetAccountID ();

		if (Storages_.isEmpty ())
		{
			qDebug () << Q_FUNC_INFO
					<< "no history storage plugins are loaded";
			return;
		}

		using RetType_t = Util::UnwrapFutureType_t<decltype (Storages_ [0]->RequestMaxTimestamp (acc))>;

		auto allStorages = std::make_shared<QFutureSynchronizer<RetType_t>> ();
		for (const auto& storage : Storages_)
			allStorages->addFuture (storage->RequestMaxTimestamp (acc));

		Util::Sequence (this, QtConcurrent::run ([allStorages] { allStorages->waitForFinished (); })) >>
				[=, this]
				{
					const auto& results = Util::Map (allStorages->futures (),
							[] (auto future) { return future.result (); });

					const auto& partition = Util::PartitionEithers (results);
					if (!partition.first.isEmpty ())
					{
						qWarning () << Q_FUNC_INFO
								<< "got storage errors:"
								<< partition.first
								<< "; aborting sync";
						return;
					}

					const auto& minDate = *std::min_element (partition.second.begin (), partition.second.end ());
					RequestAccountFrom (acc, minDate);
				};
	}

	void HistorySyncer::RequestAccountFrom (IAccount *acc, const QDateTime& from)
	{
		qDebug () << Q_FUNC_INFO
				<< acc->GetAccountID ()
				<< from;

		const auto ihsh = qobject_cast<IHaveServerHistory*> (acc->GetQObject ());
		Util::Sequence (this, ihsh->FetchServerHistory (from)) >>
				Util::Visitor
				{
					[] (const QString& err) { qWarning () << Q_FUNC_INFO << err; },
					[this, acc] (const auto& map) { AppendItems (acc, map); }
				};
	}

	void HistorySyncer::AppendItems (IAccount *acc,
			const IHaveServerHistory::MessagesSyncMap_t& map)
	{
		for (const auto& pair : Util::Stlize (map))
		{
			const auto& entry = qobject_cast<ICLEntry*> (Core::Instance ().GetEntry (pair.first));
			const auto& name = entry ?
					entry->GetEntryName () :
					pair.second.VisibleName_;

			for (const auto storage : Storages_)
				storage->AddRawMessages (acc->GetAccountID (),
						pair.first, name, pair.second.Messages_);
		}
	}
}
}
