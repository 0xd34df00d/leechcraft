/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pieceswidget.h"
#include <QPainter>
#include <QtDebug>
#include <QApplication>
#include <QPalette>
#include <libtorrent/bitfield.hpp>

namespace LC::BitTorrent
{
	namespace
	{
		QVector<QPair<int, int>> FindTrues (const libtorrent::bitfield& pieces)
		{
			QVector<QPair<int, int>> result;
			bool prevVal = pieces [0];
			int prevPos = 0;
			int size = static_cast<int> (pieces.size ());
			for (int i = 1; i < size; ++i)
				if (pieces [i] != prevVal)
				{
					if (prevVal)
						result.append ({ prevPos, i });
					prevPos = i;
					prevVal = 1 - prevVal;
				}

			if (!prevVal)
				return result;

			if (!prevPos || result.empty ())
				result.append ({ 0, size });
			else if (result.last ().second != size - 1)
				result.append ({ prevPos, size });

			return result;
		}
	}

	void PiecesWidget::SetPieceMap (const libtorrent::bitfield& pieces)
	{
		TrueRanges_ = FindTrues (pieces);
		update ();
	}

	void PiecesWidget::paintEvent (QPaintEvent*)
	{
		int s = TrueRanges_.size ();
		QPainter painter (this);
		painter.setRenderHints (QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
		if (!s)
		{
			painter.setBackgroundMode (Qt::OpaqueMode);
			painter.setBackground (Qt::white);
			painter.end ();
			return;
		}

		const auto& backgroundColor = palette ().color (QPalette::Base);
		const auto& downloadedPieceColor = palette ().color (QPalette::Highlight);

		QPixmap tempPicture (s, 1);
		QPainter tempPainter (&tempPicture);
		tempPainter.setPen (backgroundColor);
		tempPainter.drawLine (0, 0, s, 0);
		for (const auto& pair : TrueRanges_)
		{
			tempPainter.setPen (downloadedPieceColor);
			tempPainter.drawLine (pair.first, 0, pair.second, 0);
		}
		tempPainter.end ();

		painter.drawPixmap (QRect (0, 0, width (), height ()), tempPicture);
		painter.end ();
	}
}
