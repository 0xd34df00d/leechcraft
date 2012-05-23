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

#include <memory>
#include <QObject>
#include <QList>
#include <interfaces/media/ilyricsfinder.h>

class QDataStream;

namespace LeechCraft
{
namespace DeadLyrics
{
	class Searcher : public QObject
	{
		Q_OBJECT
	public:
		virtual ~Searcher ();

		virtual void Search (const Media::LyricsQuery&, Media::QueryOptions) = 0;
	signals:
		void gotLyrics (const Media::LyricsQuery&, const QStringList&);
	};

	typedef std::shared_ptr<Searcher> Searcher_ptr;
	typedef QList<Searcher_ptr> Searchers_t;
}
}
