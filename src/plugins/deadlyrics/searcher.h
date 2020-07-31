/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <functional>
#include <QObject>
#include <QList>
#include <interfaces/media/ilyricsfinder.h>

namespace LC
{
namespace DeadLyrics
{
	class Searcher : public QObject
	{
		Q_OBJECT
	public:
		virtual ~Searcher ();

		using Reporter_t = std::function<void (Media::ILyricsFinder::LyricsQueryResult_t)>;

		virtual void Search (const Media::LyricsQuery&, const Reporter_t& reporter) = 0;
	};

	typedef std::shared_ptr<Searcher> Searcher_ptr;
	typedef QList<Searcher_ptr> Searchers_t;
}
}
