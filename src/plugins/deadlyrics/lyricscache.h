/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QString>
#include <QDir>
#include <interfaces/media/ilyricsfinder.h>

namespace LC
{
namespace DeadLyrics
{
	class LyricsCache : public QObject
	{
		Q_OBJECT

		QDir Dir_;
		LyricsCache ();
	public:
		static LyricsCache& Instance ();

		QStringList GetLyrics (const Media::LyricsQuery&) const;
		void AddLyrics (const Media::LyricsQuery&, const QStringList&);
	};
}
}
