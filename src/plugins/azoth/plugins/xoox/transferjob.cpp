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
		TransferError FromQxmpp (QXmppTransferJob::Error error)
		{
			switch (error)
			{
			case QXmppTransferJob::NoError:
				return TENoError;
			case QXmppTransferJob::AbortError:
				return TEAborted;
			case QXmppTransferJob::FileAccessError:
				return TEFileAccessError;
			case QXmppTransferJob::FileCorruptError:
				return TEFileCorruptError;
			case QXmppTransferJob::ProtocolError:
				return TEProtocolError;
			}

			qWarning () << "unknown error" << error;
			return TENoError;
		}

		TransferState FromQxmpp (QXmppTransferJob::State state)
		{
			switch (state)
			{
			case QXmppTransferJob::OfferState:
			case QXmppTransferJob::StartState:
				return TSStarting;
			case QXmppTransferJob::TransferState:
				return TSTransfer;
			case QXmppTransferJob::FinishedState:
				return TSFinished;
			}

			qWarning () << "unknown state" << state;
			return TSStarting;
		}
	}

	TransferJob::TransferJob (std::unique_ptr<QXmppTransferJob> job)
	: Job_ { std::move (job) }
	{
		connect (&*Job_,
				&QXmppTransferJob::progress,
				&Emitter_,
				&Emitters::TransferJob::transferProgress);
		connect (&*Job_,
				qOverload<QXmppTransferJob::Error> (&QXmppTransferJob::error),
				this,
				[this] (QXmppTransferJob::Error error)
				{
					qWarning () << error;
					emit Emitter_.errorAppeared (FromQxmpp (error), {});
					deleteLater ();
				});
		connect (&*Job_,
				&QXmppTransferJob::stateChanged,
				this,
				[this] (QXmppTransferJob::State state)
				{
					emit Emitter_.stateChanged (FromQxmpp (state));
					if (state == QXmppTransferJob::FinishedState)
						deleteLater ();
				});
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
						emit Emitter_.errorAppeared (TEFileAccessError, msg);
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
