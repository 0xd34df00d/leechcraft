/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "transfermanager.h"
#include <QXmppTransferManager.h>
#include "accountsettingsholder.h"
#include "clientconnection.h"
#include "glooxaccount.h"
#include "glooxclentry.h"
#include "serverinfostorage.h"
#include "transferjob.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	TransferManager::TransferManager (QXmppTransferManager& mgr, ClientConnection& conn, GlooxAccount& account)
	: Conn_ { conn }
	, Manager_ { mgr }
	, Account_ { account }
	{
		connect (&Manager_,
				&QXmppTransferManager::fileReceived,
				this,
				[this] (QXmppTransferJob *job)
				{
					if (!Conn_.GetCLEntry (job->jid ()))
						Conn_.CreateEntry (job->jid ());
					emit fileOffered (new TransferJob (job, this));
				});

		auto settings = Account_.GetSettings ();

		auto handleDetectedBSProxy = [settings, this] (const QString& proxy)
		{
			if (settings->GetUseSOCKS5Proxy () && !settings->GetSOCKS5Proxy ().isEmpty ())
				return;

			Manager_.setProxy (proxy);
		};

		auto serverInfoStorage = conn.GetServerInfoStorage ();
		connect (serverInfoStorage,
				&ServerInfoStorage::bytestreamsProxyChanged,
				this,
				handleDetectedBSProxy);

		auto updateFTSettings = [=]
		{
			Manager_.setSupportedMethods (settings->GetFTMethods ());
			Manager_.setProxy (settings->GetUseSOCKS5Proxy () ? settings->GetSOCKS5Proxy () : QString {});

			handleDetectedBSProxy (serverInfoStorage->GetBytestreamsProxy ());
		};
		connect (settings,
				&AccountSettingsHolder::fileTransferSettingsChanged,
				this,
				updateFTSettings);
		updateFTSettings ();
	}

	bool TransferManager::IsAvailable () const
	{
		const auto settings = Account_.GetSettings ();
		return settings->GetFTMethods () != QXmppTransferJob::NoMethod;
	}

	QObject* TransferManager::SendFile (const QString& id,
			const QString& sourceVar, const QString& name, const QString& comment)
	{
		QString target = GlooxCLEntry::JIDFromID (&Account_, id);
		QString var = sourceVar;
		if (var.isEmpty ())
		{
			QObject *entryObj = Conn_.GetCLEntry (target, QString ());
			ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
			if (!entry)
				return nullptr;
			var = entry->Variants ().value (0);
		}
		if (!var.isEmpty ())
			target += '/' + var;
		return new TransferJob (Manager_.sendFile (target, name, comment), this);
	}

	GlooxAccount* TransferManager::GetAccount () const
	{
		return &Account_;
	}
}
}
}
