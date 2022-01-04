/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "account.h"
#include <stdexcept>
#include <QUuid>
#include <QDataStream>
#include <QInputDialog>
#include <QRecursiveMutex>
#include <QStandardItemModel>
#include <QTimer>
#include <util/xpc/util.h>
#include <util/xpc/passutils.h>
#include <util/sll/slotclosure.h>
#include <util/sll/visitor.h>
#include <util/sll/qtutil.h>
#include <util/sll/prelude.h>
#include <util/threads/monadicfuture.h>
#include <util/xpc/notificationactionhandler.h>
#include <util/gui/sslcertificateinfowidget.h>
#include <interfaces/core/ientitymanager.h>
#include "core.h"
#include "accountconfigdialog.h"
#include "accountthread.h"
#include "accountthreadworker.h"
#include "accountdatabase.h"
#include "storage.h"
#include "accountfoldermanager.h"
#include "mailmodel.h"
#include "foldersmodel.h"
#include "mailmodelsmanager.h"
#include "accountlogger.h"
#include "threadpool.h"
#include "accountthreadnotifier.h"
#include "progresslistener.h"
#include "progressmanager.h"
#include "vmimeconversions.h"
#include "outgoingmessage.h"
#include "messageinfo.h"
#include "messagebodies.h"

Q_DECLARE_METATYPE (QList<QStringList>)
Q_DECLARE_METATYPE (QList<QByteArray>)

namespace LC
{
namespace Snails
{
	namespace
	{
		void HandleCertificateException (IEntityManager *iem, const QString& accountName,
				const vmime::security::cert::certificateException& err)
		{
			auto entity = Util::MakeNotification ("Snails",
					Account::tr ("Connection failed for account %1: certificate check failed. %2")
						.arg ("<em>" + accountName + "</em>")
						.arg (QString::fromUtf8 (err.what ())),
					Priority::Critical);

			const auto& qCerts = ToSslCerts (err.getCertificate ());
			qDebug () << Q_FUNC_INFO
					<< qCerts;
			if (qCerts.size () == 1)
			{
				const auto nah = new Util::NotificationActionHandler { entity };
				nah->AddFunction (Account::tr ("View certificate..."),
						[qCerts]
						{
							const auto dia = Util::MakeCertificateViewerDialog (qCerts.at (0));
							dia->setAttribute (Qt::WA_DeleteOnClose);
							dia->show ();
						});
			}

			iem->HandleEntity (entity);
		}
	}

	Account::Account (const QByteArray& id, const AccountConfig& cfg, const Dependencies& deps)
	: QObject (deps.Parent_)
	, Logger_ (std::make_shared<AccountLogger> (cfg.AccName_))
	, WorkerPool_ (std::make_shared<ThreadPool> (this, deps.Storage_))
	, AccMutex_ (new QRecursiveMutex)
	, ID_ (id)
	, Config_ (cfg)
	, FolderManager_ (new AccountFolderManager (this))
	, FoldersModel_ (new FoldersModel (this))
	, MailModelsManager_ (new MailModelsManager (this, deps.Storage_))
	, NoopNotifier_ (std::make_shared<AccountThreadNotifier<int>> ())
	, ProgressMgr_ (deps.PM_)
	, Storage_ (deps.Storage_)
	{
		Logger_->SetEnabled (cfg.LogToFile_);

		connect (FolderManager_,
				SIGNAL (foldersUpdated ()),
				this,
				SLOT (handleFoldersUpdated ()));

		WorkerPool_->AddThreadInitializer ([this] (AccountThread *t)
				{
					t->Schedule (TaskPriority::Low, &AccountThreadWorker::SetNoopTimeoutChangeNotifier, NoopNotifier_);
				});

		UpdateNoopInterval ();

		Util::Sequence (this, WorkerPool_->TestConnectivity ()) >>
				[this] (const auto& result)
				{
					if (const auto left = result.MaybeLeft ())
					{
						const auto iem = Core::Instance ().GetProxy ()->GetEntityManager ();
						const auto emitErr = [=] (QString text)
						{
							if (!text.endsWith ('.'))
								text += '.';

							iem->HandleEntity (Util::MakeNotification ("Snails",
									tr ("Connection failed for account %1: %2")
										.arg ("<em>" + Config_.AccName_ + "</em>")
										.arg (text),
									Priority::Critical));
						};

						Util::Visit (*left,
								[&] (const vmime::exceptions::authentication_error& err)
								{
									emitErr (tr ("authentication failed: %1")
											.arg (QString::fromUtf8 (err.what ())));
								},
								[&] (const vmime::security::cert::certificateException& err)
								{
									HandleCertificateException (iem, Config_.AccName_, err);
								},
								[] (const auto& e) { qWarning () << Q_FUNC_INFO << e.what (); });
					}
				};
	}

	Account::Account (const AccountConfig& cfg, const Dependencies& deps)
	: Account { QUuid::createUuid ().toByteArray (), cfg, deps }
	{
	}

	AccountConfig Account::GetConfig () const
	{
		QMutexLocker l (GetMutex ());
		return Config_;
	}

	QByteArray Account::GetID () const
	{
		return ID_;
	}

	std::shared_ptr<AccountLogger> Account::GetLogger () const
	{
		return Logger_;
	}

	AccountDatabase_ptr Account::GetDatabase () const
	{
		return Storage_->BaseForAccount (this);
	}

	AccountFolderManager* Account::GetFolderManager () const
	{
		return FolderManager_;
	}

	MailModelsManager* Account::GetMailModelsManager () const
	{
		return MailModelsManager_;
	}

	FoldersModel* Account::GetFoldersModel () const
	{
		return FoldersModel_;
	}

	QFuture<Account::SynchronizeResult_t> Account::Synchronize ()
	{
		Util::Sequence (this, WorkerPool_->Schedule (TaskPriority::High, &AccountThreadWorker::SyncFolders)) >>
				Util::Visitor
				{
					[this] (const QList<Folder>& folders) { HandleGotFolders (folders); },
					[] (auto err)
					{
						qWarning () << Q_FUNC_INFO
								<< "error synchronizing folders list"
								<< Util::Visit (err, [] (auto e) { return e.what (); });
					}
				};

		auto folders = FolderManager_->GetSyncFolders ();
		if (folders.isEmpty ())
			folders << QStringList ("INBOX");

		return SynchronizeImpl (folders, {}, TaskPriority::Low);
	}

	QFuture<Account::SynchronizeResult_t> Account::Synchronize (const QStringList& path)
	{
		const auto& last = Storage_->GetLastID (this, path);
		return SynchronizeImpl ({ path }, last.value_or (QByteArray {}), TaskPriority::High);
	}

	QFuture<Account::SynchronizeResult_t> Account::SynchronizeImpl (const QList<QStringList>& folders,
			const QByteArray& last, TaskPriority prio)
	{
		for (const auto& folder : folders)
			SyncStatuses (folder, prio);

		const auto& newFuture = WorkerPool_->Schedule (prio, &AccountThreadWorker::Synchronize, folders, last);
		return newFuture * Util::Visitor
				{
					[=] (const AccountThreadWorker::SyncResult& right)
					{
						SyncStats stats;

						for (const auto& pair : Util::Stlize (right.Messages_))
						{
							const auto& folder = pair.first;
							const auto& msgs = pair.second;

							HandleMsgHeaders (msgs);
							UpdateFolderCount (folder);

							stats.NewMsgsCount_ += msgs.size ();
						}

						return SynchronizeResult_t::Right (stats);
					},
					[=] (auto err)
					{
						qWarning () << Q_FUNC_INFO
								<< "error synchronizing"
								<< folders
								<< "to"
								<< last
								<< ":"
								<< Util::Visit (err, [] (auto e) { return e.what (); });
						return SynchronizeResult_t::Left (err);
					}
				};
	}

	void Account::SyncStatuses (const QStringList& folder, TaskPriority prio)
	{
		Util::Sequence (this, WorkerPool_->Schedule (prio, &AccountThreadWorker::SyncMessagesStatuses, folder)) >>
				Util::Visitor
				{
					[=] (const AccountThreadWorker::SyncStatusesResult& result)
					{
						HandleReadStatusChanged (result.RemoteBecameRead_, result.RemoteBecameUnread_, folder);
						HandleMessagesRemoved (result.RemovedIds_, folder);

						UpdateFolderCount (folder);
					},
					[=] (auto err)
					{
						qWarning () << Q_FUNC_INFO
								<< "error synchronizing"
								<< folder
								<< ":"
								<< Util::Visit (err, [] (auto e) { return e.what (); });
					}
				};
	}

	Account::FetchWholeMessageResult_t Account::FetchWholeMessage (const QStringList& folder, const QByteArray& msgId)
	{
		auto future = WorkerPool_->Schedule (TaskPriority::High, &AccountThreadWorker::FetchWholeMessage, folder, msgId);
		Util::Sequence (this, future) >>
				Util::Visitor
				{
					[=] (const MessageBodies& bodies) { Storage_->SaveMessageBodies (this, folder, msgId, bodies); },
					Util::Visitor { [] (auto e) { qWarning () << Q_FUNC_INFO << e.what (); } }
				};

		return future;
	}

	void Account::PrefetchWholeMessages (const QStringList& folder, const QList<QByteArray>& msgIds)
	{
		auto future = WorkerPool_->Schedule (TaskPriority::Low,
				&AccountThreadWorker::PrefetchWholeMessages, folder, msgIds);
		Util::Sequence (this, future) >>
				Util::Visitor
				{
					[=] (const QHash<QByteArray, MessageBodies>& bodiesHash)
					{
						for (const auto& [msgId, bodies] : Util::Stlize (bodiesHash))
							Storage_->SaveMessageBodies (this, folder, msgId, bodies);
					},
					Util::Visitor { [] (auto e) { qWarning () << Q_FUNC_INFO << e.what (); } }
				};
	}

	QFuture<Account::SendMessageResult_t> Account::SendMessage (const OutgoingMessage& msg)
	{
		return WorkerPool_->Schedule (TaskPriority::High, &AccountThreadWorker::SendMessage, msg);
	}

	auto Account::FetchAttachment (const QStringList& folder, const QByteArray& msgId,
			const QString& attName, const QString& path) -> QFuture<FetchAttachmentResult_t>
	{
		return WorkerPool_->Schedule (TaskPriority::Low,
				&AccountThreadWorker::FetchAttachment, folder, msgId, attName, path);
	}

	void Account::SetReadStatus (bool read, const QList<QByteArray>& ids, const QStringList& folder)
	{
		const auto& future = WorkerPool_->Schedule (TaskPriority::High,
				&AccountThreadWorker::SetReadStatus, read, ids, folder);
		Util::Sequence (this, future) >>
				Util::Visitor
				{
					[=] (Util::Void)
					{
						auto becameRead = read ? ids : QList<QByteArray> {};
						auto becameUnread = read ? QList<QByteArray> {} : ids;
						HandleReadStatusChanged (becameRead, becameUnread, folder);
						UpdateFolderCount (folder);
					},
					[] (auto) {}
				};
	}

	QFuture<Account::CopyMessagesResult_t> Account::CopyMessages (const QList<QByteArray>& ids,
				const QStringList& from, const QList<QStringList>& to)
	{
		return WorkerPool_->Schedule (TaskPriority::High, &AccountThreadWorker::CopyMessages, ids, from, to);
	}

	void Account::MoveMessages (const QList<QByteArray>& ids,
			const QStringList& from, const QList<QStringList>& to)
	{
		emit willMoveMessages (ids, from);

		Util::Sequence (this, CopyMessages (ids, from, to)) >>
				Util::Visitor
				{
					[=] (Util::Void) { DeleteFromFolder (ids, from); },
					Util::Visitor { [] (auto e) { qWarning () << Q_FUNC_INFO << e.what (); } }
				};
	}

	namespace
	{
		AccountConfig::DeleteBehaviour RollupBehaviour (AccountConfig::DeleteBehaviour behaviour, const QString& service)
		{
			if (behaviour != AccountConfig::DeleteBehaviour::Default)
				return behaviour;

			static const QStringList knownTrashes { "imap.gmail.com" };
			return knownTrashes.contains (service) ?
					AccountConfig::DeleteBehaviour::MoveToTrash :
					AccountConfig::DeleteBehaviour::Expunge;
		}
	}

	void Account::DeleteMessages (const QList<QByteArray>& ids, const QStringList& folder)
	{
		emit willMoveMessages (ids, folder);

		const auto& trashPath = FoldersModel_->GetFolderPath (FolderType::Trash);
		const auto rollupBehaviour = RollupBehaviour (Config_.DeleteBehaviour_, Config_.InHost_);
		if (trashPath && rollupBehaviour == AccountConfig::DeleteBehaviour::MoveToTrash)
			Util::Sequence (this, CopyMessages (ids, folder, { *trashPath })) >>
					Util::Visitor
					{
						[=] (Util::Void) { DeleteFromFolder (ids, folder); },
						Util::Visitor { [] (auto e) { qWarning () << Q_FUNC_INFO << e.what (); } }
					};
		else
			DeleteFromFolder (ids, folder);
	}

	Account::CreateFolderResult_t Account::CreateFolder (const QStringList& path)
	{
		auto future = WorkerPool_->Schedule (TaskPriority::High, &AccountThreadWorker::CreateFolder, path);

		Util::Sequence (this, future) >>
				Util::Visitor
				{
					[this, path] (Util::Void) { FolderManager_->AddFolder (path); },
					[] (const auto&) {}
				};

		return future;
	}

	QByteArray Account::Serialize () const
	{
		QMutexLocker l (GetMutex ());

		QByteArray result;

		QDataStream out { &result, QIODevice::WriteOnly };
		out << static_cast<quint8> (1)
			<< ID_
			<< Config_
			<< FolderManager_->Serialize ();

		return result;
	}

	Account_ptr Account::Deserialize (const QByteArray& arr, const Dependencies& deps)
	{
		QDataStream in (arr);
		quint8 version = 0;
		in >> version;

		if (version != 1)
			throw std::runtime_error { "Unknown version " + std::to_string (version) };

		QByteArray id;
		AccountConfig cfg;
		in >> id;
		in >> cfg;

		auto acc = std::make_shared<Account> (id, cfg, deps);

		QByteArray fstate;
		in >> fstate;
		acc->FolderManager_->Deserialize (fstate);
		acc->handleFoldersUpdated ();

		return acc;
	}

	void Account::OpenConfigDialog (const std::function<void ()>& onAccepted)
	{
		auto dia = new AccountConfigDialog;
		dia->setAttribute (Qt::WA_DeleteOnClose);

		dia->SetConfig (GetConfig ());
		dia->SetAllFolders (FolderManager_->GetFoldersPaths ());
		for (const auto& folder : FolderManager_->GetFoldersPaths ())
		{
			const auto flags = FolderManager_->GetFolderFlags (folder);
			if (flags & AccountFolderManager::FolderOutgoing)
				dia->SetOutFolder (folder);
		}
		dia->SetFoldersToSync (FolderManager_->GetSyncFolders ());

		dia->show ();

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[this, onAccepted, dia]
			{
				{
					QMutexLocker l (GetMutex ());
					Config_ = dia->GetConfig ();
				}
				Logger_->SetEnabled (Config_.LogToFile_);
				FolderManager_->ClearFolderFlags ();
				const auto& out = dia->GetOutFolder ();
				if (!out.isEmpty ())
					FolderManager_->AppendFolderFlags (out, AccountFolderManager::FolderOutgoing);

				for (const auto& sync : dia->GetFoldersToSync ())
					FolderManager_->AppendFolderFlags (sync, AccountFolderManager::FolderSyncable);

				emit accountChanged ();

				if (onAccepted)
					onAccepted ();
			},
			dia,
			SIGNAL (accepted ()),
			dia
		};
	}

	bool Account::IsNull () const
	{
		return Config_.AccName_.isEmpty () ||
			Config_.Login_.isEmpty ();
	}

	ProgressListener_ptr Account::MakeProgressListener (const QString& context) const
	{
		return ProgressMgr_->MakeProgressListener (context);
	}

	QMutex* Account::GetMutex () const
	{
		return AccMutex_;
	}

	void Account::UpdateNoopInterval ()
	{
		NoopNotifier_->SetData (Config_.KeepAliveInterval_);
	}

	QFuture<QString> Account::BuildInURL ()
	{
		using Util::operator*;
		return GetPassword (Direction::In) *
				[this] (const QString& pass)
				{
					const auto& cfg = GetConfig ();

					const auto isSsl = cfg.InSecurity_ == AccountConfig::SecurityType::SSL;
					QString result { isSsl ? "imaps://" : "imap://" };
					result += cfg.Login_;
					result += ":";
					result.replace ('@', "%40");
					result += pass + '@';
					result += cfg.InHost_;

					return result;
				};
	}

	QFuture<QString> Account::BuildOutURL ()
	{
		using Util::operator*;

		const auto& cfg = GetConfig ();

		if (cfg.OutType_ == AccountConfig::OutType::Sendmail)
			return Util::MakeReadyFuture (QString { "sendmail://localhost" });

		const auto isSsl = cfg.OutSecurity_ == AccountConfig::SecurityType::SSL;
		QString result { isSsl ? "smtps://" : "smtp://" };

		if (!cfg.SMTPNeedsAuth_)
			return Util::MakeReadyFuture (result + cfg.OutHost_);

		QFuture<QString> passFuture;
		if (cfg.OutLogin_.isEmpty ())
		{
			result += cfg.Login_;
			passFuture = GetPassword (Direction::In);
		}
		else
		{
			result += cfg.OutLogin_;
			passFuture = GetPassword (Direction::Out);
		}
		auto outHost = cfg.OutHost_;

		return passFuture *
				[result, outHost] (const QString& pass)
				{
					auto full = result + ":" + pass;

					full.replace ('@', "%40");
					full += '@';

					full += outHost;

					qDebug () << Q_FUNC_INFO << full;

					return full;
				};
	}

	QByteArray Account::GetStoreID (Account::Direction dir) const
	{
		QMutexLocker l (GetMutex ());

		QByteArray result = GetID ();
		if (dir == Direction::Out)
			result += "/out";
		return result;
	}

	void Account::DeleteFromFolder (const QList<QByteArray>& ids, const QStringList& folder)
	{
		const auto& future = WorkerPool_->Schedule (TaskPriority::High,
				&AccountThreadWorker::DeleteMessages, ids, folder);
		Util::Sequence (this, future) >>
				Util::Visitor
				{
					[=] (Util::Void)
					{
						HandleMessagesRemoved (ids, folder);
						UpdateFolderCount (folder);
					},
					[] (auto) {}
				};
	}

	void Account::UpdateFolderCount (const QStringList& folder)
	{
		const auto totalCount = Storage_->GetNumMessages (this, folder);
		const auto unreadCount = Storage_->GetNumUnread (this, folder);
		FoldersModel_->SetFolderCounts (folder, unreadCount, totalCount);
	}

	QFuture<QString> Account::GetPassword (Direction dir)
	{
		QFutureInterface<QString> promise;
		promise.reportStarted ();
		QTimer::singleShot (0, this,
				[this, dir, promise] () mutable
				{
					Util::ReportFutureResult (promise,
							Util::GetPassword (GetStoreID (dir),
									tr ("Enter password for account %1:")
										.arg (Config_.AccName_),
									Core::Instance ().GetProxy ()));
				});
		return promise.future ();
	}

	void Account::HandleMsgHeaders (const QList<FetchedMessageInfo>& messages)
	{
		qDebug () << Q_FUNC_INFO << messages.size ();
		const auto& infos = Util::Map (messages, &FetchedMessageInfo::Info_);
		Storage_->SaveMessageInfos (this, infos);

		const auto base = Storage_->BaseForAccount (this);
		for (const auto& [msg, header] : messages)
			base->SetMessageHeader (msg.MessageId_, SerializeHeader (header));

		MailModelsManager_->Append (infos);
	}

	void Account::HandleReadStatusChanged (const QList<QByteArray>& read,
			const QList<QByteArray>& unread, const QStringList& folder)
	{
		Storage_->SetMessagesRead (this, folder, read, true);
		MailModelsManager_->UpdateReadStatus (folder, read, true);

		Storage_->SetMessagesRead (this, folder, unread, false);
		MailModelsManager_->UpdateReadStatus (folder, unread, false);
	}

	void Account::HandleMessagesRemoved (const QList<QByteArray>& ids, const QStringList& folder)
	{
		qDebug () << Q_FUNC_INFO << ids.size () << folder;
		for (const auto& id : ids)
			Storage_->RemoveMessage (this, folder, id);

		MailModelsManager_->Remove (ids);
	}

	void Account::RequestMessageCount (const QStringList& folder)
	{
		auto future = WorkerPool_->Schedule (TaskPriority::Low, &AccountThreadWorker::GetMessageCount, folder);
		Util::Sequence (this, future) >>
				Util::Visitor
				{
					[=] (QPair<int, int> counts) { HandleMessageCountFetched (counts.first, counts.second, folder); },
					Util::Visitor { [] (auto e) { qWarning () << Q_FUNC_INFO << e.what (); } }
				};
	}

	void Account::HandleMessageCountFetched (int count, int unread, const QStringList& folder)
	{
		FoldersModel_->SetFolderCounts (folder, unread, count);
	}

	void Account::HandleGotFolders (const QList<Folder>& folders)
	{
		FolderManager_->SetFolders (folders);
	}

	void Account::handleFoldersUpdated ()
	{
		const auto& folders = FolderManager_->GetFolders ();
		FoldersModel_->SetFolders (folders);

		for (const auto& folder : folders)
		{
			int count = -1, unread = -1;
			try
			{
				count = Storage_->GetNumMessages (this, folder.Path_);
				unread = Storage_->GetNumUnread (this, folder.Path_);
			}
			catch (const std::exception&)
			{
			}

			FoldersModel_->SetFolderCounts (folder.Path_, unread, count);

			RequestMessageCount (folder.Path_);
		}
	}
}
}
