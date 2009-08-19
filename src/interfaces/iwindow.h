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

#ifndef INTERFACES_IWINDOW_H
#define INTERFACES_IWINDOW_H
#include <QWidget>
#include <QtPlugin>

/** @brief Interface for plugins with their own main windows.
 *
 * If a plugin creates a main window and wants to show it upon some user
 * actions (like double-clicking plugin's name in plugins list), it
 * should implement this interface.
 */
class IWindow
{
public:
	/** @brief Sets the parent widget of the window.
	 *
	 * This function is called by the LeechCraft to inform the plugin
	 * about its parent widget.
	 *
	 * @param[in] parent Pointer to parent widget.
	 */
	virtual void SetParent (QWidget *parent) = 0;

	/** @brief Shows the plugin's main window.
	 *
	 * This function is called by LeechCraft when the user has done an
	 * action which means that the plugin should show or hide it's
	 * window (depending of the current state).
	 */
	virtual void ShowWindow () = 0;

	/** @brief Virtual destructor.
	 */
	virtual ~IWindow () {}
};

Q_DECLARE_INTERFACE (IWindow, "org.Deviant.LeechCraft.IWindow/1.0");

#endif

