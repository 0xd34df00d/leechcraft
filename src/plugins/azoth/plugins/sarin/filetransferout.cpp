/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filetransferout.h"
#include <tox/tox.h>
#include <util/threads/futures.h>
#include <util/sll/delayedexecutor.h>
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include "toxthread.h"
#include "util.h"
#include "threadexceptions.h"

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	FileTransferOut::FileTransferOut (const QString& azothId,
			const QByteArray& pubkey,
			const QString& filename,
			const std::shared_ptr<ToxThread>& thread,
			QObject *parent)
	: FileTransferBase { azothId, pubkey, thread, parent }
	, FilePath_ { filename }
	, File_ { filename }
	, Filesize_ { File_.size () }
	{
		if (!File_.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open local file"
					<< filename;

			new Util::DelayedExecutor
			{
				[this]
				{
					emit errorAppeared (TEFileAccessError,
							tr ("Error opening local file: %1.")
								.arg (File_.errorString ()));
					emit stateChanged (TransferState::TSFinished);
				}
			};

			return;
		}

		using SendFileResult_t = Util::Either<TOX_ERR_FILE_SEND, uint32_t>;
		auto future = Thread_->ScheduleFunction ([this] (Tox *tox)
				{
					const auto& name = FilePath_.section ('/', -1, -1).toUtf8 ();

					const auto friendNum = GetFriendId (tox, PubKey_);;
					if (!friendNum)
						throw UnknownFriendException {};
					FriendNum_ = *friendNum;

					TOX_ERR_FILE_SEND error {};
					const auto result = tox_file_send (tox,
							FriendNum_,
							TOX_FILE_KIND_DATA,
							static_cast<uint64_t> (Filesize_),
							nullptr,
							reinterpret_cast<const uint8_t*> (name.constData ()),
							name.size (),
							&error);
					return result == UINT32_MAX ?
							SendFileResult_t::Left (error) :
							SendFileResult_t::Right (result);
				});

		Util::Sequence (this, future) >>
				Util::Visitor
				{
					[this] (uint32_t filenum)
					{
						FileNum_ = filenum;
						emit stateChanged (TransferState::TSOffer);
					},
					[this] (TOX_ERR_FILE_SEND err)
					{
						qWarning () << Q_FUNC_INFO
								<< err;
						emit errorAppeared (TransferError::TEProtocolError,
								tr ("Tox file send error: %1")
										.arg (err));
						emit stateChanged (TransferState::TSFinished);
					}
				};
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
				qWarning () << Q_FUNC_INFO
						<< "unexpected control type in Waiting state:"
						<< type;
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
				qWarning () << Q_FUNC_INFO
						<< "unexpected control type in Transferring state:"
						<< type;
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
				qWarning () << Q_FUNC_INFO
						<< "unexpected control type in Killed state:"
						<< type;
				break;
			}
			break;
		case State::Idle:
			qWarning () << Q_FUNC_INFO
					<< "not doing anything in idle state, though we got"
					<< type;
			break;
		}
	}

	void FileTransferOut::HandleChunkRequested (uint32_t friendNum, uint32_t fileNum, uint64_t offset, size_t length)
	{
		if (friendNum != FriendNum_ || fileNum != FileNum_)
			return;

		if (static_cast<uint64_t> (File_.pos ()) != offset)
			if (!File_.seek (offset))
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot seek to"
						<< offset
						<< "while at pos"
						<< File_.pos ();
				return;
			}

		const auto& data = File_.read (length);
		if (static_cast<size_t> (data.size ()) != length)
			qWarning () << Q_FUNC_INFO
					<< "could not read"
					<< length
					<< "bytes, reading"
					<< data.size ()
					<< "instead";

		Thread_->ScheduleFunction ([data, friendNum, fileNum, offset] (Tox *tox)
				{
					TOX_ERR_FILE_SEND_CHUNK error {};
					const auto dataPtr = reinterpret_cast<const uint8_t*> (data.constData ());
					if (!tox_file_send_chunk (tox, friendNum, fileNum,
								offset, dataPtr, data.size (), &error))
						qWarning () << Q_FUNC_INFO
								<< "unable to send file chunk:"
								<< error;
				});
	}
}
}
}
