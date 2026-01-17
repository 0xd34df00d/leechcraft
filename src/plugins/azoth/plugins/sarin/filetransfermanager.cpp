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

	bool FileTransferManager::IsAvailable () const
	{
		return !Tox_.expired ();
	}

	QObject* FileTransferManager::SendFile (const QString& id,
			const QString&, const QString& name, const QString&)
	{
		const auto tox = Tox_.lock ();
		if (!tox)
		{
			qWarning () << "Tox thread is not available";
			return nullptr;
		}

		const auto contact = Acc_->GetByAzothId (id);
		if (!contact)
		{
			qWarning () << "unable to find contact by the ID" << id;
			return nullptr;
		}

		const auto transfer = new FileTransferOut { id, contact->GetPubKey (), name, tox };
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
			const QByteArray& pkey, uint32_t filenum, uint64_t size, const QString& name)
	{
		const auto tox = Tox_.lock ();
		if (!tox)
		{
			qWarning () << "Tox thread is not available";
			return;
		}

		const auto entry = Acc_->GetByPubkey (pkey);
		if (!entry)
		{
			qWarning () << "unable to find entry for pubkey" << pkey;
			return;
		}

		const auto transfer = new FileTransferIn
		{
			entry->GetEntryID (),
			pkey,
			friendNum,
			filenum,
			size,
			name,
			tox
		};

		connect (this,
				&FileTransferManager::gotFileControl,
				transfer,
				&FileTransferIn::HandleFileControl);
		connect (this,
				&FileTransferManager::gotData,
				transfer,
				&FileTransferIn::HandleData);

		emit fileOffered (transfer);
	}
}
