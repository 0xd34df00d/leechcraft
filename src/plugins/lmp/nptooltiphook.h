/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "interfaces/lmp/mediainfo.h"

namespace LC
{
namespace LMP
{
	class NowPlayingPixmapHandler;

	class NPTooltipHook : public QObject
	{
		NowPlayingPixmapHandler * const PxHandler_;

		QString Base64Px_;

		MediaInfo Info_;
	public:
		NPTooltipHook (NowPlayingPixmapHandler*, QObject* = nullptr);

		void SetTrackInfo (const MediaInfo&);

		bool eventFilter (QObject*, QEvent*);
	};
}
}
