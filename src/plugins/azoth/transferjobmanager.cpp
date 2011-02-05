/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#include "transferjobmanager.h"
#include <QUrl>
#include <QStandardItemModel>
#include <plugininterface/util.h>
#include "core.h"
#include "interfaces/iclentry.h"

namespace LeechCraft
{
namespace Azoth
{
	TransferJobManager::TransferJobManager (QObject *parent)
	: QObject (parent)
	, SummaryModel_ (new QStandardItemModel (this))
	{
	}

	void TransferJobManager::HandleJob (QObject *jobObj)
	{
		ITransferJob *job = qobject_cast<ITransferJob*> (jobObj);
		if (!job)
		{
			qWarning () << Q_FUNC_INFO
					<< jobObj
					<< "is not an ITransferJob";
			return;
		}

		connect (jobObj,
				SIGNAL (errorAppeared (TransferError, const QString&)),
				this,
				SLOT (handleXferError (TransferError, const QString&)));
		connect (jobObj,
				SIGNAL (stateChanged (TransferState)),
				this,
				SLOT (handleStateChanged (TransferState)));
	}

	QAbstractItemModel* TransferJobManager::GetSummaryModel () const
	{
		return SummaryModel_;
	}

	void TransferJobManager::handleXferError (TransferError error, const QString& message)
	{
		ITransferJob *job = qobject_cast<ITransferJob*> (sender ());
		if (!job)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not an ITransferJob";
			return;
		}

		QObject *entryObj = Core::Instance ().GetEntry (job->GetSourceID ());
		const QString& other = entryObj ?
				qobject_cast<ICLEntry*> (entryObj)->GetHumanReadableID () :
				job->GetSourceID ();

		QString str;
		if (job->GetDirection () == TDIn)
			str = tr ("Unable to transfer file from %1.")
					.arg (other);
		else
			str = tr ("Unable to transfer file to %1.")
					.arg (other);

		str += " ";

		switch (error)
		{
		case TEAborted:
			str += tr ("Transfer aborted.");
			break;
		case TEFileAccessError:
			str += tr ("Error accessing file.");
			break;
		case TEFileCorruptError:
			str += tr ("File is corrupted.");
			break;
		case TEProtocolError:
			str += tr ("Protocol error.");
			break;
		}

		if (!message.isEmpty ())
			str += " " + message;

		const Entity& e = Util::MakeNotification ("Azoth",
				str,
				error == TEAborted ? PWarning_ : PCritical_);
		Core::Instance ().SendEntity (e);
	}

	void TransferJobManager::handleStateChanged (TransferState state)
	{
		ITransferJob *job = qobject_cast<ITransferJob*> (sender ());
		if (!job)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not an ITransferJob";
			return;
		}

		QObject *entryObj = Core::Instance ().GetEntry (job->GetSourceID ());
		const QString& name = entryObj ?
				qobject_cast<ICLEntry*> (entryObj)->GetHumanReadableID () :
				job->GetSourceID ();
		QString msg;
		switch (state)
		{
		case TSOffer:
			msg = tr ("Transfer of file %1 with %2 has been offered.")
					.arg (job->GetName ())
					.arg (name);
			break;
		case TSStarting:
			msg = tr ("Transfer of file %1 with %2 is being started...")
					.arg (job->GetName ())
					.arg (name);
			break;
		case TSTransfer:
			msg = tr ("Transfer of file %1 with %2 is started.")
					.arg (job->GetName ())
					.arg (name);
			break;
		case TSFinished:
			msg = tr ("Transfer of file %1 with %2 is finished.")
					.arg (job->GetName ())
					.arg (name);
			break;
		}

		const Entity& e = Util::MakeNotification ("Azoth",
				msg,
				PInfo_);
		Core::Instance ().SendEntity (e);

		if (job->GetDirection () == TDIn &&
				state == TSFinished)
		{
			const Entity& e = Util::MakeEntity (QUrl::fromLocalFile (job->GetName ()),
					QString (),
					static_cast<TaskParameters> (IsDownloaded | FromUserInitiated | OnlyHandle));
			Core::Instance ().SendEntity (e);
		}
	}
}
}
