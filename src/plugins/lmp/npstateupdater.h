/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <QPixmap>
#include "util/lmp/mediainfo.h"
#include "interfaces/lmp/isourceobject.h"

class QLabel;

namespace LC
{
namespace LMP
{
	class Player;
	class NowPlayingWidget;

	class NPStateUpdater : public QObject
	{
		Q_OBJECT

		QLabel * const NPLabel_;
		NowPlayingWidget * const NPWidget_;

		Player * const Player_;

		QString LastNotificationString_;

		bool IgnoreNextStop_ = false;
	public:
		typedef std::function<void (MediaInfo, QString, QPixmap)> PixmapHandler_f;
	private:
		QList<PixmapHandler_f> PixmapHandlers_;
	public:
		NPStateUpdater (QLabel *label, NowPlayingWidget *npWidget, Player *player, QObject *parent);

		void AddPixmapHandler (const PixmapHandler_f&);
	private:
		QString BuildNotificationText (const MediaInfo&) const;
		void EmitNotification (const QString&, QPixmap);
		void ForceEmitNotification (const QString&, QPixmap);
		void Update (MediaInfo);
	public slots:
		void forceEmitNotification ();
	private slots:
		void update (SourceState);
		void update (const MediaInfo&);
	};
}
}
