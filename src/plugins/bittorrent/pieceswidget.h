/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QLabel>
#include <vector>
#include <libtorrent/bitfield.hpp>

namespace LC
{
namespace BitTorrent
{
	class PiecesWidget : public QLabel
	{
		Q_OBJECT

		libtorrent::bitfield Pieces_;
	public:
		PiecesWidget (QWidget *parent = 0);
	public slots:
		void setPieceMap (const libtorrent::bitfield&);
	private:
		void paintEvent (QPaintEvent*);
	};
}
}
