/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QVector>
#include "interfaces/monocle/idocument.h"

class QGraphicsScene;

namespace LC
{
namespace Monocle
{
	enum class LayoutMode;
	enum class ScaleMode;

	class PagesView;
	class SmoothScroller;
	class PageGraphicsItem;

	class PagesLayoutManager : public QObject
	{
		Q_OBJECT

		PagesView * const View_;
		SmoothScroller * const Scroller_;
		QGraphicsScene * const Scene_;

		IDocument_ptr CurrentDoc_;

		QList<PageGraphicsItem*> Pages_;
		QVector<double> PageRotations_;

		LayoutMode LayMode_;

		ScaleMode ScaleMode_;
		double FixedScale_ = 1;

		bool RelayoutScheduled_ = false;

		double HorMargin_ = 0;
		double VertMargin_ = 0;

		double Rotation_ = 0;
	public:
		PagesLayoutManager (PagesView*, SmoothScroller*, QObject* = nullptr);

		void HandleDoc (IDocument_ptr, const QList<PageGraphicsItem*>&);
		const QList<PageGraphicsItem*>& GetPages () const;

		LayoutMode GetLayoutMode () const;
		void SetLayoutMode (LayoutMode);
		int GetLayoutModeCount () const;

		QPoint GetViewportCenter () const;

		int GetCurrentPage () const;
		void SetCurrentPage (int, bool);

		void SetScaleMode (ScaleMode);
		ScaleMode GetScaleMode () const;
		void SetFixedScale (double);
		double GetCurrentScale () const;

		void SetRotation (double);
		void AddRotation (double);
		double GetRotation () const;

		void SetRotation (double, int);
		void AddRotation (double, int);
		double GetRotation (int) const;

		void SetMargins (double horizontal, double vertical);

		void Relayout ();
	private:
		QSizeF GetRotatedSize (int page) const;
	public slots:
		void scheduleRelayout ();
		void handleRelayout ();
	private slots:
		void handlePageSizeChanged (int);
	signals:
		void scheduledRelayoutFinished ();
		void rotationUpdated (double);
		void rotationUpdated (double, int);
		void layoutModeChanged ();
	};
}
}
