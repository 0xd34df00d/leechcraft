/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "interfaces/ihavetabs.h"

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

	/** @brief Returns the preferred window for the given \em tabclass.
	 *
	 * This function returns the window that would be used to host a new
	 * tab of the given \em tabclass if the tab is opened right after
	 * this call.
	 *
	 * Please note that preferred windows algorithm can be dynamic, so
	 * the result of this function may become irrelevant after a few
	 * other tabs are opened, closed or moved.
	 *
	 * @param[in] tabclass The tab class to check.
	 * @return The window that would host the tablcass if it's opened
	 * right now.
	 */
	virtual int GetPreferredWindowIndex (const QByteArray& tabclass) const = 0;

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

	enum class AddTabFlag
	{
		None = 0x00,

		Background = 0x01,
	};

	Q_DECLARE_FLAGS (AddTabFlags, AddTabFlag)

	virtual void AddTab (const QString& name, QWidget *tab, AddTabFlags flags = AddTabFlag::None) = 0;

	void AddTab (QWidget *tab, AddTabFlags flags = AddTabFlag::None)
	{
		AddTab (qobject_cast<ITabWidget*> (tab)->GetTabClassInfo ().VisibleName_, tab, flags);
	}

	/** @brief Returns the index of the window containing the tab widget.
	 *
	 * @param[in] ictw The tab widget to search for.
	 * @return The index of the given tab widget, or -1 if not found.
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
	 * @sa GetQObject()
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
	 * @sa GetQObject()
	 */
	virtual void windowRemoved (int index) = 0;

	/** @brief Emitted when current LeechCraft window changes.
	 *
	 * @param[out] to The new active window index.
	 * @param[out] from The previous active window index.
	 *
	 * @sa GetQObject()
	 */
	virtual void currentWindowChanged (int to, int from) = 0;

	/** @brief Emitted after a new tab is added to the given window.
	 *
	 * @param[out] windowIdx The index of the window tab is added to.
	 * @param[out] tab The tab that was added.
	 *
	 * @sa tabIsRemoving()
	 * @sa GetQObject()
	 */
	virtual void tabAdded (int windowIdx, QWidget *tab) = 0;

	/** @brief Emitted before a tab is removed from the given window.
	 *
	 * @param[out] windowIdx The index of the window tab is being removed
	 * from.
	 * @param[out] tab The tab that will be removed.
	 *
	 * @sa tabAdded()
	 * @sa GetQObject()
	 */
	virtual void tabIsRemoving (int windowIdx, QWidget *tab) = 0;

	/** @brief Emitted before a tab is moved from a window to another one.
	 *
	 * The tab index is given in the window \em from which the tab is moved.
	 *
	 * @param[out] fromWin The window from which the tab is being removed.
	 * @param[out] toWin The window to which the tab is being added.
	 * @param[out] tabIdx The index of the tab being moved in the window
	 * it is moved from.
	 *
	 * @sa GetQObject()
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
	 *
	 * @sa GetQObject()
	 */
	virtual void tabMoved (int fromWin, int toWin, int tabIdx) = 0;
};

Q_DECLARE_OPERATORS_FOR_FLAGS (IRootWindowsManager::AddTabFlags)

Q_DECLARE_INTERFACE (IRootWindowsManager, "org.LeechCraft.IRootWindowsManager/1.0")
