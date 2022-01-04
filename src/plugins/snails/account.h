/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <functional>
#include <QObject>
#include <QHash>
#include "accountthread.h"
#include "accountthreadworkerfwd.h"
#include "progressfwd.h"
#include "accountconfig.h"

class QRecursiveMutex;
class QAbstractItemModel;
class QStandardItemModel;
class QStandardItem;
class QModelIndex;

template<typename T>
class QFuture;

namespace LC
{
namespace Snails
{
	class AccountLogger;
	class AccountThread;
	class AccountThreadWorker;
	class AccountFolderManager;
	class AccountDatabase;
	class MailModel;
	class FoldersModel;
	class MailModelsManager;
	class ThreadPool;
	class Storage;
	struct Folder;
	struct OutgoingMessage;

	enum class TaskPriority;

	template<typename T>
	class AccountThreadNotifier;

	class Account : public QObject
	{
		Q_OBJECT

		std::shared_ptr<AccountLogger> Logger_;

		std::shared_ptr<ThreadPool> WorkerPool_;

		QRecursiveMutex * const AccMutex_;

		QByteArray ID_;
		AccountConfig Config_;

		AccountFolderManager *FolderManager_;
		FoldersModel *FoldersModel_;

		MailModelsManager * const MailModelsManager_;

		std::shared_ptr<AccountThreadNotifier<int>> NoopNotifier_;

		ProgressManager * const ProgressMgr_;
		Storage * const Storage_;
	public:
		enum class Direction
		{
			In,
			Out
		};

		struct Dependencies
		{
			Storage *Storage_;
			ProgressManager *PM_;
			QObject *Parent_ = nullptr;
		};

		Account (const QByteArray&, const AccountConfig&, const Dependencies&);
		Account (const AccountConfig&, const Dependencies&);

		AccountConfig GetConfig () const;

		QByteArray GetID () const;

		std::shared_ptr<AccountLogger> GetLogger () const;

		std::shared_ptr<AccountDatabase> GetDatabase () const;

		AccountFolderManager* GetFolderManager () const;
		MailModelsManager* GetMailModelsManager () const;
		FoldersModel* GetFoldersModel () const;

		struct SyncStats
		{
			size_t NewMsgsCount_ = 0;
		};
		using SynchronizeResult_t = Util::Either<InvokeError_t<>, SyncStats>;

		QFuture<SynchronizeResult_t> Synchronize ();
		QFuture<SynchronizeResult_t> Synchronize (const QStringList&);

		using FetchWholeMessageResult_t = QFuture<WrapReturnType_t<Snails::FetchWholeMessageResult_t>>;
		FetchWholeMessageResult_t FetchWholeMessage (const QStringList&, const QByteArray&);
		void PrefetchWholeMessages (const QStringList&, const QList<QByteArray>&);

		using SendMessageResult_t = Util::Either<InvokeError_t<>, Util::Void>;
		QFuture<SendMessageResult_t> SendMessage (const OutgoingMessage&);

		using FetchAttachmentResult_t = WrapReturnType_t<Snails::FetchAttachmentResult_t>;
		QFuture<FetchAttachmentResult_t> FetchAttachment (const QStringList& folder,
				const QByteArray& msgId, const QString& attName, const QString& path);

		void SetReadStatus (bool, const QList<QByteArray>&, const QStringList&);

		using CopyMessagesResult_t = Util::Either<InvokeError_t<>, Util::Void>;
		QFuture<CopyMessagesResult_t> CopyMessages (const QList<QByteArray>& ids, const QStringList& from, const QList<QStringList>& to);
		void MoveMessages (const QList<QByteArray>& ids, const QStringList& from, const QList<QStringList>& to);
		void DeleteMessages (const QList<QByteArray>& ids, const QStringList& folder);

		using CreateFolderResult_t = QFuture<WrapReturnType_t<Snails::CreateFolderResult_t>>;
		CreateFolderResult_t CreateFolder (const QStringList&);

		QByteArray Serialize () const;
		static std::shared_ptr<Account> Deserialize (const QByteArray&, const Dependencies&);

		void OpenConfigDialog (const std::function<void ()>& onAccepted = {});

		bool IsNull () const;

		ProgressListener_ptr MakeProgressListener (const QString&) const;

		QFuture<QString> BuildInURL ();
		QFuture<QString> BuildOutURL ();
		QFuture<QString> GetPassword (Direction);
	private:
		QFuture<SynchronizeResult_t> SynchronizeImpl (const QList<QStringList>&, const QByteArray&, TaskPriority);
		void SyncStatuses (const QStringList&, TaskPriority);
		QMutex* GetMutex () const;

		void UpdateNoopInterval ();

		QByteArray GetStoreID (Direction) const;

		void DeleteFromFolder (const QList<QByteArray>& ids, const QStringList& folder);

		void UpdateFolderCount (const QStringList&);

		void RequestMessageCount (const QStringList&);
		void HandleMessageCountFetched (int, int, const QStringList&);

		void HandleReadStatusChanged (const QList<QByteArray>&, const QList<QByteArray>&, const QStringList&);
		void HandleMessagesRemoved (const QList<QByteArray>&, const QStringList&);
		void HandleMsgHeaders (const QList<FetchedMessageInfo>&);

		void HandleGotFolders (const QList<Folder>&);
	private slots:
		void handleFoldersUpdated ();
	signals:
		void accountChanged ();
		void willMoveMessages (const QList<QByteArray>& ids, const QStringList& folder);
	};

	typedef std::shared_ptr<Account> Account_ptr;
}
}

Q_DECLARE_METATYPE (LC::Snails::Account_ptr)
