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

#include "transferjobmanager.h"
#include <boost/bind.hpp>
#include <QUrl>
#include <QStandardItemModel>
#include <QDesktopServices>
#include <QMessageBox>
#include <QFileDialog>
#include <plugininterface/util.h>
#include "interfaces/iclentry.h"
#include "core.h"
#include "notificationactionhandler.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
	TransferJobManager::TransferJobManager (QObject *parent)
	: QObject (parent)
	, SummaryModel_ (new QStandardItemModel (this))
	{
	}

	void TransferJobManager::AddAccountManager (QObject *mgrObj)
	{
		ITransferManager *mgr = qobject_cast<ITransferManager*> (mgrObj);
		if (!mgr)
		{
			qWarning () << Q_FUNC_INFO
					<< mgrObj
					<< "could not be casted to ITransferManager";
			return;
		}

		connect (mgrObj,
				SIGNAL (fileOffered (QObject*)),
				this,
				SLOT (handleFileOffered (QObject*)));
	}
	
	QObjectList TransferJobManager::GetPendingIncomingJobsFor (const QString& id)
	{
		return Entry2Incoming_ [id];
	}

	namespace
	{
		QString GetContactName (const QString& id)
		{
			QObject *entryObj = Core::Instance ().GetEntry (id);
			return entryObj ?
					qobject_cast<ICLEntry*> (entryObj)->GetHumanReadableID () :
					id;
		}
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

		QList<QStandardItem*> items;

		const QString& name = (job->GetDirection () == TDIn ?
					tr ("Transferring %1 from %2") :
					tr ("Transferring %1 to %2"))
							.arg (job->GetName ())
							.arg (GetContactName (job->GetSourceID ()));
		items << new QStandardItem (name);
		items << new QStandardItem (tr ("offered"));
		items << new QStandardItem (tr ("%1 of %2 (%3%).")
					.arg (Util::MakePrettySize (0))
					.arg (Util::MakePrettySize (job->GetSize ()))
					.arg (0));
		Object2Status_ [jobObj] = items.at (1);
		Object2Progress_ [jobObj] = items.at (2);

		SummaryModel_->appendRow (items);

		connect (jobObj,
				SIGNAL (errorAppeared (TransferError, const QString&)),
				this,
				SLOT (handleXferError (TransferError, const QString&)));
		connect (jobObj,
				SIGNAL (stateChanged (TransferState)),
				this,
				SLOT (handleStateChanged (TransferState)));
		connect (jobObj,
				SIGNAL (transferProgress (qint64, qint64)),
				this,
				SLOT (handleXferProgress (qint64, qint64)));
	}

	QString TransferJobManager::CheckSavePath (QString path)
	{
		QFileInfo pathInfo (path);
		if (!pathInfo.exists () ||
			!pathInfo.isDir () ||
			!pathInfo.isWritable ())
		{
			if (QMessageBox::warning (0,
					"Azoth",
					tr ("Default path for incoming files doesn't exist, is not a directory or is unwritable. "
						"Would you like to adjust the path now? Refusing will abort the transfer."),
					QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
				return QString ();

			path = QFileDialog::getSaveFileName (0,
					tr ("Select default path for incoming files"),
					path);
			
			if (!path.isEmpty ())
				XmlSettingsManager::Instance ().setProperty ("DefaultXferSavePath", path);
		}

		return path;
	}

	void TransferJobManager::AcceptJob (QObject *jobObj, QString path)
	{
		ITransferJob *job = qobject_cast<ITransferJob*> (jobObj);
		if (!job)
		{
			qWarning () << Q_FUNC_INFO
					<< jobObj
					<< "is not an ITransferJob";
			return;
		}

		if (path.isEmpty ())
		{
			path = XmlSettingsManager::Instance ().property ("DefaultXferSavePath").toString ();
			const QString& homePath = QDir::homePath ();
			if (!QFileInfo (path).exists () &&
					path.startsWith (homePath))
			{
				QDir dir = QDir::home ();
				QString relPath = path.mid (homePath.size ());
				if (relPath.at (0) == '/')
					relPath = relPath.mid (1);
				dir.mkpath (relPath);
			}

			path = CheckSavePath (path);
			if (path.isEmpty ())
			{
				DenyJob (jobObj);
				return;
			}
		}

		HandleDeoffer (jobObj);

		HandleJob (jobObj);

		job->Accept (path);
	}

	void TransferJobManager::DenyJob (QObject *jobObj)
	{
		ITransferJob *job = qobject_cast<ITransferJob*> (jobObj);
		if (!job)
		{
			qWarning () << Q_FUNC_INFO
					<< jobObj
					<< "is not an ITransferJob";
			return;
		}

		HandleDeoffer (jobObj);

		job->Abort ();
		sender ()->deleteLater ();
	}

	QAbstractItemModel* TransferJobManager::GetSummaryModel () const
	{
		return SummaryModel_;
	}
	
	void TransferJobManager::HandleDeoffer (QObject *jobObj)
	{
		ITransferJob *job = qobject_cast<ITransferJob*> (jobObj);
		if (!job)
		{
			qWarning () << Q_FUNC_INFO
					<< jobObj
					<< "could not be casted to ITransferJob";
			return;
		}
		if (Entry2Incoming_ [job->GetSourceID ()].removeAll (jobObj))
			emit jobNoLongerOffered (jobObj);
	}

	void TransferJobManager::handleFileOffered (QObject *jobObj)
	{
		ITransferJob *job = qobject_cast<ITransferJob*> (jobObj);
		if (!job)
		{
			qWarning () << Q_FUNC_INFO
					<< jobObj
					<< "could not be casted to ITransferJob";
			return;
		}
		
		const QString& id = job->GetSourceID ();
		
		Entry2Incoming_ [id] << jobObj;

		Entity e = Util::MakeNotification ("Azoth",
				tr ("File %1 (%2) offered from %3.")
					.arg (job->GetName ())
					.arg (Util::MakePrettySize (job->GetSize ()))
					.arg (GetContactName (id)),
				PInfo_);
		NotificationActionHandler *nh =
				new NotificationActionHandler (e, this);
		nh->AddFunction ("Accept",
				boost::bind (&TransferJobManager::AcceptJob, this, jobObj, QString ()));
		nh->AddFunction ("Deny",
				boost::bind (&TransferJobManager::DenyJob, this, jobObj));
		Core::Instance ().SendEntity (e);
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
		
		HandleDeoffer (sender ());

		const QString& other = GetContactName (job->GetSourceID ());

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
		case TENoError:
			str += tr ("No error.");
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

		const QString& name = GetContactName (job->GetSourceID ());
		QString msg;
		QString status;
		switch (state)
		{
		case TSOffer:
			msg = tr ("Transfer of file %1 with %2 has been offered.")
					.arg (job->GetName ())
					.arg (name);
			status = tr ("offered");
			break;
		case TSStarting:
			msg = tr ("Transfer of file %1 with %2 is being started...")
					.arg (job->GetName ())
					.arg (name);
			status = tr ("starting");
			break;
		case TSTransfer:
			msg = tr ("Transfer of file %1 with %2 is started.")
					.arg (job->GetName ())
					.arg (name);
			status = tr ("transferring");
			break;
		case TSFinished:
			msg = tr ("Transfer of file %1 with %2 is finished.")
					.arg (job->GetName ())
					.arg (name);
			break;
		}
		
		if (state != TSOffer)
			HandleDeoffer (sender ());

		if (state != TSFinished)
			Object2Status_ [sender ()]->setText (status);
		else
		{
			SummaryModel_->removeRow (Object2Status_ [sender ()]->row ());
			Object2Status_.remove (sender ());
			Object2Progress_.remove (sender ());
			sender ()->deleteLater ();
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

	void TransferJobManager::handleXferProgress (qint64 done, qint64 total)
	{
		if (!Object2Progress_.contains (sender ()))
			return;

		Object2Progress_ [sender ()]->setText (tr ("%1 of %2 (%3%).")
					.arg (Util::MakePrettySize (done))
					.arg (Util::MakePrettySize (total))
					.arg (done * 100 / total));
	}
}
}
