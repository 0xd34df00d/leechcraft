/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QVector>
#include <QFile>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/torrent_info.hpp>

namespace libtorrent
{
	class read_piece_alert;
}

namespace LC::BitTorrent
{
	class CachedStatusKeeper;

	class LiveStreamDevice : public QIODevice
	{
		CachedStatusKeeper * const StatusKeeper_;

		const libtorrent::torrent_handle Handle_;
		const libtorrent::torrent_info TI_;
		const int NumPieces_ = TI_.num_pieces ();

		// Which piece would be read next.
		int ReadPos_ = 0;
		// Offset in the next piece pointed by ReadPos_;
		int Offset_ = 0;
		bool IsReady_ = 0;
		QFile File_;
	public:
		LiveStreamDevice (const libtorrent::torrent_handle&, CachedStatusKeeper*, QObject* = nullptr);

		qint64 bytesAvailable () const override;
		bool isSequential () const override;
		bool open (OpenMode) override;
		qint64 pos () const override;
		bool seek (qint64) override;
		qint64 size () const override;

		void PieceRead (const libtorrent::read_piece_alert&);
		void CheckReady ();
	protected:
		qint64 readData (char*, qint64) override;
		qint64 writeData (const char*, qint64) override;
	private:
		void CheckNextChunk ();
		void EmitReadyEntity ();
		void Reschedule ();
	};
}
