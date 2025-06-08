/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "storagemanager.h"
#include <cmath>
#include <QMessageBox>
#include <util/util.h>
#include <util/threads/coro.h>
#include <util/threads/workerthreadbase.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <util/db/consistencychecker.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/irichtextmessage.h>
#include "storage.h"
#include "loggingstatekeeper.h"

namespace LC
{
namespace Azoth
{
namespace ChatHistory
{
	StorageManager::StorageManager (LoggingStateKeeper *keeper)
	: StorageThread_ { std::make_shared<StorageThread> () }
	, LoggingStateKeeper_ { keeper }
	{
		StorageThread_->SetPaused (true);
		StorageThread_->SetAutoQuit (true);

		Util::Sequence (this, StorageThread_->ScheduleImpl (&Storage::Initialize)) >>
				[this] (const Storage::InitializationResult_t& res)
				{
					if (res.IsRight ())
					{
						StorageThread_->SetPaused (false);
						return;
					}

					HandleStorageError (res.GetLeft ());
				};

		CheckStorage ();
	}

	namespace
	{
		QString GetVisibleName (const ICLEntry *entry)
		{
			if (entry->GetEntryType () == ICLEntry::EntryType::PrivateChat)
			{
				const auto parent = entry->GetParentCLEntry ();
				return parent->GetEntryName () + "/" + entry->GetEntryName ();
			}
			else
				return entry->GetEntryName ();
		}
	}

	void StorageManager::Process (QObject *msgObj)
	{
		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (msg->GetMessageType () != IMessage::Type::ChatMessage &&
			msg->GetMessageType () != IMessage::Type::MUCMessage)
			return;

		if (msg->GetBody ().isEmpty ())
			return;

		if (msg->GetDirection () == IMessage::Direction::Out &&
				msg->GetMessageType () == IMessage::Type::MUCMessage)
			return;

		const double secsDiff = msg->GetDateTime ().secsTo (QDateTime::currentDateTime ());
		if (msg->GetMessageType () == IMessage::Type::MUCMessage &&
				std::abs (secsDiff) >= 2)
			return;

		ICLEntry *entry = qobject_cast<ICLEntry*> (msg->ParentCLEntry ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "message's other part doesn't implement ICLEntry"
					<< msg->GetQObject ()
					<< msg->OtherPart ();
			return;
		}
		if (!LoggingStateKeeper_->IsLoggingEnabled (entry))
			return;

		const auto irtm = qobject_cast<IRichTextMessage*> (msgObj);

		AddLogItems (entry->GetParentAccount ()->GetAccountID (),
				entry->GetEntryID (),
				GetVisibleName (entry),
				{
					LogItem
					{
						msg->GetDateTime (),
						msg->GetDirection (),
						msg->GetBody (),
						msg->GetOtherVariant (),
						msg->GetMessageType (),
						irtm ? irtm->GetRichBody () : QString {},
						msg->GetEscapePolicy ()
					}
				},
				false);
	}

	void StorageManager::AddLogItems (const QString& accountId, const QString& entryId,
			const QString& visibleName, const QList<LogItem>& items, bool fuzzy)
	{
		StorageThread_->ScheduleImpl (&Storage::AddMessages,
				accountId,
				entryId,
				visibleName,
				items,
				fuzzy);
	}

	QFuture<IHistoryPlugin::MaxTimestampResult_t> StorageManager::GetMaxTimestamp (const QString& accId)
	{
		return StorageThread_->ScheduleImpl (&Storage::GetMaxTimestamp, accId);
	}

	QFuture<QStringList> StorageManager::GetOurAccounts ()
	{
		return StorageThread_->ScheduleImpl (&Storage::GetOurAccounts);
	}

	QFuture<UsersForAccountResult_t> StorageManager::GetUsersForAccount (const QString& accountID)
	{
		return StorageThread_->ScheduleImpl (&Storage::GetUsersForAccount, accountID);
	}

	QFuture<ChatLogsResult_t> StorageManager::GetChatLogs (const QString& accountId,
			const QString& entryId, int backpages, int amount)
	{
		return StorageThread_->ScheduleImpl (&Storage::GetChatLogs, accountId, entryId, backpages, amount);
	}

	QFuture<SearchResult_t> StorageManager::Search (const QString& accountId, const QString& entryId,
			const QString& text, int shift, bool cs)
	{
		return StorageThread_->ScheduleImpl (&Storage::Search, accountId, entryId, text, shift, cs);
	}

	QFuture<SearchResult_t> StorageManager::Search (const QString& accountId, const QString& entryId, const QDateTime& dt)
	{
		return StorageThread_->ScheduleImpl (&Storage::SearchDate, accountId, entryId, dt);
	}

	QFuture<DaysResult_t> StorageManager::GetDaysForSheet (const QString& accountId, const QString& entryId, int year, int month)
	{
		return StorageThread_->ScheduleImpl (&Storage::GetDaysForSheet, accountId, entryId, year, month);
	}

	void StorageManager::ClearHistory (const QString& accountId, const QString& entryId)
	{
		StorageThread_->ScheduleImpl (&Storage::ClearHistory, accountId, entryId);
	}

	void StorageManager::RegenUsersCache ()
	{
		StorageThread_->ScheduleImpl (&Storage::RegenUsersCache);
	}

	void StorageManager::StartStorage ()
	{
		StorageThread_->SetPaused (false);
		StorageThread_->start (QThread::LowestPriority);
	}

	namespace
	{
		void ReportRecoverResult (const QString& context, const std::optional<int>& count,
				const Util::ConsistencyChecker::RecoverFinished& recoverFinished)
		{
			const auto& text = StorageManager::tr ("Finished restoring history database contents. "
					"Old file size: %1, new file size: %2, %3 records recovered.");
			const auto& greet = recoverFinished.NewFileSize_ > recoverFinished.OldFileSize_ * 0.9 ?
					StorageManager::tr ("Yay, seems like most of the contents are intact!") :
					StorageManager::tr ("Sadly, seems like quite some history is lost.");

			QMessageBox::information (nullptr, context,
					text.arg (Util::MakePrettySize (recoverFinished.OldFileSize_))
						.arg (Util::MakePrettySize (recoverFinished.NewFileSize_))
						.arg (count.value_or (0)) +
						" " + greet);
		}
	}

	Util::ContextTask<void> StorageManager::CheckStorage ()
	{
		co_await Util::AddContextObject { *this };

		namespace CC = Util::ConsistencyChecker;

		const auto& dbPath = Storage::GetDatabasePath ();
		const auto checkResult = co_await CC::Check (dbPath);
		if (checkResult.IsRight ())
		{
			StartStorage ();
			co_return;
		}

		qWarning () << "db is broken, gonna repair";
		const auto& context = "Azoth ChatHistory"_qs;
		const auto recoverResult = co_await CC::RecoverWithUserInteraction (dbPath, context);
		const auto recoverFinished = co_await Util::WithHandler (recoverResult,
				[&] (const auto&)
				{
					QMessageBox::critical (nullptr, context, tr ("Unable to recover storage, not initializing ChatHistory."));
				});

		StartStorage ();

		const auto count = co_await StorageThread_->ScheduleImpl (&Storage::GetAllHistoryCount);
		ReportRecoverResult (context, count, recoverFinished);
	}

	void StorageManager::HandleStorageError (const Storage::InitializationError_t& error)
	{
		Util::Visit (error,
				[] (const Storage::GeneralError& err)
				{
					QMessageBox::critical (nullptr,
							"Azoth ChatHistory",
							tr ("Unable to initialize permanent storage. %1.")
								.arg (err.ErrorText_));
				});
	}
}
}
}
