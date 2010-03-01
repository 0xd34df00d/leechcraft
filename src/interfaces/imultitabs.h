/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef INTERFACES_IMULTITABS_H
#define INTERFACES_IMULTITABS_H
#include <QtPlugin>

class QToolBar;

/** @brief Interface for a single tab in a multitab plugin.
 *
 * Provides means to communicate with the single tab in a plugin.
 */
class IMultiTabsWidget
{
public:
	virtual ~IMultiTabsWidget () {}

	/** @brief Requests to remove the tab.
	 *
	 * This is usually called when a user has requested to close the tab
	 * via LeechCraft's UI. For example, clicking the "x" on the tab.
	 */
	virtual void Remove () = 0;

	/** @brief Requests page's toolbar.
	 *
	 * The returned toolbar would be shown on top of the LeechCraft's
	 * main window. If there is no toolbar, 0 should be returned.
	 *
	 * @return The toolbar of this page.
	 */
	virtual QToolBar* GetToolBar () const = 0;

	/** Notifies this plugin that a new tab was requested.
	 */
	virtual void NewTabRequested () = 0;

	/** Returns the list of QActions for the context menu of the tab bar.
	 */
	virtual QList<QAction*> GetTabBarContextMenuActions () const = 0;
};

/** @brief Interface for plugins having (and opening/closing) multiple
 * tabs.
 *
 * Plugin is expected to have a slot 'newTabRequested()' which is
 * similar to IMultiTabsWidget::NewTabRequested() in functionality.
 *
 * When a plugin wants to add a new tab into LeechCraft, it emits the
 * addNewTab(const QString&, QWidget*) signal, where the first parameter
 * is the name of the new tab, and the second one is the pointer to the
 * widget with tab contents. Newly added widget would be reparented by
 * LeechCraft.
 * To remove a tab, it emits removeTab(QWidget*), where the parameter is
 * the pointer to a previously added tab's widget.
 * To change tab's name, plugin emits changeTabName(QWidget*, const
 * QString&), where the first parameter is the pointer to previously
 * inserted tab and the second one is the new name.
 * To change tab's icon, plugin emits changeTabIcon(QWidget*, const
 * QIcon&), where the first parameter is the pointer to previously
 * inserted tab and the seocnd one is the new icon.
 * To bring the tab to front, plugin emits raiseTab(QWidget*) signal,
 * where the first parameter is the pointer to previously inserted tab.
 *
 * @sa IEmbedTab
 * @sa IWindow
 */
class IMultiTabs
{
public:
	/** @brief Virtual destructor.
	 */
	virtual ~IMultiTabs () {}
};

Q_DECLARE_INTERFACE (IMultiTabsWidget, "org.Deviant.LeechCraft.IMultiTabsWidget/1.0");
Q_DECLARE_INTERFACE (IMultiTabs, "org.Deviant.LeechCraft.IMultiTabs/1.0");

#endif

