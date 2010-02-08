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
#include "core.h"

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
			, ReadPos_ (0)
			, Offset_ (0)
			, IsReady_ (false)
			{
				boost::filesystem::path tpath = h.save_path ();
				boost::filesystem::path fpath = h.get_torrent_info ().file_at (0).path;
				boost::filesystem::path abspath = tpath / fpath;
				File_.setFileName (QString::fromUtf8 (abspath.string ().c_str ()));

				if (!QIODevice::open (QIODevice::ReadOnly | QIODevice::Unbuffered))
					qWarning () << Q_FUNC_INFO
						<< "could not open internal IO device"
						<< QIODevice::errorString ();

				reschedule ();
			}

			qint64 LiveStreamDevice::bytesAvailable () const
			{
				qint64 result = 0;
				const libtorrent::torrent_info& ti = Handle_.get_torrent_info ();;
				const libtorrent::bitfield& pieces = Handle_.status ().pieces;
				qDebug () << Q_FUNC_INFO << Offset_ << ReadPos_ << pieces [ReadPos_];
				for (int i = ReadPos_; pieces [i]; ++i)
				{
					result += ti.piece_size (i);
					qDebug () << "added:" << result;
				}
				result -= Offset_;
				if (result < 0)
					result = 0;
				qDebug () << "result:" << result;
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
				result += Offset_;
				return result;
			}

			bool LiveStreamDevice::seek (qint64 pos)
			{
				QIODevice::seek (pos);
				qDebug () << Q_FUNC_INFO << pos;

				const libtorrent::torrent_info& ti = Handle_.get_torrent_info ();

				int i = 0;
				while (pos >= ti.piece_size (i))
					pos -= ti.piece_size (i++);

				ReadPos_ = i;
				Offset_ = pos;

				qDebug () << ReadPos_ << Offset_;

				reschedule ();

				return true;
			}

			qint64 LiveStreamDevice::size () const
			{
				return Handle_.status ().total_wanted;
			}

			void LiveStreamDevice::PieceRead (const libtorrent::read_piece_alert& a)
			{
				int index = a.piece;

				CheckReady ();
				CheckNextChunk ();
				reschedule ();
			}

			void LiveStreamDevice::CheckReady ()
			{
				const libtorrent::bitfield& pieces = Handle_.status ().pieces;
				if (!IsReady_ &&
						pieces [0] &&
						pieces [NumPieces_ - 1])
				{
					std::vector<int> prios (NumPieces_, 1); 
					Handle_.prioritize_pieces (prios);

					IsReady_ = true;
					emit ready ();
				}
			}

			qint64 LiveStreamDevice::readData (char *data, qint64 max)
			{
				const libtorrent::torrent_info& ti = Handle_.get_torrent_info ();
				if (!File_.open (QIODevice::ReadOnly))
				{
					qWarning () << Q_FUNC_INFO
						<< "could not open underlying file"
						<< File_.fileName ()
						<< File_.errorString ();
					return -1;
				}
				qint64 ba = bytesAvailable ();
				File_.seek (pos ());
				const qint64 result = File_.read (data, std::min (max, ba));
				File_.close ();

				qDebug () << Q_FUNC_INFO << result << Offset_ << ReadPos_ << max << ba;

				Offset_ += result;
				while (Offset_ >= ti.piece_size (ReadPos_))
					Offset_ -= ti.piece_size (ReadPos_++);

				qDebug () << Offset_ << ReadPos_ << bytesAvailable ();

				return result;
			}

			qint64 LiveStreamDevice::writeData (const char*, qint64)
			{
				return -1;
			}

			void LiveStreamDevice::CheckNextChunk ()
			{
				bool hasMoreData = false;
				const libtorrent::bitfield& pieces = Handle_.status ().pieces;
				for (int i = ReadPos_ + 1;
						i < NumPieces_ && pieces [i];
						++i, hasMoreData = true);

				if (hasMoreData)
					emit readyRead ();
			}

			void LiveStreamDevice::reschedule ()
			{
				Core::Instance ()->queryLibtorrentForWarnings ();

				const libtorrent::bitfield& pieces = Handle_.status ().pieces;

				int speed = Handle_.status ().download_payload_rate;
				int size = Handle_.get_torrent_info ().piece_length ();
				const int time = speed ?
					static_cast<double> (size) / speed * 1000 :
					60000;

				int thisDeadline = 0;
				for (int i = ReadPos_; i < NumPieces_; ++i)
					if (!pieces [i])
						Handle_.set_piece_deadline (i,
								IsReady_ ?
									(thisDeadline += time) :
									1000000,
								1);
				if (!IsReady_)
				{
					std::vector<int> prios (NumPieces_, 0); 
					if (pieces.size () > 1)
						prios [1] = 1;

					if (!pieces [0])
					{
						qDebug () << "scheduling first piece";
						Handle_.set_piece_deadline (0, 500, 1);
						prios [0] = 7;
					}
					if (!pieces [NumPieces_ - 1])
					{
						qDebug () << "scheduling last piece";
						Handle_.set_piece_deadline (NumPieces_ - 1, 500, 1);
						prios [NumPieces_ - 1] = 7;
					}
					Handle_.prioritize_pieces (prios);
				}
			}
		};
	};
};

