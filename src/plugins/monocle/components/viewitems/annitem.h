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

namespace LC::Monocle
{
	class AnnBaseItem
	{
	public:
		using Handler_f = std::function<void (IAnnotation_ptr)>;
	protected:
		const IAnnotation_ptr BaseAnn_;
		Handler_f Handler_;

		bool IsSelected_ = false;
	public:
		AnnBaseItem (IAnnotation_ptr);

		virtual ~AnnBaseItem () = default;

		AnnBaseItem (const AnnBaseItem&) = delete;
		AnnBaseItem (AnnBaseItem&&) = delete;
		AnnBaseItem& operator= (const AnnBaseItem&) = delete;
		AnnBaseItem& operator= (AnnBaseItem&&) = delete;

		QGraphicsItem* GetItem ();
		void SetHandler (const Handler_f&);

		bool IsSelected () const;
		virtual void SetSelected (bool) = 0;

		virtual void UpdateRect (const PageAbsoluteRect& rect) = 0;
	protected:
		static QPen GetPen (bool selected);
		static QBrush GetBrush (bool selected);
	};

	AnnBaseItem* MakeItem (const IAnnotation_ptr&, QGraphicsItem*, LinkExecutionContext&);

	template<typename T>
	class AnnBaseGraphicsItem : public AnnBaseItem
							  , public T
	{
		NonDragClickFilter NonDragFilter_;
	public:
		template<typename... TArgs>
		AnnBaseGraphicsItem (const IAnnotation_ptr& ann, TArgs&&... args)
		: AnnBaseItem { ann }
		, T { std::forward<TArgs> (args)... }
		{
		}
	protected:
		void mousePressEvent (QGraphicsSceneMouseEvent *event) override
		{
			NonDragFilter_.RecordPressed (event);
			T::mousePressEvent (event);
			event->accept ();
		}

		void mouseReleaseEvent (QGraphicsSceneMouseEvent *event) override
		{
			if (Handler_ && NonDragFilter_.IsNonDragRelease (event))
				Handler_ (BaseAnn_);

			T::mouseReleaseEvent (event);
		}
	};

	template<typename T>
	class AnnRectGraphicsItem : public AnnBaseGraphicsItem<T>
	{
	public:
		using AnnBaseGraphicsItem<T>::AnnBaseGraphicsItem;

		void SetSelected (bool selected) override
		{
			AnnBaseItem::SetSelected (selected);

			this->setPen (this->GetPen (selected));
			this->setBrush (this->GetBrush (selected));
		}

		void UpdateRect (const PageAbsoluteRect& rect) override
		{
			const auto& rr = rect.ToRectF ();
			this->setPos (rr.topLeft ());
			this->setRect (0, 0, rr.width (), rr.height ());
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

		const PageRelativeRect Bounding_;
	public:
		HighAnnItem (const IHighlightAnnotation_ptr&, QGraphicsItem*);

		void SetSelected (bool) override;

		void UpdateRect (const PageAbsoluteRect& rect) override;
	private:
		static QList<PolyData> ToPolyData (const QList<QPolygonF>&);
	};

	class LinkAnnItem : public AnnRectGraphicsItem<LinkItem>
	{
	public:
		LinkAnnItem (const ILinkAnnotation_ptr&, QGraphicsItem*, LinkExecutionContext&);
	};

	class CaretAnnItem : public AnnRectGraphicsItem<QGraphicsRectItem>
	{
	public:
		using AnnRectGraphicsItem<QGraphicsRectItem>::AnnRectGraphicsItem;
	};
}
