/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "commonpl.h"
#include <QFileInfo>
#include <QDir>
#include <QUrl>

namespace LC
{
namespace LMP
{
	Playlist CommonRead2Sources (const ReadParams& params)
	{
		const auto& plDir = QFileInfo (params.Path_).absoluteDir ();

		Playlist result;

		for (const auto& raw : params.RawParser_ (params.Path_))
		{
			const auto& src = raw.SourceStr_;

			QUrl url (src);
			if (!url.scheme ().isEmpty ())
			{
				result.Append ({
						url.scheme () == "file" ? url.toLocalFile () : url,
						raw.Additional_
					});
				continue;
			}

			const QFileInfo fi (src);
			if (params.Suffixes_.contains (fi.suffix ()))
				result += CommonRead2Sources ({ params.Suffixes_,
							plDir.absoluteFilePath (src), params.RawParser_ });
			else if (fi.isRelative ())
				result.Append ({ plDir.absoluteFilePath (src), raw.Additional_ });
			else
				result.Append ({ src, raw.Additional_ });
		}

		return result;
	}
}
}
