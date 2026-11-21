/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "contactslistview.h"
#include <QCoreApplication>
#include <QDropEvent>
#include <QHelpEvent>
#include <QToolTip>
#include <util/gui/util.h>
#include "roles.h"
#include "interfaces/azoth/iclentry.h"

namespace LC::Azoth
{
	QRect ContactsListView::visualRect (const QModelIndex& index) const
	{
		auto rect = QTreeView::visualRect (index);
		if (index.data (CLREntryType).value<CLEntryType> () == CLETContact)
			rect.setLeft (0);
		return rect;
	}

	void ContactsListView::RedisplayTooltip (const GlobalStrongestId& entryId)
	{
		if (!QToolTip::isVisible ())
			return;

		const auto& pos = QCursor::pos ();
		const auto& local = viewport ()->mapFromGlobal (pos);
		if (const auto entry = indexAt (local).data (CLRIEntry).value<ICLEntry*> ();
			!entry || entry->GetGlobalStrongestId () != entryId)
			return;

		QHelpEvent event { QEvent::ToolTip, local, pos };
		QCoreApplication::sendEvent (viewport (), &event);
	}

	namespace
	{
		void HandleAsCopy (QDropEvent& event, const auto& handle)
		{
			if (!(event.possibleActions () & Qt::CopyAction))
			{
				event.ignore ();
				return;
			}

			event.setDropAction (Qt::CopyAction);
			handle ();
			if (event.isAccepted ())
				Util::AcceptEventAsCopy (event);
		}
	}

	void ContactsListView::dragEnterEvent (QDragEnterEvent *event)
	{
		HandleAsCopy (*event, [&] { QTreeView::dragEnterEvent (event); });
	}

	void ContactsListView::dragMoveEvent (QDragMoveEvent *event)
	{
		HandleAsCopy (*event, [&] { QTreeView::dragMoveEvent (event); });
	}

	void ContactsListView::dropEvent (QDropEvent *event)
	{
		HandleAsCopy (*event, [&] { QTreeView::dropEvent (event); });
	}

	void ContactsListView::drawBranches (QPainter *painter, const QRect& rect, const QModelIndex& index) const
	{
		if (index.data (CLREntryType).value<CLEntryType> () != CLETContact)
			QTreeView::drawBranches (painter, rect, index);
	}
}
