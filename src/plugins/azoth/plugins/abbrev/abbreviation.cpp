/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "abbreviation.h"
#include <QDataStream>
#include <QtDebug>

namespace LC::Azoth::Abbrev
{
	QDataStream& operator<< (QDataStream& str, const Abbreviation& abbr)
	{
		str << static_cast<quint8> (1)
				<< abbr.Pattern_
				<< abbr.Expansion_;
		return str;
	}

	QDataStream& operator>> (QDataStream& str, Abbreviation& abbr)
	{
		quint8 version = 0;
		str >> version;
		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return str;
		}

		str >> abbr.Pattern_
				>> abbr.Expansion_;
		return str;
	}
}
