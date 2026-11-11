/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "avatarsmanager.h"
#include <QBuffer>
#include <QImage>
#include <util/sll/util.h>
#include <util/sll/qobjectrefcast.h>
#include <util/sll/trackingpointer.h>
#include <util/sll/visitor.h>
#include <util/threads/coro.h>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/iextselfinfoaccount.h"
#include "util/azoth/util.h"
#include "components/managers/entriesmanager.h"
#include "avatarsstorage.h"
#include "xmlsettingsmanager.h"

namespace LC::Azoth
{
	namespace
	{
		constexpr auto CacheBiggestMultiplier = 2;
	}

	AvatarsManager::AvatarsManager (const EntriesManager& entries, QObject *parent)
	: QObject { parent }
	, Entries_ { entries }
	{
		XmlSettingsManager::Instance ().RegisterObject ("AvatarsCacheSize",
				this,
				[this] (int size)
				{
					const auto target = static_cast<qsizetype> (size) * 1024 * 1024;
					ImageCache_.setMaxCost (std::max (target, BiggestImageSeen_ * CacheBiggestMultiplier));
				});

		connect (&Entries_,
				&EntriesManager::added,
				this,
				[this] (const QList<ICLEntry*>& entries)
				{
					for (const auto entry : entries)
						if (qobject_cast<IHaveAvatars*> (entry->GetQObject ()))
							connect (entry->GetQObject (),
									SIGNAL (avatarChanged (QObject*)),
									this,
									SLOT (invalidateAvatar (QObject*)));
				});

		connect (&Entries_,
				&EntriesManager::strongestIdChanged,
				this,
				[this] (const GlobalConventionalId& oldId, const GlobalStrongestId& newId)
				{
					Storage_.Run (&AvatarsStorage::MoveAvatars, oldId, newId);

					for (const auto& [id, size] : ImageCache_.keys ())
						if (id == newId)
							ImageCache_.remove ({ id, size });

					for (const auto& [id, size] : ImageCache_.keys ())
						if (id == oldId)
							if (const std::unique_ptr<QImage> img { ImageCache_.take ({ id, size }) })
								InsertCache (newId, size, *img);

					if (const auto pos = FailedFetches_.find (oldId);
						pos != FailedFetches_.end ())
					{
						const auto failure = *pos;
						FailedFetches_.erase (pos);
						FailedFetches_ [newId] = failure;
					}
				});
	}

	AvatarsManager::~AvatarsManager () = default;

	Util::ContextTask<std::optional<QByteArray>> AvatarsManager::GetAvatarData (GlobalStrongestId entryId)
	{
		if (IsAlwaysAvatarless (entryId))
			co_return QByteArray {};

		co_await Util::AddContext { CoroContext_ };

		const auto stored = co_await RunStorageCoalescing (entryId, PendingCachedDataRequests_,
				&AvatarsStorage::GetAvatarData, entryId);
		if (stored)
			co_return stored;

		const auto fetchResult = co_await RunCoalescing (entryId, PendingProtoDataRequests_, [&] { return FetchAndSave (entryId); });
		co_return fetchResult.MaybeRight ();
	}

	Util::ContextTask<std::optional<QByteArray>> AvatarsManager::GetLocalAvatarData (GlobalStrongestId entryId)
	{
		return RunStorageCoalescing (entryId, PendingCachedDataRequests_, &AvatarsStorage::GetAvatarData, entryId);
	}

	namespace
	{
		template<typename T>
		bool HasReadyResult (const AvatarsStorage::Result_t<T>& result)
		{
			return !std::holds_alternative<AvatarsStorage::NotStored> (result);
		}

		template<typename T>
		std::optional<T> ExtractMaybe (const AvatarsStorage::Result_t<T>& result)
		{
			return Util::Visit (result,
					[&] (AvatarsStorage::NotStored) { return std::optional<T> {}; },
					[] (AvatarsStorage::Empty) { return std::optional<T> {}; },
					[] (const T& data) { return std::optional { data }; });
		}
	}

	Util::ContextTask<std::optional<QImage>> AvatarsManager::GetAvatarImage (GlobalStrongestId entryId, AvatarSize size)
	{
		if (IsAlwaysAvatarless (entryId))
			co_return {};

		if (const auto img = ImageCache_ [{ entryId, size }])
			co_return img->isNull () ? std::optional<QImage> {} : *img;

		co_await Util::AddContext { CoroContext_ };

		if (const auto onDisk = co_await GetAvatarFromDiskOnly (entryId, size);
			HasReadyResult (onDisk))
			co_return ExtractMaybe (onDisk);

		if (HasActiveFailure (entryId))
			co_return {};

		const auto fetchResult = co_await RunCoalescing (entryId, PendingProtoDataRequests_, [&] { return FetchAndSave (entryId); });
		if (fetchResult.IsLeft ())
			co_return {};

		if (const auto onDisk = co_await GetAvatarFromDiskOnly (entryId, size);
			HasReadyResult (onDisk))
			co_return ExtractMaybe (onDisk);

		// likely a storage write failure, so just decode it here
		QBuffer buffer;
		buffer.setData (fetchResult.GetRight ());
		const auto image = ExtractMaybe (AvatarsStorage::ReadAvatarImage (entryId, size, buffer));
		InsertCache (entryId, size, image.value_or ({}));
		co_return image;
	}

	AvatarsManager::MaybeNowResult_t<std::optional<QImage>> AvatarsManager::GetAvatarImageNow (const GlobalStrongestId& entryId, const AvatarSize& size)
	{
		if (IsAlwaysAvatarless (entryId))
			return {};

		if (const auto img = ImageCache_ [{ entryId, size }])
			return img->isNull () ? std::optional<QImage> {} : *img;

		return GetAvatarImage (entryId, size);
	}

	std::optional<QImage> AvatarsManager::GetAvatarImageIfCached (const GlobalStrongestId& entryId, const AvatarSize& size) const
	{
		const auto img = ImageCache_ [{ entryId, size }];
		return !img || img->isNull () ? std::optional<QImage> {} : *img;
	}

	bool AvatarsManager::HasAvatar (QObject *entryObj) const
	{
		const auto iha = qobject_cast<IHaveAvatars*> (entryObj);
		return iha && iha->HasAvatar ();
	}

	Util::ContextTask<AvatarsStorage::Result_t<QImage>> AvatarsManager::GetAvatarFromDiskOnly (GlobalStrongestId entryId, AvatarSize size)
	{
		co_await Util::AddContext { CoroContext_ };
		const auto img = co_await RunStorageCoalescing ({ entryId, size }, PendingCachedImageRequests_,
				&AvatarsStorage::GetAvatarImage, entryId, size);
		Util::Visit (img,
				[] (AvatarsStorage::NotStored) {},
				[&] (AvatarsStorage::Empty) { InsertCache (entryId, size, {}); },
				[&] (const QImage& image) { InsertCache (entryId, size, image); });
		co_return img;
	}

	bool AvatarsManager::HasActiveFailure (const GlobalStrongestId& entryId) const
	{
		const auto pos = FailedFetches_.find (entryId);
		if (pos == FailedFetches_.end ())
			return false;
		return Util::Visit (*pos,
				[] (const QDeadlineTimer& timer) { return !timer.hasExpired (); },
				[&] (const EntryUnavailable&) { return !Entries_.GetEntryOrNull (entryId); },
				[&] (const AccountOffline&) { return !IsEntryAccountOnline (entryId); });
	}

	bool AvatarsManager::IsEntryAccountOnline (const GlobalStrongestId& entryId) const
	{
		const auto entry = Entries_.GetEntryOrNull (entryId);
		return entry && IsOnline (entry->GetParentAccount ()->GetState ().State_);
	}

	bool AvatarsManager::IsAlwaysAvatarless (const GlobalStrongestId& entryId) const
	{
		const auto entry = Entries_.GetEntryOrNull (entryId);
		return entry && !qobject_cast<IHaveAvatars*> (entry->GetQObject ());
	}

	Util::SharedContextTask<IHaveAvatars::FetchResult_t> AvatarsManager::FetchAndSave (GlobalStrongestId entryId)
	{
		using namespace std::chrono_literals;

		if (IsAlwaysAvatarless (entryId))
			co_return QByteArray {};

		if (HasActiveFailure (entryId))
			co_return Util::Left { IHaveAvatars::FetchError {} };

		FailedFetches_.remove (entryId);

		co_await Util::AddContext { CoroContext_ };

		auto removeGuard = Util::MakeScopeGuard ([this, entryId] { FetchStates_.remove (entryId); });

		constexpr size_t numAttempts = 3;
		for (size_t attempt = 0; attempt < numAttempts; ++attempt)
		{
			// entry may die mid async call, so need to reresolve each iteration
			const auto entry = Entries_.GetEntryOrNull<IHaveAvatars> (entryId);
			if (!entry)
			{
				FailedFetches_ [entryId] = EntryUnavailable {};
				co_return Util::Left { IHaveAvatars::FetchError {} };
			}

			if (!IsEntryAccountOnline (entryId))
			{
				FailedFetches_ [entryId] = AccountOffline {};
				co_return Util::Left { IHaveAvatars::FetchError {} };
			}

			FetchStates_ [entryId] = FetchState::Running;
			const auto response = co_await entry->FetchAvatar ();
			if (FetchStates_ [entryId] == FetchState::Invalidated)
				continue;

			co_await Util::Visit (response,
					[&] (IHaveAvatars::FetchError) -> Util::SharedContextTask<void>
					{
						if (IsEntryAccountOnline (entryId))
							FailedFetches_ [entryId] = QDeadlineTimer { 1min };
						else
							FailedFetches_ [entryId] = AccountOffline {};
						co_return;
					},
					[&] (const QByteArray& data) { return Storage_.Run (&AvatarsStorage::SetAvatar, entryId, data); });

			if (FetchStates_ [entryId] == FetchState::Invalidated)
				continue;

			co_return response;
		}
		co_return Util::Left { IHaveAvatars::FetchError {} };
	}

	template<typename R, typename K, typename... RunArgs>
	Util::ContextTask<R> AvatarsManager::RunStorageCoalescing (const K& key, QHash<K, Util::SharedContextTask<R>>& pending, RunArgs&&... runArgs)
	{
		return RunCoalescing (key, pending, [&] { return Storage_.Run (std::forward<RunArgs> (runArgs)...); });
	}

	template<typename R, typename K, typename Fun>
	Util::ContextTask<R> AvatarsManager::RunCoalescing (K key, QHash<K, Util::SharedContextTask<R>>& pending, Fun&& fun)
	{
		co_await Util::AddContext { CoroContext_ };

		auto pos = pending.find (key);
		if (pos == pending.end ())
			pos = pending.emplace (key, std::forward<Fun> (fun) ());
		const auto guard = Util::MakeScopeGuard ([&] { pending.remove (key); });
		co_return co_await *pos;
	}

	void AvatarsManager::InsertCache (const GlobalStrongestId& entryId, const AvatarSize& size, const QImage& image)
	{
		const auto cost = image.isNull () ? 1 : image.sizeInBytes ();

		BiggestImageSeen_ = std::max (BiggestImageSeen_, cost);
		if (const auto watermark = BiggestImageSeen_ * CacheBiggestMultiplier;
			ImageCache_.maxCost () < watermark)
		{
			qWarning () << "expanding cache from" << ImageCache_.maxCost () << "to" << watermark;
			ImageCache_.setMaxCost (watermark);
		}

		ImageCache_.insert ({ entryId, size }, new QImage { image }, cost);
	}

	void AvatarsManager::AddAccount (IAccount& acc)
	{
		connect (&acc.GetAccountEmitter (),
				&Emitters::Account::statusChanged,
				this,
				[this, &acc] (const EntryStatus& status)
				{
					if (!IsOnline (status.State_))
						return;

					QList<QObject*> availableEntries;

					const auto& accId = acc.GetAccountId ();
					for (auto it = FailedFetches_.begin (); it != FailedFetches_.end (); )
					{
						const auto entryId = it.key ();
						if (GetAccountId (entryId) == accId && std::holds_alternative<AccountOffline> (*it))
						{
							it = FailedFetches_.erase (it);
							if (const auto entry = Entries_.GetEntryOrNull<QObject> (entryId))
								availableEntries << entry;
						}
						else
							++it;
					}

					for (const auto entry : availableEntries)
						emit avatarInvalidated (entry);

					if (const auto iesia = qobject_cast<IExtSelfInfoAccount*> (acc.GetQObject ());
						iesia && availableEntries.contains (iesia->GetSelfContact ()))
						emit accountAvatarInvalidated (&acc);
				});
	}

	Util::ContextTask<void> AvatarsManager::invalidateAvatar (QObject *that)
	{
		Util::TrackingPointer entry { &qobject_ref_cast<ICLEntry> (that) };

		co_await Util::AddContext { CoroContext_ };

		const auto& entryId = entry->GetGlobalStrongestId ();

		if (const auto pos = FetchStates_.find (entryId);
			pos != FetchStates_.end ())
			*pos = FetchState::Invalidated;

		// Sort of a sync point: ensure all previous reads of this `entryId` have completed
		// before moving on to clearing the caches, since the reads could insert into the
		// cache what would be stale entries.
		co_await Storage_.Run (&AvatarsStorage::DeleteAvatars, entryId);

		FailedFetches_.remove (entryId);
		for (const auto& [id, size] : ImageCache_.keys ())
			if (id == entryId)
				ImageCache_.remove ({ id, size });

		emit avatarInvalidated (entry->GetQObject ());

		const auto acc = entry->GetParentAccount ();
		if (const auto iesia = qobject_cast<IExtSelfInfoAccount*> (acc->GetQObject ());
			iesia && iesia->GetSelfContact () == entry->GetQObject ())
			emit accountAvatarInvalidated (acc);
	}
}
