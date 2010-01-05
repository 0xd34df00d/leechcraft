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

#include "livestreamdevice.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			LiveStreamDevice::LiveStreamDevice (const libtorrent::torrent_handle& h,
					QObject *parent)
			: QIODevice (parent)
			, Handle_ (h)
			, FinishedPieces_ (h.get_torrent_info ().num_pieces (), false)
			, LastIndex_ (0)
			, EndRangePos_ (0)
			, ReadPos_ (0)
			, Available_ (0)
			{
			}

			void LiveStreamDevice::GotPiece (int index)
			{
				Handle_.read_piece (index);
			}

			void LiveStreamDevice::PieceRead (const libtorrent::read_piece_alert& a)
			{
				if (!a.buffer)
					return;

				int index = a.piece;
				FinishedPieces_ [index] = true;
				PieceData_ [index] = QByteArray (a.buffer.get (), a.size);

				CheckNextChunk ();
			}

			qint64 LiveStreamDevice::readData (char*, qint64)
			{
				return -1;
			}

			qint64 LiveStreamDevice::writeData (const char*, qint64)
			{
				return -1;
			}

			void LiveStreamDevice::CheckNextChunk ()
			{
				bool hasMoreData = false;
				for ( ; EndRangePos_ < FinishedPieces_.size () &&
						FinishedPieces_ [EndRangePos_]; ++EndRangePos_, hasMoreData = true);

				if (hasMoreData)
					emit readyRead ();
			}
		};
	};
};

