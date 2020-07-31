/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lyricscache.h"
#include <stdexcept>
#include <QFile>
#include <QtDebug>
#include <util/util.h>
#include <util/sys/paths.h>

namespace LC
{
namespace DeadLyrics
{
	LyricsCache::LyricsCache ()
	{
		try
		{
			Dir_ = Util::CreateIfNotExists ("deadlyrics/cache");
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO << e.what ();
		}
	}

	LyricsCache& LyricsCache::Instance ()
	{
		static LyricsCache lc;
		return lc;
	}

	QStringList LyricsCache::GetLyrics (const Media::LyricsQuery&) const
	{
		return QStringList ();
	}

	void LyricsCache::AddLyrics (const Media::LyricsQuery& , const QStringList&)
	{
	}
}
}
