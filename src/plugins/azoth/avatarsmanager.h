/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCache>
#include <QDeadlineTimer>
#include <QHash>
#include <QObject>
#include <util/threads/coro/corocontext.h>
#include "interfaces/azoth/core/iavatarsmanager.h"
#include "interfaces/azoth/ihaveavatars.h"
#include "avatarsstorage.h"

namespace LC::Azoth
{
	class IAccount;

	class EntriesManager;

	class AvatarsManager
		: public QObject
		, public IAvatarsManager
	{
		Q_OBJECT

		const EntriesManager& Entries_;
		AvatarsStorageThread Storage_;

		QHash<GlobalStrongestId, Util::SharedContextTask<std::optional<QByteArray>>> PendingCachedDataRequests_;
		QHash<std::pair<GlobalStrongestId, AvatarSize>, Util::SharedContextTask<AvatarsStorage::Result_t<QImage>>> PendingCachedImageRequests_;
		QHash<GlobalStrongestId, Util::SharedContextTask<IHaveAvatars::FetchResult_t>> PendingProtoDataRequests_;

		qsizetype BiggestImageSeen_ = 0;
		mutable QCache<std::pair<GlobalStrongestId, AvatarSize>, QImage> ImageCache_;

		struct AccountOffline {};
		struct EntryUnavailable {};
		using FetchFailure = std::variant<QDeadlineTimer, AccountOffline, EntryUnavailable>;
		QHash<GlobalStrongestId, FetchFailure> FailedFetches_;

		enum class FetchState : std::uint8_t { Running, Invalidated };
		QHash<GlobalStrongestId, FetchState> FetchStates_;
	public:
		explicit AvatarsManager (const EntriesManager&, QObject* = nullptr);
		~AvatarsManager ();

		void AddAccount (IAccount&);

		Util::ContextTask<std::optional<QByteArray>> GetAvatarData (GlobalStrongestId);

		Util::ContextTask<std::optional<QByteArray>> GetLocalAvatarData (GlobalStrongestId) override;
		Util::ContextTask<std::optional<QImage>> GetAvatarImage (GlobalStrongestId, AvatarSize) override;

		template<typename T>
		using MaybeNowResult_t = std::variant<T, Util::ContextTask<T>>;

		MaybeNowResult_t<std::optional<QImage>> GetAvatarImageNow (const GlobalStrongestId&, const AvatarSize&);

		std::optional<QImage> GetAvatarImageIfCached (const GlobalStrongestId&, const AvatarSize&) const;

		bool HasAvatar (QObject*) const;
	private:
		Util::ContextTask<AvatarsStorage::Result_t<QImage>> GetAvatarFromDiskOnly (GlobalStrongestId, AvatarSize);
		Util::SharedContextTask<IHaveAvatars::FetchResult_t> FetchAndSave (GlobalStrongestId);

		template<typename R, typename K, typename... RunArgs>
		Util::ContextTask<R> RunStorageCoalescing (const K& key, QHash<K, Util::SharedContextTask<R>>& pending, RunArgs&&... runArgs);

		template<typename R, typename K, typename Fun>
		Util::ContextTask<R> RunCoalescing (K key, QHash<K, Util::SharedContextTask<R>>& pending, Fun&& fun);

		void InsertCache (const GlobalStrongestId&, const AvatarSize&, const QImage&);

		bool HasActiveFailure (const GlobalStrongestId&) const;
		bool IsEntryAccountOnline (const GlobalStrongestId&) const;
		bool IsAlwaysAvatarless (const GlobalStrongestId&) const;
	private slots:
		Util::ContextTask<void> invalidateAvatar (QObject*);
	signals:
		void avatarInvalidated (QObject*);
		void accountAvatarInvalidated (const IAccount*);

	LC_CORO_CONTEXT
	};
}
