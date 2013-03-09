/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "bookmark.h"
#include <QDataStream>
#include <QtDebug>

namespace LeechCraft
{
namespace Monocle
{
	Bookmark::Bookmark ()
	: Page_ (0)
	{
	}

	Bookmark::Bookmark (int page, const QPoint& position)
	: Page_ (page)
	, Position_ (position)
	{
	}

	int Bookmark::GetPage () const
	{
		return Page_;
	}

	void Bookmark::SetPage (int page)
	{
		Page_ = page;
	}

	QPoint Bookmark::GetPosition () const
	{
		return Position_;
	}

	void Bookmark::SetPosition (const QPoint& p)
	{
		Position_ = p;
	}

	QDataStream& operator<< (QDataStream& out, const Bookmark& bm)
	{
		return out << static_cast<quint8> (1)
				<< bm.GetPage ()
				<< bm.GetPosition ();
	}

	QDataStream& operator>> (QDataStream& in, Bookmark& bm)
	{
		quint8 version = 0;
		in >> version;
		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return in;
		}

		int page = 0;
		QPoint p;

		in >> page >> p;

		bm.SetPage (page);
		bm.SetPosition (p);

		return in;
	}
}
}
