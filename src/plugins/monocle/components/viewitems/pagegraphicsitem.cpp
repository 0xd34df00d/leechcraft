/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pagegraphicsitem.h"
#include <limits>
#include <cmath>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QStyleOptionGraphicsItem>
#include <util/threads/futures.h>
#include "components/layout/positions.h"
#include "components/services/pixmapcachemanager.h"
#include "core.h"

namespace LC::Monocle
{
	struct PageGraphicsItem::RectInfo
	{
		PageRelativeRect Rect_;
		RectSetter_f Setter_;
	};

	PageGraphicsItem::PageGraphicsItem (IDocument& doc, PixmapCacheManager& pxCache, int page, QGraphicsItem *parent)
	: QGraphicsPixmapItem { parent }
	, Doc_ { doc }
	, PxCache_ { pxCache }
	, PageNum_ { page }
	{
		setTransformationMode (Qt::SmoothTransformation);
		setShapeMode (QGraphicsPixmapItem::BoundingRectShape);
		setAcceptHoverEvents (true);

		ClearPixmap ();
	}

	PageGraphicsItem::~PageGraphicsItem ()
	{
		PxCache_.PixmapDeleted (this);
	}

	void PageGraphicsItem::SetReleaseHandler (ReleaseHandler_f handler)
	{
		ReleaseHandler_ = std::move (handler);
	}

	void PageGraphicsItem::SetScale (double xs, double ys)
	{
		if (std::abs (xs - XScale_) < std::numeric_limits<double>::epsilon () &&
			std::abs (ys - YScale_) < std::numeric_limits<double>::epsilon ())
			return;

		XScale_ = xs;
		YScale_ = ys;

		Invalid_ = true;

		if (ShouldRender ())
			update ();
		else
			prepareGeometryChange ();

		for (const auto& info : Item2RectInfo_)
			info.Setter_ (info.Rect_.ToPageAbsolute (*this));
	}

	int PageGraphicsItem::GetPageNum () const
	{
		return PageNum_;
	}

	void PageGraphicsItem::setPos (const SceneAbsolutePos& pos)
	{
		QGraphicsItem::setPos (pos.ToPointF ());
	}

	void PageGraphicsItem::RegisterChildRect (QGraphicsItem *item, const PageRelativeRect& srcRect, RectSetter_f setter)
	{
		setter (srcRect.ToPageAbsolute (*this));
		Item2RectInfo_ [item] = { srcRect, std::move (setter) };
	}

	void PageGraphicsItem::UnregisterChildRect (QGraphicsItem *item)
	{
		Item2RectInfo_.remove (item);
	}

	void PageGraphicsItem::ClearPixmap ()
	{
		setPixmap (QPixmap {});

		Invalid_ = true;
	}

	void PageGraphicsItem::UpdatePixmap ()
	{
		Invalid_ = true;
		if (ShouldRender ())
			update ();
	}

	namespace
	{
		void DrawEmptyPixmap (QPainter& painter, const QStyleOptionGraphicsItem& option)
		{
			painter.fillRect (option.rect, QBrush { Qt::white });
		}
	}

	void PageGraphicsItem::paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *w)
	{
		PxCache_.PixmapPainted (this);
		if (!Invalid_ && !pixmap ().isNull ())
		{
			QGraphicsPixmapItem::paint (painter, option, w);
			return;
		}

		if (!IsDisplayed ())
			return;

		DrawEmptyPixmap (*painter, *option);

		if (IsRenderingEnabled_ && Invalid_)
		{
			Invalid_ = false;
			Util::Sequence (this, Doc_.RenderPage (PageNum_, XScale_, YScale_)) >>
					[&, prevXScale = XScale_, prevYScale = YScale_] (const QImage& img)
					{
						setPixmap (QPixmap::fromImage (img));

						if (std::abs (prevXScale - XScale_) > std::numeric_limits<double>::epsilon () * XScale_ ||
							std::abs (prevYScale - YScale_) > std::numeric_limits<double>::epsilon () * YScale_)
							UpdatePixmap ();
						else
							PxCache_.PixmapChanged (this);
					};
		}
	}

	void PageGraphicsItem::mousePressEvent (QGraphicsSceneMouseEvent *event)
	{
		if (!ReleaseHandler_)
			QGraphicsItem::mousePressEvent (event);
	}

	void PageGraphicsItem::mouseReleaseEvent (QGraphicsSceneMouseEvent *event)
	{
		if (ReleaseHandler_)
			ReleaseHandler_ (PageNum_, PageAbsolutePos { event->pos () });
		else
			QGraphicsItem::mouseReleaseEvent (event);
	}

	int PageGraphicsItem::GetMemorySize () const
	{
		const auto& px = pixmap ();
		if (px.isNull ())
			return 0;

		return px.width () * px.height () * px.defaultDepth () / 8 * 1.5;
	}

	bool PageGraphicsItem::IsDisplayed () const
	{
		const auto& thisMapped = mapToScene (boundingRect ()).boundingRect ();
		return std::ranges::any_of (scene ()->views (),
				[&thisMapped] (auto view)
				{
					const auto& rect = view->viewport ()->rect ();
					return view->mapToScene (rect).boundingRect ().intersects (thisMapped);
				});
	}

	void PageGraphicsItem::SetRenderingEnabled (bool enabled)
	{
		if (IsRenderingEnabled_ == enabled)
			return;

		IsRenderingEnabled_ = enabled;
		if (ShouldRender ())
			update ();
	}

	bool PageGraphicsItem::ShouldRender () const
	{
		return IsRenderingEnabled_ && IsDisplayed ();
	}

	QRectF PageGraphicsItem::boundingRect () const
	{
		QSizeF size { Doc_.GetPageSize (PageNum_) };
		size.rwidth () *= XScale_;
		size.rheight () *= YScale_;
		return QRectF { offset (), size };
	}

	QPainterPath PageGraphicsItem::shape () const
	{
		QPainterPath path;
		path.addRect (boundingRect ());
		return path;
	}
}
