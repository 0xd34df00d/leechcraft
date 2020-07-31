/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pieceswidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QVector>
#include <QtDebug>
#include <QApplication>
#include <QPalette>

namespace LC
{
namespace BitTorrent
{
	PiecesWidget::PiecesWidget (QWidget *parent)
	: QLabel (parent)
	{
	}

	void PiecesWidget::setPieceMap (const libtorrent::bitfield& pieces)
	{
		Pieces_ = pieces;

		update ();
	}

	QList<QPair<int, int>> FindTrues (const libtorrent::bitfield& pieces)
	{
		QList<QPair<int, int>> result;
		bool prevVal = pieces [0];
		int prevPos = 0;
		int size = static_cast<int> (pieces.size ());
		for (int i = 1; i < size; ++i)
			if (pieces [i] != prevVal)
			{
				if (prevVal)
					result << qMakePair (prevPos, i);
				prevPos = i;
				prevVal = 1 - prevVal;
			}

		if (!prevPos && prevVal)
			result << qMakePair<int, int> (0, pieces.size ());
		else if (prevVal && result.size () && result.last ().second != size - 1)
			result << qMakePair<int, int> (prevPos, size);
		else if (prevVal && !result.size ())
			result << qMakePair<int, int> (0, size);

		return result;
	}

	void PiecesWidget::paintEvent (QPaintEvent *e)
	{
		int s = Pieces_.size ();
		QPainter painter (this);
		painter.setRenderHints (QPainter::Antialiasing |
				QPainter::SmoothPixmapTransform);
		if (!s)
		{
			painter.setBackgroundMode (Qt::OpaqueMode);
			painter.setBackground (Qt::white);
			painter.end ();
			return;
		}

		const QPalette& palette = QApplication::palette ();
		const QColor& backgroundColor = palette.color (QPalette::Base);
		const QColor& downloadedPieceColor = palette.color (QPalette::Highlight);

		QPixmap tempPicture (s, 1);
		QPainter tempPainter (&tempPicture);
		tempPainter.setPen (backgroundColor);
		tempPainter.drawLine (0, 0, s, 0);
		QList<QPair<int, int>> trues = FindTrues (Pieces_);
		for (int i = 0; i < trues.size (); ++i)
		{
			QPair<int, int> pair = trues.at (i);

			tempPainter.setPen (downloadedPieceColor);
			tempPainter.drawLine (pair.first, 0, pair.second, 0);
		}
		tempPainter.end ();

		painter.drawPixmap (QRect (0, 0, width (), height ()), tempPicture);
		painter.end ();

		e->accept ();
	}

}
}
