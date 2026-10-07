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
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/sll/debugprinters.h>
#include <util/sll/either.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>
#include <util/threads/coro/throttle.h>
#include <util/threads/coro/inparallel.h>
#include <util/xpc/progressmanager.h>
#include <util/xpc/util.h>
#include "components/network/availability.h"
#include "components/parsers/parse.h"
#include "components/storage/sqlstoragebackend.h"
#include "components/storage/storagebackendmanager.h"
#include "dbupdatethread.h"
#include "xmlsettingsmanager.h"
#include "feedserrormanager.h"

namespace LC::Aggregator
{
	namespace
	{
		using ParseResult = Util::Either<QString, channels_container_t>;

		ParseResult ParseChannels (const QByteArray& data, const QString& url, IDType_t feedId)
		{
			QDomDocument doc;
			if (const auto parseResult = doc.setContent (data, QDomDocument::ParseOption::UseNamespaceProcessing);
				!parseResult)
			{
				qWarning () << "error parsing XML for" << url << parseResult;
				return Util::Left { UpdatesManager::tr ("XML parse error for the feed %1.").arg (url) };
			}

			if (auto maybeChannels = Parsers::TryParse (doc, feedId, QUrl { url }))
				return *maybeChannels;

			qWarning () << "no parser for" << url;
			return Util::Left { UpdatesManager::tr ("Could not find parser to parse %1.").arg (url) };
		}
	}

	using namespace std::chrono_literals;
	using minutes = std::chrono::minutes;
	using seconds = std::chrono::seconds;

	UpdatesManager::UpdatesManager (const InitParams& initParams, QObject *parent)
	: QObject { parent }
	, DBUpThread_ { initParams.DBUpThread_ }
	, FeedsErrorManager_ { initParams.FeedsErrorManager_ }
	, UpdateThrottle_ { 500ms }
	, ProgressManager_ { *new Util::ProgressManager { this } }
	, DoStartupUpdate_ { XmlSettingsManager::Instance ().property ("UpdateOnStartup").toBool () }
	{
		const auto tickTimer = new QTimer { this };
		tickTimer->setTimerType (Qt::VeryCoarseTimer);
		tickTimer->start (60s);
		tickTimer->callOnTimeout (this, &UpdatesManager::Tick);

		QTimer::singleShot (5s,
				this,
				&UpdatesManager::Tick);
	}

	IJobHolderRepresentationHandler_ptr UpdatesManager::CreateJobRepresentationHandler ()
	{
		return ProgressManager_.CreateDefaultHandler ();
	}

	namespace
	{
		bool IsCustomTimer (const SQLStorageBackend& sb, IDType_t feedId)
		{
			return sb.GetFeedSettings (feedId).value_or (Feed::FeedSettings {}).UpdateTimeout_;
		}

		bool ShouldUpdateNow ()
		{
			auto& xsm = XmlSettingsManager::Instance ();

			const auto now = QDateTime::currentDateTime ();
			const seconds sinceLast { xsm.Property ("LastUpdateDateTime", now).toDateTime ().secsTo (now) };

			const auto interval = minutes { xsm.property ("UpdateInterval").toInt () };
			return interval != minutes::zero () && sinceLast >= interval;
		}
	}

	void UpdatesManager::UpdateFeeds ()
	{
		XmlSettingsManager::Instance ().setProperty ("LastUpdateDateTime", QDateTime::currentDateTime ());

		const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		const auto isStandardTimer = [&sb] (IDType_t id) { return !IsCustomTimer (*sb, id); };
		UpdateFeedsAsync (Util::Filter (sb->GetFeedsIDs (), isStandardTimer), sb);
	}

	void UpdatesManager::UpdateFeed (IDType_t feedId)
	{
		UpdateFeedsAsync ({ feedId }, StorageBackendManager::Instance ().MakeStorageBackendForThread ());
	}

	void UpdatesManager::Tick ()
	{
		if (!IsNetworkAllowed<FeedsUpdate> ())
			return;

		if (std::exchange (DoStartupUpdate_, false) || ShouldUpdateNow ())
			UpdateFeeds ();

		HandleCustomUpdates ();
	}

	void UpdatesManager::HandleCustomUpdates ()
	{
		const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();

		ids_t feeds;

		const auto& current = QDateTime::currentDateTime ();
		for (const auto id : sb->GetFeedsIDs ())
		{
			const auto& feedSettings = sb->GetFeedSettings (id);
			// It's handled by normal timer.
			if (!feedSettings || !feedSettings->UpdateTimeout_)
				continue;

			if (!Updates_.contains (id) ||
					seconds { Updates_ [id].secsTo (current) } >= minutes { feedSettings->UpdateTimeout_ })
			{
				feeds << id;
				Updates_ [id] = QDateTime::currentDateTime ();
			}
		}

		UpdateFeedsAsync (feeds, sb);
	}

	namespace
	{
		QString GetRowName (const ids_t& feeds, const SQLStorageBackend_ptr& sb)
		{
			if (feeds.size () != 1)
				return UpdatesManager::tr ("Updating feeds…");

			const auto feed = sb->GetFeed (feeds [0]);
			const auto channels = sb->GetChannels (feeds [0]);
			const auto& title = channels.empty () ? feed.URL_ : channels [0].GetEffectiveTitle ();
			return UpdatesManager::tr ("Updating %1…").arg (title);
		}
	}

	Util::ContextTask<void> UpdatesManager::UpdateFeedsAsync (ids_t feeds, SQLStorageBackend_ptr sb)
	{
		if (feeds.isEmpty ())
			co_return;

		co_await Util::AddContext { CoroContext_ };

		const auto& iconName = feeds.size () == 1 ? "view-refresh"_qs : "mail-receive"_qs;
		const auto row = ProgressManager_.AddRow ({
					.Name_ = GetRowName (feeds, sb),
					.Specific_ = ProcessInfo { .Kind_ = ProcessKind::Generic },
				},
				{
					.Total_ = feeds.size (),
					.Icon_ = GetProxyHolder ()->GetIconThemeManager ()->GetIcon (iconName),
				});

		co_await Util::InParallel (feeds, [&] (IDType_t id) { return UpdateFeedAsync (id, *row, sb); });
	}

	namespace
	{
		Util::Task<Util::Either<QString, channels_container_t>> FetchChannels (IDType_t feedId, QString urlStr)
		{
			QNetworkRequest req { QUrl { urlStr } };
			req.setHeader (QNetworkRequest::UserAgentHeader, "LeechCraft.Aggregator/"_qba + GetProxyHolder ()->GetVersion ().toLatin1 ());
			const auto result = co_await *GetProxyHolder ()->GetNetworkAccessManager ()->get (req);
			const auto response = co_await result.ToEither ();
			co_return co_await ParseChannels (response, urlStr, feedId);
		}
	}

	Util::ContextTask<void> UpdatesManager::UpdateFeedAsync (IDType_t feedId, Util::ProgressModelRow& row, SQLStorageBackend_ptr sb)
	{
		const auto bumpRow = Util::MakeScopeGuard ([&row] { ++row; });

		co_await Util::AddContext { CoroContext_ };
		co_await UpdateThrottle_;

		const auto& url = sb->GetFeed (feedId).URL_;

		const auto channelsResult = co_await FetchChannels (feedId, url);
		const auto channels = co_await WithHandler (channelsResult,
				[=, this] (const QString& error)
				{
					const auto& existingChannels = sb->GetChannels (feedId);
					const auto& feedName = existingChannels.size () == 1 ? existingChannels [0].Title_ : url;
					FeedsErrorManager_->AddFeedError (feedId, feedName, FeedsErrorManager::Error { error });
				});
		FeedsErrorManager_->ClearFeedErrors (feedId);
		DBUpThread_->UpdateFeed (channels, url);
	}
}
