/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bookmark.h"
#include <QDataStream>
#include <QDomElement>
#include <QtDebug>
#include "interfaces/monocle/ilink.h"

namespace LC::Monocle
{
	NavigationAction Bookmark::ToNavigationAction () const
	{
		const QRectF targetArea { Position_, QSizeF { 1, 1 } };
		return { .PageNumber_ = Page_, .TargetArea_ = targetArea };
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
		return
		{
			name,
			page,
			{ posElem.attribute ("x").toDouble (), posElem.attribute ("y").toDouble () }
		};
	}

	bool operator== (const Bookmark& b1, const Bookmark& b2)
	{
		return b1.Position_ == b2.Position_ && b1.Name_ == b2.Name_;
	}
}
