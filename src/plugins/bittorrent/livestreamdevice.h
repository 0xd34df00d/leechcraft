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

#ifndef PLUGINS_BITTORRENT_LIVESTREAMDEVICE_H
#define PLUGINS_BITTORRENT_LIVESTREAMDEVICE_H
#include <QIODevice>
#include <QVector>
#include <QMap>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/alert_types.hpp>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class LiveStreamDevice : public QIODevice
			{
				Q_OBJECT

				libtorrent::torrent_handle Handle_;
				QVector<bool> FinishedPieces_;
				QMap<int, QByteArray> PieceData_;
				int LastIndex_;
				// The last piece in the [start; end] range where all
				// the pieces are finished.
				int EndRangePos_;
				int ReadPos_;
				qint64 Available_;
			public:
				LiveStreamDevice (const libtorrent::torrent_handle&,
						QObject* = 0);

				void GotPiece (int);
				void PieceRead (const libtorrent::read_piece_alert&);

			protected:
				virtual qint64 readData (char*, qint64);
				virtual qint64 writeData (const char*, qint64);
			private:
				void CheckNextChunk ();
			};
		};
	};
};

#endif

