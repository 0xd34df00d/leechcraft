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

#ifndef GLANCEITEM_H
#define GLANCEITEM_H
#include <QObject>
#include <QGraphicsPixmapItem>

class QPropertyAnimation;

namespace LeechCraft
{
	class GlanceItem : public QObject
					 , public QGraphicsPixmapItem
	{
		Q_OBJECT

		Q_PROPERTY (QPointF Pos READ pos WRITE setPos)
		Q_PROPERTY (qreal Opacity READ opacity WRITE setOpacity)
		Q_PROPERTY (qreal Scale READ scale WRITE setScale)

		int Index_;		
		qreal Scale_;
		QPropertyAnimation *ScaleAnim_;
		bool Current_;
		QList<GlanceItem*> ItemsList_;
	public:
		GlanceItem (const QPixmap&, QGraphicsItem* = 0);

		void SetIndex (int);
		void SetIdealScale (qreal);
		void SetCurrent (bool);
		void SetItemList (QList<QGraphicsItem*>);
		bool IsCurrent () const;
	private:
		void QueueScaleAnim (qreal, qreal);
	protected:
		virtual void hoverEnterEvent (QGraphicsSceneHoverEvent*);
		virtual void hoverLeaveEvent (QGraphicsSceneHoverEvent*);
		virtual void mousePressEvent (QGraphicsSceneMouseEvent*);
		virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent*);
	signals:
		void clicked (int);
	};
};

#endif

