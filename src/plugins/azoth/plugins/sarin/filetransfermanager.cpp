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
#include "callbackmanager.h"

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	FileTransferManager::FileTransferManager (ToxAccount *acc)
	: QObject { acc }
	, Acc_ { acc }
	{
		connect (this,
				&FileTransferManager::requested,
				this,
				&FileTransferManager::HandleRequest);
	}

	bool FileTransferManager::IsAvailable () const
	{
		return !ToxThread_.expired ();
	}

	QObject* FileTransferManager::SendFile (const QString& id,
			const QString&, const QString& name, const QString&)
	{
		const auto toxThread = ToxThread_.lock ();
		if (!toxThread)
		{
			qWarning () << Q_FUNC_INFO
					<< "Tox thread is not available";
			return nullptr;
		}

		const auto contact = Acc_->GetByAzothId (id);
		if (!contact)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to find contact by the ID"
					<< id;
			return nullptr;
		}

		const auto transfer = new FileTransferOut { id, contact->GetPubKey (), name, toxThread };
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

	void FileTransferManager::HandleToxThreadChanged (const std::shared_ptr<ToxThread>& thread)
	{
		ToxThread_ = thread;
		if (!thread)
			return;

		const auto cbMgr = thread->GetCallbackManager ();
		cbMgr->Register<tox_callback_file_recv_control> (this,
				[] (FileTransferManager *pThis, uint32_t friendNum, uint32_t fileNum, TOX_FILE_CONTROL ctrl)
					{ pThis->gotFileControl (friendNum, fileNum, ctrl); });
		cbMgr->Register<tox_callback_file_recv> (this,
				[] (FileTransferManager *pThis,
						uint32_t friendNum,
						uint32_t filenum, uint32_t kind, uint64_t filesize,
						const uint8_t *rawFilename, size_t filenameLength)
				{
					const auto thread = pThis->ToxThread_.lock ();
					if (!thread)
					{
						qWarning () << Q_FUNC_INFO
								<< "thread is dead";
						return;
					}

					const auto filenameStr = reinterpret_cast<const char*> (rawFilename);
					const auto name = QString::fromUtf8 (filenameStr, filenameLength);
					Util::Sequence (pThis, thread->GetFriendPubkey (friendNum)) >>
							[=] (const QByteArray& id) { pThis->requested (friendNum, id, filenum, filesize, name); };
				});
		cbMgr->Register<tox_callback_file_recv_chunk> (this,
				[] (FileTransferManager *pThis,
						uint32_t friendNum, uint32_t fileNum, uint64_t position,
						const uint8_t *rawData, size_t rawSize)
				{
					const QByteArray data
					{
						reinterpret_cast<const char*> (rawData),
						static_cast<int> (rawSize)
					};
					pThis->gotData (friendNum, fileNum, data, position);
				});
		cbMgr->Register<tox_callback_file_chunk_request> (this, &FileTransferManager::gotChunkRequest);
	}

	void FileTransferManager::HandleRequest (uint32_t friendNum,
			const QByteArray& pkey, uint32_t filenum, uint64_t size, const QString& name)
	{
		const auto toxThread = ToxThread_.lock ();
		if (!toxThread)
		{
			qWarning () << Q_FUNC_INFO
					<< "Tox thread is not available";
			return;
		}

		const auto entry = Acc_->GetByPubkey (pkey);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to find entry for pubkey"
					<< pkey;
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
			toxThread
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
}
}
