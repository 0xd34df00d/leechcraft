/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <memory>
#include <QCoreApplication>
#include <QGraphicsPixmapItem>
#include "interfaces/monocle/idocument.h"

namespace LC::Monocle
{
	class PagesLayoutManager;
	struct PageAbsolutePos;
	struct PageAbsoluteRect;
	struct PageRelativeRect;
	struct SceneAbsolutePos;

	class PageGraphicsItem : public QObject
						   , public QGraphicsPixmapItem
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Monocle::PageGraphicsItem)

		IDocument& Doc_;
		PagesLayoutManager *LayoutManager_ = nullptr;

		qreal XScale_ = 1;
		qreal YScale_ = 1;

		const int PageNum_;

		bool Invalid_ = true;
		bool IsRenderingEnabled_ = true;
	public:
		using RectSetter_f = std::function<void (PageAbsoluteRect)>;
		using ReleaseHandler_f = std::function<void (int, PageAbsolutePos)>;
	private:
		ReleaseHandler_f ReleaseHandler_;

		struct RectInfo;
		QMap<QGraphicsItem*, RectInfo> Item2RectInfo_;
	public:
		PageGraphicsItem (IDocument&, int, QGraphicsItem* = nullptr);
		~PageGraphicsItem () override;

		void SetLayoutManager (PagesLayoutManager*);

		void SetReleaseHandler (ReleaseHandler_f);

		void SetScale (double, double);
		int GetPageNum () const;

		void setPos (const SceneAbsolutePos&);

		void RegisterChildRect (QGraphicsItem*, const PageRelativeRect&, RectSetter_f);
		void UnregisterChildRect (QGraphicsItem*);

		void ClearPixmap ();
		void UpdatePixmap ();

		bool IsDisplayed () const;

		void SetRenderingEnabled (bool);

		QRectF boundingRect () const override;
		QPainterPath shape () const override;
	protected:
		void paint (QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;
		void mousePressEvent (QGraphicsSceneMouseEvent*) override;
		void mouseReleaseEvent (QGraphicsSceneMouseEvent*) override;
		void contextMenuEvent (QGraphicsSceneContextMenuEvent*) override;
	private:
		bool ShouldRender () const;
		QPixmap GetEmptyPixmap () const;
	};
}
