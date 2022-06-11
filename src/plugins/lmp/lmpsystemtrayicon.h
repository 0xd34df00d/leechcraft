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

namespace LC::Util
{
	class FancyTrayIcon;
}

namespace LC::LMP
{
	class PlayerTab;

	class LMPSystemTrayIcon : public QObject
	{
		Q_OBJECT

		Util::FancyTrayIcon& Icon_;
	public:
		explicit LMPSystemTrayIcon (const QIcon& icon, QObject *parent = nullptr);

		void SetMenu (QMenu*);
		void SetVisible (bool);
		void SetIcon (const QIcon&);

		void UpdateSongInfo (const MediaInfo& song);
	signals:
		void changedVolume (qreal);
		void playPauseToggled ();
	};
}
