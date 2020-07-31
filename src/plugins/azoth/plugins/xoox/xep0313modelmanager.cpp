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
	Xep0313ModelManager::Xep0313ModelManager (GlooxAccount *acc)
	: QObject { acc }
	, Model_ { new QStandardItemModel { this } }
	{
		Model_->setHorizontalHeaderLabels ({ tr ("Entry name"), tr ("JID") });

		connect (acc,
				SIGNAL (gotCLItems (QList<QObject*>)),
				this,
				SLOT (handleGotCLItems (QList<QObject*>)));
		connect (acc,
				SIGNAL (removedCLItems (QList<QObject*>)),
				this,
				SLOT (handleRemovedCLItems (QList<QObject*>)));
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
			qWarning () << Q_FUNC_INFO
					<< "no index for JID"
					<< jid;
			return {};
		}
		return item->index ();
	}

	void Xep0313ModelManager::PerformWithEntries (const QList<QObject*>& items,
			const std::function<void (ICLEntry*)>& f)
	{
		for (auto itemObj : items)
		{
			auto entry = qobject_cast<ICLEntry*> (itemObj);
			if (entry->GetEntryType () == ICLEntry::EntryType::MUC)
				continue;

			f (entry);
		}
	}

	void Xep0313ModelManager::handleGotCLItems (const QList<QObject*>& items)
	{
		PerformWithEntries (items,
				[this] (ICLEntry *entry)
				{
					const auto& jid = entry->GetHumanReadableID ();
					if (Jid2Item_.contains (jid))
						return;

					const QList<QStandardItem*> row
					{
						new QStandardItem (entry->GetEntryName ()),
						new QStandardItem (jid)
					};
					for (const auto item : row)
					{
						item->setEditable (false);
						item->setData (QVariant::fromValue (entry->GetQObject ()),
								ServerHistoryRole::CLEntry);
					}
					Jid2Item_ [jid] = row.first ();

					Model_->appendRow (row);
				});
	}

	void Xep0313ModelManager::handleRemovedCLItems (const QList<QObject*>& items)
	{
		PerformWithEntries (items,
				[this] (ICLEntry *entry) -> void
				{
					const auto& jid = entry->GetHumanReadableID ();
					if (!Jid2Item_.contains (jid))
						return;

					const auto row = Jid2Item_.take (jid)->row ();
					Model_->removeRow (row);
				});
	}
}
}
}
