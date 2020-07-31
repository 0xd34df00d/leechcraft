/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xspf.h"
#include <QFile>
#include <QDomDocument>
#include <QtDebug>
#include "commonpl.h"

namespace LC
{
namespace LMP
{
namespace XSPF
{
	QList<RawReadData> Read (const QString& path)
	{
		QFile file (path);
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open"
					<< path
					<< file.errorString ();
			return {};
		}

		QDomDocument doc;
		if (!doc.setContent (file.readAll ()))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to parse"
					<< path;
			return {};
		}

		QList<RawReadData> result;
		auto track = doc.documentElement ()
				.firstChildElement ("trackList")
				.firstChildElement ("track");
		while (!track.isNull ())
		{
			const auto& loc = track.firstChildElement ("location").text ();
			if (!loc.isEmpty ())
				result.append ({ loc, {} });

			track = track.nextSiblingElement ("track");
		}
		return result;
	}

	Playlist Read2Sources (const QString& path)
	{
		return CommonRead2Sources ({ { "xspf" }, path, Read });
	}
}
}
}
