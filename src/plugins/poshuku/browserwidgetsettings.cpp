/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "browserwidgetsettings.h"
#include <QDataStream>
#include <QtDebug>

namespace LC
{
namespace Poshuku
{
	QDataStream& operator<< (QDataStream& out, const BrowserWidgetSettings& s)
	{
		qint8 version = 4;
		out << version
			<< s.ZoomFactor_
			<< s.NotifyWhenFinished_
			<< s.ReloadInterval_
			<< s.WebHistorySerialized_
			<< s.ScrollPosition_
			<< s.DefaultEncoding_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, BrowserWidgetSettings& s)
	{
		qint8 version;
		in >> version;
		if (version >= 1)
			in >> s.ZoomFactor_
				>> s.NotifyWhenFinished_
				>> s.ReloadInterval_;
		if (version >= 2)
			in >> s.WebHistorySerialized_;
		if (version >= 3)
			in >> s.ScrollPosition_;
		if (version >= 4)
			in >> s.DefaultEncoding_;

		if (version > 4 || version < 1)
			qWarning () << Q_FUNC_INFO
				<< "unknown version"
				<< version;
		return in;
	}
}
}
