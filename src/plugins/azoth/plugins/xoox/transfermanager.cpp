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
				&TransferManager::HandleQxmppJob);

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

		auto updateFTSettings = [=, this]
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

	Emitters::TransferManager& TransferManager::GetTransferManagerEmitter ()
	{
		return Emitter_;
	}

	ITransferJob* TransferManager::Accept (const IncomingOffer& offer, const QString& savePath)
	{
		if (const auto node = PendingJobs_.extract (offer.JobId_))
			return new TransferJob { std::move (node.mapped ()), savePath };
		return {};
	}

	void TransferManager::Decline (const IncomingOffer& offer)
	{
		if (const auto node = PendingJobs_.extract (offer.JobId_))
			node.mapped ()->abort ();
	}

	bool TransferManager::IsAvailable () const
	{
		const auto settings = Account_.GetSettings ();
		return settings->GetFTMethods () != QXmppTransferJob::NoMethod;
	}

	ITransferJob* TransferManager::SendFile (const QString& id,
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
		return new TransferJob { std::unique_ptr<QXmppTransferJob> { Manager_.sendFile (target, name, comment) } };
	}

	GlooxAccount* TransferManager::GetAccount () const
	{
		return &Account_;
	}

	void TransferManager::HandleQxmppJob (QXmppTransferJob *rawJob)
	{
		std::unique_ptr<QXmppTransferJob> job { rawJob };
		if (!Conn_.GetCLEntry (job->jid ()))
			Conn_.CreateEntry (job->jid ());

		const auto entry = qobject_cast<ICLEntry*> (Conn_.GetCLEntry (job->jid ()));
		if (!entry)
		{
			job->abort ();
			return;
		}

		const auto& info = job->fileInfo ();

		const auto jobId = JobIdGen_++;
		PendingJobs_ [jobId] = std::move (job);

		emit Emitter_.fileOffered ({
					.Manager_ = this,
					.JobId_ = jobId,
					.EntryId_ = entry->GetEntryID (),
					.Name_ = info.name (),
					.Size_ = info.size (),
					.Description_ = info.description (),
				});
	}
}
}
}
