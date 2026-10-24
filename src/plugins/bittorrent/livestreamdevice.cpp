/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "livestreamdevice.h"
#include <QtDebug>
#include <libtorrent/alert_types.hpp>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <util/xpc/util.h>
#include "cachedstatuskeeper.h"
#include "ltutils.h"

namespace LC::BitTorrent
{
	using th = libtorrent::torrent_handle;
	using PieceIdx_t = libtorrent::piece_index_t;

	LiveStreamDevice::LiveStreamDevice (const libtorrent::torrent_handle& h, CachedStatusKeeper& keeper, QObject *parent)
	: QIODevice (parent)
	, StatusKeeper_ (keeper)
	, Handle_ (h)
	, TI_
	{
		[&]
		{
			const auto tf = keeper.GetStatus (h, th::query_pieces | th::query_torrent_file).torrent_file.lock ();
			if (!tf)
				throw std::runtime_error { LiveStreamDevice::tr ("No metadata is available yet.").toStdString () };
			return *tf;
		} ()
	}
	{
		const auto& tpath = keeper.GetStatus (h, th::query_save_path).save_path;
		const auto& fpath = GetCurrentFilePath (h, TI_, libtorrent::file_index_t { 0 });
		File_.setFileName (QString::fromStdString (tpath + '/' + fpath));

		if (!QIODevice::open (QIODevice::ReadOnly | QIODevice::Unbuffered))
		{
			qWarning () << Q_FUNC_INFO
					<< "could not open internal IO device"
					<< QIODevice::errorString ();
			throw std::runtime_error { QIODevice::errorString ().toStdString () };
		}

		Reschedule ();
	}

	qint64 LiveStreamDevice::bytesAvailable () const
	{
		qint64 result = 0;
		const auto& pieces = StatusKeeper_.GetStatus (Handle_, th::query_pieces).pieces;
		const auto piecesCount = std::min (NumPieces_, static_cast<int> (pieces.size ()));
		for (int i = ReadPos_; i < piecesCount && pieces [PieceIdx_t { i }]; ++i)
			result += TI_.piece_size (PieceIdx_t { i });
		result -= Offset_;
		if (result < 0)
			result = 0;
		return result;
	}

	bool LiveStreamDevice::isSequential () const
	{
		return false;
	}

	bool LiveStreamDevice::open (QIODevice::OpenMode)
	{
		return true;
	}

	qint64 LiveStreamDevice::pos () const
	{
		qint64 result = 0;
		for (int i = 0; i < ReadPos_; ++i)
			result += TI_.piece_size (PieceIdx_t { i });
		result += Offset_;
		return result;
	}

	bool LiveStreamDevice::seek (qint64 pos)
	{
		if (pos < 0 || pos > size () || !QIODevice::seek (pos))
			return false;

		int i = 0;
		while (i < NumPieces_ && pos >= TI_.piece_size (PieceIdx_t { i }))
			pos -= TI_.piece_size (PieceIdx_t { i++ });
		ReadPos_ = i;
		Offset_ = pos;

		Reschedule ();

		return true;
	}

	qint64 LiveStreamDevice::size () const
	{
		return TI_.total_size ();
	}

	void LiveStreamDevice::PieceRead (const libtorrent::read_piece_alert&)
	{
		CheckReady ();
		CheckNextChunk ();
		Reschedule ();
	}

	void LiveStreamDevice::CheckReady ()
	{
		if (IsReady_ || !NumPieces_)
			return;

		const auto& pieces = StatusKeeper_.GetStatus (Handle_, th::query_pieces).pieces;
		if (pieces.size () < NumPieces_)
			return;

		if (pieces [PieceIdx_t { 0 }] &&
				pieces [PieceIdx_t { NumPieces_ - 1 }])
		{
			std::vector<libtorrent::download_priority_t> prios (NumPieces_, libtorrent::low_priority);
			Handle_.prioritize_pieces (prios);

			IsReady_ = true;
			EmitReadyEntity ();
		}
	}

	qint64 LiveStreamDevice::readData (char *data, qint64 max)
	{
		if (max <= 0 || ReadPos_ >= NumPieces_)
			return 0;

		if (!File_.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
				<< "could not open underlying file"
				<< File_.fileName ()
				<< File_.errorString ();
			return -1;
		}
		const qint64 ba = bytesAvailable ();
		if (!File_.seek (pos ()))
		{
			File_.close ();
			return -1;
		}
		const qint64 result = File_.read (data, std::min (max, ba));
		File_.close ();
		if (result <= 0)
			return result;

		qint64 offset = Offset_ + result;
		while (ReadPos_ < NumPieces_ && offset >= TI_.piece_size (PieceIdx_t { ReadPos_ }))
			offset -= TI_.piece_size (PieceIdx_t { ReadPos_++ });
		Offset_ = static_cast<int> (offset);

		return result;
	}

	qint64 LiveStreamDevice::writeData (const char*, qint64)
	{
		return -1;
	}

	void LiveStreamDevice::CheckNextChunk ()
	{
		bool hasMoreData = false;
		const auto& pieces = StatusKeeper_.GetStatus (Handle_, th::query_pieces).pieces;
		const auto piecesCount = std::min (NumPieces_, static_cast<int> (pieces.size ()));
		for (int i = ReadPos_ + 1;
				i < piecesCount && pieces [PieceIdx_t { i }];
				++i, hasMoreData = true);

		if (hasMoreData)
			emit readyRead ();
	}

	void LiveStreamDevice::EmitReadyEntity ()
	{
		Entity e
		{
			.Entity_ = QVariant::fromValue<QIODevice*> (this),
			.Location_ = {},
			.Mime_ = QStringLiteral ("x-leechcraft/media-qiodevice"),
			.Parameters_ = FromUserInitiated,
			.Additional_ = {},
		};
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	void LiveStreamDevice::Reschedule ()
	{
		const auto& status = StatusKeeper_.GetStatus (Handle_, th::query_pieces);
		const auto& pieces = status.pieces;
		if (!NumPieces_ || pieces.size () < NumPieces_)
			return;

		int speed = status.download_payload_rate;
		int size = TI_.piece_length ();
		const int time = speed ?
			static_cast<double> (size) / speed * 1000 :
			60000;

		constexpr auto alertFlag = libtorrent::torrent_handle::alert_when_available;

		int thisDeadline = 0;
		for (int i = ReadPos_; i < NumPieces_; ++i)
			if (!pieces [PieceIdx_t { i }])
				Handle_.set_piece_deadline (PieceIdx_t { i },
						IsReady_ ?
							(thisDeadline += time) :
							1000000,
						alertFlag);
		if (!IsReady_)
		{
			std::vector<libtorrent::download_priority_t > prios (NumPieces_, libtorrent::dont_download);
			if (pieces.size () > 1)
				prios [1] = libtorrent::default_priority;

			if (!pieces [PieceIdx_t { 0 }])
			{
				Handle_.set_piece_deadline (PieceIdx_t { 0 }, 500, alertFlag);
				prios [0] = libtorrent::top_priority;
			}
			if (!pieces [PieceIdx_t { NumPieces_ - 1 }])
			{
				Handle_.set_piece_deadline (PieceIdx_t { NumPieces_ - 1 }, 500, alertFlag);
				prios [NumPieces_ - 1] = libtorrent::top_priority;
			}
			Handle_.prioritize_pieces (prios);
		}
	}
}
