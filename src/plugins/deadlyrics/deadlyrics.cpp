/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "deadlyrics.h"
#include <QIcon>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/threads/futures.h>
#include <util/sll/either.h>
#include <util/util.h>
#include "hascirylsearcher.h"

namespace LC
{
namespace DeadLyrics
{
	void DeadLyRicS::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("deadlyrics");

		Proxy_ = proxy;

		Searchers_ << std::make_shared<HascirylSearcher> (proxy->GetNetworkAccessManager ());
	}

	void DeadLyRicS::SecondInit ()
	{
	}

	void DeadLyRicS::Release ()
	{
		Searchers_.clear ();
	}

	QByteArray DeadLyRicS::GetUniqueID () const
	{
		return "org.LeechCraft.DeadLyrics";
	}

	QString DeadLyRicS::GetName () const
	{
		return "DeadLyRicS";
	}

	QString DeadLyRicS::GetInfo () const
	{
		return tr ("Song lyrics searcher.");
	}

	QIcon DeadLyRicS::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	QFuture<DeadLyRicS::LyricsQueryResult_t> DeadLyRicS::RequestLyrics (const Media::LyricsQuery& query)
	{
		if (query.Artist_.isEmpty () || query.Title_.isEmpty ())
			return Util::MakeReadyFuture (LyricsQueryResult_t { tr ("Not enough parameters to build a search query.") });

		QFutureInterface<LyricsQueryResult_t> promise;
		promise.reportStarted ();
		promise.setExpectedResultCount (Searchers_.size ());

		for (auto searcher : Searchers_)
			searcher->Search (query,
					[promise] (const LyricsQueryResult_t& result) mutable
					{
						const auto currentCount = promise.resultCount ();
						promise.reportResult (result, currentCount);
						if (currentCount + 1 == promise.expectedResultCount ())
							promise.reportFinished ();
					});

		return promise.future ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_deadlyrics, LC::DeadLyrics::DeadLyRicS);
