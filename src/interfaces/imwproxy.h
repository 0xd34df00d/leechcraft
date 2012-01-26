/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef INTERFACES_IMWPROXY_H
#define INTERFACES_IMWPROXY_H
#include <Qt>

class QDockWidget;
class QToolBar;
class QWidget;
class QKeySequence;

/** @brief This interface is used for manipulating the main window.
 *
 * All the interaction with LeechCraft main window should be done
 * through this interface.
 */
class IMWProxy
{
public:
	enum WidgetArea
	{
		WALeft,
		WARight,
		WABottom
	};

	virtual ~IMWProxy () {}

	/** @brief Adds the given dock widget to the given area.
	 *
	 * This function merely calls QMainWindow::addDockWidget().
	 *
	 * The action for toggling the visibility of this dock widget is
	 * also added to the corresponding menus by default. The
	 * ToggleViewActionVisiblity() method could be used to change that.
	 *
	 * @param[in] area The area to add widget to.
	 * @param[in] widget The dock widget to add.
	 *
	 * @sa ToggleViewActionVisiblity()
	 */
	virtual void AddDockWidget (Qt::DockWidgetArea area, QDockWidget *widget) = 0;

	/** @brief Toggles the visibility of the toggle view action.
	 *
	 * By default all newly added dock widgets have their toggle view
	 * actions shown.
	 *
	 * @param[in] widget The widget for which to update the toggle
	 * action visibility.
	 * @param[in] visible Whether the corresponding action should be
	 * visible.
	 */
	virtual void ToggleViewActionVisiblity (QDockWidget *widget, bool visible) = 0;

	/** @brief Sets the visibility action shortcut of the given widget.
	 *
	 * @param[in] widget The widget for which the visibility action
	 * shortcut.
	 * @param[in] sequence The widget's visibility action shortcut
	 * sequence.
	 */
	virtual void SetViewActionShortcut (QDockWidget *widget, const QKeySequence& seq) = 0;

	/** @brief Adds the given toolbar at the given area.
	 *
	 * If the toolbar is already added, it will be just moved to the
	 * area.
	 *
	 * @param[in] toolbar The toolbar to add.
	 * @param[in] area The area where the toolbar should be added.
	 */
	virtual void AddToolbar (QToolBar *toolbar, Qt::ToolBarArea area = Qt::TopToolBarArea) = 0;

	/** @brief Adds the given widget at the given area.
	 *
	 * @param[in] widget The widget to add.
	 * @param[in] area The area where the widget should be added.
	 */
	virtual void AddSideWidget (QWidget *widget, WidgetArea area = WALeft) = 0;

	/** @brief Activates the given tab.
	 *
	 * @param[in] widget The widget of the tab to activate.
	 */
};

Q_DECLARE_INTERFACE (IMWProxy, "org.Deviant.LeechCraft.IMWProxy/1.0");

#endif
