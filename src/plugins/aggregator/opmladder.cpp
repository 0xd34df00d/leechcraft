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
#include <util/sll/visitor.h>
#include <util/sys/paths.h>
#include <util/threads/futures.h>
#include <util/xpc/util.h>
#include "xmlsettingsmanager.h"
#include "importopml.h"
#include "opmlparser.h"
#include "common.h"
#include "dbutils.h"

namespace LC::Aggregator::Opml
{
	bool IsOpmlEntity (const Entity& e)
	{
		if (!e.Entity_.canConvert<QUrl> ())
			return false;

		const auto& url = e.Entity_.toUrl ();
		if (e.Mime_ != "text/x-opml")
			return false;

		return url.scheme () == "file" ||
				url.scheme () == "http" ||
				url.scheme () == "https" ||
				url.scheme () == "itpc";
	}

	namespace
	{
		void ReportError (const QString& body)
		{
			auto e = Util::MakeNotification (QObject::tr ("OPML import error"), body, Priority::Critical);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
		}

		void HandleOpmlRemote (const QUrl& url, std::weak_ptr<UpdatesManager> updatesManager)
		{
			const auto& name = Util::GetTemporaryName ();

			const auto& dlEntity = Util::MakeEntity (url,
					name,
					Internal |
							DoNotNotifyUser |
							DoNotSaveInHistory |
							NotPersistent |
							DoNotAnnounceEntity);

			const auto& handleResult = GetProxyHolder ()->GetEntityManager ()->DelegateEntity (dlEntity);
			if (!handleResult)
			{
				ReportError (QObject::tr ("Could not find plugin to download OPML %1.")
						.arg (url.toString ()));
				return;
			}

			Util::Sequence (nullptr, handleResult.DownloadResult_) >>
					Util::Visitor
					{
						[name, updatesManager] (IDownload::Success)
						{
							if (const auto um = updatesManager.lock ())
								HandleOpmlFile (name, *um);
						},
						[] (const IDownload::Error&)
						{
							ReportError (QObject::tr ("Unable to download the OPML file."));
						}
					}.Finally ([name] { QFile::remove (name); });
		}

		void HandleOpmlGlobalSettings (const Entity& e)
		{
			auto copyVal = [&e] (const QByteArray& name)
			{
				if (e.Additional_.contains (name))
					XmlSettingsManager::Instance ().setProperty (name, e.Additional_.value (name));
			};
			copyVal ("UpdateOnStartup");
			copyVal ("UpdateTimeout");
			copyVal ("MaxArticles");
			copyVal ("MaxAge");
		}
	}

	void HandleOpmlFile (const QString& file, UpdatesManager& updatesManager)
	{
		ImportOPML importDialog { file };
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
		auto url = e.Entity_.toUrl ();
		if (url.scheme () == "file")
			HandleOpmlFile (url.toLocalFile (), *updatesManager.lock ());
		else
			HandleOpmlRemote (url, std::move (updatesManager));

		HandleOpmlGlobalSettings (e);
	}
}
