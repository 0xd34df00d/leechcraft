/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filetransferin.h"
#include <QFile>
#include <tox/tox.h>
#include "toxthread.h"
#include "util.h"

namespace LC::Azoth::Sarin
{
	FileTransferIn::FileTransferIn (Pubkey pubkey,
			quint32 friendNum,
			quint32 fileNum,
			quint64 filesize,
			const std::shared_ptr<ToxRunner>& tox,
			QObject *parent)
	: FileTransferBase { pubkey, tox, parent }
	, FriendNum_ { friendNum }
	, FileNum_ { fileNum }
	, Filesize_ { filesize }
	{
	}

	void FileTransferIn::Accept (const QString& outName)
	{
		File_ = std::make_shared<QFile> (outName);
		if (!File_->open (QIODevice::WriteOnly))
		{
			qWarning () << "unable to open" << outName << "for write" << File_->errorString ();
			using namespace Transfers;
			emit Emitter_.stateChanged (Error { ErrorReason::FileInaccessible, File_->errorString () });
			return;
		}

		// TODO proper error handling
		Tox_->RunWithError (&tox_file_control, FriendNum_, FileNum_, TOX_FILE_CONTROL_RESUME);
	}

	void FileTransferIn::Abort ()
	{
		// TODO proper error handling
		Tox_->RunWithError (&tox_file_control, FriendNum_, FileNum_, TOX_FILE_CONTROL_CANCEL);
	}

	void FileTransferIn::HandleData (quint32 friendNum, quint32 fileNum, const QByteArray& data, uint64_t position)
	{
		if (friendNum != FriendNum_ || fileNum != FileNum_)
			return;

		if (position == Filesize_ || data.isEmpty ())
		{
			emit Emitter_.transferProgress (Filesize_, Filesize_);
			emit Emitter_.stateChanged (Transfers::Phase::Finished);
			return;
		}

		File_->seek (position);
		File_->write (data);
		emit Emitter_.transferProgress (File_->pos (), Filesize_);
	}

	void FileTransferIn::HandleFileControl (uint32_t friendNum, uint32_t fileNum, int type)
	{
		if (friendNum != FriendNum_ || fileNum != FileNum_)
			return;

		switch (type)
		{
		case TOX_FILE_CONTROL_CANCEL:
			using namespace Transfers;
			emit Emitter_.stateChanged (Error { ErrorReason::Aborted, tr ("Remote party aborted the transfer.") });
			break;
		default:
			qWarning () << "unknown filecontrol type" << type;
			break;
		}
	}
}
