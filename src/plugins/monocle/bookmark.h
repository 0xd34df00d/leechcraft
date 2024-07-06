/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QPoint>
#include <QString>
#include <QMetaType>

class QDomElement;
class QDomDocument;

namespace LC
{
namespace Monocle
{
	struct NavigationAction;

	class Bookmark
	{
		QString Name_;
		int Page_ = 0;
		QPoint Position_;
	public:
		Bookmark () = default;
		Bookmark (const QString&, int page, const QPoint& position);

		QString GetName () const;
		void SetName (const QString&);
		int GetPage () const;
		void SetPage (int);
		QPoint GetPosition () const;
		void SetPosition (const QPoint& p);

		NavigationAction ToNavigationAction () const;

		void ToXML (QDomElement&, QDomDocument&) const;
		static Bookmark FromXML (const QDomElement&);
	};

	bool operator== (const Bookmark&, const Bookmark&);
}
}

Q_DECLARE_METATYPE (LC::Monocle::Bookmark)
