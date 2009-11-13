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

#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H
#include <QFrame>
#include <QList>

namespace LeechCraft
{
	namespace Util
	{
		/** Draws a scrollling graph with two speed curves.
		 */
		class GraphWidget : public QFrame
		{
			Q_OBJECT

			QList<quint64> DownSpeeds_;
			QList<quint64> UpSpeeds_;
			QColor DownColor_;
			QColor UpColor_;
		public:
			/** Constructs the graph widget and sets color for download
			 * and upload speeds.
			 *
			 * @param[in] down The color of the download speed graph.
			 * @param[in] up The color of the upload speed graph.
			 * @param[in] parent The parent widget.
			 */
			GraphWidget (const QColor& down, const QColor& up,
					QWidget *parent = 0);

			/** Adds a pair of download and upload speed to the graph.
			 * Removes older speeds if needed. Repaints itself.
			 *
			 * @param[in] down Download speed.
			 * @param[in] up Upload speed.
			 */
			void PushSpeed (quint64 down, quint64 up);
		protected:
			/** Finds out max speeds and calls PaintSingle() to actually
			 * paint the graphs.
			 */
			virtual void paintEvent (QPaintEvent*);
		private:
			/** Paints single graph with known max value, list of speeds
			 * on the given painter.
			 *
			 * @param[in] max The maximum speed.
			 * @param[in] speeds The list of speeds.
			 * @param[in] painter The painter to paint on.
			 */
			virtual void PaintSingle (quint64 max, const QList<quint64>& speeds,
					QPainter *painter);
		};
	};
};

#endif

