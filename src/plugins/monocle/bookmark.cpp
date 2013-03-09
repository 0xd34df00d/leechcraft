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
#include <QDomElement>
#include <QtDebug>

namespace LeechCraft
{
namespace Monocle
{
	Bookmark::Bookmark ()
	: Page_ (0)
	{
	}

	Bookmark::Bookmark (const QString& name, int page, const QPoint& position)
	: Name_ (name)
	, Page_ (page)
	, Position_ (position)
	{
	}

	QString Bookmark::GetName () const
	{
		return Name_;
	}

	void Bookmark::SetName (const QString& name)
	{
		Name_ = name;
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

	void Bookmark::ToXML (QDomElement& elem, QDomDocument& doc) const
	{
		auto pageElem = doc.createElement ("page");
		pageElem.setAttribute ("num", Page_);
		elem.appendChild (pageElem);

		auto posElem = doc.createElement ("pos");
		posElem.setAttribute ("x", Position_.x ());
		posElem.setAttribute ("y", Position_.y ());
		elem.appendChild (posElem);

		elem.setAttribute ("name", Name_);
	}

	Bookmark Bookmark::FromXML (const QDomElement& elem)
	{
		const auto page = elem.firstChildElement ("page").attribute ("num").toInt ();
		const auto& posElem = elem.firstChildElement ("pos");
		const auto& name = elem.attribute ("name");
		return Bookmark (name,
				page,
				{ posElem.attribute ("x").toInt (), posElem.attribute ("y").toInt () });
	}

	QDataStream& operator<< (QDataStream& out, const Bookmark& bm)
	{
		return out << static_cast<quint8> (1)
				<< bm.GetName ()
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

		QString name;
		int page = 0;
		QPoint p;

		in >> name >> page >> p;

		bm = Bookmark (name, page, p);

		return in;
	}
}
}
