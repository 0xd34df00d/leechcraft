/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "historyentry.h"
#include <QDataStream>
#include <util/structuresops.h>

namespace LC
{
namespace HistoryHolder
{
	QDataStream& operator<< (QDataStream& out, const HistoryEntry& e)
	{
		quint16 version = 1;
		out << version;

		out << e.Entity_
			<< e.DateTime_;

		return out;
	}

	QDataStream& operator>> (QDataStream& in, HistoryEntry& e)
	{
		quint16 version;
		in >> version;
		if (version == 1)
		{
			in >> e.Entity_
				>> e.DateTime_;
		}
		else
		{
			qWarning () << Q_FUNC_INFO
				<< "unknown version"
				<< version;
		}
		return in;
	}
}
}
