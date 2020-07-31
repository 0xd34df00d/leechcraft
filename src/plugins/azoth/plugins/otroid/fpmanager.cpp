/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fpmanager.h"
#include <QStandardItemModel>
#include <QMessageBox>
#include <QtDebug>
#include <QTimer>

extern "C"
{
#include <libotr/context.h>
#include <libotr/proto.h>
#include <libotr/privkey.h>
}

#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/iaccount.h>
#include "util.h"

namespace LC
{
namespace Azoth
{
namespace OTRoid
{
	FPManager::FPManager (const OtrlUserState state, IProxyObject *azothProxy, QObject *parent)
	: QObject { parent }
	, UserState_ { state }
	, AzothProxy_ { azothProxy }
	, Model_ { new QStandardItemModel { this } }
	{
	}

	namespace
	{
		QStandardItem* MakeAccountItem (IProxyObject *proxy, const QString& accId)
		{
			const auto accObj = proxy->GetAccount (accId);
			if (!accObj)
			{
				qWarning () << Q_FUNC_INFO
						<< "no account for"
						<< accId;
				return nullptr;
			}

			const auto acc = qobject_cast<IAccount*> (accObj);

			auto item = new QStandardItem { GetAccountIcon (acc), acc->GetAccountName () };
			item->setEditable (false);
			item->setData (FPManager::TypeAcc, FPManager::RoleType);
			return item;
		}

		QList<QStandardItem*> MakeEntryItems (IProxyObject *proxy,
				const QString& accId, const QString& entryId, const QString& protoId)
		{
			const auto entryObj = proxy->GetEntry (entryId, accId);
			if (!entryObj)
			{
				qWarning () << Q_FUNC_INFO
						<< "no entry for"
						<< accId
						<< entryId;
				return {};
			}

			const auto entry = qobject_cast<ICLEntry*> (entryObj);

			const QList<QStandardItem*> result
			{
				new QStandardItem { entry->GetEntryName () },
				new QStandardItem { entry->GetHumanReadableID () },
				new QStandardItem { "0" }
			};
			for (const auto item : result)
			{
				item->setEditable (false);
				item->setData (FPManager::TypeEntry, FPManager::RoleType);
				item->setData (entryId, FPManager::RoleEntryId);
				item->setData (accId, FPManager::RoleAccId);
				item->setData (protoId, FPManager::RoleProtoId);
			}
			return result;
		}
	}

	int FPManager::HandleNew (const char *account,
			const char*, const char *user, unsigned char [20])
	{
		const bool matchAll = !account || !user;

		int count = 0;

		for (auto context = UserState_->context_root; context; context = context->next)
		{
			if (!matchAll &&
					(strcmp (user, context->username) ||
					strcmp (account, context->accountname)))
				continue;

			const auto& accStr = QString::fromUtf8 (context->accountname);
			const auto& userStr = QString::fromUtf8 (context->username);
			const auto& protoStr = QString::fromUtf8 (context->protocol);

			auto& accInfo = Account2User2Fp_ [accStr];
			if (!accInfo.AccItem_)
			{
				if ((accInfo.AccItem_ = MakeAccountItem (AzothProxy_, accStr)))
					Model_->appendRow (accInfo.AccItem_);
				else
					continue;
			}

			auto& entryInfo = accInfo.Entries_ [userStr] ;
			if (entryInfo.EntryItems_.isEmpty ())
			{
				entryInfo.EntryItems_ = MakeEntryItems (AzothProxy_, accStr, userStr, protoStr);

				if (!entryInfo.EntryItems_.isEmpty ())
					accInfo.AccItem_->appendRow (entryInfo.EntryItems_);
				else
					continue;
			}
			else if (auto rc = entryInfo.EntryItems_.at (0)->rowCount ())
				entryInfo.EntryItems_.at (0)->removeRows (0, rc);

			auto& fpInfos = entryInfo.FPs_;
			fpInfos.clear ();

			int fpCount = 0;
			for (auto fp = context->fingerprint_root.next; fp; ++fpCount, fp = fp->next)
			{
				char fpHash [OTRL_PRIVKEY_FPRINT_HUMAN_LEN];
				otrl_privkey_hash_to_human (fpHash, fp->fingerprint);
				QString fpHashStr { fpHash };
				fpInfos.append ({ fpHashStr, QString::fromUtf8 (fp->trust) });

				auto fpItem = new QStandardItem { fpHashStr };
				fpItem->setEditable (false);
				fpItem->setFont (QFont { "Monospace" });
				fpItem->setData (FPManager::TypeFP, FPManager::RoleType);
				fpItem->setData (QByteArray { reinterpret_cast<char*> (fp->fingerprint), 20 },
						FPManager::RoleSourceFP);
				entryInfo.EntryItems_.at (0)->appendRow (fpItem);
			}
			entryInfo.EntryItems_.at (ColumnKeysCount)->setText (QString::number (fpCount));
			count += fpCount;
		}

		return count;
	}

	FPInfos_t FPManager::GetFingerprints (const QString& accId, const QString& userId) const
	{
		return Account2User2Fp_.value (accId).Entries_.value (userId).FPs_;
	}

	QAbstractItemModel* FPManager::GetModel () const
	{
		return Model_;
	}

	void FPManager::reloadAll ()
	{
		Account2User2Fp_.clear ();
		Model_->clear ();
		Model_->setHorizontalHeaderLabels ({ tr ("Name"), tr ("Entry ID"), tr ("Keys count") });

		const int count = HandleNew (nullptr, nullptr, nullptr, nullptr);
		qDebug () << Q_FUNC_INFO
				<< "reloaded"
				<< count
				<< "fps";

		ReloadScheduled_ = false;
	}

	void FPManager::scheduleReload ()
	{
		if (ReloadScheduled_)
			return;

		ReloadScheduled_ = true;
		QTimer::singleShot (2000,
				this,
				SLOT (reloadAll ()));
	}

	namespace
	{
		QModelIndexList CollectLeafs (const QModelIndex& index)
		{
			const auto model = index.model ();

			QModelIndexList result;
			switch (index.data (FPManager::RoleType).toInt ())
			{
			case FPManager::TypeFP:
				result << index;
				break;
			case FPManager::TypeEntry:
			case FPManager::TypeAcc:
				for (int i = 0; i < model->rowCount (index); ++i)
					result = CollectLeafs (model->index (i, 0, index)) + result;
				break;
			default:
				qWarning () << Q_FUNC_INFO
						<< "unknown item type"
						<< index.data (FPManager::RoleType);
				break;
			}

			return result;
		}
	}

	void FPManager::removeRequested (const QString&, const QModelIndexList& srcIndexes)
	{
		QModelIndexList toRemove;
		for (const auto& index : srcIndexes)
			toRemove << CollectLeafs (index.sibling (index.row (), 0));

		if (QMessageBox::question (nullptr,
					tr ("Confirm fingerprints deletion"),
					tr ("Are you sure you want to delete %n fingerprint(s)?", 0, toRemove.size ()),
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		for (const auto& index : toRemove)
		{
			const auto item = Model_->itemFromIndex (index);
			if (!item)
			{
				qWarning () << Q_FUNC_INFO
						<< "no item for index"
						<< index;
				continue;
			}

			const auto parent = item->parent ();

			const auto& entryId = parent->data (RoleEntryId).toString ().toUtf8 ();
			const auto& accId = parent->data (RoleAccId).toString ().toUtf8 ();
			const auto& protoId = parent->data (RoleProtoId).toString ().toUtf8 ();
			const auto context = otrl_context_find (UserState_,
					entryId.constData (),
					accId.constData (),
					protoId.constData (),
					OTRL_INSTAG_BEST,
					false, nullptr, nullptr, nullptr);

			const auto fp = context ?
					otrl_context_find_fingerprint (context,
							reinterpret_cast<unsigned char*> (item->data (RoleSourceFP).toByteArray ().data ()),
							false,
							nullptr) :
					nullptr;
			if (fp)
			{
				if (context->active_fingerprint == fp)
					otrl_context_force_finished (context);

				otrl_context_forget_fingerprint (fp, true);
			}
			else
				qWarning () << Q_FUNC_INFO
						<< "fingerprint for"
						<< entryId
						<< accId
						<< protoId
						<< context
						<< item->data (RoleSourceFP).toByteArray ().toHex ()
						<< "not found";

			parent->removeRow (item->row ());

			if (!parent->rowCount ())
				parent->parent ()->removeRow (parent->row ());
		}

		emit fingerprintsChanged ();
	}

	void FPManager::customButtonPressed (const QString&, const QByteArray& id, int)
	{
		if (id == "refresh")
			reloadAll ();
	}
}
}
}
