/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDate>
#include <QString>
#include <QStringList>

namespace LC
{
namespace BitTorrent
{
	struct NewTorrentParams
	{
		QString Output_
			, AnnounceURL_
			, Comment_
			, Path_;
		QDate Date_;
		int PieceSize_;
		QStringList URLSeeds_;
		QStringList DHTNodes_;
		bool DHTEnabled_;
	};
}
}
