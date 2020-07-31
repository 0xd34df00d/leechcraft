/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "searcher.h"

class QNetworkAccessManager;

namespace LC
{
namespace DeadLyrics
{
	class HascirylSearcher : public Searcher
	{
		QNetworkAccessManager * const NAM_;
	public:
		HascirylSearcher (QNetworkAccessManager*);

		void Search (const Media::LyricsQuery&, const Reporter_t&) override;
	private:
		void HandleLyricsUrls (const Reporter_t&, const QByteArray&);
		void HandleLyricsPageFetched (const Reporter_t&, const QString&, const QByteArray&);
		void HandleGotLyricsReply (const Reporter_t&, const QString&, const QByteArray&);
	};
}
}
