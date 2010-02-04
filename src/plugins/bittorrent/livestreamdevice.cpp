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
#include <QtDebug>

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
			, NumPieces_ (h.get_torrent_info ().num_pieces ())
			, FinishedPieces_ (h.get_torrent_info ().num_pieces (), false)
			, EndRangePos_ (0)
			, ReadPos_ (0)
			, LastReadOffset_ (0)
			, IsReady_ (false)
			{
				boost::filesystem::path tpath = h.save_path ();
				boost::filesystem::path fpath = h.get_torrent_info ().file_at (0).path;
				boost::filesystem::path abspath = tpath / fpath;
				File_.setFileName (QString::fromUtf8 (abspath.string ().c_str ()));
				File_.open (QIODevice::ReadOnly);

				open (QIODevice::ReadOnly | QIODevice::Unbuffered);

				Handle_.set_piece_deadline (0, 10000, 1);
				if (NumPieces_ > 1)
					Handle_.set_piece_deadline (NumPieces_ - 1, 10000, 1);
			}

			qint64 LiveStreamDevice::bytesAvailable () const
			{
				qint64 result = 0;
				const libtorrent::torrent_info& ti = Handle_.get_torrent_info ();;
				for (int i = ReadPos_; FinishedPieces_.at (i); ++i)
					result += ti.piece_size (i);
				result -= LastReadOffset_;
				return result;
			}

			bool LiveStreamDevice::isSequential () const
			{
				return false;
			}

			bool LiveStreamDevice::isWritable () const
			{
				return false;
			}

			bool LiveStreamDevice::open (QIODevice::OpenMode mode)
			{
				return true;
			}

			qint64 LiveStreamDevice::pos () const
			{
				qint64 result = 0;
				const libtorrent::torrent_info& ti = Handle_.get_torrent_info ();;
				for (int i = 0; i < ReadPos_; ++i)
					result += ti.piece_size (i);
				result += LastReadOffset_;
				return result;
			}

			bool LiveStreamDevice::seek (qint64 pos)
			{
				QIODevice::seek (pos);

				const libtorrent::torrent_info& ti = Handle_.get_torrent_info ();
				int piece = ReadPos_ = pos / ti.piece_length ();
				qint64 offset = pos;
				if (piece == NumPieces_ - 1)
					offset -= ti.piece_size (piece--);
				if (piece > 0)
					offset -= static_cast<qint64> (ti.piece_length ()) * piece;
				LastReadOffset_ = offset;

				reschedule ();
			}

			qint64 LiveStreamDevice::size () const
			{
				return Handle_.status ().total_wanted;
			}

			void LiveStreamDevice::GotPiece (int index)
			{
				Handle_.read_piece (index);
			}

			void LiveStreamDevice::PieceRead (const libtorrent::read_piece_alert& a)
			{
				int index = a.piece;
				qDebug () << Q_FUNC_INFO << index << NumPieces_;
				FinishedPieces_ [index] = true;

				if (!IsReady_ &&
						FinishedPieces_.at (0) &&
						FinishedPieces_.at (NumPieces_ - 1))
				{
					IsReady_ = true;
					qDebug () << "ready";
					emit ready ();
				}

				CheckNextChunk ();
				reschedule ();
			}

			qint64 LiveStreamDevice::readData (char *data, qint64 max)
			{
				const libtorrent::torrent_info& ti = Handle_.get_torrent_info ();
				File_.seek (pos ());
				const qint64 result = File_.read (data, max);

				qint64 offset = result;
				if (offset + LastReadOffset_ < ti.piece_size (ReadPos_))
					LastReadOffset_;
				else
				{
					offset -= LastReadOffset_;
					++ReadPos_;
					while (offset > ti.piece_size (ReadPos_))
						offset -= ti.piece_size (ReadPos_);
					LastReadOffset_ = offset;
				}

				return result;
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

			void LiveStreamDevice::reschedule ()
			{
				int speed = Handle_.status ().download_payload_rate;
				int size = Handle_.get_torrent_info ().piece_length () / speed;
				const int time = static_cast<double> (size) / speed * 1000;
				int thisDeadline = 0;
				for (int i = ReadPos_; i < NumPieces_; ++i)
					if (!FinishedPieces_.at (i))
						Handle_.set_piece_deadline (i, (thisDeadline += time), 1);

				if (!IsReady_ &&
						NumPieces_ > 1)
					Handle_.set_piece_deadline (NumPieces_ - 1, 10000, 1);
			}
		};
	};
};

