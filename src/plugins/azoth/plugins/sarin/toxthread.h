/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <interfaces/azoth/iclentry.h>
#include <util/threads/coro.h>
#include <util/threads/coro/metamethod.h>
#include <util/threads/coro/workerthread.h>
#include "toxaccountconfiguration.h"
#include "types.h"
#include "util.h"

using Tox = struct Tox;

namespace LC::Azoth::Sarin
{
	class ToxLogger;
	class ToxRunner;

	class ToxW final : public QObject
	{
		const QString Name_;
		const ToxAccountConfiguration Config_;
		QByteArray ToxState_;
		const std::unique_ptr<ToxLogger> Logger_;

		std::unique_ptr<Tox, void (*) (Tox*)> Tox_;

		EntryStatus TargetStatus_;

		ToxRunner& Runner_;
	public:
		struct InitContext
		{
			QString Name_;
			QByteArray State_;
			ToxAccountConfiguration Config_;
		};

		explicit ToxW (const InitContext&, ToxRunner&);
		~ToxW () override;

		Tox* GetTox ();

		QByteArray GetToxId () const;

		bool SetStatus (const EntryStatus&);

		Util::Either<ToxError<InitError>, Util::Void> Init (EntryStatus);

		AddFriendResult AddFriend (const QByteArray&, QString);
		AddFriendResult AddFriendNoRequest (const QByteArray&);
		bool RemoveFriend (const QByteArray&);

		struct FriendInfo
		{
			QByteArray Pubkey_;
			QString Name_;
			EntryStatus Status_;
		};

		std::optional<uint32_t> ResolveFriendNum (const QByteArray&);
		QByteArray GetFriendPubkey (uint32_t);

		using ResolveResult = Util::Either<ToxError<FriendQueryError>, FriendInfo>;
		ResolveResult ResolveFriend (uint32_t) const;

		QList<uint32_t> GetFriendList () const;
	private:
		static ToxRunner& Runner (void *udata);

		template<auto Reg, auto Handler>
		void Register ();

		void Iterate ();

		void SaveState ();
		void InitializeCallbacks ();
	};

	class ToxRunner final : public Util::Coro::WorkerThread<ToxW, ToxRunner>
	{
		Q_OBJECT
	public:
		using WorkerThread::WorkerThread;

		auto RunWithError (auto f, auto... args)
		{
			return Run ([f, ...args = args] (ToxW& tox)
			{
				return WithError (f, tox.GetTox (), args...);;
			});
		}

		auto RunWithStrError (auto f, auto... args)
		{
			return Run ([f, ...args = args] (ToxW& tox)
			{
				return WithStrError (f, tox.GetTox (), args...);
			});
		}
	signals:
		void statusChanged (const EntryStatus&);

		void toxStateChanged (const QByteArray& newState);

		void gotFriendRequest (const QByteArray& pubkey, const QString& msg);

		void friendNameChanged (const QByteArray& pubkey, const QString&);
		void friendStatusChanged (const QByteArray& pubkey, const EntryStatus& status);
		void friendTypingChanged (const QByteArray& pubkey, bool isTyping);

		void gotFileControl (uint32_t, uint32_t, int);
		void gotData (quint32, quint32, const QByteArray&, uint64_t);
		void gotChunkRequest (uint32_t friendNum, uint32_t fileNum, uint64_t position, size_t length);
		void requested (uint32_t, const QByteArray&, uint32_t, uint64_t, const QString&);

		void incomingMessage (uint32_t, const QString&);
		void readReceipt (uint32_t);

		void confInvited (const ConfInvitationEvent&);
		void confConnected (uint32_t confNum);
		void confMessage (uint32_t confNum, const ConfMessageEvent&);
		void confTitleChanged (uint32_t confNum, const ConfTitleChangedEvent&);
		void confPeerNameChanged (uint32_t confNum, const ConfPeerNameChangedEvent&);
		void confPeerListChanged (uint32_t confNum);

		void groupPeerJoined (uint32_t groupNum, uint32_t peerId);
		void groupPeerExited (uint32_t groupNum, uint32_t peerId, const GroupPeerExitedEvent&);
	};
}
