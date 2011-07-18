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

#ifndef INTERFACES_IMWPROXY_H
#define INTERFACES_IMWPROXY_H
#include <Qt>

class QDockWidget;

/** @brief This interface is used for manipulating the main window.
 * 
 * All the interaction with LeechCraft main window should be done
 * through this interface.
 */
class IMWProxy
{
public:
	virtual ~IMWProxy () {}
	
	/** @brief Adds the given dockwidget to the given area.
	 * 
	 * This function merely calls QMainWindow::addDockWidget().
	 * 
	 * @param[in] area The area to add widget to.
	 * @param[in] widget The dock widget to add.
	 */
	virtual void AddDockWidget (Qt::DockWidgetArea area, QDockWidget *widget) = 0;
};

Q_DECLARE_INTERFACE (IMWProxy, "org.Deviant.LeechCraft.IMWProxy/1.0");

#endif
