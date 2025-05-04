/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "updatesmanager.h"
#include <QDateTime>
#include <QDomDocument>
#include <QTimer>
#include <interfaces/idownload.h>
#include <interfaces/core/ientitymanager.h>
#include <util/gui/util.h>
#include <util/sll/debugprinters.h>
#include <util/sll/either.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <util/threads/coro/future.h>
#include <util/threads/coro.h>
#include <util/threads/coro/throttle.h>
#include <util/sys/paths.h>
#include <util/xpc/util.h>
#include "components/parsers/parse.h"
#include "components/storage/storagebackend.h"
#include "components/storage/storagebackendmanager.h"
#include "dbupdatethread.h"
#include "xmlsettingsmanager.h"
#include "feedserrormanager.h"

namespace LC::Aggregator
{
	namespace
	{
		using ParseResult = Util::Either<FeedsErrorManager::ParseError, channels_container_t>;

		ParseResult ParseChannels (const QString& path, const QString& url, IDType_t feedId)
		{
			QFile file { path };
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << "unable to open the local file" << path;
				return Util::Left { UpdatesManager::tr ("Unable to open the temporary file.") };
			}

			QDomDocument doc;
			if (const auto parseResult = doc.setContent (&file, QDomDocument::ParseOption::UseNamespaceProcessing);
				!parseResult)
			{
				qWarning () << "error parsing XML for" << url << parseResult;
				return Util::Left { UpdatesManager::tr ("XML parse error for the feed %1.").arg (url) };
			}

			if (auto maybeChannels = Parsers::TryParse (doc, feedId))
				return *maybeChannels;

			qWarning () << "no parser for" << url;
			return Util::Left { UpdatesManager::tr ("Could not find parser to parse %1.").arg (url) };
		}
	}

	using namespace std::chrono_literals;

	UpdatesManager::UpdatesManager (const InitParams& initParams, QObject *parent)
	: QObject { parent }
	, DBUpThread_ { initParams.DBUpThread_ }
	, FeedsErrorManager_ { initParams.FeedsErrorManager_ }
	, UpdateTimer_ { new QTimer { this } }
	, CustomUpdateTimer_ { new QTimer { this } }
	, UpdateThrottle_ { 500ms }
	{
		UpdateTimer_->setSingleShot (true);
		connect (UpdateTimer_,
				&QTimer::timeout,
				this,
				&UpdatesManager::UpdateFeeds);

		CustomUpdateTimer_->start (60 * 1000);
		connect (CustomUpdateTimer_,
				&QTimer::timeout,
				this,
				&UpdatesManager::HandleCustomUpdates);

		auto& xsm = XmlSettingsManager::Instance ();

		auto now = QDateTime::currentDateTime ();
		auto lastUpdated = xsm.Property ("LastUpdateDateTime", now).toDateTime ();
		if (auto interval = xsm.property ("UpdateInterval").toInt ())
		{
			auto updateDiff = lastUpdated.secsTo (now);
			if (xsm.property ("UpdateOnStartup").toBool () ||
					updateDiff > interval * 60)
				QTimer::singleShot (7000,
						this,
						&UpdatesManager::UpdateFeeds);
			else
				UpdateTimer_->start (updateDiff * 1000);
		}

		xsm.RegisterObject ("UpdateInterval", this,
				[this] (int min)
				{
					if (min)
					{
						if (UpdateTimer_->isActive ())
							UpdateTimer_->setInterval (min * 60 * 1000);
						else
							UpdateTimer_->start (min * 60 * 1000);
					}
					else
						UpdateTimer_->stop ();
				});
	}

	namespace
	{
		bool IsCustomTimer (const StorageBackend& sb, IDType_t feedId)
		{
			return sb.GetFeedSettings (feedId).value_or (Feed::FeedSettings {}).UpdateTimeout_;
		}
	}

	void UpdatesManager::UpdateFeeds ()
	{
		if (const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ())
			for (const auto id : sb->GetFeedsIDs ())
				if (!IsCustomTimer (*sb, id))
					UpdateFeed (id);

		XmlSettingsManager::Instance ().setProperty ("LastUpdateDateTime", QDateTime::currentDateTime ());
		if (int interval = XmlSettingsManager::Instance ().property ("UpdateInterval").toInt ())
			UpdateTimer_->start (interval * 60 * 1000);
	}

	void UpdatesManager::UpdateFeed (IDType_t feedId)
	{
		UpdateFeedAsync (feedId);
	}

	void UpdatesManager::HandleCustomUpdates ()
	{
		const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		if (!sb)
			return;

		const auto& current = QDateTime::currentDateTime ();
		for (const auto id : sb->GetFeedsIDs ())
		{
			const auto& feedSettings = sb->GetFeedSettings (id);
			// It's handled by normal timer.
			if (!feedSettings || !feedSettings->UpdateTimeout_)
				continue;

			if (!Updates_.contains (id) ||
					Updates_ [id].secsTo (current) >= feedSettings->UpdateTimeout_ * 60)
			{
				UpdateFeed (id);
				Updates_ [id] = QDateTime::currentDateTime ();
			}
		}
	}

	namespace
	{
		Util::ContextTask<Util::Either<QString, channels_container_t>> FetchChannels (IDType_t feedId,
				QString urlStr,
				auto errorHandler)
		{
			const auto& filename = Util::GetTemporaryName ();
			const auto& e = Util::MakeEntity (QUrl { urlStr }, filename,
					Internal | DoNotNotifyUser | DoNotSaveInHistory | NotPersistent | DoNotAnnounceEntity);
			const auto fileGuard = Util::MakeScopeGuard ([filename] { QFile::remove (filename); });

			const auto& delegateResult = GetProxyHolder ()->GetEntityManager ()->DelegateEntity (e);
			if (!delegateResult)
				co_return Util::Left { UpdatesManager::tr ("Could not find plugin for feed with URL %1").arg (urlStr) };

			const auto downloadResult = co_await delegateResult.DownloadResult_;
			const auto success [[maybe_unused]] = co_await WithHandler (downloadResult, errorHandler);
			co_return co_await WithHandler (ParseChannels (filename, urlStr, feedId), errorHandler);
		}
	}

	Util::ContextTask<void> UpdatesManager::UpdateFeedAsync (IDType_t feedId)
	{
		const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		if (!sb)
			co_return;

		co_await Util::AddContextObject { *this };
		co_await UpdateThrottle_;

		const auto& url = sb->GetFeed (feedId).URL_;

		const auto channelsResult = co_await FetchChannels (feedId, url,
				[=, errMgr = FeedsErrorManager_] (const auto& error)
				{
					const auto& channels = sb->GetChannels (feedId);
					const auto& feedName = channels.size () == 1 ? channels [0].Title_ : url;
					errMgr->AddFeedError (feedId, feedName, error);
					return error.Message_;
				});
		const auto channels = co_await WithHandler (channelsResult,
				[] (const QString& error)
				{
					const auto& e = Util::MakeNotification (NotificationTitle, error, Priority::Critical);
					GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
				});
		FeedsErrorManager_->ClearFeedErrors (feedId);
		DBUpThread_->UpdateFeed (channels, url);
	}
}
