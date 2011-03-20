/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "transferjob.h"
#include <QFile>
#include <plugininterface/util.h>
#include "core.h"
#include "clientconnection.h"
#include "transfermanager.h"

namespace LeechCraft
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
		QString jid;
		QString var;
		ClientConnection::Split (Job_->jid (), &jid, &var);
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

	TransferDirection TransferJob::GetDirection () const
	{
		switch (Job_->direction ())
		{
		case QXmppTransferJob::OutgoingDirection:
			return TDOut;
		case QXmppTransferJob::IncomingDirection:
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
