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
#include <interfaces/an/entityfields.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <util/sll/visitor.h>
#include <util/threads/futures.h>
#include <util/xpc/notificationactionhandler.h>
#include <util/xpc/util.h>
#include <util/util.h>
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/iaccount.h"
#include "util/azoth/emitters/transfermanager.h"
#include "components/dialogs/filesenddialog.h"
#include "components/util/misc.h"
#include "../../core.h"
#include "../../xmlsettingsmanager.h"
#include "../../avatarsmanager.h"

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
		const auto mgr = qobject_cast<ITransferManager*> (mgrObj);
		if (!mgr)
		{
			qWarning () << mgrObj << "is not a ITransferManager";
			return;
		}

		connect (mgrObj,
				&QObject::destroyed,
				this,
				[this, mgr]
				{
					for (auto& entryIncomingOffers : Entry2Incoming_)
					{
						const auto it = std::ranges::partition (entryIncomingOffers,
								[mgr] (const IncomingOffer& offer) { return offer.Manager_ != mgr; });
						for (const auto& offer : it)
							NotifyDeoffer (offer);
						entryIncomingOffers.erase (it.begin (), it.end ());
					}
				});

		const auto& emitter = mgr->GetTransferManagerEmitter ();
		connect (&emitter,
				&Emitters::TransferManager::fileOffered,
				this,
				&TransferJobManager::HandleFileOffered);
	}

	QList<IncomingOffer> TransferJobManager::GetIncomingOffers (const QString& id)
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

		QString GetRowLabelTemplate (const TransferJobManager::JobContext& context)
		{
			return Util::Visit (context.Dir_,
					[] (TransferJobManager::JobContext::In) { return TransferJobManager::tr ("Receiving %1 from %2"); },
					[] (TransferJobManager::JobContext::Out) { return TransferJobManager::tr ("Sending %1 to %2"); });
		}

		QString GetFilename (const TransferJobManager::JobContext& context)
		{
			return Util::Visit (context.Dir_,
					[] (TransferJobManager::JobContext::In in) { return QFileInfo { in.SavePath_ }.fileName (); },
					[&] (TransferJobManager::JobContext::Out) { return context.OrigFilename_; });
		}
	}

	void TransferJobManager::HandleJob (ITransferJob *job, const JobContext& context)
	{
		const auto& filename = GetFilename (context);
		QList items
		{
			new QStandardItem (GetRowLabelTemplate (context).arg (filename, context.EntryName_)),
			new QStandardItem (tr ("offered")),
			new QStandardItem (tr ("%1 of %2 (%3%).")
					.arg (Util::MakePrettySize (0))
					.arg (Util::MakePrettySize (context.Size_))
					.arg (0))
		};
		for (const auto item : items)
		{
			item->setData (QVariant::fromValue<QToolBar*> (ReprBar_), RoleControls);
			item->setData (QVariant::fromValue<ITransferJob*> (job), MRJobObject);
			item->setEditable (false);
		}

		const auto statusItem = items.at (JobHolderColumn::JobStatus);
		const auto progressItem = items.at (JobHolderColumn::JobProgress);
		progressItem->setData (QVariant::fromValue<ProcessStateInfo> ({
					0,
					context.Size_,
					FromUserInitiated
				}),
				JobHolderRole::ProcessState);

		SummaryModel_->appendRow (items);

		auto& emitter = job->GetTransferJobEmitter ();
		connect (&emitter,
				&Emitters::TransferJob::stateChanged,
				this,
				[this, context, statusItem] (const TransferState& state) { HandleStateChanged (state, context, statusItem); });
		connect (&emitter,
				&Emitters::TransferJob::transferProgress,
				this,
				[progressItem] (qint64 done, qint64 total)
				{
					if (total > 0)
					{
						progressItem->setText (tr ("%1 of %2 (%3%).")
									.arg (Util::MakePrettySize (done))
									.arg (Util::MakePrettySize (total))
									.arg (done * 100 / total));
						Util::SetJobHolderProgress (progressItem, done, total);
					}
				});
	}

	namespace
	{
		QString GetDefaultPath ()
		{
			auto path = XmlSettingsManager::Instance ().property ("DefaultXferSavePath").toString ();
			if (const QFileInfo pathInfo { path };
				!pathInfo.exists () && !QDir {}.mkpath (path))
			{
				qWarning () << "unable to create" << path;
				return {};
			}

			if (const QFileInfo pathInfo { path };
				pathInfo.exists () && pathInfo.isDir () && pathInfo.isWritable ())
				return path;

			if (QMessageBox::warning (nullptr,
					"Azoth",
					TransferJobManager::tr ("Default path for incoming files doesn't exist, is not a directory or is unwritable. "
						"Would you like to change the path now? Refusing will abort the transfer."),
					QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
				return QString ();

			path = QFileDialog::getExistingDirectory (nullptr,
					TransferJobManager::tr ("Select default path for incoming files"),
					path);
			if (!path.isEmpty ())
				XmlSettingsManager::Instance ().setProperty ("DefaultXferSavePath", path);

			return path;
		}

		QString GetSavePath (const QString& dirPath, const QString& filename)
		{
			if (const QDir dir { dirPath };
				!dir.exists (filename))
				return dir.filePath (filename);

			return QFileDialog::getSaveFileName (nullptr,
					TransferJobManager::tr ("Select save path"),
					dirPath);
		}
	}

	void TransferJobManager::AcceptOffer (const IncomingOffer& offer, QString savePath)
	{
		if (savePath.isEmpty ())
			savePath = GetSavePath (GetDefaultPath (), offer.Name_);

		if (savePath.isEmpty ())
		{
			DeclineOffer (offer);
			return;
		}

		const auto entry = GetContact (offer.EntryId_);
		if (!entry)
		{
			DeclineOffer (offer);
			const auto& e = Util::MakeNotification ("Azoth",
					tr ("Unable to accept %1: the contact is no longer available.").arg (offer.Name_),
					Priority::Critical);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
			return;
		}

		if (const auto job = offer.Manager_->Accept (offer, savePath))
			HandleJob (job,
					{
						.Dir_ = JobContext::In { savePath },
						.OrigFilename_ = offer.Name_,
						.Size_ = offer.Size_,
						.EntryName_ = entry->GetEntryName (),
						.EntryId_ = entry->GetEntryID (),
					});
		else
		{
			qWarning () << "unable to accept job";
			const auto& e = Util::MakeNotification ("Azoth",
					tr ("Unable to accept %1.").arg (offer.Name_),
					Priority::Critical);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
		}

		Deoffer (offer);
	}

	void TransferJobManager::DeclineOffer (const IncomingOffer& offer)
	{
		offer.Manager_->Decline (offer);
		Deoffer (offer);
	}

	QAbstractItemModel* TransferJobManager::GetSummaryModel () const
	{
		return SummaryModel_;
	}

	bool TransferJobManager::SendFile (const OutgoingFileOffer& offer)
	{
		const auto acc = offer.Entry_.GetParentAccount ();
		const auto mgr = qobject_cast<ITransferManager*> (acc->GetTransferManager ());
		if (!mgr)
		{
			qWarning () << offer.Entry_.GetHumanReadableID () << "does not support transfers";
			return false;
		}

		const auto job = mgr->SendFile (offer.Entry_.GetEntryID (), offer.Variant_, offer.FilePath_, offer.Comment_);
		if (!job)
		{
			qWarning () << offer.Entry_.GetHumanReadableID () << "unable to create job";
			return false;
		}

		const QFileInfo info { offer.FilePath_ };
		HandleJob (job,
				{
					.Dir_ = JobContext::Out {},
					.OrigFilename_ = info.fileName (),
					.Size_ = info.size (),
					.EntryName_ = offer.Entry_.GetEntryName (),
					.EntryId_ = offer.Entry_.GetEntryID (),
				});
		return true;
	}

	void TransferJobManager::Deoffer (const IncomingOffer& offer)
	{
		if (Entry2Incoming_ [offer.EntryId_].removeOne (offer))
			NotifyDeoffer (offer);
	}

	void TransferJobManager::NotifyDeoffer (const IncomingOffer& offer)
	{
		auto e = Util::MakeNotification ("Azoth", {}, Priority::Info);
		e.Additional_ [AN::EF::SenderID] = "org.LeechCraft.Azoth";
		e.Additional_ [AN::EF::EventID] = "org.LC.Plugins.Azoth.IncomingFileFrom/" + offer.EntryId_ + '/' + offer.Name_;
		e.Additional_ [AN::EF::EventCategory] = AN::CatEventCancel;
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);

		emit jobNoLongerOffered (offer);

		if (const auto entry = GetContact (offer.EntryId_))
		{
			Core::Instance ().IncreaseUnreadCount (entry, -1);
			Core::Instance ().CheckFileIcon (offer.EntryId_);
		}
	}

	void TransferJobManager::HandleIncomingFinished (const JobContext& context, const JobContext::In& inInfo)
	{
		const auto& fileUrl = QUrl::fromLocalFile (inInfo.SavePath_);
		const auto& openEntity = Util::MakeEntity (fileUrl,
				{},
				IsDownloaded | FromUserInitiated | OnlyHandle);
		auto opener = [openEntity] { GetProxyHolder ()->GetEntityManager ()->HandleEntity (openEntity); };
		if (XmlSettingsManager::Instance ().property ("AutoOpenIncomingFiles").toBool ())
			opener ();

		auto e = Util::MakeAN ("Azoth",
				tr ("Received file from %1: %2.")
					.arg (context.EntryName_, QFileInfo { inInfo.SavePath_ }.fileName ()),
				Priority::Info,
				"org.LeechCraft.Azoth",
				AN::CatDownloads,
				AN::TypeDownloadFinished,
				"org.LC.Plugins.Azoth.IncomingFileFinished/" + context.EntryId_ + '/' + context.OrigFilename_,
				{ context.EntryName_, context.OrigFilename_ });
		auto nh = new Util::NotificationActionHandler { e, this };
		nh->AddFunction (tr ("Open"), opener);
		nh->AddFunction (tr ("Open externally"), [fileUrl] { QDesktopServices::openUrl (fileUrl); });

		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	void TransferJobManager::HandleFileOffered (const IncomingOffer& offer)
	{
		Entry2Incoming_ [offer.EntryId_] << offer;

		const auto entry = GetContact (offer.EntryId_);
		if (!entry)
		{
			qWarning () << "unknown contact for" << offer.EntryId_;
			return;
		}

		const auto e = Util::MakeNotification ("Azoth",
				tr ("File %1 (%2) offered from %3.")
					.arg (offer.Name_)
					.arg (Util::MakePrettySize (offer.Size_))
					.arg (entry->GetHumanReadableID ()),
				Priority::Info);

		Util::Sequence (this, BuildNotification (AvatarsMgr_, e, entry)) >>
				[this, entry, offer] (Entity e)
				{
					e.Additional_ [AN::EF::EventType] = AN::TypeIMIncFile;
					e.Additional_ [AN::EF::EventID] = "org.LC.Plugins.Azoth.IncomingFileFrom/" + entry->GetEntryID () + "/" + offer.Name_;
					e.Additional_ [AN::EF::VisualPath] = QStringList { entry->GetEntryName (), offer.Name_ };
					e.Additional_ [AN::EF::DeltaCount] = 1;
					e.Additional_ [AN::EF::ExtendedText] = tr ("Incoming file: %1")
								.arg (offer.Description_.isEmpty () ? offer.Name_ : offer.Description_);

					auto nh = new Util::NotificationActionHandler { e };
					nh->AddFunction (tr ("Accept"), [this, offer] { AcceptOffer (offer, {}); });
					nh->AddFunction (tr ("Deny"), [this, offer] { DeclineOffer (offer); });
					nh->AddDependentObject (this);

					GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
				};

		Core::Instance ().IncreaseUnreadCount (entry, 1);
		Core::Instance ().CheckFileIcon (offer.EntryId_);
	}

	namespace
	{
		QString XferError2Str (Transfers::ErrorReason error)
		{
			using enum Transfers::ErrorReason;
			switch (error)
			{
			case Aborted:
				return TransferJobManager::tr ("Transfer aborted.");
			case FileInaccessible:
				return TransferJobManager::tr ("Error accessing file.");
			case FileCorrupted:
				return TransferJobManager::tr ("File is corrupted.");
			case ProtocolError:
				return TransferJobManager::tr ("Protocol error.");
			}

			qWarning () << "unknown error" << static_cast<int> (error);
			return {};
		}

		QString GetErrorMessageTemplate (const TransferJobManager::JobContext& context)
		{
			return Util::Visit (context.Dir_,
					[] (TransferJobManager::JobContext::In) { return TransferJobManager::tr ("Unable to receive file from %1."); },
					[] (TransferJobManager::JobContext::Out) { return TransferJobManager::tr ("Unable to send file to %1."); });
		}

		QString GetNotificationMessageTemplate (Transfers::Phase phase)
		{
			using enum Transfers::Phase;
			switch (phase)
			{
			case Starting:
				return TransferJobManager::tr ("Transfer of %1 with %2 is being started...");
			case Transferring:
				return TransferJobManager::tr ("Transfer of %1 with %2 is started.");
			case Finished:
				return TransferJobManager::tr ("Transfer of %1 with %2 has finished.");
			}

			qWarning () << "unhandled state" << static_cast<int> (phase);
			return {};
		}

		QString GetStatusString (Transfers::Phase state)
		{
			using enum Transfers::Phase;
			switch (state)
			{
			case Starting:
				return TransferJobManager::tr ("starting");
			case Transferring:
				return TransferJobManager::tr ("transferring");
			case Finished:
				return {};
			}

			qWarning () << "unhandled state" << static_cast<int> (state);
			return {};
		}
	}

	void TransferJobManager::HandleStateChanged (const TransferState& state, const JobContext& context, QStandardItem *status)
	{
		Util::Visit (state,
				[&] (Transfers::Phase phase)
				{
					if (const auto in = std::get_if<JobContext::In> (&context.Dir_);
						in && phase == Transfers::Phase::Finished)
						HandleIncomingFinished (context, *in);
					else
					{
						status->setText (GetStatusString (phase));
						const auto& e = Util::MakeNotification ("Azoth",
								GetNotificationMessageTemplate (phase).arg (GetFilename (context), context.EntryName_),
								Priority::Info);
						GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
					}
				},
				[&] (const Transfers::Error& error)
				{
					auto str = GetErrorMessageTemplate (context).arg (context.EntryName_);
					str += ' ' + XferError2Str (error.Reason_);
					if (!error.Message_.isEmpty ())
						str += ' ' + error.Message_;

					const auto& e = Util::MakeNotification ("Azoth",
							str,
							error.Reason_ == Transfers::ErrorReason::Aborted ?
									Priority::Warning :
									Priority::Critical);
					GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
				});

		if (IsTerminal (state))
			status->model ()->removeRow (status->row ());
	}

	void TransferJobManager::handleAbortAction ()
	{
		if (!Selected_.isValid ())
			return;

		const auto item = SummaryModel_->itemFromIndex (Selected_);
		if (!item)
		{
			qWarning () << "null item for index" << Selected_;
			return;
		}

		if (const auto job = item->data (MRJobObject).value<ITransferJob*> ())
			job->Abort ();
		else
			qWarning () << "null transfer job for" << Selected_;
	}
}
}
