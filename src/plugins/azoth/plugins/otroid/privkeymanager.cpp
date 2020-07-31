/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "privkeymanager.h"
#include <QStandardItemModel>
#include <QMessageBox>
#include <QtDebug>

extern "C"
{
#include <libotr/privkey.h>
}

#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iprotocol.h>
#include "util.h"

namespace LC
{
namespace Azoth
{
namespace OTRoid
{
	PrivKeyManager::PrivKeyManager (const OtrlUserState state, IProxyObject *proxy)
	: UserState_ { state }
	, AzothProxy_ { proxy }
	, Model_ { new QStandardItemModel { this } }
	{
	}

	QAbstractItemModel* PrivKeyManager::GetModel () const
	{
		return Model_;
	}

	void PrivKeyManager::GenerateRequested (int row)
	{
		const auto nameItem = Model_->item (row, ColumnAccName);
		const auto item = Model_->item (row, ColumnKey);
		if (!item)
			return;

		const auto& accId = item->data (RoleAccId).toString ();
		const auto& protoId = item->data (RoleProtoId).toString ();

		if (!item->text ().isEmpty ())
			if (QMessageBox::question (nullptr,
						tr ("Private keys generation"),
						tr ("Account %1 already has a private key, do you want to generate a new one?")
							.arg ("<em>" + nameItem->text () + "</em>"),
						QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
				return;

		emit keysGenerationRequested (accId, protoId);
	}

	void PrivKeyManager::reloadAll ()
	{
		Model_->clear ();
		Model_->setHorizontalHeaderLabels ({ tr ("Account"), tr ("Private key") });

		QHash<QString, QString> accId2key;
		for (auto pkey = UserState_->privkey_root; pkey; pkey = pkey->next)
		{
			char fpHash [OTRL_PRIVKEY_FPRINT_HUMAN_LEN];
			if (!otrl_privkey_fingerprint (UserState_,
						fpHash, pkey->accountname, pkey->protocol))
				continue;

			const auto& accId = QString::fromUtf8 (pkey->accountname);
			accId2key [accId] = QString::fromUtf8 (fpHash);
		}

		for (const auto accObj : AzothProxy_->GetAllAccounts ())
		{
			const auto acc = qobject_cast<IAccount*> (accObj);
			if (!acc)
				continue;

			const auto& hash = accId2key.value (acc->GetAccountID ());

			QList<QStandardItem*> row
			{
				new QStandardItem { GetAccountIcon (acc), acc->GetAccountName () },
				new QStandardItem { hash }
			};

			const auto proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());

			for (auto item : row)
			{
				item->setEditable (false);
				item->setData (acc->GetAccountID (), RoleAccId);
				item->setData (proto->GetProtocolID (), RoleProtoId);
			}

			row.value (ColumnKey)->setFont (QFont { "Monospace" });

			Model_->appendRow (row);
		}
	}

	void PrivKeyManager::removeRequested (const QString&, QModelIndexList indexes)
	{
		for (auto i = indexes.begin (); i != indexes.end (); )
		{
			auto index = *i;
			index = index.sibling (index.row (), ColumnKey);
			if (index.data ().toString ().isEmpty ())
				i = indexes.erase (i);
			else
				++i;
		}

		if (indexes.isEmpty ())
			return;

		if (QMessageBox::question (nullptr,
					tr ("Confirm private keys deletion"),
					tr ("Are you sure you want to delete %n private key(s)?", 0, indexes.size ()),
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		for (const auto& index : indexes)
		{
			const auto& accId = index.data (RoleAccId).toString ();
			const auto& protoId = index.data (RoleProtoId).toString ();

			const auto pkey = otrl_privkey_find (UserState_,
					accId.toUtf8 ().constData (),
					protoId.toUtf8 ().constData ());
			if (!pkey)
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot find private key for"
						<< accId
						<< protoId;
				continue;
			}

			otrl_privkey_forget (pkey);
		}

		reloadAll ();
		emit keysChanged ();
	}

	void PrivKeyManager::customButtonPressed (const QString&, const QByteArray& id, int row)
	{
		if (id == "refresh")
			reloadAll ();
		else if (id == "generate")
			GenerateRequested (row);
	}
}
}
}
