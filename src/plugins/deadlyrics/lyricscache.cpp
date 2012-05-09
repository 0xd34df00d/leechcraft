/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "lyricscache.h"
#include <stdexcept>
#include <QFile>
#include <QtDebug>
#include <util/util.h>

namespace LeechCraft
{
namespace DeadLyrics
{
	LyricsCache::LyricsCache ()
	{
		try
		{
			LeechCraft::Util::CreateIfNotExists ("deadlyrics/cache");
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO << e.what ();
			return;
		}

		Dir_ = QDir::homePath ();
		Dir_.cd (".leechcraft/deadlyrics/cache");
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
