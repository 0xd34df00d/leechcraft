/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QHash>

namespace LC::Monocle::Boop
{
	struct PathItem
	{
		QString Path_;
		QString Mime_;
	};

	struct Manifest
	{
		QHash<QString, PathItem> Id2Item_;
		QVector<QString> Spine_;
	};

	Manifest ParseManifest (const QString& epubFile);
}
