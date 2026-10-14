/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fpmanager.h"
#include <QGuiApplication>
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
#include <interfaces/azoth/iclentry.h>
#include <util/azoth/entryidops.h>
#include <util/azoth/util.h>
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
			const auto acc = qobject_cast<IAccount*> (proxy->GetAccount (accId));
			if (!acc)
				qWarning () << "no account for" << accId;

			const auto& icon = acc ? GetAccountIcon (acc) : QIcon::fromTheme (QIcon::ThemeIcon::DialogWarning);
			const auto item = new QStandardItem { icon, acc ? acc->GetAccountName () : accId };
			item->setEditable (false);
			item->setData (FPManager::TypeAcc, FPManager::RoleType);
			return item;
		}

		QList<QStandardItem*> MakeEntryItems (IProxyObject *proxy,
				const QString& accId, const QString& entryIdStr, const QString& protoId, const QIcon& defaultAvatar)
		{
			const auto& entryId = Deserialize<EntryStrongestId> (entryIdStr);
			const auto entry = entryId ? proxy->GetEntry (AsGlobalId (accId, *entryId)) : nullptr;
			const QList result
			{
				new QStandardItem { defaultAvatar, entry ? entry->GetEntryName () : FPManager::tr ("Unknown entry") },
				new QStandardItem { entry ? entry->GetHumanReadableAddress () : entryIdStr },
				new QStandardItem { "0" }
			};

			for (const auto item : result)
			{
				item->setEditable (false);
				item->setData (FPManager::TypeEntry, FPManager::RoleType);
				item->setData (entryIdStr, FPManager::RoleEntryId);
				item->setData (accId, FPManager::RoleAccId);
				item->setData (protoId, FPManager::RoleProtoId);
			}

			if (!entry)
			{
				const auto fgBrush = QGuiApplication::palette ().brush (QPalette::Disabled, QPalette::Text);
				const auto& tooltip = entryId ?
						FPManager::tr ("Entry not found.") :
						FPManager::tr ("Unable to parse entry ID.");
				for (const auto item : result)
				{
					item->setForeground (fgBrush);
					item->setToolTip (tooltip);
				}
				result [0]->setIcon (QIcon::fromTheme (QIcon::ThemeIcon::DialogWarning));
			}

			return result;
		}
	}

	int FPManager::RunReload ()
	{
		int count = 0;

		const QIcon defaultAvatar { QPixmap::fromImage (AzothProxy_->GetDefaultAvatar ()) };

		for (auto context = UserState_->context_root; context; context = context->next)
		{
			if (context->their_instance != OTRL_INSTAG_MASTER)
				continue;

			const auto& accStr = QString::fromUtf8 (context->accountname);
			const auto& entryIdStr = QString::fromUtf8 (context->username);
			const auto& protoStr = QString::fromUtf8 (context->protocol);

			auto& accInfo = Account2User2Fp_ [accStr];
			if (!accInfo.AccItem_)
			{
				accInfo.AccItem_ = MakeAccountItem (AzothProxy_, accStr);
				Model_->appendRow (accInfo.AccItem_);
			}

			auto& entryInfo = accInfo.Entries_ [entryIdStr] ;
			if (entryInfo.EntryItems_.isEmpty ())
			{
				entryInfo.EntryItems_ = MakeEntryItems (AzothProxy_, accStr, entryIdStr, protoStr, defaultAvatar);
				accInfo.AccItem_->appendRow (entryInfo.EntryItems_);
			}

			int fpCount = 0;
			for (auto fp = context->fingerprint_root.next; fp; ++fpCount, fp = fp->next)
			{
				char fpHash [OTRL_PRIVKEY_FPRINT_HUMAN_LEN];
				otrl_privkey_hash_to_human (fpHash, fp->fingerprint);

				auto fpItem = new QStandardItem { QString::fromLatin1 (fpHash) };
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

	QAbstractItemModel* FPManager::GetModel () const
	{
		return Model_;
	}

	void FPManager::reloadAll ()
	{
		Account2User2Fp_.clear ();
		Model_->clear ();
		Model_->setHorizontalHeaderLabels ({ tr ("Name"), tr ("Entry ID"), tr ("Keys count") });

		const int count = RunReload ();
		qDebug () << "reloaded" << count << "fps";

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
				qWarning () << "fingerprint for" << entryId << accId << protoId << context
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
