/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "opmladder.h"
#include <QUrl>
#include <interfaces/structures.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/itagsmanager.h>
#include <util/sll/either.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <util/sys/paths.h>
#include <util/threads/coro/future.h>
#include <util/threads/coro.h>
#include <util/xpc/util.h>
#include "xmlsettingsmanager.h"
#include "common.h"
#include "dbutils.h"
#include "importopmldialog.h"
#include "opmlparser.h"

namespace LC::Aggregator::Opml
{
	bool IsOpmlEntity (const Entity& e)
	{
		if (!e.Entity_.canConvert<QUrl> ())
			return false;

		const auto& url = e.Entity_.toUrl ();
		if (e.Mime_ != "text/x-opml"_ql)
			return false;

		constexpr std::array schemes { "file"_ql, "http"_ql, "https"_ql, "itpc"_ql };
		return std::ranges::contains (schemes, url.scheme ());
	}

	namespace
	{
		void ReportError (const QString& body)
		{
			auto e = Util::MakeNotification (QObject::tr ("OPML import error"), body, Priority::Critical);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
		}

		struct LocalFile
		{
			QString Path_;
			[[maybe_unused]] Util::SharedScopeGuard RemoveGuard_;
		};

		Util::Task<Util::Either<QString, LocalFile>> HandleOpmlUrl (const QUrl& url)
		{
			if (url.isLocalFile ())
				co_return LocalFile { url.toLocalFile (), [] {} };

			const auto& name = Util::GetTemporaryName ();

			auto removeGuard = Util::MakeScopeGuard ([name] { QFile::remove (name); });

			const auto& dlEntity = Util::MakeEntity (url,
					name,
					Internal |
							DoNotNotifyUser |
							DoNotSaveInHistory |
							NotPersistent |
							DoNotAnnounceEntity);

			const auto& handleResult = GetProxyHolder ()->GetEntityManager ()->DelegateEntity (dlEntity);
			if (!handleResult)
				co_return Util::Left { QObject::tr ("Could not find plugin to download OPML %1.")
						.arg (url.toString ()) };

			const auto result = co_await handleResult.DownloadResult_;
			co_await WithHandler (result, [] (const IDownload::Error& error)
					{
						return QObject::tr ("Unable to download the OPML file: %1.").arg (error.Message_);
					});

			co_return LocalFile { name, std::move (removeGuard).Shared () };
		}

		void HandleOpmlGlobalSettings (const Entity& e)
		{
			auto copyVal = [&e] (const QByteArray& name)
			{
				if (e.Additional_.contains (name))
					XmlSettingsManager::Instance ().setProperty (name, e.Additional_.value (name));
			};
			copyVal ("UpdateOnStartup"_qba);
			copyVal ("UpdateTimeout"_qba);
			copyVal ("MaxArticles"_qba);
			copyVal ("MaxAge"_qba);
		}
	}

	void HandleOpmlFile (const QString& file, UpdatesManager& updatesManager)
	{
		ImportOPMLDialog importDialog { file };
		if (importDialog.exec () == QDialog::Rejected)
			return;

		const auto& tags = GetProxyHolder ()->GetTagsManager ()->Split (importDialog.GetTags ());
		const auto& selectedUrls = importDialog.GetSelectedUrls ();

		Util::Visit (ParseOPML (importDialog.GetFilename ()),
				[] (const QString& error) { ReportError (error); },
				[&] (const OPMLParseResult& result)
				{
					for (const auto& item : result.Items_)
					{
						if (!selectedUrls.contains (item.URL_))
							continue;

						int interval = 0;
						if (item.CustomFetchInterval_)
							interval = item.FetchInterval_;

						AddFeed ({
									.URL_ = item.URL_,
									.Tags_ = tags + item.Categories_,
									.FeedSettings_ = { { IDNotFound, interval, item.MaxArticleNumber_, item.MaxArticleAge_, false } },
									.UpdatesManager_ = updatesManager,
								});
					}
				});
	}

	void HandleOpmlEntity (const Entity& e, std::weak_ptr<UpdatesManager> updatesManager)
	{
		HandleOpmlGlobalSettings (e);

		[] (QUrl url, std::weak_ptr<UpdatesManager> updatesManager) -> Util::Task<void>
		{
			const auto remoteResult = co_await HandleOpmlUrl (url);
			const auto localFile = co_await WithHandler (remoteResult, ReportError);
			HandleOpmlFile (localFile.Path_, *updatesManager.lock ());
		} (e.Entity_.toUrl (), std::move (updatesManager));
	}
}
