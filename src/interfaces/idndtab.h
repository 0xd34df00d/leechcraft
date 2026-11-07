/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

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

	/** @brief Checks whether this tab could handle a drop of the `data`.
	 *
	 * This function is called by the LeechCraft Core while a drag
	 * operation is in progress over the tab corresponding to this
	 * widget in the tab bar, possibly many times. It must not have
	 * side effects.
	 *
	 * This function is somewhat similar to
	 * QAbstractItemModel::canDropMimeData().
	 *
	 * @param[in] data The data being dragged.
	 * @return Whether `HandleDrop()` would handle a drop of the `data`.
	 */
	virtual bool CouldHandleDrop (const QMimeData& data) const = 0;

	/** @brief Handles a drop of the `data` on this tab.
	 *
	 * This function is called by the LeechCraft Core when a drag
	 * operation is dropped on the tab corresponding to this widget in
	 * the tab bar, and only if `CouldHandleDrop()` accepted the `data`.
	 *
	 * A handled drop is reported to the drag source as `Qt::CopyAction`
	 * whatever action the source proposed, so the source keeps its data:
	 * a tab cannot move data out of the source through this interface.
	 *
	 * @param[in] data The dropped data.
	 * @return Whether the `data` was handled.
	 */
	virtual bool HandleDrop (const QMimeData& data) = 0;
};

Q_DECLARE_INTERFACE (IDNDTab, "org.Deviant.LeechCraft.IDNDTab/1.0")
