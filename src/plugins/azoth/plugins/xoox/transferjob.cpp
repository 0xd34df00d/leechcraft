/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "transferjob.h"
#include <QFile>
#include <QTimer>
#include <QXmppTransferManager.h>

namespace LC::Azoth::Xoox
{
	namespace
	{
		TransferState FromQxmpp (QXmppTransferJob::Error error, QXmppTransferJob::State state)
		{
			using namespace Transfers;

			switch (error)
			{
			case QXmppTransferJob::NoError:
				switch (state)
				{
				case QXmppTransferJob::OfferState:
				case QXmppTransferJob::StartState:
					return Phase::Starting;
				case QXmppTransferJob::TransferState:
					return Phase::Transferring;
				case QXmppTransferJob::FinishedState:
					return Phase::Finished;
				}
				break;
			case QXmppTransferJob::AbortError:
				return Error { ErrorReason::Aborted };
			case QXmppTransferJob::FileAccessError:
				return Error { ErrorReason::FileInaccessible };
			case QXmppTransferJob::FileCorruptError:
				return Error { ErrorReason::FileCorrupted };
			case QXmppTransferJob::ProtocolError:
				return Error { ErrorReason::ProtocolError };
			}

			qWarning () << "unknown error or state" << error << state;
			return Phase::Starting;
		}
	}

	TransferJob::TransferJob (std::unique_ptr<QXmppTransferJob> job)
	: Job_ { std::move (job) }
	{
		const auto emitState = [this]
		{
			if (Job_->error () != QXmppTransferJob::NoError)
				qWarning () << Job_->error ();

			const auto state = FromQxmpp (Job_->error (), Job_->state ());
			emit Emitter_.stateChanged (state);
			if (IsTerminal (state))
				deleteLater ();
		};
		connect (&*Job_,
				&QXmppTransferJob::progress,
				&Emitter_,
				&Emitters::TransferJob::transferProgress);
		connect (&*Job_,
				qOverload<QXmppTransferJob::Error> (&QXmppTransferJob::error),
				this,
				emitState);
		connect (&*Job_,
				&QXmppTransferJob::stateChanged,
				this,
				emitState);
	}

	TransferJob::TransferJob (std::unique_ptr<QXmppTransferJob> job, const QString& savePath)
	: TransferJob { std::move (job) }
	{
		Q_ASSERT (job->direction () == QXmppTransferJob::IncomingDirection);

		SaveFile_ = std::make_unique<QFile> (savePath);
		if (!SaveFile_->open (QIODevice::WriteOnly))
		{
			qWarning () << "could not open file" << savePath << SaveFile_->errorString ();

			const auto& msg = tr ("could not open incoming file %1: %2")
					.arg (savePath)
					.arg (SaveFile_->errorString ());
			QTimer::singleShot (0, this,
					[msg, this]
					{
						using namespace Transfers;
						emit Emitter_.stateChanged (Error { ErrorReason::FileInaccessible, msg });
						deleteLater ();
					});
			return;
		}

		Job_->accept (&*SaveFile_);
	}

	TransferJob::~TransferJob () = default;

	Emitters::TransferJob& TransferJob::GetTransferJobEmitter ()
	{
		return Emitter_;
	}

	void TransferJob::Abort ()
	{
		Job_->abort ();
	}
}
