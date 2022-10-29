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

namespace LC::Aggregator
{
	OpmlAdder::OpmlAdder (const AddFeedHandler& handler, QObject *parent)
	: QObject { parent }
	, AddFeedHandler_ { handler }
	{
	}

	bool OpmlAdder::IsOpmlEntity (const Entity& e) const
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

	bool OpmlAdder::HandleOpmlEntity (const Entity& e)
	{
		if (e.Mime_ != "text/x-opml")
			return false;

		auto url = e.Entity_.toUrl ();
		if (url.scheme () == "file")
			StartAddingOpml (url.toLocalFile ());
		else
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
				ReportError (tr ("Could not find plugin to download OPML %1.")
						.arg (url.toString ()));
				return true;
			}

			Util::Sequence (this, handleResult.DownloadResult_) >>
					Util::Visitor
					{
						[this, name] (IDownload::Success) { StartAddingOpml (name); },
						[this] (const IDownload::Error&)
						{
							ReportError (tr ("Unable to download the OPML file."));
						}
					}.Finally ([name] { QFile::remove (name); });
		}

		const auto& s = e.Additional_;
		auto copyVal = [&s] (const QByteArray& name)
		{
			if (s.contains (name))
				XmlSettingsManager::Instance ()->setProperty (name, s.value (name));
		};
		copyVal ("UpdateOnStartup");
		copyVal ("UpdateTimeout");
		copyVal ("MaxArticles");
		copyVal ("MaxAge");

		return true;
	}

	void OpmlAdder::StartAddingOpml (const QString& file)
	{
		ImportOPML importDialog { file };
		if (importDialog.exec () == QDialog::Rejected)
			return;

		const auto& tags = GetProxyHolder ()->GetTagsManager ()->Split (importDialog.GetTags ());
		const auto& selectedUrls = importDialog.GetSelectedUrls ();

		Util::Visit (ParseOPMLItems (importDialog.GetFilename ()),
				[this] (const QString& error) { ReportError (error); },
				[&] (const OPMLParser::items_container_t& items)
				{
					for (const auto& item : items)
					{
						if (!selectedUrls.contains (item.URL_))
							continue;

						int interval = 0;
						if (item.CustomFetchInterval_)
							interval = item.FetchInterval_;
						AddFeedHandler_ (item.URL_, tags + item.Categories_,
								{ { IDNotFound, interval, item.MaxArticleNumber_, item.MaxArticleAge_, false } });
					}
				});
	}

	void OpmlAdder::ReportError (const QString& body) const
	{
		auto e = Util::MakeNotification (tr ("OPML import error"), body, Priority::Critical);
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}
}
