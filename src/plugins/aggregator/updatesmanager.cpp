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
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include <util/threads/futures.h>
#include <util/sys/paths.h>
#include <util/xpc/util.h>
#include "components/parsers/parse.h"
#include "dbupdatethread.h"
#include "storagebackend.h"
#include "storagebackendmanager.h"
#include "xmlsettingsmanager.h"
#include "feedserrormanager.h"

namespace LC::Aggregator
{
	namespace
	{
		using ParseResult = Util::Either<QString, channels_container_t>;

		ParseResult ParseChannels (const QString& path, const QString& url, IDType_t feedId)
		{
			QFile file { path };
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to open the local file"
						<< path;
				return ParseResult::Left (UpdatesManager::tr ("Unable to open the temporary file."));
			}

			QDomDocument doc;
			QString errorMsg;
			int errorLine, errorColumn;
			if (!doc.setContent (&file, true, &errorMsg, &errorLine, &errorColumn))
			{
				const auto& copyPath = Util::GetTemporaryName ("lc_aggregator_failed.XXXXXX");
				file.copy (copyPath);
				qWarning () << Q_FUNC_INFO
						<< "error parsing XML for"
						<< url
						<< errorMsg
						<< errorLine
						<< errorColumn
						<< "; copy at"
						<< file.fileName ();
				return ParseResult::Left (UpdatesManager::tr ("XML parse error for the feed %1.")
						.arg (url));
			}

			if (auto maybeChannels = Parsers::TryParse (doc, feedId))
				return ParseResult::Right (*maybeChannels);

			const auto& copyPath = Util::GetTemporaryName ("lc_aggregator_failed.XXXXXX");
			file.copy (copyPath);
			qWarning () << Q_FUNC_INFO
					<< "no parser for"
					<< url
					<< "; copy at"
					<< copyPath;
			return ParseResult::Left (UpdatesManager::tr ("Could not find parser to parse %1.")
					.arg (url));
		}
	}

	UpdatesManager::UpdatesManager (const InitParams& initParams, QObject *parent)
	: QObject { parent }
	, EntityManager_ { initParams.EntityManager_ }
	, DBUpThread_ { initParams.DBUpThread_ }
	, FeedsErrorManager_ { initParams.FeedsErrorManager_ }
	, UpdateTimer_ { new QTimer { this } }
	, CustomUpdateTimer_ { new QTimer { this } }
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

		auto now = QDateTime::currentDateTime ();
		auto lastUpdated = XmlSettingsManager::Instance ()->Property ("LastUpdateDateTime", now).toDateTime ();
		if (auto interval = XmlSettingsManager::Instance ()->property ("UpdateInterval").toInt ())
		{
			auto updateDiff = lastUpdated.secsTo (now);
			if (XmlSettingsManager::Instance ()->property ("UpdateOnStartup").toBool () ||
					updateDiff > interval * 60)
				QTimer::singleShot (7000,
						this,
						&UpdatesManager::UpdateFeeds);
			else
				UpdateTimer_->start (updateDiff * 1000);
		}

		XmlSettingsManager::Instance ()->RegisterObject ("UpdateInterval", this,
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

		XmlSettingsManager::Instance ()->setProperty ("LastUpdateDateTime", QDateTime::currentDateTime ());
		if (int interval = XmlSettingsManager::Instance ()->property ("UpdateInterval").toInt ())
			UpdateTimer_->start (interval * 60 * 1000);
	}

	void UpdatesManager::UpdateFeed (IDType_t id)
	{
		if (UpdatesQueue_.isEmpty ())
			QTimer::singleShot (500,
					this,
					&UpdatesManager::RotateUpdatesQueue);

		UpdatesQueue_ << id;
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

	void UpdatesManager::RotateUpdatesQueue ()
	{
		const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		if (!sb)
			return;

		if (UpdatesQueue_.isEmpty ())
			return;

		const auto feedId = UpdatesQueue_.takeFirst ();

		if (!UpdatesQueue_.isEmpty ())
			QTimer::singleShot (2000,
					this,
					&UpdatesManager::RotateUpdatesQueue);

		const auto& url = sb->GetFeed (feedId).URL_;

		auto filename = Util::GetTemporaryName ();

		auto e = Util::MakeEntity (QUrl (url),
				filename,
				Internal |
					DoNotNotifyUser |
					DoNotSaveInHistory |
					NotPersistent |
					DoNotAnnounceEntity);

		auto emitError = [this] (const QString& body)
		{
			EntityManager_->HandleEntity (Util::MakeNotification ("Aggregator", body, Priority::Critical));
		};

		const auto& delegateResult = EntityManager_->DelegateEntity (e);
		if (!delegateResult)
		{
			emitError (tr ("Could not find plugin for feed with URL %1")
					.arg (url));
			return;
		}

		Util::Sequence (this, delegateResult.DownloadResult_) >>
				Util::Visitor
				{
					[=, this] (IDownload::Success)
					{
						Util::Visit (ParseChannels (filename, url, feedId),
								[&] (const channels_container_t& channels)
								{
									FeedsErrorManager_->ClearFeedErrors (feedId);
									DBUpThread_->UpdateFeed (channels, url);
								},
								[&] (const QString& error)
								{
									FeedsErrorManager_->AddFeedError (feedId, FeedsErrorManager::ParseError { error });
								});
					},
					[=, this] (const IDownload::Error& error) { FeedsErrorManager_->AddFeedError (feedId, error); }
				}.Finally ([filename] { QFile::remove (filename); });
	}
}
