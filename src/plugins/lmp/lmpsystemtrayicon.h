/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSystemTrayIcon>
#include "mediainfo.h"

namespace LC::LMP
{
	class PlayerTab;

	class LMPSystemTrayIcon : public QSystemTrayIcon
	{
	public:
		explicit LMPSystemTrayIcon (const QIcon& icon, QObject *parent = nullptr);

		void UpdateSongInfo (const MediaInfo& song);
	};
}
