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
#include "common.h"
#include "components/layout/scenepositiontracker.h"

namespace LC::Monocle
{
	class PagesView;
	class PageGraphicsItem;

	class PagesLayoutManager : public QObject
	{
		Q_OBJECT

		PagesView * const View_;

		IDocument *CurrentDoc_ = nullptr;

		QVector<PageGraphicsItem*> Pages_;
		QVector<double> PageRotations_;

		LayoutMode LayMode_ = LayoutMode::OnePage;
		ScaleMode ScaleMode_ = FitWidth {};

		ScenePositionTracker PosTracker_ { Pages_ };

		bool RelayoutScheduled_ = false;

		QSizeF Margins_ { 0, 0 };

		double Rotation_ = 0;
	public:
		explicit PagesLayoutManager (PagesView*, QObject* = nullptr);

		void HandleDoc (IDocument*, const QVector<PageGraphicsItem*>&);

		LayoutMode GetLayoutMode () const;
		void SetLayoutMode (LayoutMode);
		int GetLayoutModeCount () const;

		std::optional<PageWithRelativePos> GetCurrentPagePos () const;

		int GetCurrentPage () const;

		void SetScaleMode (ScaleMode);
		ScaleMode GetScaleMode () const;
		double GetCurrentScale () const;

		void SetRotation (double, RotationChange);
		double GetRotation () const;

		void SetPageRotation (int, double, RotationChange);
		double GetPageRotation (int) const;

		void SetMargins (QSizeF);

		void Relayout ();
	private:
		std::pair<SceneAbsolutePos, QSizeF> GetPagePos (int pageIdx, double scale) const;

		void ApplyPagesGeometry (double scale);

		void LayoutOneCol (double scale) const;
		void LayoutTwoCols (double scale, bool firstSeparate) const;

		QSizeF GetRotatedSize (int page) const;
		void ScheduleRelayout ();
	signals:
		void layoutFinished ();
		void rotationUpdated (double);
		void rotationUpdated (double, int);

		void layoutModeChanged (LayoutMode);
		void scaleModeChanged (ScaleMode);
	};
}
