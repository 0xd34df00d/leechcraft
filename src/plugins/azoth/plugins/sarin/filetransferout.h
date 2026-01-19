/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QFile>
#include <util/threads/coro/taskfwd.h>
#include "filetransferbase.h"

namespace LC::Azoth::Sarin
{
	class FileTransferOut final : public FileTransferBase
	{
		const QString FilePath_;

		uint32_t FriendNum_ = -1;
		uint32_t FileNum_ = -1;

		enum class State
		{
			Idle,
			Waiting,
			Transferring,
			Paused
		} State_ = State::Waiting;

		QFile File_;
		qint64 Filesize_;

		bool TransferAllowed_ = true;
	public:
		FileTransferOut (const QString& azothId,
				Pubkey pubkey,
				const QString& filename,
				const std::shared_ptr<ToxRunner>& tox,
				QObject *parent = nullptr);

		QString GetName () const override;
		qint64 GetSize () const override;
		TransferDirection GetDirection () const override;

		void Accept (const QString&) override;
		void Abort () override;

		void HandleFileControl (uint32_t, uint32_t, int);
		Util::ContextTask<void> HandleChunkRequested (uint32_t, uint32_t, uint64_t, size_t);
	private:
		Util::ContextTask<void> Start ();

		void HandleAccept ();
		void HandleKill ();
		void HandlePause ();
		void HandleResume ();
	};
}
