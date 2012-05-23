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
#include <QHash>
#include <interfaces/media/ilyricsfinder.h>
#include <interfaces/core/icoreproxy.h>

class QDomElement;

namespace LeechCraft
{
namespace DeadLyrics
{
	class MatcherBase;
	typedef std::shared_ptr<MatcherBase> MatcherBase_ptr;

	struct ConcreteSiteDesc
	{
		QString Name_;
		QString Charset_;
		QString URLTemplate_;

		QHash<QChar, QString> Replacements_;

		QList<MatcherBase_ptr> Matchers_;

		ConcreteSiteDesc (const QDomElement&);
	};

	class ConcreteSite : public QObject
	{
		Q_OBJECT

		const Media::LyricsQuery Query_;
		const ConcreteSiteDesc Desc_;
	public:
		ConcreteSite (const Media::LyricsQuery&,
				const ConcreteSiteDesc&, ICoreProxy_ptr proxy, QObject* = 0);
	private slots:
		void handleReplyFinished ();
	signals:
		void gotLyrics (const Media::LyricsQuery&, const QStringList&);
	};
}
}
