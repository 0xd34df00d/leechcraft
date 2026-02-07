/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filetransfermanager.h"
#include <QtDebug>
#include <tox/tox.h>
#include "toxaccount.h"
#include "toxthread.h"
#include "filetransferin.h"
#include "filetransferout.h"
#include "toxcontact.h"
#include "util.h"

namespace LC::Azoth::Sarin
{
	FileTransferManager::FileTransferManager (ToxAccount *acc)
	: QObject { acc }
	, Acc_ { acc }
	{
	}

	Emitters::TransferManager& FileTransferManager::GetTransferManagerEmitter ()
	{
		return Emitter_;
	}

	bool FileTransferManager::IsAvailable () const
	{
		return !Tox_.expired ();
	}

	namespace
	{
		void SetupLifetime (FileTransferBase *job)
		{
			auto& emitter = job->GetTransferJobEmitter ();
			QObject::connect (&emitter,
					&Emitters::TransferJob::stateChanged,
					job,
					[job] (TransferState state)
					{
						if (IsTerminal (state))
							job->deleteLater ();
					},
					Qt::QueuedConnection);
		}
	}

	ITransferJob* FileTransferManager::Accept (const IncomingOffer& offer, const QString& savePath)
	{
		if (!Offers_.contains (offer.JobId_))
			return {};

		const auto ctx = Offers_.take (offer.JobId_);

		const auto tox = Tox_.lock ();
		if (!tox)
		{
			qWarning () << "Tox thread is not available";
			return {};
		}

		const auto job = new FileTransferIn { ctx.Pkey_, ctx.FriendNum_, ctx.FileNum_, ctx.Size_, tox };
		SetupLifetime (job);
		connect (this,
				&FileTransferManager::gotFileControl,
				job,
				&FileTransferIn::HandleFileControl);
		connect (this,
				&FileTransferManager::gotData,
				job,
				&FileTransferIn::HandleData);
		QTimer::singleShot (0, job, [job, savePath] { job->Accept (savePath); });
		return job;
	}

	void FileTransferManager::Decline (const IncomingOffer& offer)
	{
		Offers_.remove (offer.JobId_);
	}

	ITransferJob* FileTransferManager::SendFile (const QString& id, const QString&, const QString& name, const QString&)
	{
		const auto tox = Tox_.lock ();
		if (!tox)
		{
			qWarning () << "Tox thread is not available";
			return {};
		}

		const auto contact = Acc_->GetByAzothId (id);
		if (!contact)
		{
			qWarning () << "unable to find contact by the ID" << id;
			return {};
		}

		const auto transfer = new FileTransferOut { contact->GetPubKey (), name, tox };
		SetupLifetime (transfer);
		connect (this,
				&FileTransferManager::gotFileControl,
				transfer,
				&FileTransferOut::HandleFileControl);
		connect (this,
				&FileTransferManager::gotChunkRequest,
				transfer,
				&FileTransferOut::HandleChunkRequested);
		return transfer;
	}

	void FileTransferManager::HandleToxThreadChanged (const std::shared_ptr<ToxRunner>& tox)
	{
		Tox_ = tox;
		if (!tox)
			return;

		connect (&*tox,
				&ToxRunner::gotFileControl,
				this,
				&FileTransferManager::gotFileControl);
		connect (&*tox,
				&ToxRunner::gotData,
				this,
				&FileTransferManager::gotData);
		connect (&*tox,
				&ToxRunner::gotChunkRequest,
				this,
				&FileTransferManager::gotChunkRequest);
		connect (&*tox,
				&ToxRunner::requested,
				this,
				&FileTransferManager::HandleRequest);
	}

	void FileTransferManager::HandleRequest (uint32_t friendNum,
			Pubkey pkey, uint32_t filenum, uint64_t size, const QString& name)
	{
		const auto entry = Acc_->GetByPubkey (pkey);
		if (!entry)
		{
			qWarning () << "unable to find entry for pubkey" << pkey;
			return;
		}

		const auto jobId = JobIdGen_++;
		Offers_ [jobId] = OfferContext
		{
			.FriendNum_ = friendNum,
			.Pkey_ = pkey,
			.FileNum_ = filenum,
			.Size_ = size,
		};

		emit Emitter_.fileOffered ({
					.Manager_ = this,
					.JobId_ = jobId,
					.EntryId_ = entry->GetEntryID (),
					.Name_ = name,
					.Size_ = static_cast<qsizetype> (size)
				});
	}
}
