/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QObject>
#include "interfaces/monocle/idocument.h"

class QGraphicsScene;

namespace LeechCraft
{
namespace Monocle
{
	enum class LayoutMode;
	enum class ScaleMode;

	class PagesView;
	class PageGraphicsItem;

	class PagesLayoutManager : public QObject
	{
		Q_OBJECT

		PagesView * const View_;
		QGraphicsScene * const Scene_;

		IDocument_ptr CurrentDoc_;

		QList<PageGraphicsItem*> Pages_;

		LayoutMode LayMode_;

		ScaleMode ScaleMode_;
		double FixedScale_;

		bool RelayoutScheduled_;

		double HorMargin_;
		double VertMargin_;
	public:
		PagesLayoutManager (PagesView*, QObject* = 0);

		void HandleDoc (IDocument_ptr, const QList<PageGraphicsItem*>&);
		const QList<PageGraphicsItem*>& GetPages () const;

		LayoutMode GetLayoutMode () const;
		void SetLayoutMode (LayoutMode);
		int GetLayoutModeCount () const;

		QPoint GetViewportCenter () const;

		int GetCurrentPage () const;
		void SetCurrentPage (int, bool);

		void SetScaleMode (ScaleMode mode);
		void SetFixedScale (double);
		double GetCurrentScale () const;

		void SetMargins (double horizontal, double vertical);

		void Relayout ();
	public slots:
		void scheduleRelayout ();
		void handleRelayout ();
	private slots:
		void handlePageSizeChanged (int);
	signals:
		void scheduledRelayoutFinished ();
	};
}
}
