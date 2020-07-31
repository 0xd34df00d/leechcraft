/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "transferjob.h"
#include <QFile>
#include <util/util.h>
#include "core.h"
#include "clientconnection.h"
#include "transfermanager.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	TransferJob::TransferJob (QXmppTransferJob *job, TransferManager *mgr)
	: QObject (job)
	, Job_ (job)
	, Manager_ (mgr)
	{
		connect (Job_,
				SIGNAL (progress (qint64, qint64)),
				this,
				SIGNAL (transferProgress (qint64,qint64)));
		connect (Job_,
				SIGNAL (error (QXmppTransferJob::Error)),
				this,
				SLOT (handleErrorAppeared (QXmppTransferJob::Error)));
		connect (Job_,
				SIGNAL (stateChanged (QXmppTransferJob::State)),
				this,
				SLOT (handleStateChanged (QXmppTransferJob::State)));
	}

	QString TransferJob::GetSourceID () const
	{
		auto [jid, var] = ClientConnection::Split (Job_->jid ());
		return Manager_->GetAccount ()->GetAccountID () + '_' + jid;
	}

	QString TransferJob::GetName () const
	{
		return Job_->fileInfo ().name ();
	}

	qint64 TransferJob::GetSize () const
	{
		return Job_->fileInfo ().size ();
	}

	QString TransferJob::GetComment () const
	{
		return Job_->fileInfo ().description ();
	}

	TransferDirection TransferJob::GetDirection () const
	{
		switch (Job_->direction ())
		{
		case QXmppTransferJob::OutgoingDirection:
			return TDOut;
		case QXmppTransferJob::IncomingDirection:
			return TDIn;
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown direction"
					<< Job_->direction ();
			return TDIn;
		}
	}

	void TransferJob::Accept (const QString& out)
	{
		const QString& filename = QFileInfo (out).isDir () ?
				QDir (out).filePath (GetName ()) :
				out;

		QFile *file = new QFile (filename);
		if (!file->open (QIODevice::WriteOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "could not open file"
					<< filename
					<< file->errorString ();

			const QString& msg = tr ("could not open incoming file %1: %2")
					.arg (filename)
					.arg (file->errorString ());
			emit errorAppeared (TEFileAccessError, msg);
			return;
		}

		Job_->accept (file);
	}

	void TransferJob::Abort ()
	{
		Job_->abort ();
	}

	void TransferJob::handleErrorAppeared (QXmppTransferJob::Error error)
	{
		qWarning () << Q_FUNC_INFO
				<< error;
		emit errorAppeared (static_cast<TransferError> (error), QString ());
	}

	void TransferJob::handleStateChanged (QXmppTransferJob::State state)
	{
		emit stateChanged (static_cast<TransferState> (state));
	}
}
}
}
