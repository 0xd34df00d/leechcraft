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

namespace LC::BitTorrent
{
	using th = libtorrent::torrent_handle;

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
		const auto& fpath = TI_.files ().file_path (0);
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
		for (int i = ReadPos_; pieces [i]; ++i)
			result += TI_.piece_size (i);
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
			result += TI_.piece_size (i);
		result += Offset_;
		return result;
	}

	bool LiveStreamDevice::seek (qint64 pos)
	{
		QIODevice::seek (pos);

		int i = 0;
		while (pos >= TI_.piece_size (i))
			pos -= TI_.piece_size (i++);
		ReadPos_ = i;
		Offset_ = pos;

		Reschedule ();

		return true;
	}

	qint64 LiveStreamDevice::size () const
	{
		return StatusKeeper_.GetStatus (Handle_).total_wanted;
	}

	void LiveStreamDevice::PieceRead (const libtorrent::read_piece_alert&)
	{
		CheckReady ();
		CheckNextChunk ();
		Reschedule ();
	}

	void LiveStreamDevice::CheckReady ()
	{
		if (IsReady_)
			return;

		const auto& pieces = StatusKeeper_.GetStatus (Handle_, th::query_pieces).pieces;
		if (pieces [0] &&
				pieces [NumPieces_ - 1])
		{
			std::vector<libtorrent::download_priority_t> prios { NumPieces_, libtorrent::low_priority };
			Handle_.prioritize_pieces (prios);

			IsReady_ = true;
			EmitReadyEntity ();
		}
	}

	qint64 LiveStreamDevice::readData (char *data, qint64 max)
	{
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

		Offset_ += result;
		while (Offset_ >= TI_.piece_size (ReadPos_))
			Offset_ -= TI_.piece_size (ReadPos_++);

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
		for (int i = ReadPos_ + 1;
				i < NumPieces_ && pieces [i];
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
		int speed = status.download_payload_rate;
		int size = TI_.piece_length ();
		const int time = speed ?
			static_cast<double> (size) / speed * 1000 :
			60000;

		constexpr auto alertFlag = libtorrent::torrent_handle::alert_when_available;

		int thisDeadline = 0;
		for (int i = ReadPos_; i < NumPieces_; ++i)
			if (!pieces [i])
				Handle_.set_piece_deadline (i,
						IsReady_ ?
							(thisDeadline += time) :
							1000000,
						alertFlag);
		if (!IsReady_)
		{
			std::vector<libtorrent::download_priority_t > prios (NumPieces_, libtorrent::dont_download);
			if (pieces.size () > 1)
				prios [1] = libtorrent::default_priority;

			if (!pieces [0])
			{
				Handle_.set_piece_deadline (0, 500, alertFlag);
				prios [0] = libtorrent::top_priority;
			}
			if (!pieces [NumPieces_ - 1])
			{
				Handle_.set_piece_deadline (NumPieces_ - 1, 500, alertFlag);
				prios [NumPieces_ - 1] = libtorrent::top_priority;
			}
			Handle_.prioritize_pieces (prios);
		}
	}
}
