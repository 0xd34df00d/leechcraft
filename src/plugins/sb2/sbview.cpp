/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sbview.h"
#include <QLayout>
#include <QQmlEngine>
#include <QQuickItem>
#include <util/sll/delayedexecutor.h>

namespace LC::SB2
{
	SBView::SBView (QWidget *parent)
	: QQuickWidget (parent)
	{
		setResizeMode (SizeRootObjectToView);
		setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Expanding);
	}

	void SBView::SetDimensions (int dim)
	{
		Dim_ = dim;
		if (!parentWidget ())
			return;

		if (auto lay = parentWidget ()->layout ())
			lay->update ();
	}

	QSize SBView::minimumSizeHint () const
	{
		return { Dim_, Dim_ };
	}

	QSize SBView::sizeHint () const
	{
		return { Dim_, Dim_ };
	}

	void SBView::enterEvent (QEvent *lev)
	{
		for (const auto& item : UnhoverItems_)
		{
			if (!item.Item_)
				continue;

			QHoverEvent ev { QEvent::HoverEnter, item.OldPos_, { -1, -1 } };
			static_cast<QObject*> (item.Item_)->event (&ev);
		}

		UnhoverItems_.clear ();

		QQuickWidget::enterEvent (lev);
	}

	namespace
	{
		QList<QQuickItem*> GetLogicalChildrenRepeater (QQuickItem *item)
		{
			QList<QQuickItem*> result;

			const auto count = item->property ("count").toInt ();
			for (int i = 0; i < count; ++i)
			{
				QQuickItem *childItem = nullptr;
				QMetaObject::invokeMethod (item,
						"itemAt",
						Q_RETURN_ARG (QQuickItem*, childItem),
						Q_ARG (int, i));
				if (childItem)
					result << childItem << childItem->findChildren<QQuickItem*> ();
			}

			return result;
		}

		QList<QQuickItem*> GetLogicalChildrenListView (QQuickItem *item)
		{
			QList<QQuickItem*> result;

			for (const auto child : item->children ())
			{
				QQmlListReference ref { child, "children" };
				if (ref.count () <= 1)
					continue;

				for (int i = 0; i < ref.count (); ++i)
				{
					if (const auto item = qobject_cast<QQuickItem*> (ref.at (i)))
						result << item << item->findChildren<QQuickItem*> ();
				}
			}

			return result;
		}

		QList<QQuickItem*> GetLogicalChildren (QQuickItem *item, const QByteArray& className)
		{
			if (className == "QQuickRepeater")
				return GetLogicalChildrenRepeater (item);
			if (className.startsWith ("QQuickListView"))
				return GetLogicalChildrenListView (item);

			return {};
		}
	}

	void SBView::leaveEvent (QEvent *lev)
	{
		UnhoverItems_.clear ();

		auto items = rootObject ()->findChildren<QQuickItem*> ();

		while (!items.isEmpty ())
		{
			auto item = items.takeFirst ();

			const QByteArray className { item->metaObject ()->className () };
			items += GetLogicalChildren (item, className);

			if (!className.startsWith ("QQuickMouseArea"))
				continue;

			if (!item->property ("containsMouse").toBool ())
				continue;

			const QPointF oldPos
			{
				item->property ("mouseX").toDouble (),
				item->property ("mouseY").toDouble ()
			};
			UnhoverItems_.append ({ item, oldPos });

			QHoverEvent ev { QEvent::HoverLeave, { -1, -1 }, oldPos };
			static_cast<QObject*> (item)->event (&ev);
		}

		QQuickWidget::leaveEvent (lev);
	}
}
