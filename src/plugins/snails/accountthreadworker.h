/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <vmime/net/session.hpp>
#include <vmime/net/message.hpp>
#include <vmime/net/folder.hpp>
#include <vmime/net/store.hpp>
#include <vmime/security/cert/certificateVerifier.hpp>
#include <util/sll/assoccache.h>
#include <util/sll/either.h>
#include <interfaces/structures.h>
#include "progresslistener.h"
#include "messageinfo.h"
#include "account.h"
#include "accountthreadworkerfwd.h"

class QTimer;

namespace LC
{
namespace Snails
{
	class Account;
	class Storage;
	class MessageChangeListener;

	template<typename T>
	class AccountThreadNotifier;

	struct Folder;

	struct OutgoingMessage;

	using MessageVector_t = std::vector<vmime::shared_ptr<vmime::net::message>>;
	using VmimeFolder_ptr = vmime::shared_ptr<vmime::net::folder>;
	using CertList_t = std::vector<vmime::shared_ptr<vmime::security::cert::X509Certificate>>;

	class AccountThreadWorker : public QObject
	{
		Account * const A_;
		Storage * const Storage_;

		QTimer * const NoopTimer_;

		const bool IsListening_;

		const QString ThreadName_;

		MessageChangeListener * const ChangeListener_;

		vmime::shared_ptr<vmime::net::session> Session_;
		vmime::shared_ptr<vmime::net::store> CachedStore_;

		Util::AssocCache<QStringList, vmime::shared_ptr<vmime::net::folder>> CachedFolders_;

		const vmime::shared_ptr<vmime::security::cert::certificateVerifier> CertVerifier_;
		const vmime::shared_ptr<vmime::security::authenticator> InAuth_;

		enum class FolderMode
		{
			ReadOnly,
			ReadWrite,
			NoChange
		};
	public:
		AccountThreadWorker (bool, const QString&, Account*, Storage*);

		using Folder2Messages_t = QHash<QStringList, QList<FetchedMessageInfo>>;

		struct SyncStatusesResult
		{
			QList<QByteArray> RemovedIds_;

			QList<QByteArray> RemoteBecameRead_;
			QList<QByteArray> RemoteBecameUnread_;
		};
	private:
		vmime::shared_ptr<vmime::net::store> MakeStore ();
		vmime::shared_ptr<vmime::net::transport> MakeTransport ();

		VmimeFolder_ptr GetFolder (const QStringList& folder, FolderMode mode);

		FetchedMessageInfo FromHeaders (const vmime::shared_ptr<vmime::net::message>&) const;

		QList<FetchedMessageInfo> FetchMessagesInFolder (const QStringList&, const VmimeFolder_ptr&, const QByteArray&);

		SyncStatusesResult SyncMessagesStatusesImpl (const QStringList&, const VmimeFolder_ptr&);

		void SetNoopTimeout (int);
		void SendNoop ();
	public:
		void SetNoopTimeoutChangeNotifier (const std::shared_ptr<AccountThreadNotifier<int>>&);

		void Disconnect ();

		void TestConnectivity ();

		struct SyncResult
		{
			Folder2Messages_t Messages_;
		};
		QList<Folder> SyncFolders ();
		SyncResult Synchronize (const QList<QStringList>&, const QByteArray& last);

		SyncStatusesResult SyncMessagesStatuses (const QStringList&);

		using MsgCountError_t = std::variant<FolderNotFound>;
		using MsgCountResult_t = Util::Either<MsgCountError_t, QPair<int, int>>;
		MsgCountResult_t GetMessageCount (const QStringList& folder);

		using SetReadStatusResult_t = Util::Either<std::variant<FolderNotFound>, Util::Void>;
		SetReadStatusResult_t SetReadStatus (bool read, const QList<QByteArray>& ids, const QStringList& folder);

		FetchWholeMessageResult_t FetchWholeMessage (const QStringList& folder, const QByteArray& msgId);
		PrefetchWholeMessagesResult_t PrefetchWholeMessages (const QStringList& folder, const QList<QByteArray>& msgIds);

		FetchAttachmentResult_t FetchAttachment (const QStringList& folder,
				const QByteArray& msgId, const QString& attName, const QString& path);

		void CopyMessages (const QList<QByteArray>& ids, const QStringList& from, const QList<QStringList>& tos);

		using DeleteResult_t = Util::Either<std::variant<FolderNotFound>, Util::Void>;
		DeleteResult_t DeleteMessages (const QList<QByteArray>& ids, const QStringList& folder);

		void SendMessage (const OutgoingMessage&);

		CreateFolderResult_t CreateFolder (const QStringList& folder);
	};
}
}
