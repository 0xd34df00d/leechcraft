/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QList>

class QUrl;

namespace LC::Poshuku::SpeedDial
{
	extern const QString SpeedDialHost;
	extern const QString SpeedDialUrl;

	extern const QString ThumbPath;
	extern const QString ThumbUrlBase;
	extern const QString ThumbUrlKey;

	const size_t Rows = 2;
	const size_t Cols = 4;

	using TopList_t = QList<QPair<QUrl, QString>>;
}
