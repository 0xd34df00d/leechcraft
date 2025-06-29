/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <qnamespace.h>

namespace LC::BitTorrent
{
	enum class AddState
	{
		Started,
		Paused
	};

	enum class WebSeedType
	{
		Bep17,
		Bep19
	};

	enum TorrentState
	{
		TSIdle,
		TSPreparing,
		TSDownloading,
		TSSeeding
	};

	struct Roles
	{
		enum
		{
			FullProgressText = Qt::UserRole,
			SortRole,
			HandleIndex,
			IsLeeching,
			IsSeeding,
			TorrentHandle,
			TorrentTags,
			IsManaged,
			IsSequentialDownloading,
			IsSuperSeeding,
			TorrentStats,
			TorrentProgress,
		};
	};

	struct Columns
	{
		enum
		{
			ColumnID,
			ColumnName,
			ColumnState,
			ColumnProgress,  // percentage, Downloaded of Size
			ColumnDownSpeed,
			ColumnUpSpeed,
			ColumnLeechers,
			ColumnSeeders,
			ColumnSize,
			ColumnDownloaded,
			ColumnUploaded,
			ColumnRatio
		};
	};
}
