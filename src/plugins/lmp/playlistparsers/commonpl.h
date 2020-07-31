/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QStringList>
#include <QVariantMap>
#include "engine/audiosource.h"
#include "playlist.h"

namespace LC
{
namespace LMP
{
	struct ReadParams
	{
		QStringList Suffixes_;
		QString Path_;

		std::function<QList<RawReadData> (QString)> RawParser_;
	};

	Playlist CommonRead2Sources (const ReadParams&);
}
}
