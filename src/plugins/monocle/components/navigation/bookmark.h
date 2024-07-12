/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QPointF>
#include <QString>
#include <QMetaType>
#include "components/layout/positions.h"

class QDomElement;
class QDomDocument;

namespace LC::Monocle
{
	struct NavigationAction;

	struct Bookmark
	{
		QString Name_;
		int Page_ = 0;
		PageRelativePos Position_;

		NavigationAction ToNavigationAction () const;

		bool operator== (const Bookmark&) const = default;
	};

	QDebug operator<< (QDebug, const Bookmark&);
}


Q_DECLARE_METATYPE (LC::Monocle::Bookmark)
