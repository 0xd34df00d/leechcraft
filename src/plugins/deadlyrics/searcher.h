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

class QDataStream;

namespace LeechCraft
{
namespace DeadLyrics
{
	struct Lyrics
	{
		QString Author_;
		QString Album_;
		QString Title_;
		QString Text_;
		QString URL_;
	};

	bool operator== (const Lyrics&, const Lyrics&);
	QDataStream& operator<< (QDataStream&, const Lyrics&);
	QDataStream& operator>> (QDataStream&, Lyrics&);
	typedef QList<Lyrics> LyricsList_t;

	class Searcher : public QObject
	{
		Q_OBJECT
	public:
		virtual ~Searcher ();
		virtual void Start (const QStringList&, QByteArray&) = 0;
		virtual void Stop (const QByteArray&) = 0;
	signals:
		void textFetched (const LeechCraft::DeadLyrics::Lyrics&, const QByteArray&);
		void error (const QString&);
	};

	typedef std::shared_ptr<Searcher> Searcher_ptr;
	typedef QList<Searcher_ptr> Searchers_t;
}
}
