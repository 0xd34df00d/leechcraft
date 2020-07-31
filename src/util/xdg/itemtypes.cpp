/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemtypes.h"
#include <QStringList>
#include <QDir>
#include <QtDebug>
#include <util/sll/prelude.h>

namespace LC
{
namespace Util
{
namespace XDG
{
	namespace
	{
		QStringList ToPathsImpl (Type type)
		{
			switch (type)
			{
			case Type::Application:
			case Type::Dir:
			case Type::URL:
				return
				{
					"/usr/share/applications",
					"/usr/local/share/applications"
				};
			case Type::Other:
				return {};
			}

			qWarning () << Q_FUNC_INFO
					<< "unknown type"
					<< static_cast<int> (type);
			return {};
		}

		QStringList Recurse (const QString& path)
		{
			const auto& infos = QDir { path }.entryInfoList (QDir::AllDirs | QDir::NoDotAndDotDot);

			QStringList result { path };
			result += Util::ConcatMap (infos,
					[] (const QFileInfo& info)
					{
						return Recurse (info.absoluteFilePath ());
					});
			return result;
		}

		QStringList ToPathsRecurse (Type type)
		{
			return Util::ConcatMap (ToPathsImpl (type), Recurse);
		}
	}

	QStringList ToPaths (const QList<Type>& types)
	{
		return Util::ConcatMap (types, ToPathsRecurse);
	}
}
}
}
