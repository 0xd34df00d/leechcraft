/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

class QDropEvent;
class QDragMoveEvent;
class QMimeData;

/** @brief Interface for tabs supporting Drag'n'Drop on tab level.
 *
 * Tabs (implementing ITabWidget) that support Drag'n'Drop on tab level,
 * like dragging one tab to another, can implement this interface. For
 * example, a Poshuku tab can fill a DnD object with current page URL and
 * its snapshot upon drag start.
 *
 * @sa ITabWidget
 */
class Q_DECL_EXPORT IDNDTab
{
public:
	virtual ~IDNDTab () {}

	/** @brief Called when this tab starts being dragged.
	 *
	 * The implementation should fill the \em data with the information
	 * relevant to the current page, like a web page URL.
	 *
	 * @param[out] data The QMimeData object to fill.
	 */
	virtual void FillMimeData (QMimeData *data) = 0;

	/** @brief Called when something is being dragged over this tab.
	 *
	 * This function is called by the LeechCraft Core when a drag
	 * operation is in progress and the mouse cursor enters the tab
	 * corresponding to this widget in the tab bar. The implementation
	 * may call QDragMoveEvent::acceptProposedEvent() on the \em event
	 * or ignore it.
	 *
	 * This function is somewhat similar to QWidget::dragEnterEvent() or
	 * QWidget::dragMoveEvent().
	 *
	 * @param[in,out] event The object describing the DnD operation.
	 */
	virtual void HandleDragEnter (QDragMoveEvent *event) = 0;

	/** @brief Called when something is dropped over this tab.
	 *
	 * This function is called by the LeechCraft Core when a drag
	 * operation is in progress and the drag is dropped on the
	 * corresponding tab.
	 *
	 * This function is somewhat similar to QWidget::dropEvent().
	 *
	 * @param[in] event The object describing the DnD operation.
	 */
	virtual void HandleDrop (QDropEvent *event) = 0;
};

Q_DECLARE_INTERFACE (IDNDTab, "org.Deviant.LeechCraft.IDNDTab/1.0")
