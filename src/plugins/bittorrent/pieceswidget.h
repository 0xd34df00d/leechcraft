/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QLabel>
#include <QVector>

namespace libtorrent
{
	class bitfield;
}

namespace LC::BitTorrent
{
	class PiecesWidget : public QLabel
	{
		int PiecesCount_ = 1;
		QVector<QPair<int, int>> TrueRanges_;
	public:
		using QLabel::QLabel;

		void SetPieceMap (const libtorrent::bitfield&);
	private:
		void paintEvent (QPaintEvent*) override;
	};
}
