/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_BITTORRENT_LIVESTREAMMANAGER_H
#define PLUGINS_BITTORRENT_LIVESTREAMMANAGER_H
#include <QObject>
#include <QList>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/alert_types.hpp>
#include <interfaces/structures.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class LiveStreamDevice;

			class LiveStreamManager : public QObject
			{
				Q_OBJECT

				QMap<libtorrent::torrent_handle, LiveStreamDevice*> Handle2Device_;
			public:
				LiveStreamManager (QObject* = 0);

				void EnableOn (libtorrent::torrent_handle);
				void PieceRead (const libtorrent::read_piece_alert&);
			public slots:
				void handleGotPiece (int, void*);
			signals:
				void gotEntity (const LeechCraft::DownloadEntity&);
			};
		};
	};
};

#endif

