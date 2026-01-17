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
	FileTransferIn::FileTransferIn (const QString& azothId,
			const QByteArray& pubkey,
			quint32 friendNum,
			quint32 fileNum,
			quint64 filesize,
			const QString& offeredName,
			const std::shared_ptr<ToxRunner>& tox,
			QObject *parent)
	: FileTransferBase { azothId, pubkey, tox, parent }
	, FriendNum_ { friendNum }
	, FileNum_ { fileNum }
	, Filename_ { offeredName }
	, Filesize_ { filesize }
	{
	}

	QString FileTransferIn::GetName () const
	{
		return Filename_;
	}

	qint64 FileTransferIn::GetSize () const
	{
		return Filesize_;
	}

	TransferDirection FileTransferIn::GetDirection () const
	{
		return TDIn;
	}

	void FileTransferIn::Accept (const QString& dirName)
	{
		const auto& outName = dirName + '/' + Filename_;
		File_ = std::make_shared<QFile> (outName);
		if (!File_->open (QIODevice::WriteOnly))
		{
			qWarning () << "unable to open" << outName << "for write" << File_->errorString ();
			emit errorAppeared (TEFileAccessError, File_->errorString ());
			emit stateChanged (TSFinished);
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
			emit transferProgress (Filesize_, Filesize_);
			emit stateChanged (TSFinished);
			return;
		}

		File_->seek (position);
		File_->write (data);
		emit transferProgress (File_->pos (), Filesize_);
	}

	void FileTransferIn::HandleFileControl (uint32_t friendNum, uint32_t fileNum, int type)
	{
		if (friendNum != FriendNum_ || fileNum != FileNum_)
			return;

		switch (type)
		{
		case TOX_FILE_CONTROL_CANCEL:
			emit errorAppeared (TEAborted, tr ("Remote party aborted the transfer."));
			emit stateChanged (TSFinished);
			break;
		default:
			qWarning () << "unknown filecontrol type" << type;
			break;
		}
	}
}
