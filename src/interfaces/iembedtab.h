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
 * name for the tab. If your plugin could open/close multiple tabs, have
 * a look at IMultiTabs.
 *
 * Plugin is expected to implement following signals:
 * - changeTabName(QWidget*,const QString&) which changes tab name of
 *   the tab with the given widget.
 * - changeTabIcon(QWidget*,const QIcon&) which changes the icon of the
 *   tab with the given widget.
 * - statusBarChanged(QWidget*,const QString&) notifies that the status
 *   bar message of the given widget is changed. Note that the message
 *   would be updated only if the given widget is visible.
 * - raiseTab(QWidget*) brings the tab to front.
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
};

Q_DECLARE_INTERFACE (IEmbedTab, "org.Deviant.LeechCraft.IEmbedTab/1.0");

#endif

