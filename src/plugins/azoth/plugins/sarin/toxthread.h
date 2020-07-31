/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <atomic>
#include <memory>
#include <functional>
#include <type_traits>
#include <QThread>
#include <QMutex>
#include <QFuture>
#include <interfaces/azoth/iclentry.h>
#include <util/threads/futures.h>
#include "threadexceptions.h"
#include "toxaccountconfiguration.h"

typedef struct Tox Tox;

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	class CallManager;
	class CallbackManager;
	class ToxLogger;

	class ToxThread : public QThread
	{
		Q_OBJECT

		std::atomic_bool ShouldStop_ { false };

		const QString Name_;

		QByteArray ToxState_;

		const ToxAccountConfiguration Config_;

		EntryStatus Status_;

		QList<std::function<void (Tox*)>> FQueue_;
		QMutex FQueueMutex_;

		std::shared_ptr<Tox> Tox_;
		std::shared_ptr<CallManager> CallManager_;

		const std::shared_ptr<CallbackManager> CbMgr_;

		const std::unique_ptr<ToxLogger> Logger_;
	public:
		ToxThread (const QString& name, const QByteArray& toxState, const ToxAccountConfiguration&);
		~ToxThread ();

		CallManager* GetCallManager () const;

		EntryStatus GetStatus () const;
		void SetStatus (const EntryStatus&);

		void Stop ();
		bool IsStoppable () const;

		QFuture<QByteArray> GetToxId ();

		enum class AddFriendResult
		{
			Added,
			InvalidId,
			TooLong,
			NoMessage,
			OwnKey,
			AlreadySent,
			BadChecksum,
			NoSpam,
			NoMem,
			Unknown
		};

		struct FriendInfo
		{
			QByteArray Pubkey_;
			QString Name_;
			EntryStatus Status_;
		};

		CallbackManager* GetCallbackManager () const;

		QFuture<AddFriendResult> AddFriend (QByteArray, QString);
		void AddFriend (QByteArray);
		void RemoveFriend (const QByteArray&);

		QFuture<std::optional<qint32>> ResolveFriendId (const QByteArray&);
		QFuture<QByteArray> GetFriendPubkey (qint32);
		QFuture<FriendInfo> ResolveFriend (qint32);

		template<typename F>
		auto ScheduleFunction (F&& func)
		{
			QFutureInterface<decltype (func ({}))> iface;
			iface.reportStarted ();
			ScheduleFunctionImpl ([iface, func] (Tox *tox) mutable
					{
						Util::ReportFutureResult (iface, std::forward<F> (func), tox);
					});
			return iface.future ();
		}
	private:
		void ScheduleFunctionImpl (const std::function<void (Tox*)>&);

		void SaveState ();

		void LoadFriends ();

		void HandleFriendRequest (const uint8_t*, const uint8_t*, size_t);
		void HandleNameChange (uint32_t, const uint8_t*, uint16_t);
		void UpdateFriendStatus (uint32_t);
		void HandleTypingChange (uint32_t, bool);

		void SetCallbacks ();
		void RunTox ();
	protected:
		virtual void run ();
	signals:
		void statusChanged (const EntryStatus&);

		void toxCreated ();

		void toxStateChanged (const QByteArray&);

		void gotFriend (qint32);
		void gotFriendRequest (const QByteArray& pubkey, const QString& msg);
		void removedFriend (const QByteArray& pubkey);

		void friendNameChanged (const QByteArray& pubkey, const QString&);

		void friendStatusChanged (const QByteArray& pubkey, const EntryStatus& status);

		void friendTypingChanged (const QByteArray& pubkey, bool isTyping);

		void fatalException (const LC::Util::QtException_ptr&);
	};
}
}
}
