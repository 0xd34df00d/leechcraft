/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QLabel>
#include <libtorrent/bitfield.hpp>

namespace LC::BitTorrent
{
	class PiecesWidget : public QLabel
	{
		libtorrent::bitfield Pieces_;
	public:
		using QLabel::QLabel;

		void SetPieceMap (const libtorrent::bitfield&);
	private:
		void paintEvent (QPaintEvent*) override;
	};
}
