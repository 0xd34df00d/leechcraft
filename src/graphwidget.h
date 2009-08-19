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
		class GraphWidget : public QFrame
		{
			Q_OBJECT

			QList<quint64> DownSpeeds_;
			QList<quint64> UpSpeeds_;
			QColor DownColor_;
			QColor UpColor_;
		public:
			GraphWidget (const QColor&, const QColor&,
					QWidget *parent = 0);

			void PushSpeed (quint64, quint64);
		protected:
			virtual void paintEvent (QPaintEvent*);
		private:
			virtual void PaintSingle (quint64, const QList<quint64>&,
					QPainter*);
		};
	};
};

#endif

