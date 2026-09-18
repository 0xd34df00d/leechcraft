/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QModelIndexList>
#include <QObject>
#include "guiconfig.h"

class QAbstractItemView;

namespace LC::Util
{
	/** @brief Tracks the row selection of a QAbstractItemView and reports
	 * it from a clean stack.
	 *
	 * Modifying a model in response to a selection change (like removing a
	 * news item or an email after it's been read and deselected) is unsafe
	 * inside the selection model's signals: those are also emitted from
	 * within the model's own structural changes, where Qt forbids further
	 * modification. Besides, the selection model updates the current index
	 * and the selection in separate steps, so a synchronous observer also
	 * sees intermediate states. Hence changes are coalesced and reported
	 * via selectionChanging() from a clean stack, before the next repaint.
	 *
	 * Moreover, removing rows while the left mouse button is held shifts
	 * the rows under the press position the view's drag selection is
	 * anchored to, so model modifications shall wait for selectionSettled(),
	 * which is withheld until the button is released.
	 *
	 * @note The view's model and selection model must be set before
	 * constructing this object and never replaced afterwards.
	 *
	 * @ingroup GuiUtil
	 */
	class UTIL_GUI_API ViewSelectionTracker : public QObject
	{
		Q_OBJECT

		QAbstractItemView& View_;
		bool ScheduledSyncToSelection_ = false;
		bool SelectionDirty_ = false;
		bool GestureActive_ = false;
	public:
		explicit ViewSelectionTracker (QAbstractItemView&, QObject* = nullptr);

		bool eventFilter (QObject*, QEvent*) override;

		/** @brief Ends the current left mouse button gesture, if any.
		 *
		 * A press that turns into a drag never delivers its release to the
		 * viewport, so a view with drag enabled shall call this from its
		 * QAbstractItemView::startDrag() override, after the base
		 * implementation returns (that is, once the drag is over). Otherwise
		 * the tracker stays gated until the next click.
		 */
		void EndGesture ();
	private:
		void HandleSelectionChange ();
		void ScheduleSyncToSelection ();
		void SyncToSelection ();
	signals:
		/** @brief Emitted on a left mouse button press on the view's viewport.
		 */
		void gestureStarted ();

		/** @brief Emitted from a clean stack whenever the selection or the
		 * current row changes, or the model is reset.
		 *
		 * Changes within one event loop iteration are coalesced into a
		 * single emission before the next repaint. This may be emitted while
		 * the left mouse button is held on the viewport, so receivers may
		 * update the display, but must not modify the model or the
		 * selection.
		 */
		void selectionChanging ();

		/** @brief Emitted from a clean stack once the selection has settled.
		 *
		 * This follows selectionChanging(), but nothing is emitted while the
		 * left mouse button is held on the viewport. Receivers may modify
		 * the model.
		 *
		 * @param[out] rows The selected rows, as in QItemSelectionModel::selectedRows().
		 * @param[out] current The current index, as in QItemSelectionModel::currentIndex().
		 */
		void selectionSettled (const QModelIndexList& rows, const QModelIndex& current);
	};
}
