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

#pragma once

#include <QStringList>

namespace Media
{
	struct LyricsQuery
	{
		QString Artist_;
		QString Album_;
		QString Title_;

		LyricsQuery ()
		{
		}

		LyricsQuery (const QString& artist, const QString& album, const QString& title)
		: Artist_ (artist)
		, Album_ (album)
		, Title_ (title)
		{
		}
	};

	enum QueryOption
	{
		NoOption = 0x0,
		Refresh = 0x1
	};

	Q_DECLARE_FLAGS (QueryOptions, QueryOption);

	class Q_DECL_EXPORT ILyricsFinder
	{
	public:
		virtual ~ILyricsFinder () {}

		virtual void RequestLyrics (const LyricsQuery& query, QueryOptions = NoOption) = 0;
	protected:
		virtual void gotLyrics (const LyricsQuery& query, const QStringList& lyrics) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::ILyricsFinder, "org.LeechCraft.Media.ILyricsFinder/1.0");
