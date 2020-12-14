/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "extensionsdata.h"
#include <QIcon>
#include <QMimeDatabase>

namespace LC::Util
{
	ExtensionsData& ExtensionsData::Instance ()
	{
		static ExtensionsData ed;
		return ed;
	}

	namespace
	{
		auto GetMimeTypeForExt (const QString& extension)
		{
			return QMimeDatabase {}.mimeTypeForFile (extension, QMimeDatabase::MatchExtension);
		}
	}

	QString ExtensionsData::GetMime (const QString& extension) const
	{
		return GetMimeTypeForExt (extension).name ();
	}

	QIcon ExtensionsData::GetExtIcon (const QString& extension) const
	{
		return QIcon::fromTheme (GetMimeTypeForExt (extension).iconName ());
	}

	QIcon ExtensionsData::GetMimeIcon (const QString& mime) const
	{
		return QIcon::fromTheme (QMimeDatabase {}.mimeTypeForName (mime).iconName ());
	}
}
