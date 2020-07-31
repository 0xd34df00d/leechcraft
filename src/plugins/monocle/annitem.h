/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include "interfaces/monocle/iannotation.h"
#include "linkitem.h"

namespace LC
{
namespace Monocle
{
	class AnnBaseItem
	{
	public:
		typedef std::function<void (IAnnotation_ptr)> Handler_f;
	protected:
		const IAnnotation_ptr BaseAnn_;
		Handler_f Handler_;

		bool IsSelected_ = false;
	public:
		AnnBaseItem (const IAnnotation_ptr&);

		QGraphicsItem* GetItem ();
		void SetHandler (const Handler_f&);

		bool IsSelected () const;
		virtual void SetSelected (bool) = 0;

		virtual void UpdateRect (QRectF rect) = 0;
	protected:
		QPen GetPen (bool selected) const;
		QBrush GetBrush (bool selected) const;
	};

	AnnBaseItem* MakeItem (const IAnnotation_ptr&, QGraphicsItem*);

	template<typename T>
	class AnnBaseGraphicsItem : public AnnBaseItem
							  , public T
	{
		QPointF PressedPos_;
	public:
		template<typename... TArgs>
		AnnBaseGraphicsItem (const IAnnotation_ptr& ann, TArgs... args)
		: AnnBaseItem { ann }
		, T { args... }
		{
		}
	protected:
		void mousePressEvent (QGraphicsSceneMouseEvent *event)
		{
			PressedPos_ = event->pos ();
			T::mousePressEvent (event);
			event->accept ();
		}

		void mouseReleaseEvent (QGraphicsSceneMouseEvent *event)
		{
			if (Handler_ &&
					(event->pos () - PressedPos_).manhattanLength () < 4)
				Handler_ (BaseAnn_);

			T::mouseReleaseEvent (event);
		}
	};

	template<typename T>
	class AnnRectGraphicsItem : public AnnBaseGraphicsItem<T>
	{
	public:
		using AnnBaseGraphicsItem<T>::AnnBaseGraphicsItem;

		void SetSelected (bool selected)
		{
			AnnBaseItem::SetSelected (selected);

			this->setPen (this->GetPen (selected));
			this->setBrush (this->GetBrush (selected));
		}

		void UpdateRect (QRectF rect)
		{
			this->setPos (rect.topLeft ());
			this->setRect (0, 0, rect.width (), rect.height ());
		}
	};

	class TextAnnItem : public AnnRectGraphicsItem<QGraphicsRectItem>
	{
	public:
		using AnnRectGraphicsItem<QGraphicsRectItem>::AnnRectGraphicsItem;
	};

	class HighAnnItem : public AnnBaseGraphicsItem<QGraphicsItemGroup>
	{
		struct PolyData
		{
			QPolygonF Poly_;
			QGraphicsPolygonItem *Item_;
		};
		const QList<PolyData> Polys_;

		QRectF Bounding_;
	public:
		HighAnnItem (const IHighlightAnnotation_ptr&, QGraphicsItem*);

		void SetSelected (bool);

		void UpdateRect (QRectF rect);
	private:
		static QList<PolyData> ToPolyData (const QList<QPolygonF>&);
	};

	class LinkAnnItem : public AnnRectGraphicsItem<LinkItem>
	{
	public:
		LinkAnnItem (const ILinkAnnotation_ptr&, QGraphicsItem*);
	};

	class CaretAnnItem : public AnnRectGraphicsItem<QGraphicsRectItem>
	{
	public:
		using AnnRectGraphicsItem<QGraphicsRectItem>::AnnRectGraphicsItem;
	};
}
}
