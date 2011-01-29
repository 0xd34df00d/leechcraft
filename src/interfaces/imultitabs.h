/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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
#include <QMap>

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

	/** Returns the pointer to the plugin that it belongs to.
	 */
	virtual QObject* ParentMultiTabs () const = 0;

	/** Returns the list of QActions for the context menu of the tab bar.
	 */
	virtual QList<QAction*> GetTabBarContextMenuActions () const = 0;

	/** Returns the list of QActions for menus identified by the keys of
	 * the returned map.
	 *
	 * Currently the key "view" serves for View menu, "tools" for Tools,
	 * and other keys can have any name — they will be prepended before
	 * the "Help" menu.
	 *
	 * Cause this function is expected to be rarely used, the default
	 * implementation does nothing and returns an empty map.
	 *
	 * @return The map with keys identifying menus and values — the
	 * corresponding list of actions.
	 */
	virtual QMap<QString, QList<QAction*> > GetWindowMenus () const
	{
		return QMap<QString, QList<QAction*> > ();
	}

	/** This function is called when the corresponding widget becomes
	 * current.
	 */
	virtual void TabMadeCurrent () {}
};

/** @brief Interface for plugins having (and opening/closing) multiple
 * tabs.
 *
 * @note You can't use the the tab-related signals before SecondInit()
 * has been called on your plugin, but you can use them in SecondInit()
 * and later.
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

	/** @brief This slot is similar to the function
	 * IMultiTabsWidget::NewTabRequested().
	 *
	 * This slot is called when a new tab of this plugin is requested.
	 *
	 * @note This function is expected to be a slot in subclasses.
	 */
	virtual void newTabRequested () = 0;

	/** @brief This signal is emitted by plugin when it wants to add a
	 * new tab to the LeechCraft window.
	 *
	 * @note The added widget would be reparented by LeechCraft.
	 *
	 * @note This function is expected to be a signal in subclasses.
	 *
	 * @param[out] name The name of the tab.
	 * @param[out] tabContents The pointer to the widget with the
	 * contents of the tab that needs to be added.
	 *
	 * @sa removeTab(), changeTabName(), changeTabIcon(),
	 * statusBarChanged(), raiseTab()
	 */
	virtual void addNewTab (const QString& name, QWidget *tabContents) = 0;

	/** @brief This signal is emitted by plugin when it wants to remove
	 * a tab from LeechCraft window.
	 *
	 * The tab is identified by the tabContents, which should be the
	 * pointer to a widget previously added to LeechCraft by emitting
	 * addNewTab() signal.
	 *
	 * @param[out] tabContents The pointer to the widget with the
	 * contents of the tab that needs to be removed.
	 *
	 * @sa addNewTab(), changeTabName(), changeTabIcon(),
	 * statusBarChanged(), raiseTab()
	 */
	virtual void removeTab (QWidget *tabContents) = 0;

	/** @brief This signal is emitted by plugin when it wants to change
	 * the name of a tab.
	 *
	 * The name of the tab is shown in the tab bar of the tab widget.
	 * It also may be shown in other places and contexts, like in
	 * LeechCraft title bar when the corresponding tab is active.
	 *
	 * The tab is identified by the tabContents, which should be the
	 * pointer to a widget previously added to LeechCraft by emitting
	 * addNewTab() signal.
	 *
	 * @param[out] tabContents The pointer to the widget with the
	 * contents of the tab for which the name should be updated.
	 * @param[out] name The new name of the tab.
	 *
	 * @sa addNewTab(), removeTab(), changeTabIcon(),
	 * statusBarChanged(), raiseTab()
	 */
	virtual void changeTabName (QWidget *tabContents, const QString& name) = 0;

	/** @brief This signal is emitted by plugin when it wants to
	 * associate a new icon with a tab.
	 *
	 * You can pass a null icon to clear it.
	 *
	 * The tab is identified by the tabContents, which should be the
	 * pointer to a widget previously added to LeechCraft by emitting
	 * addNewTab() signal.
	 *
	 * @note This function is expected to be a signal in subclasses.
	 *
	 * @param[out] tabContents The pointer to the widget with the
	 * contents of the tab for which the icon should be updated.
	 * @param[out] icon The new icon of the tab or null icon.
	 *
	 * @sa addNewTab(), removeTab(), changeTabName(),
	 * statusBarChanged(), raiseTab()
	 */
	virtual void changeTabIcon (QWidget *tabContents, const QIcon& icon) = 0;

	/** @brief This signal is emitted by plugin when it wants to set
	 * new status bar text for a tab.
	 *
	 * The text set by this signal would be shown when the tab with
	 * tabContents is active. To clear the status bar, just emit this
	 * signal with empty text.
	 *
	 * The tab is identified by the tabContents, which should be the
	 * pointer to a widget returned by GetTabContents().
	 *
	 * @note This function is expected to be a signal in subclasses.
	 *
	 * @note Please note, that user may choose to hide the status bar,
	 * so no important text should be output this way.
	 *
	 * @param[out] tabContents The pointer to the widget with the
	 * contents of the tab for which the status bar text should be
	 * updated.
	 * @param[out] text The new status bar text or empty string.
	 *
	 * @sa addNewTab(), removeTab(), changeTabName(), changeTabIcon(),
	 * raiseTab()
	 */
	virtual void statusBarChanged (QWidget *tabContents, const QString& text) = 0;

	/** @brief This signal is emitted by plugin when it wants to make
	 * the widget with tabContents the currently active one.
	 *
	 * The tab is identified by the tabContents, which should be the
	 * pointer to a widget previously added to LeechCraft by emitting
	 * addNewTab() signal.
	 *
	 * @note This function is expected to be a signal in subclasses.
	 *
	 * @param[out] tabContents The pointer to the widget with the
	 * contents of the tab that needs to be made current.
	 *
	 * @sa addNewTab(), removeTab(), changeTabName(), changeTabIcon(),
	 * statusBarChanged()
	 */
	virtual void raiseTab (QWidget *tabContents) = 0;
};

Q_DECLARE_INTERFACE (IMultiTabsWidget, "org.Deviant.LeechCraft.IMultiTabsWidget/1.0");
Q_DECLARE_INTERFACE (IMultiTabs, "org.Deviant.LeechCraft.IMultiTabs/1.0");

#endif

