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

#include <QPainter>
#include <QPaintEvent>
#include <QVector>
#include <QtDebug>
#include "pieceswidget.h"

namespace LeechCraft
{
	namespace Plugins
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
			
			QList<QPair<int, int> > FindTrues (const libtorrent::bitfield& pieces)
			{
				QList<QPair<int, int> > result;
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
			
				QPixmap tempPicture (s, 1);
				QPainter tempPainter (&tempPicture);
				tempPainter.setPen (Qt::red);
				tempPainter.drawLine (0, 0, s, 0);
				QList<QPair<int, int> > trues = FindTrues (Pieces_);
				for (int i = 0; i < trues.size (); ++i)
				{
					QPair<int, int> pair = trues.at (i);
			
					tempPainter.setPen (Qt::darkGreen);
					tempPainter.drawLine (pair.first, 0, pair.second, 0);
				}
				tempPainter.end ();
			
				painter.drawPixmap (QRect (0, 0, width (), height ()), tempPicture);
				painter.end ();
			
				e->accept ();
			}
			
		};
	};
};

