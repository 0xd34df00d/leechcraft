/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QList>
#include <QMetaType>
#include <QString>

class QDataStream;

namespace LC::Azoth::Abbrev
{
	struct Abbreviation
	{
		QString Pattern_;
		QString Expansion_;
	};

	QDataStream& operator<< (QDataStream&, const Abbreviation&);
	QDataStream& operator>> (QDataStream&, Abbreviation&);
}

Q_DECLARE_METATYPE (LC::Azoth::Abbrev::Abbreviation)
Q_DECLARE_METATYPE (QList<LC::Azoth::Abbrev::Abbreviation>)
