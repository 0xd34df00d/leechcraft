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
#include <QPointer>
#include "interfaces/monocle/idocument.h"

template<typename T>
class QFutureWatcher;

namespace LC
{
namespace Monocle
{
	class PagesLayoutManager;
	class ArbitraryRotationWidget;

	class PageGraphicsItem : public QObject
						   , public QGraphicsPixmapItem
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Monocle::PageGraphicsItem)

		bool IsRenderingEnabled_ = true;

		IDocument_ptr Doc_;
		const int PageNum_;

		qreal XScale_ = 1;
		qreal YScale_ = 1;

		bool Invalid_ = true;

		std::function<void (int, QPointF)> ReleaseHandler_;

		PagesLayoutManager *LayoutManager_ = nullptr;

		QPointer<ArbitraryRotationWidget> ArbWidget_;
	public:
		typedef std::function<void (QRectF)> RectSetter_f;
	private:
		struct RectInfo
		{
			QRectF DocRect_;
			RectSetter_f Setter_;
		};
		QMap<QGraphicsItem*, RectInfo> Item2RectInfo_;
	public:
		PageGraphicsItem (IDocument_ptr, int, QGraphicsItem* = nullptr);
		~PageGraphicsItem () override;

		void SetLayoutManager (PagesLayoutManager*);

		void SetReleaseHandler (std::function<void (int, QPointF)>);

		void SetScale (double, double);
		int GetPageNum () const;

		QRectF MapFromDoc (const QRectF&) const;
		QRectF MapToDoc (const QRectF&) const;

		void RegisterChildRect (QGraphicsItem*, const QRectF&, RectSetter_f);
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
		QPixmap GetEmptyPixmap (bool fill) const;
	};
}
}
