/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "icondatabase.h"
#include <functional>
#include <QUrl>
#include <QIcon>
#include "icondatabaseondisk.h"
#include "stringpathtrie.h"

namespace LC::Poshuku::WebEngineView
{
	IconDatabase::IconDatabase (QObject *parent)
	: QObject { parent }
	, DB_ { std::make_shared<IconDatabaseOnDisk> () }
	, Trie_ { std::make_shared<StringPathTrie<QUrl>> () }
	{
		for (const auto& [pageUrl, iconUrl] : DB_->GetAllPages ())
			MarkUrl (pageUrl, iconUrl);
	}

	void IconDatabase::UpdateIcon (const QUrl& pageUrl, const QIcon& icon, const QUrl& iconUrl)
	{
		if (icon.isNull ())
			return;

		DB_->UpdateIcon (pageUrl, icon, iconUrl);
		MarkUrl (pageUrl, iconUrl);
	}

	namespace
	{
		template<typename F>
		auto WithRefs (const QUrl& url, F&& fun)
		{
			auto path = url.path ();
			auto refs = path.splitRef ('/', Qt::SkipEmptyParts);
			auto host = url.host ();
			refs.prepend ({ &host });

			return std::invoke (std::forward<F> (fun), std::move (refs));
		}
	}

	QIcon IconDatabase::GetIcon (const QUrl& pageUrl)
	{
		const auto maybeMatch = WithRefs (pageUrl, [this] (const auto& refs) { return Trie_->Find (refs); }).Value_;
		return maybeMatch ?
				DB_->GetIcon (*maybeMatch) :
				QIcon {};
	}

	void IconDatabase::MarkUrl (const QUrl& pageUrl, const QUrl& iconUrl)
	{
		WithRefs (pageUrl, [this, iconUrl] (const auto& refs) { Trie_->Add (refs, std::move (iconUrl)); });
	}
}
