/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "transferjobmanager.h"
#include <QUrl>
#include <QStandardItemModel>
#include <QDesktopServices>
#include <QMessageBox>
#include <QFileDialog>
#include <QToolBar>
#include <QAction>
#include <interfaces/ijobholder.h>
#include <interfaces/an/constants.h>
#include <util/util.h>
#include <util/xpc/notificationactionhandler.h>
#include <util/xpc/util.h>
#include <util/threads/futures.h>
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/iaccount.h"
#include "components/dialogs/filesenddialog.h"
#include "core.h"
#include "xmlsettingsmanager.h"
#include "util.h"
#include "avatarsmanager.h"

Q_DECLARE_METATYPE (QToolBar*)

namespace LC
{
namespace Azoth
{
	TransferJobManager::TransferJobManager (AvatarsManager *am, QObject *parent)
	: QObject { parent }
	, AvatarsMgr_ { am }
	, SummaryModel_ { new QStandardItemModel { this } }
	, ReprBar_ { new QToolBar }
	{
		QAction *abort = new QAction (tr ("Abort"), this);
		abort->setProperty ("ActionIcon", "process-stop");
		connect (abort,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAbortAction ()));
		ReprBar_->addAction (abort);
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

	void TransferJobManager::SelectionChanged (const QModelIndex& idx)
	{
		Selected_ = idx;
	}

	namespace
	{
		ICLEntry* GetContact (const QString& id)
		{
			return qobject_cast<ICLEntry*> (Core::Instance ().GetEntry (id));
		}

		QString GetContactName (const QString& id)
		{
			ICLEntry *contact = GetContact (id);
			return contact ?
					contact->GetHumanReadableID () :
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

		const auto& name = (job->GetDirection () == TDIn ?
					tr ("Transferring %1 from %2") :
					tr ("Transferring %1 to %2"))
							.arg (job->GetName ())
							.arg (GetContactName (job->GetSourceID ()));
		QList<QStandardItem*> items
		{
			new QStandardItem (name),
			new QStandardItem (tr ("offered")),
			new QStandardItem (tr ("%1 of %2 (%3%).")
					.arg (Util::MakePrettySize (0))
					.arg (Util::MakePrettySize (job->GetSize ()))
					.arg (0))
		};
		const auto& barVar = QVariant::fromValue<QToolBar*> (ReprBar_);
		const auto& jobObjVar = QVariant::fromValue<QObject*> (jobObj);
		for (const auto item : items)
		{
			item->setData (barVar, RoleControls);
			item->setData (jobObjVar, MRJobObject);
			item->setEditable (false);
		}
		Object2Status_ [jobObj] = items.at (1);
		Object2Progress_ [jobObj] = items.at (2);

		auto progressItem = items.at (JobHolderColumn::JobProgress);
		progressItem->setData (QVariant::fromValue<ProcessStateInfo> ({
					0,
					job->GetSize (),
					FromUserInitiated
				}),
				JobHolderRole::ProcessState);

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

		Job2SavePath_ [job] = path;
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

	namespace
	{
		void CleanupUrls (QList<QUrl>& urls)
		{
			for (auto i = urls.begin (); i != urls.end (); )
				if (!i->isLocalFile ())
					i = urls.erase (i);
				else
					++i;
		}
	}

	bool TransferJobManager::OfferURLs (ICLEntry *entry, QList<QUrl> urls)
	{
		if (entry->Variants ().isEmpty ())
			return false;

		const auto acc = entry->GetParentAccount ();
		const auto mgr = qobject_cast<ITransferManager*> (acc->GetTransferManager ());
		if (!mgr)
			return false;

		CleanupUrls (urls);

		if (urls.isEmpty ())
			return false;

		if (urls.size () == 1)
		{
			new FileSendDialog (entry, urls.value (0).toLocalFile ());
			return true;
		}

		const auto& text = tr ("Are you sure you want to send %n files to %1?", 0, urls.size ())
				.arg (entry->GetEntryName ());
		if (QMessageBox::question (0,
					"LeechCraft",
					text,
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return false;

		for (const auto& url : urls)
		{
			const QString& path = url.toLocalFile ();
			if (!QFileInfo (path).exists ())
				continue;

			QObject *job = mgr->SendFile (entry->GetEntryID (),
					entry->Variants ().first (),
					path,
					QString ());
			Core::Instance ().GetTransferJobManager ()->HandleJob (job);
		}

		return true;
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
		{
			Entity e = Util::MakeNotification ("Azoth", QString (), Priority::Info);
			e.Additional_ ["org.LC.AdvNotifications.SenderID"] = "org.LeechCraft.Azoth";
			e.Additional_ ["org.LC.AdvNotifications.EventID"] =
					"org.LC.Plugins.Azoth.IncomingFileFrom/" +
						GetContact (job->GetSourceID ())->GetEntryID () +
						"/" + job->GetName ();
			e.Additional_ ["org.LC.AdvNotifications.EventCategory"] =
					"org.LC.AdvNotifications.Cancel";
			Core::Instance ().SendEntity (e);

			emit jobNoLongerOffered (jobObj);
		}
	}

	void TransferJobManager::HandleTaskFinished (ITransferJob *job)
	{
		const auto& path = Job2SavePath_.take (job);
		if (job->GetDirection () != TDIn)
			return;

		const auto& fileUrl = QUrl::fromLocalFile (path + '/' + job->GetName ());
		const auto& openEntity = Util::MakeEntity (fileUrl,
				{},
				IsDownloaded | FromUserInitiated | OnlyHandle);
		auto opener = [openEntity] { Core::Instance ().SendEntity (openEntity); };
		if (XmlSettingsManager::Instance ().property ("AutoOpenIncomingFiles").toBool ())
			opener ();

		const auto entry = GetContact (job->GetSourceID ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown contact for"
					<< job->GetSourceID ();
			return;
		}

		auto e = Util::MakeAN ("Azoth",
				tr ("Received file from %1: %2.")
					.arg (entry->GetEntryName ())
					.arg (QFileInfo { job->GetName () }.fileName ()),
				Priority::Info,
				"org.LeechCraft.Azoth",
				AN::CatDownloads,
				AN::TypeDownloadFinished,
				"org.LC.Plugins.Azoth.IncomingFileFinished/" + entry->GetEntryID () + "/" + job->GetName (),
				{ entry->GetEntryName (), job->GetName () });
		auto nh = new Util::NotificationActionHandler { e, this };
		nh->AddFunction (tr ("Open"), opener);
		nh->AddFunction (tr ("Open externally"),
				[fileUrl] { QDesktopServices::openUrl (fileUrl); });

		Core::Instance ().SendEntity (e);
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
				Priority::Info);

		ICLEntry *entry = GetContact (id);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown contact for"
					<< id;
			return;
		}

		Util::Sequence (jobObj, BuildNotification (AvatarsMgr_, e, entry)) >>
				[this, entry, job, jobObj] (Entity e)
				{
					e.Additional_ ["org.LC.AdvNotifications.EventID"] =
							"org.LC.Plugins.Azoth.IncomingFileFrom/" + entry->GetEntryID () + "/" + job->GetName ();
					e.Additional_ ["org.LC.AdvNotifications.VisualPath"] = QStringList { entry->GetEntryName (), job->GetName () };
					e.Additional_ ["org.LC.AdvNotifications.DeltaCount"] = 1;
					e.Additional_ ["org.LC.AdvNotifications.ExtendedText"] = tr ("Incoming file: %1")
								.arg (job->GetComment ().isEmpty () ?
										job->GetName () :
										job->GetComment ());

					e.Additional_ ["org.LC.AdvNotifications.EventType"] = AN::TypeIMIncFile;

					auto nh = new Util::NotificationActionHandler { e };
					nh->AddFunction (tr ("Accept"), [this, jobObj] { AcceptJob (jobObj, {}); });
					nh->AddFunction (tr ("Deny"), [this, jobObj] { DenyJob (jobObj); });
					nh->AddDependentObject (jobObj);

					Core::Instance ().SendEntity (e);
				};
	}

	namespace
	{
		QString XferError2Str (TransferError error)
		{
			switch (error)
			{
			case TEAborted:
				return TransferJobManager::tr ("Transfer aborted.");
			case TEFileAccessError:
				return TransferJobManager::tr ("Error accessing file.");
			case TEFileCorruptError:
				return TransferJobManager::tr ("File is corrupted.");
			case TEProtocolError:
				return TransferJobManager::tr ("Protocol error.");
			case TENoError:
				return TransferJobManager::tr ("No error.");
			}

			qWarning () << Q_FUNC_INFO
					<< "unknown error"
					<< error;

			return {};
		}
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

		auto str = job->GetDirection () == TDIn ?
			tr ("Unable to transfer file from %1.")
				.arg (other) :
			tr ("Unable to transfer file to %1.")
				.arg (other);

		str += " " + XferError2Str (error);

		if (!message.isEmpty ())
			str += " " + message;

		const Entity& e = Util::MakeNotification ("Azoth",
				str,
				error == TEAborted ? Priority::Warning : Priority::Critical);
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
		{
			Object2Status_ [sender ()]->setText (status);

			const Entity& e = Util::MakeNotification ("Azoth",
					msg,
					Priority::Info);
			Core::Instance ().SendEntity (e);
		}
		else
		{
			SummaryModel_->removeRow (Object2Status_ [sender ()]->row ());
			Object2Status_.remove (sender ());
			Object2Progress_.remove (sender ());
			sender ()->deleteLater ();

			HandleTaskFinished (job);
		}
	}

	void TransferJobManager::handleXferProgress (qint64 done, qint64 total)
	{
		if (!Object2Progress_.contains (sender ()))
			return;

		if (total <= 0)
			return;

		auto progress = Object2Progress_ [sender ()];
		progress->setText (tr ("%1 of %2 (%3%).")
					.arg (Util::MakePrettySize (done))
					.arg (Util::MakePrettySize (total))
					.arg (done * 100 / total));
		Util::SetJobHolderProgress (progress, done, total);
	}

	void TransferJobManager::handleAbortAction ()
	{
		if (!Selected_.isValid ())
			return;

		QStandardItem *item = SummaryModel_->itemFromIndex (Selected_);
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "null item for index"
					<< Selected_;
			return;
		}

		QObject *jobObj = item->data (MRJobObject).value<QObject*> ();
		ITransferJob *job = qobject_cast<ITransferJob*> (jobObj);
		if (!job)
		{
			qWarning () << Q_FUNC_INFO
					<< "null transfer job for"
					<< jobObj
					<< Selected_;
			return;
		}

		job->Abort ();
	}
}
}
