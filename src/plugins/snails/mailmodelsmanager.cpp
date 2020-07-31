/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mailmodelsmanager.h"
#include "account.h"
#include "mailmodel.h"
#include "core.h"
#include "storage.h"
#include "messageinfo.h"
#include "messagelistactionsmanager.h"

namespace LC
{
namespace Snails
{
	MailModelsManager::MailModelsManager (Account *acc, Storage *st)
	: QObject { acc }
	, Acc_ { acc }
	, Storage_ { st }
	, MsgListActionsMgr_ { new MessageListActionsManager { Acc_, this } }
	{
		connect (acc,
				&Account::willMoveMessages,
				[this] (const QList<QByteArray>& ids, const QStringList& folder)
				{
					for (const auto model : Models_)
						if (model->GetCurrentFolder () == folder)
							model->MarkUnavailable (ids);
				});
	}

	std::unique_ptr<MailModel> MailModelsManager::CreateModel ()
	{
		auto model = std::make_unique<MailModel> (MsgListActionsMgr_, Acc_);
		Models_ << model.get ();

		connect (model.get (),
				&QObject::destroyed,
				this,
				[this, obj = model.get ()] { Models_.removeAll (obj); });

		return model;
	}

	void MailModelsManager::ShowFolder (const QStringList& path, MailModel *mailModel)
	{
		if (!Models_.contains (mailModel))
		{
			qWarning () << Q_FUNC_INFO
					<< "unmanaged model"
					<< mailModel
					<< Models_;
			return;
		}

		mailModel->Clear ();

		qDebug () << Q_FUNC_INFO << path;
		if (path.isEmpty ())
			return;

		mailModel->SetFolder (path);

		try
		{
			mailModel->Append (Storage_->GetMessageInfos (Acc_, path));
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}

		Acc_->Synchronize (path);
	}

	void MailModelsManager::Append (const QList<MessageInfo>& messages)
	{
		for (const auto model : Models_)
			model->Append (messages);
	}

	void MailModelsManager::Remove (const QList<QByteArray>& ids)
	{
		for (const auto model : Models_)
			for (const auto& id : ids)
				model->Remove (id);
	}

	void MailModelsManager::UpdateReadStatus (const QStringList& folderId, const QList<QByteArray>& msgIds, bool read)
	{
		for (const auto model : Models_)
			if (model->GetCurrentFolder () == folderId)
				model->UpdateReadStatus (msgIds, read);
	}
}
}
