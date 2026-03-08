/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xep0313modelmanager.h"
#include <QStandardItemModel>
#include <QtDebug>
#include "glooxaccount.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	namespace
	{
		void PerformWithEntries (const QList<QObject*>& items, const auto& f)
		{
			for (auto itemObj : items)
				if (const auto entry = qobject_cast<ICLEntry*> (itemObj);
					entry->GetEntryType () != ICLEntry::EntryType::MUC)
					f (entry);
		}
	}

	Xep0313ModelManager::Xep0313ModelManager (GlooxAccount *acc)
	: QObject { acc }
	, Model_ { new QStandardItemModel { this } }
	{
		Model_->setHorizontalHeaderLabels ({ tr ("Entry name"), tr ("JID") });

		auto& emitter = acc->GetAccountEmitter ();
		connect (&emitter,
				&Emitters::Account::gotCLItems,
				this,
				&Xep0313ModelManager::HandleGotCLItems);
		connect (&emitter,
				&Emitters::Account::removedCLItems,
				this,
				[this] (const QList<QObject*>& items)
				{
					PerformWithEntries (items,
							[this] (ICLEntry *entry)
							{
								if (const auto item = Jid2Item_.take (entry->GetHumanReadableID ()))
									Model_->removeRow (item->row ());
							});
				});
	}

	QAbstractItemModel* Xep0313ModelManager::GetModel () const
	{
		return Model_;
	}

	QString Xep0313ModelManager::Index2Jid (const QModelIndex& index) const
	{
		const auto item = Model_->itemFromIndex (index.sibling (index.row (), 0));
		return Jid2Item_.key (item);
	}

	QModelIndex Xep0313ModelManager::Jid2Index (const QString& jid) const
	{
		const auto item = Jid2Item_.value (jid);
		if (!item)
		{
			qWarning () << "no index for JID" << jid;
			return {};
		}
		return item->index ();
	}

	void Xep0313ModelManager::HandleGotCLItems (const QList<QObject*>& items)
	{
		PerformWithEntries (items,
				[this] (ICLEntry *entry)
				{
					const auto& jid = entry->GetHumanReadableID ();
					if (Jid2Item_.contains (jid))
						return;

					const QList row
					{
						new QStandardItem (entry->GetEntryName ()),
						new QStandardItem (jid)
					};
					for (const auto item : row)
					{
						item->setEditable (false);
						item->setData (QVariant::fromValue (entry->GetQObject ()), ServerHistoryRole::CLEntry);
					}
					Jid2Item_ [jid] = row.first ();

					Model_->appendRow (row);
				});
	}
}
}
}
