/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "opmladder.h"
#include <QTimer>
#include <QUrl>
#include <interfaces/structures.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/itagsmanager.h>
#include <util/sll/either.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <util/threads/coro.h>
#include <util/xpc/util.h>
#include "components/network/util.h"
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

		struct LocalFile { QString Path_; };
		struct RemoteContents { QByteArray Contents_; QUrl Url_; };
		using FetchedOpml = std::variant<RemoteContents, LocalFile>;

		Util::Task<Util::Either<QString, FetchedOpml>> HandleOpmlUrl (QUrl url)
		{
			if (url.isLocalFile ())
				co_return LocalFile { url.toLocalFile () };

			QNetworkRequest req { FixupFeedScheme (url) };
			SetStandardHeaders (req);
			const auto result = co_await *GetProxyHolder ()->GetNetworkAccessManager ()->get (req);
			const auto contents = co_await result.ToEither ()
					.MapLeft ([&] (const QString& error)
					{
						return QObject::tr ("Unable to download the OPML file %1: %2.").arg (url.toString (), error);
					});
			co_return RemoteContents { contents, url };
		}

		auto ToDialogSource (const FetchedOpml& opml)
		{
			using DialogSource = ImportOPMLDialog::Source;
			return Util::Visit (opml,
					[] (const LocalFile& file) -> DialogSource { return ImportOPMLDialog::FilePath { file.Path_ }; },
					[] (const RemoteContents& remote) -> DialogSource
					{
						return ImportOPMLDialog::OpmlContents { remote.Contents_, remote.Url_.toString () };
					});
		}

		void HandleOpml (const ImportOPMLDialog::Source& source, UpdatesManager& updatesManager)
		{
			ImportOPMLDialog importDialog { source };
			if (importDialog.exec () == QDialog::Rejected)
				return;

			QList<AddFeedParams> feeds;
			const auto& tags = GetProxyHolder ()->GetTagsManager ()->Split (importDialog.GetTags ());
			for (const auto& item : importDialog.GetSelectedItems ())
			{
				int interval = 0;
				if (item.CustomFetchInterval_)
					interval = item.FetchInterval_;

				feeds.push_back ({
							.URL_ = item.URL_,
							.Tags_ = tags + item.Categories_,
							.FeedSettings_ = { { IDNotFound, interval, item.MaxArticleNumber_, item.MaxArticleAge_, false } },
						});
			}
			AddFeeds (updatesManager, feeds);
		}
	}

	void StartFileImport (UpdatesManager& updatesManager)
	{
		return HandleOpml (ImportOPMLDialog::Browse {}, updatesManager);
	}

	void HandleOpmlEntity (const Entity& e, std::weak_ptr<UpdatesManager> updatesManager)
	{
		[] (QUrl url, std::weak_ptr<UpdatesManager> updatesManager) -> Util::Task<void>
		{
			const auto remoteResult = co_await HandleOpmlUrl (url);
			const auto opml = co_await WithHandler (remoteResult, ReportError);
			if (const auto um = updatesManager.lock ())
			{
				// `HandleOpml` spins a nested event loop inside, and while it's safe in this particular coro,
				// it's annoyingly fragile, so it's safer and more robust to invoke it from outside the coro frame.
				QTimer::singleShot (0, [=] { HandleOpml (ToDialogSource (opml), *um); });
			}
		} (e.Entity_.toUrl (), std::move (updatesManager));
	}
}
