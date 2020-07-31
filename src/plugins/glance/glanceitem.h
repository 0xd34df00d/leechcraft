/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_GLANCE_GLANCEITEM_H
#define PLUGINS_GLANCE_GLANCEITEM_H
#include <QObject>
#include <QGraphicsPixmapItem>

class QPropertyAnimation;

namespace LC
{
namespace Plugins
{
namespace Glance
{
	class GlanceItem : public QObject
					 , public QGraphicsPixmapItem
	{
		Q_OBJECT

		Q_PROPERTY (QPointF Pos READ pos WRITE setPos)
		Q_PROPERTY (qreal Opacity READ opacity WRITE setOpacity)
		Q_PROPERTY (qreal Scale READ scale WRITE setScale)

		int Index_ = -1;
		qreal Scale_ = 0;
		QPropertyAnimation *ScaleAnim_;
		bool Current_ = false;
		QList<GlanceItem*> ItemsList_;
		QRect CloseButtonRect_;
		QPixmap Pixmap_;
	public:
		GlanceItem (const QPixmap&, const QRect&, QGraphicsItem* = 0);

		void SetIndex (int);
		void SetIdealScale (qreal);
		void SetCurrent (bool);
		void SetItemList (QList<QGraphicsItem*>);
		bool IsCurrent () const;
	private:
		void QueueScaleAnim (qreal, qreal);
		void DrawCloseButton (bool);
	protected:
		virtual void hoverEnterEvent (QGraphicsSceneHoverEvent*);
		virtual void hoverMoveEvent (QGraphicsSceneHoverEvent*);
		virtual void hoverLeaveEvent (QGraphicsSceneHoverEvent*);
		virtual void mousePressEvent (QGraphicsSceneMouseEvent*);
		virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent*);
	signals:
		void clicked (int, bool);
	};
};
};
};
#endif

