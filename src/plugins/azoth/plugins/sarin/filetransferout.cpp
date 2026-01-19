/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filetransferout.h"
#include <QTimer>
#include <tox/tox.h>
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include <util/threads/coro.h>
#include "toxthread.h"
#include "util.h"

namespace LC::Azoth::Sarin
{
	FileTransferOut::FileTransferOut (const QString& azothId,
			Pubkey pubkey,
			const QString& filename,
			const std::shared_ptr<ToxRunner>& tox,
			QObject *parent)
	: FileTransferBase { azothId, pubkey, tox, parent }
	, FilePath_ { filename }
	, File_ { filename }
	, Filesize_ { File_.size () }
	{
		if (!File_.open (QIODevice::ReadOnly))
		{
			qWarning () << "unable to open local file" << filename;

			QTimer::singleShot (0, this,
					[this]
					{
						emit errorAppeared (TEFileAccessError,
								tr ("Error opening local file: %1.")
									.arg (File_.errorString ()));
						emit stateChanged (TransferState::TSFinished);
					});

			return;
		}

		Start ();
	}

	QString FileTransferOut::GetName () const
	{
		return FilePath_;
	}

	qint64 FileTransferOut::GetSize () const
	{
		return Filesize_;
	}

	TransferDirection FileTransferOut::GetDirection () const
	{
		return TransferDirection::TDOut;
	}

	void FileTransferOut::Accept (const QString&)
	{
	}

	void FileTransferOut::Abort ()
	{
	}

	Util::ContextTask<void> FileTransferOut::Start ()
	{
		co_await Util::AddContextObject { *this };

		const auto friendNum = co_await Tox_->Run (&ToxW::ResolveFriendNum, PubKey_);
		if (!friendNum)
		{
			emit errorAppeared (TEProtocolError, tr ("Unknown friend."));
			emit stateChanged (TSFinished);
			co_return;
		}
		FriendNum_ = *friendNum;

		const auto sendResult = co_await Tox_->Run ([this] (ToxW& tox)
				{
					return WithStrError (&tox_file_send,
							tox.GetTox (),
							FilePath_.section ('/', -1, -1),
							FriendNum_,
							TOX_FILE_KIND_DATA,
							static_cast<uint64_t> (Filesize_),
							nullptr);
				});

		FileNum_ = co_await Util::WithHandler (sendResult,
				[this] (TOX_ERR_FILE_SEND err)
				{
					qWarning () << err;
					emit errorAppeared (TEProtocolError,
							tr ("Tox file send error: %1").arg (tox_err_file_send_to_string (err)));
					emit stateChanged (TSFinished);
				});
		emit stateChanged (TSOffer);
	}

	void FileTransferOut::HandleAccept ()
	{
		State_ = State::Transferring;
		emit stateChanged (TSTransfer);
	}

	void FileTransferOut::HandleKill ()
	{
		TransferAllowed_ = false;
		emit errorAppeared (TEAborted, tr ("Remote party aborted file transfer."));
		emit stateChanged (TSFinished);
		State_ = State::Idle;
	}

	void FileTransferOut::HandlePause ()
	{
		TransferAllowed_ = false;
		State_ = State::Paused;
	}

	void FileTransferOut::HandleResume ()
	{
		TransferAllowed_ = true;
		State_ = State::Transferring;
	}

	void FileTransferOut::HandleFileControl (uint32_t friendNum, uint32_t fileNum, int type)
	{
		if (friendNum != FriendNum_ || fileNum != FileNum_)
			return;

		switch (State_)
		{
		case State::Waiting:
			switch (type)
			{
			case TOX_FILE_CONTROL_RESUME:
				HandleAccept ();
				break;
			case TOX_FILE_CONTROL_CANCEL:
				HandleKill ();
				break;
			default:
				qWarning () << "unexpected control type in Waiting state:" << type;
				break;
			}
			break;
		case State::Transferring:
			switch (type)
			{
			case TOX_FILE_CONTROL_CANCEL:
				HandleKill ();
				break;
			case TOX_FILE_CONTROL_PAUSE:
				HandlePause ();
				break;
			default:
				qWarning () << "unexpected control type in Transferring state:" << type;
				break;
			}
			break;
		case State::Paused:
			switch (type)
			{
			case TOX_FILE_CONTROL_RESUME:
				HandleResume ();
				break;
			case TOX_FILE_CONTROL_CANCEL:
				HandleKill ();
				break;
			default:
				qWarning () << "unexpected control type in Killed state:" << type;
				break;
			}
			break;
		case State::Idle:
			qWarning () << "not doing anything in idle state, though we got" << type;
			break;
		}
	}

	Util::ContextTask<void> FileTransferOut::HandleChunkRequested (uint32_t friendNum, uint32_t fileNum, uint64_t offset, size_t length)
	{
		if (friendNum != FriendNum_ || fileNum != FileNum_)
			co_return;

		if (static_cast<uint64_t> (File_.pos ()) != offset)
			if (!File_.seek (offset))
			{
				qWarning () << "cannot seek to" << offset << "while at pos" << File_.pos ();
				co_return;
			}

		const auto& data = File_.read (length);
		if (static_cast<size_t> (data.size ()) != length)
			qWarning () << "could not read" << length << "bytes, reading" << data.size () << "instead";

		// TODO error handling
		co_await Tox_->RunWithError (&tox_file_send_chunk,
				friendNum, fileNum, offset, std::bit_cast<const uint8_t*> (data.constData ()), data.size ());
	}
}
