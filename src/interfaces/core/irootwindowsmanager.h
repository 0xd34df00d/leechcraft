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

class ITabWidget;
class QMainWindow;
class ICoreTabWidget;
class IMWProxy;

/** @brief Interface to the core windows manager.
 *
 * The implementation of this interface is the core manager of root
 * windows (or main windows). It is used to support various multiwindow
 * features.
 *
 * Since the implementation is guaranteed to have signals (see the
 * protected functions below), it also has a GetQObject() method that
 * returns a QObject that can be used to expose these signals.
 *
 * \section MultiwindowNotes Multiple window notes
 *
 * There is always at least one window (that is, with the index 0). It is
 * the window initially created during LeechCraft startup, and currently
 * it cannot be destroyed at all (though this may change some time in the
 * future).
 *
 * There is a concept of preferred window. It's the window best used to
 * be a parent of a message box, or used to determine the preferred
 * screen on a multiscreen system, etc. Typically, it is the window that
 * currently has focus. The functions GetPreferredWindowIndex() and
 * GetPreferredWindow() are used to get the currently preferred window.
 *
 * Window indices are simply indices in some internal data structure.
 * That's why if a window gets removed all indices following it get
 * invalidated (in fact, decremented by one).
 */
class Q_DECL_EXPORT IRootWindowsManager
{
public:
	virtual ~IRootWindowsManager () {}

	/** @brief Returns this object as a QObject.
	 *
	 * The returned QObject can be used to connect its signals, which
	 * are declared as protected functions here.
	 *
	 * @return This object as a QObject.
	 */
	virtual QObject* GetQObject () = 0;

	/** @brief Returns the current window count.
	 *
	 * Please note that there is always at least one window.
	 *
	 * @return Current window count.
	 */
	virtual int GetWindowsCount () const = 0;

	/** @brief Returns the index of the currently preferred window.
	 *
	 * @return The currently preferred window's index.
	 *
	 * @sa GetPreferredWindow()
	 */
	virtual int GetPreferredWindowIndex () const = 0;

	/** @brief Returns the currently preferred window.
	 *
	 * It is a helper function returning the window with the index of
	 * GetPreferredWindowIndex().
	 *
	 * @return The currently preferred window.
	 *
	 * @sa GetPreferredWindowIndex()
	 */
	virtual QMainWindow* GetPreferredWindow () const
	{
		return GetMainWindow (GetPreferredWindowIndex ());
	}

	/** @brief Returns the window index containing the given tab.
	 *
	 * If no window contains the tab, returns -1.
	 *
	 * @param[in] tab The tab to search for.
	 * @return The index of the window containing the tab, or -1.
	 */
	virtual int GetWindowForTab (ITabWidget *tab) const = 0;

	/** @brief Returns the window proxy for the given window index.
	 *
	 * @param[in] winIdx The window index for which the proxy is
	 * requested.
	 * @return The proxy for that window.
	 */
	virtual IMWProxy* GetMWProxy (int winIdx) const = 0;

	/** @brief Returns the window for the given index.
	 *
	 * @param[in] idx The index of the window.
	 * @return The window at the given index, or nullptr if not found.
	 *
	 * @sa GetWindowIndex()
	 */
	virtual QMainWindow* GetMainWindow (int idx) const = 0;

	/** @brief Returns the index of the given window.
	 *
	 * @param[in] window The window to search for.
	 * @return The index of the given window, or -1 if not found.
	 *
	 * @sa GetMainWindow()
	 */
	virtual int GetWindowIndex (QMainWindow *window) const = 0;

	/** @brief Returns the tab widget of the window identified by idx.
	 *
	 * @param[in] idx The index of the window for which to return the
	 * tab widget.
	 * @return The tab widget of the given index, or nullptr if not
	 * found.
	 *
	 * @sa GetTabWidgetIndex()
	 */
	virtual ICoreTabWidget* GetTabWidget (int idx) const = 0;

	/** @brief Returns the index of the window containing the tab widget.
	 *
	 * @param[in] ictw The tab widget to search for.
	 * @return THe index of the given tab widget, or -1 if not found.
	 *
	 * @sa GetTabWidget()
	 */
	virtual int GetTabWidgetIndex (ICoreTabWidget *ictw) const
	{
		for (int i = 0; i < GetWindowsCount (); ++i)
			if (GetTabWidget (i) == ictw)
				return i;

		return -1;
	}
protected:
	/** @brief Emitted after a new window is added.
	 *
	 * @param[out] index The index of the newly added window.
	 *
	 * @sa windowRemoved()
	 */
	virtual void windowAdded (int index) = 0;

	/** @brief Emitted before a window at the given index is removed.
	 *
	 * Since the signal is emitted before the window is removed, the
	 * window still can be obtained via GetMainWindow() (as well as its
	 * helper classes via GetMWProxy() and GetTabWidget()).
	 *
	 * @param[out] index The index of the window being removed.
	 *
	 * @sa windowAdded()
	 */
	virtual void windowRemoved (int index) = 0;

	/** @brief Emitted when current LeechCraft window changes.
	 *
	 * @param[out] to The new active window index.
	 * @param[out] from The previous active window index.
	 */
	virtual void currentWindowChanged (int to, int from) = 0;

	/** @brief Emitted after a new tab is added to the given window.
	 *
	 * @param[out] windowIdx The index of the window tab is added to.
	 * @param[out] tabIdx The index of the tab.
	 *
	 * @sa tabIsRemoving()
	 */
	virtual void tabAdded (int windowIdx, int tabIdx) = 0;

	/** @brief Emitted before a tab is removed from the given window.
	 *
	 * @param[out] windowIdx The index of the window tab is being removed
	 * from.
	 * @param[out] tabIdx The index of the tab.
	 *
	 * @sa tabAdded()
	 */
	virtual void tabIsRemoving (int windowIdx, int tabIdx) = 0;

	/** @brief Emitted before a tab is moved from a window to another one.
	 *
	 * The tab index is given in the window \em from which the tab is moved.
	 *
	 * @param[out] fromWin The window from which the tab is being removed.
	 * @param[out] toWin The window to which the tab is being added.
	 * @param[out] tabIdx The index of the tab being moved in the window
	 * it is moved from.
	 */
	virtual void tabIsMoving (int fromWin, int toWin, int tabIdx) = 0;

	/** @brief Emitted after a tab is moved from a window to another one.
	 *
	 * The tab index is given in the window \em to which the tab is moved.
	 *
	 * @param[out] fromWin The window from which the tab is removed.
	 * @param[out] toWin The window to which the tab is added.
	 * @param[out] tabIdx The index of the tab being moved in the window
	 * it is moved to.
	 */
	virtual void tabMoved (int fromWin, int toWin, int tabIdx) = 0;
};

Q_DECLARE_INTERFACE (IRootWindowsManager, "org.LeechCraft.IRootWindowsManager/1.0");
