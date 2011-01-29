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

#ifndef INTERFACES_IEMBEDTAB_H
#define INTERFACES_IEMBEDTAB_H
#include <QtPlugin>

class QWidget;
class QToolBar;

/** @brief Interface for plugins embedding a tab into main LeechCraft's
 * window.
 *
 * Implementing this interface means that plugin wants to embed a tab
 * into LeechCraft's main window. IInfo::GetName() would be used as a
 * name for the tab. If your plugin needs to open/close multiple tabs,
 * take a look at IMultiTabs.
 *
 * @sa IMultiTabs
 * @sa IWindow
 */
class IEmbedTab
{
public:
	/** @brief Returns the widget with tab contents.
	 *
	 * This function is called after the IInfo::SecondInit() has been
	 * called on this plugin.
	 *
	 * @return Widget with tab contents.
	 */
	virtual QWidget* GetTabContents () = 0;

	/** @brief Requests plugin's toolbar.
	 *
	 * The returned toolbar would be shown on top of the LeechCraft's
	 * main window. If there is no toolbar, 0 should be returned.
	 *
	 * This function is called after the IInfo::SecondInit() has been
	 * called on this plugin.
	 *
	 * @return The toolbar of this plugin.
	 */
	virtual QToolBar* GetToolBar () const = 0;

	/** @brief Virtual destructor.
	 */
	virtual ~IEmbedTab () {}

	/** @brief This signal is emitted by plugin when it wants to change
	 * the name of its tab.
	 *
	 * The name of the tab is shown in the tab bar of the tab widget.
	 * It also may be shown in other places and contexts, like in
	 * LeechCraft title bar when the corresponding tab is active.
	 *
	 * The tab is identified by the tabContents, which should be the
	 * pointer to a widget returned by GetTabContents().
	 *
	 * @param[out] tabContents The pointer to the widget with the
	 * contents of the tab for which the name should be updated.
	 * @param[out] name The new name of the tab.
	 */
	virtual void changeTabName (QWidget *tabContents, const QString& name) = 0;

	/** @brief This signal is emitted by plugin when it wants to
	 * associate a new icon with a tab.
	 *
	 * You can pass a null icon to clear it.
	 *
	 * The tab is identified by the tabContents, which should be the
	 * pointer to a widget returned by GetTabContents().
	 *
	 * @note This function is expected to be a signal in subclasses.
	 *
	 * @param[out] tabContents The pointer to the widget with the
	 * contents of the tab for which the icon should be updated.
	 * @param[out] icon The new icon of the tab or null icon.
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
	 */
	virtual void statusBarChanged (QWidget *tabContents, const QString& text) = 0;

	/** @brief This signal is emitted by plugin when it wants to make
	 * the widget with tabContents the currently active one.
	 *
	 * The tab is identified by the tabContents, which should be the
	 * pointer to a widget returned by GetTabContents().
	 *
	 * @note This function is expected to be a signal in subclasses.
	 *
	 * @param[out] tabContents The pointer to the widget with the
	 * contents of the tab that needs to be made current.
	 */
	virtual void raiseTab (QWidget *widget) = 0;
};

Q_DECLARE_INTERFACE (IEmbedTab, "org.Deviant.LeechCraft.IEmbedTab/1.0");

#endif

