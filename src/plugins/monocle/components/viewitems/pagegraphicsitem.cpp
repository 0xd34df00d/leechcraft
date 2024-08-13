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

namespace LC::Monocle
{
	struct PageGraphicsItem::RectInfo
	{
		PageRelativeRect Rect_;
		RectSetter_f Setter_;
	};

	PageGraphicsItem::PageGraphicsItem (IDocument& doc, int page, QGraphicsItem *parent)
	: QGraphicsItem { parent }
	, Doc_ { doc }
	, PageNum_ { page }
	{
		setAcceptHoverEvents (true);

		ClearPixmap ();
	}

	PageGraphicsItem::~PageGraphicsItem () = default;

	void PageGraphicsItem::SetReleaseHandler (ReleaseHandler_f handler)
	{
		ReleaseHandler_ = std::move (handler);
	}

	void PageGraphicsItem::SetScale (double xs, double ys)
	{
		if (std::abs (xs - XScale_) < std::numeric_limits<double>::epsilon () &&
			std::abs (ys - YScale_) < std::numeric_limits<double>::epsilon ())
			return;

		prepareGeometryChange ();

		XScale_ = xs;
		YScale_ = ys;

		UpdatePixmap ();

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
		Image_ = QImage {};
	}

	void PageGraphicsItem::UpdatePixmap ()
	{
		ClearPixmap ();
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

	void PageGraphicsItem::paint (QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget*)
	{
		emit itemPainted ();

		if (!Image_.isNull ())
		{
			painter->drawImage (option->rect, Image_);
			return;
		}

		if (!IsDisplayed ())
			return;

		DrawEmptyPixmap (*painter, *option);

		if (IsRenderingEnabled_ && !IsRenderingScheduled_)
		{
			IsRenderingScheduled_ = true;
			Util::Sequence (this, Doc_.RenderPage (PageNum_, XScale_, YScale_)) >>
					[&, prevXScale = XScale_, prevYScale = YScale_] (QImage img)
					{
						IsRenderingScheduled_ = false;
						Image_ = std::move (img);
						update ();

						emit itemPixmapChanged ();

						if (std::abs (prevXScale - XScale_) > std::numeric_limits<double>::epsilon () * XScale_ ||
							std::abs (prevYScale - YScale_) > std::numeric_limits<double>::epsilon () * YScale_)
							UpdatePixmap ();
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
		if (Image_.isNull ())
			return 0;

		return Image_.width () * Image_.height () * Image_.depth () / 8;
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
		return { { 0, 0 }, size };
	}

	QPainterPath PageGraphicsItem::shape () const
	{
		QPainterPath path;
		path.addRect (boundingRect ());
		return path;
	}
}
