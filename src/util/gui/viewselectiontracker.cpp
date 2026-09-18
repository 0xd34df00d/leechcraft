/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "viewselectiontracker.h"
#include <QAbstractItemView>
#include <QItemSelectionModel>
#include <QMouseEvent>
#include <QTimer>

namespace LC::Util
{
	ViewSelectionTracker::ViewSelectionTracker (QAbstractItemView& view, QObject* parent)
	: QObject { parent }
	, View_ { view }
	{
		View_.viewport ()->installEventFilter (this);

		const auto sm = view.selectionModel ();
		connect (sm,
				&QItemSelectionModel::selectionChanged,
				this,
				&ViewSelectionTracker::HandleSelectionChange);
		connect (sm,
				&QItemSelectionModel::currentRowChanged,
				this,
				&ViewSelectionTracker::HandleSelectionChange);
		connect (view.model (),
				&QAbstractItemModel::modelReset,
				this,
				&ViewSelectionTracker::HandleSelectionChange);
	}

	bool ViewSelectionTracker::eventFilter (QObject*, QEvent *ev)
	{
		switch (ev->type ())
		{
		case QEvent::MouseButtonDblClick:
		case QEvent::MouseButtonPress:
			if (static_cast<QMouseEvent*> (ev)->button () == Qt::LeftButton)
			{
				GestureActive_ = true;
				emit gestureStarted ();
			}
			break;
		case QEvent::MouseButtonRelease:
			if (static_cast<QMouseEvent*> (ev)->button () == Qt::LeftButton)
				EndGesture ();
			break;
		case QEvent::UngrabMouse:
			EndGesture ();
			break;
		default:
			break;
		}

		return false;
	}

	void ViewSelectionTracker::EndGesture ()
	{
		if (std::exchange (GestureActive_, false))
			ScheduleSyncToSelection ();
	}

	void ViewSelectionTracker::HandleSelectionChange ()
	{
		emit selectionStartedChanging ();

		SelectionDirty_ = true;
		ScheduleSyncToSelection ();
	}

	void ViewSelectionTracker::ScheduleSyncToSelection ()
	{
		if (!std::exchange (ScheduledSyncToSelection_, true))
			QTimer::singleShot (0, this, &ViewSelectionTracker::SyncToSelection);
	}

	void ViewSelectionTracker::SyncToSelection ()
	{
		if (!std::exchange (ScheduledSyncToSelection_, false))
			return;

		if (std::exchange (SelectionDirty_, false))
			emit selectionChanging ();

		if (GestureActive_)
			return;

		const auto sm = View_.selectionModel ();
		emit selectionSettled (sm->selectedRows (), sm->currentIndex ());
	}
}
