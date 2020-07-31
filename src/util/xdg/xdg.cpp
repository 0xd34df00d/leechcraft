/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xdg.h"
#include <QIcon>
#include <QFile>

namespace LC
{
namespace Util
{
namespace XDG
{
	QIcon GetAppIcon (const QString& name)
	{
		return GetAppPixmap (name);
	}

	QPixmap GetAppPixmap (const QString& name)
	{
		const auto prefixes =
		{
			"/usr/share/pixmaps/",
			"/usr/local/share/pixmaps/"
		};

		const auto sizes = { "192", "128", "96", "72", "64", "48", "36", "32" };
		const QStringList themes
		{
			"/usr/local/share/icons/hicolor/",
			"/usr/share/icons/hicolor/"
		};

		for (auto ext : { ".png", ".svg", ".xpm", ".jpg", "" })
		{
			for (auto prefix : prefixes)
				if (QFile::exists (prefix + name + ext))
					return { prefix + name + ext };

			for (auto themeDir : themes)
				for (const auto& size : sizes)
				{
					const auto& str = themeDir + size + 'x' + size + "/apps/" + name + ext;
					if (QFile::exists (str))
						return { str };
				}
		}

		return {};
	}
}
}
}
